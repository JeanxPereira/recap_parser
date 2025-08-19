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

#include "catalog.h"
#include "exporter.h"

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
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c){ return std::tolower(c); });
    return s;
}

static inline bool has_any_extension(const fs::path& p, const std::unordered_set<std::string>& exts) {
    if (exts.empty()) return true;
    std::string e = to_lower(p.extension().string());
    if (!e.empty() && e[0] == '.') e.erase(0,1);
    return exts.count(e) > 0;
}

static inline std::unordered_set<std::string> parse_ext_filter(const std::string& filter) {
    std::unordered_set<std::string> out;
    if (filter.empty()) return out;
    std::string tmp;
    for (char c : filter) {
        if (c == ',' || c == ';' || std::isspace(static_cast<unsigned char>(c))) {
            if (!tmp.empty()) { out.insert(to_lower(tmp)); tmp.clear(); }
        } else if (c == '.') {
        } else {
            tmp.push_back(c);
        }
    }
    if (!tmp.empty()) out.insert(to_lower(tmp));
    return out;
}

static inline void ensure_dir(const fs::path& p) {
    std::error_code ec;
    fs::create_directories(p, ec);
}

int main(int argc, char** argv) {
    CLI::App app{"ReCap Parser"};

    std::string inputPath;
    bool xmlMode = false;
    bool yamlMode = false;
    bool silentMode = false;
    bool debugMode = false;
    std::string outputDir;
    bool logEnabled = false;
    bool organizeByExtension = false;
    std::string gameVersion = "5.3.0.103";
    bool recursiveMode = false;
    std::string recursiveFilter;

    app.add_option("file", inputPath)->required();
    app.add_flag("--xml", xmlMode);
    app.add_flag("--yaml,--yml,-y", yamlMode);
    app.add_flag("--silent", silentMode);
    app.add_flag("--debug,-d", debugMode);
    app.add_option("--output,-o", outputDir);
    app.add_flag("--log,-l", logEnabled);
    app.add_flag("--sort-ext,-s", organizeByExtension);
    app.add_option("--game-version,--gv", gameVersion);
    CLI::Option* optRecursive = app.add_option("--recursive,-r", recursiveFilter)->expected(0,1);
    CLI11_PARSE(app, argc, argv);
    recursiveMode = optRecursive->count() > 0;
    if (xmlMode && yamlMode) { std::cerr << "Error: --xml and --yaml are mutually exclusive\n"; return 1; }


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

    std::vector<std::string> failedFiles;
    std::unordered_set<std::string> extFilter = parse_ext_filter(recursiveFilter);

    fs::path in = inputPath;
    if (!fs::exists(in)) {
        std::cerr << "Error: path not found: " << inputPath << "\n";
        if (logFile.is_open()) {
            std::cout.rdbuf(coutOrig);
            std::cerr.rdbuf(cerrOrig);
        }
        return 1;
    }

    auto process_one = [&](const fs::path& file) {
        Catalog tempCatalog;
        tempCatalog.setGameVersion(gameVersion);
        try {
            Parser parser(tempCatalog, file.string(), silentMode, debugMode, exportFormat);
            if (!parser.parse()) {
                failedFiles.push_back(file.string());
                return;
            }
            if (exportFormat != "none") {
                fs::path baseOut = outputDir.empty() ? file.parent_path() : fs::path(outputDir);
                fs::path targetDir = baseOut;
                if (organizeByExtension) {
                    std::string e = to_lower(file.extension().string());
                    if (!e.empty() && e[0]=='.') e.erase(0,1);
                    targetDir /= e.empty() ? "unknown" : e;
                }
                ensure_dir(targetDir);

                fs::path outName = file.stem();
                if (exportFormat == "xml") outName += ".xml";
                else if (exportFormat == "yaml") outName += ".yaml";
                fs::path outPath = targetDir / outName;
                parser.exportToFile(outPath.string());
            }
        } catch (const std::exception& e) {
            failedFiles.push_back(file.string());
            if (!silentMode) std::cerr << "Parse failed: " << file << " : " << e.what() << "\n";
        }
    };

    if (fs::is_regular_file(in)) {
        process_one(in);
    } else if (fs::is_directory(in)) {
        if (recursiveMode) {
            for (auto it = fs::recursive_directory_iterator(in); it != fs::recursive_directory_iterator(); ++it) {
                if (!it->is_regular_file()) continue;
                const fs::path& p = it->path();
                if (!has_any_extension(p, extFilter)) continue;
                process_one(p);
            }
        } else {
            for (auto& de : fs::directory_iterator(in)) {
                if (!de.is_regular_file()) continue;
                const fs::path& p = de.path();
                process_one(p);
            }
        }
    } else {
        std::cerr << "Error: path type not supported\n";
        if (logFile.is_open()) {
            std::cout.rdbuf(coutOrig);
            std::cerr.rdbuf(cerrOrig);
        }
        return 1;
    }

    if (logFile.is_open()) {
        std::cout.rdbuf(coutOrig);
        std::cerr.rdbuf(cerrOrig);
    }

    return failedFiles.empty() ? 0 : 1;
}
