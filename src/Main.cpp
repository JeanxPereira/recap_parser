#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <memory>
#include <algorithm>
#include <cctype>
#include <unordered_set>
#include <system_error>
#include <cstdio>

#include <CLI/CLI.hpp>

#include "AssetCatalog.h"
#include "Exporter.h"
#include "AssetProvider.h"

#if ((defined(_MSVC_LANG) && _MSVC_LANG >= 201703L) || __cplusplus >= 201703L)
#include <filesystem>
namespace fs = std::filesystem;
#else
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#endif

struct TeeStreamBuffer : public std::streambuf {
    std::streambuf* a;
    std::streambuf* b;

    TeeStreamBuffer(std::streambuf* a_, std::streambuf* b_) : a(a_), b(b_) {}

    int overflow(int c) override {
        if (c == EOF) return !EOF;
        const int r1 = a ? a->sputc(c) : c;
        const int r2 = b ? b->sputc(c) : c;
        return (r1 == EOF || r2 == EOF) ? EOF : c;
    }

    std::streamsize xsputn(const char* s, std::streamsize n) override {
        const auto w1 = a ? a->sputn(s, n) : n;
        const auto w2 = b ? b->sputn(s, n) : n;
        return std::min(w1, w2);
    }

    int sync() override {
        const int r1 = a ? a->pubsync() : 0;
        const int r2 = b ? b->pubsync() : 0;
        return (r1 == 0 && r2 == 0) ? 0 : -1;
    }
};

static inline std::string to_lower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return std::tolower(c); });
    return s;
}

static inline void ensure_dir(const fs::path& p) {
    std::error_code ec;
    fs::create_directories(p, ec);
}

int main(int argc, char** argv) {
    CLI::App app{ "ReCap Parser" };

    std::string inputPath;
    std::string specificAsset;
    std::string registryDir;

    bool xmlMode = false;
    bool yamlMode = false;
    bool silentMode = false;
    bool debugMode = false;
    std::string outputDir;
    bool logEnabled = false;
    bool organizeByExtension = false;
    std::string gameVersion = "5.3.0.103";
    std::string recursiveFilter;
    bool skipExisting = false;

    app.add_option("-d,--data,--input", inputPath, "Input DBPF package or directory")->required();
    app.add_option("-a,--asset", specificAsset, "Specific asset name");
    app.add_option("-r,--registry", registryDir, "Registry folder");

    app.add_flag("--xml", xmlMode);
    app.add_flag("--yaml,--yml,-y", yamlMode);
    app.add_flag("--silent", silentMode);
    app.add_flag("--debug", debugMode);
    app.add_option("-o,--output", outputDir);
    app.add_flag("-l,--log", logEnabled);
    app.add_flag("-s,--sort-ext", organizeByExtension);
    app.add_option("--gv,--game-version", gameVersion);
    app.add_flag("--skip-existing", skipExisting);
    app.add_option("--recursive", recursiveFilter);

    CLI11_PARSE(app, argc, argv);

    if (xmlMode && yamlMode) {
        std::cerr << "Error: --xml and --yaml are mutually exclusive\n";
        return 1;
    }

    std::string exportFormat = "none";
    if (xmlMode) exportFormat = "xml";
    else if (yamlMode) exportFormat = "yaml";

    std::ofstream logFile;
    std::streambuf* coutOrig = nullptr;
    std::streambuf* cerrOrig = nullptr;
    std::unique_ptr<TeeStreamBuffer> teeOut;
    std::unique_ptr<TeeStreamBuffer> teeErr;

    if (logEnabled) {
        logFile.open("recap_parser.log", std::ios::out | std::ios::trunc);
        if (logFile.is_open()) {
            coutOrig = std::cout.rdbuf();
            cerrOrig = std::cerr.rdbuf();
            teeOut = std::make_unique<TeeStreamBuffer>(coutOrig, logFile.rdbuf());
            teeErr = std::make_unique<TeeStreamBuffer>(cerrOrig, logFile.rdbuf());
            std::cout.rdbuf(teeOut.get());
            std::cerr.rdbuf(teeErr.get());
        }
    }

    fs::path in = inputPath;
    if (!fs::exists(in)) {
        std::cerr << "Error: Input path not found: " << inputPath << "\n";
        return 1;
    }

    std::unique_ptr<IAssetProvider> provider;
    bool isDbpf = false;

    try {
        if (fs::is_regular_file(in)) {
            std::string ext = to_lower(in.extension().string());
            if (ext == ".package" || ext == ".dat") {
                if (!silentMode) std::cout << "Loading DBPF Archive...\n";
                provider = std::make_unique<DbpfAssetProvider>(in, registryDir);
                isDbpf = true;
            }
            else {
                provider = std::make_unique<DiskAssetProvider>(in);
            }
        }
        else if (fs::is_directory(in)) {
            provider = std::make_unique<DiskAssetProvider>(in, recursiveFilter);
        }
        else {
            std::cerr << "Error: path type not supported\n";
            return 1;
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Failed to initialize provider: " << e.what() << "\n";
        return 1;
    }

    std::vector<std::string> assetsToProcess;

    if (!specificAsset.empty()) {
        assetsToProcess.push_back(specificAsset);
    }
    else {
        if (!silentMode) std::cout << "Indexing assets...\n";
        assetsToProcess = provider->listAssets();
    }

    if (assetsToProcess.empty()) {
        std::cerr << "No assets found to process.\n";
        return 0;
    }

    if (!silentMode && specificAsset.empty()) {
        std::cout << "Found " << assetsToProcess.size() << " assets.\n";
    }

    int successCount = 0;
    int failCount = 0;

    for (const auto& assetName : assetsToProcess) {
        if (!silentMode) {
            std::cout << "\n>>> Processing: " << assetName << "\n";
        }

        std::vector<uint8_t> buffer;
        if (!provider->getAsset(assetName, buffer)) {
            std::cerr << "Error: Failed to load asset data for: " << assetName << "\n";
            failCount++;
            continue;
        }

        if (buffer.empty()) {
            if (debugMode) std::cout << "Skipping empty file.\n";
            continue;
        }

        bool willExport = (exportFormat != "none");
        fs::path fileP(assetName);
        fs::path outPath;
        fs::path targetDir;

        if (willExport) {
            fs::path baseOut = outputDir.empty() ? (isDbpf ? in.parent_path() / in.stem() : fileP.parent_path()) : fs::path(outputDir);
            targetDir = baseOut;

            if (organizeByExtension) {
                std::string e = to_lower(fileP.extension().string());
                if (!e.empty() && e[0] == '.') e.erase(0, 1);
                targetDir /= e.empty() ? "unknown" : e;
            }

            fs::path outName = fileP.stem();
            if (exportFormat == "xml") outName += ".xml";
            else if (exportFormat == "yaml") outName += ".yaml";

            outPath = targetDir / outName;

            if (skipExisting && fs::exists(outPath)) {
                if (!silentMode) std::cout << "Skipping existing: " << outPath.string() << "\n";
                continue;
            }
        }

        Catalog tempCatalog;
        tempCatalog.setGameVersion(gameVersion);

        try {
            Parser parser(tempCatalog, buffer, assetName, silentMode, debugMode, exportFormat);

            if (parser.parse()) {
                if (willExport) {
                    ensure_dir(targetDir);
                    parser.exportToFile(outPath.string());
                    if (!silentMode) std::cout << "Exported: " << outPath.string() << "\n";
                }
                successCount++;
            }
            else {
                std::cerr << "Parser rejected file (unknown format or version mismatch): " << assetName << "\n";
                failCount++;
            }
        }
        catch (const std::exception& e) {
            std::cerr << "Exception parsing " << assetName << ": " << e.what() << "\n";
            failCount++;
        }
    }

    if (!silentMode) {
        std::cout << "\nDone. Success: " << successCount << ", Failed: " << failCount << "\n";
    }

    if (logFile.is_open()) {
        std::cout.rdbuf(coutOrig);
        std::cerr.rdbuf(cerrOrig);
    }

    return (failCount > 0 && !specificAsset.empty()) ? 1 : 0;
}