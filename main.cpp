#include <iostream>
#include <fstream>
#include <vector>
#include <boost/program_options.hpp>
#include "catalog.h"
#include "exporter.h"

#if ((defined(_MSVC_LANG) && _MSVC_LANG >= 201703L) || __cplusplus >= 201703L)
#include <filesystem>
namespace fs = std::filesystem;
#else
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#endif

namespace po = boost::program_options;

std::string getFileExtension(const std::string& filepath) {
    std::string extension = fs::path(filepath).extension().string();
    for (auto& c : extension) {
        c = std::tolower(c);
    }
    return extension;
}

bool hasExtension(const std::string& filepath, const std::string& extension) {
    std::string fileExt = getFileExtension(filepath);

    std::string extToCompare = extension;
    if (!extension.empty() && extension[0] != '.') {
        extToCompare = "." + extension;
    }

    for (auto& c : extToCompare) {
        c = std::tolower(c);
    }

    return fileExt == extToCompare;
}

bool isSupportedFileType(const std::string& filepath, const Catalog& catalog) {
    std::string extension = getFileExtension(filepath);
    const FileTypeInfo* info = catalog.getFileType(extension);

    if (!info) {
        info = catalog.getFileTypeByName(filepath);
    }

    return info != nullptr;
}

class TeeStreamBuffer : public std::streambuf {
public:
    TeeStreamBuffer(std::streambuf* sb1, std::streambuf* sb2) : sb1(sb1), sb2(sb2) {}

protected:
    virtual int overflow(int c) {
        if (c == EOF) {
            return !EOF;
        }
        else {
            int const r1 = sb1->sputc(c);
            int const r2 = sb2->sputc(c);
            return r1 == EOF || r2 == EOF ? EOF : c;
        }
    }

    virtual int sync() {
        int const r1 = sb1->pubsync();
        int const r2 = sb2->pubsync();
        return r1 == 0 && r2 == 0 ? 0 : -1;
    }

private:
    std::streambuf* sb1;
    std::streambuf* sb2;
};

bool processFile(const std::string& filepath, const std::string& outputDir,
    const std::string& exportFormat, bool debugMode, std::ofstream& logFile,
    std::vector<std::string>& failedFiles) {

    std::streambuf* coutOriginal = nullptr;
    std::streambuf* cerrOriginal = nullptr;
    TeeStreamBuffer* teeStdout = nullptr;
    TeeStreamBuffer* teeStderr = nullptr;

    if (logFile.is_open()) {
        coutOriginal = std::cout.rdbuf();
        cerrOriginal = std::cerr.rdbuf();

        teeStdout = new TeeStreamBuffer(coutOriginal, logFile.rdbuf());
        teeStderr = new TeeStreamBuffer(cerrOriginal, logFile.rdbuf());

        std::cout.rdbuf(teeStdout);
        std::cerr.rdbuf(teeStderr);
    }

    bool success = true;

    if (!fs::exists(filepath)) {
        std::cerr << "Error: File " << filepath << " does not exist" << std::endl;
        failedFiles.push_back(filepath);
        success = false;
    }
    else {
        try {
            std::string filename = fs::path(filepath).filename().string();

            Catalog catalog;
            catalog.initialize();

            if (!isSupportedFileType(filepath, catalog)) {
                std::cerr << "Unsupported file type: " << fs::path(filepath).filename().string() << std::endl;

                std::cerr << "Registered file types:" << std::endl;
                std::vector<std::string> fileTypes = catalog.getRegisteredFileTypes();

                std::vector<std::string> extensionTypes;
                std::vector<std::string> exactNameTypes;

                for (const auto& type : fileTypes) {
                    if (type.find("[exact]") != std::string::npos) {
                        exactNameTypes.push_back(type);
                    }
                    else {
                        extensionTypes.push_back(type);
                    }
                }

                if (!extensionTypes.empty()) {
                    std::cerr << "\nExtension-based file types:" << std::endl;
                    const int TYPES_PER_LINE = 2;
                    for (size_t i = 0; i < extensionTypes.size(); i++) {
                        if (i > 0 && i % TYPES_PER_LINE == 0) {
                            std::cerr << std::endl;
                        }
                        std::cerr << std::setw(50) << std::left << extensionTypes[i];
                    }
                    std::cerr << std::endl;
                }

                if (!exactNameTypes.empty()) {
                    std::cerr << "\nExact filename matches:" << std::endl;
                    for (const auto& type : exactNameTypes) {
                        std::cerr << "  " << type << std::endl;
                    }
                }

                std::cerr << std::endl;

                failedFiles.push_back(filepath);
                return false;
            }

            Parser parser(catalog, filepath, debugMode, exportFormat);

            std::cout << "Parsing \"" << filename << "\"" << std::endl;

            if (!parser.parse()) {
                std::cerr << "Error parsing file: " << filepath << std::endl;
                failedFiles.push_back(filepath);
                success = false;
            }
            else if (exportFormat != "none") {
                std::string originalExtension = fs::path(filepath).extension().string();

                std::string outputExtension = (exportFormat == "xml") ? ".xml" : ".yaml";

                std::string outputFilename = fs::path(filepath).stem().string() + originalExtension + outputExtension;

                std::string outputPath;
                if (!outputDir.empty()) {
                    outputPath = (fs::path(outputDir) / outputFilename).string();
                }
                else {
                    outputPath = outputFilename;
                }

                parser.exportToFile(outputPath);
                std::cout << "Exported to " << outputPath << std::endl;
            }
        }
        catch (const std::exception& e) {
            std::cerr << "Exception while processing file " << filepath << ": " << e.what() << std::endl;
            failedFiles.push_back(filepath);
            success = false;
        }
        catch (...) {
            std::cerr << "Unknown exception while processing file " << filepath << std::endl;
            failedFiles.push_back(filepath);
            success = false;
        }
    }

    if (logFile.is_open()) {
        std::cout.rdbuf(coutOriginal);
        std::cerr.rdbuf(cerrOriginal);

        delete teeStdout;
        delete teeStderr;
    }

    return success;
}

void processDirectory(const std::string& dirPath, const std::string& outputDir,
    const std::string& exportFormat, bool debugMode, std::ofstream& logFile,
    std::vector<std::string>& failedFiles, const std::string& formatFilter = "") {

    Catalog tempCatalog;
    tempCatalog.initialize();

    std::streambuf* coutOriginal = nullptr;
    std::streambuf* cerrOriginal = nullptr;
    TeeStreamBuffer* teeStdout = nullptr;
    TeeStreamBuffer* teeStderr = nullptr;

    if (logFile.is_open()) {
        coutOriginal = std::cout.rdbuf();
        cerrOriginal = std::cerr.rdbuf();

        teeStdout = new TeeStreamBuffer(coutOriginal, logFile.rdbuf());
        teeStderr = new TeeStreamBuffer(cerrOriginal, logFile.rdbuf());

        std::cout.rdbuf(teeStdout);
        std::cerr.rdbuf(teeStderr);
    }

    std::cout << "Processing directory: " << dirPath << std::endl;
    if (!formatFilter.empty()) {
        std::cout << "Filtering for files with extension: " << formatFilter << std::endl;
    }

    try {
        for (const auto& entry : fs::recursive_directory_iterator(dirPath)) {
            if (entry.is_regular_file()) {
                std::string filePath = entry.path().string();

                if (formatFilter.empty()) {
                    if (isSupportedFileType(filePath, tempCatalog)) {
                        processFile(filePath, outputDir, exportFormat, debugMode, logFile, failedFiles);
                    }
                }
                else {
                    if (hasExtension(filePath, formatFilter)) {
                        std::cout << "Processing matching file: " << entry.path().filename().string() << std::endl;
                        processFile(filePath, outputDir, exportFormat, debugMode, logFile, failedFiles);
                    }
                }
            }
        }
    }
    catch (const fs::filesystem_error& e) {
        std::cerr << "Filesystem error: " << e.what() << std::endl;
        failedFiles.push_back(dirPath);
    }

    if (logFile.is_open()) {
        std::cout.rdbuf(coutOriginal);
        std::cerr.rdbuf(cerrOriginal);

        delete teeStdout;
        delete teeStderr;
    }
}

int main(int argc, char* argv[]) {
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help,h", "produce help message")
        ("file", po::value<std::string>(), "input file or directory to parse")
        ("xml", "export to XML")
        ("yaml,y,yml", "export to YAML")
        ("debug,d", "enable debug mode to show offsets")
        ("recursive,r", po::value<std::string>()->implicit_value(""), "process all supported files in directory recursively. Optionally specify a format to filter by.")
        ("output,o", po::value<std::string>(), "specify output directory for exported files")
        ("log,l", "export complete log to a txt file");

    po::positional_options_description p;
    p.add("file", 1);

    po::variables_map vm;
    try {
        po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), vm);
        po::notify(vm);
    }
    catch (const po::error& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    if (vm.count("help") || !vm.count("file")) {
        std::cout << "ReCap Parser for Darkspore" << std::endl;
        std::cout << "Usage: recap_parser [options] <file|directory>" << std::endl;
        std::cout << desc << std::endl;
        return 0;
    }

    std::string inputPath = vm["file"].as<std::string>();
    bool xmlMode = vm.count("xml") > 0;
    bool yamlMode = vm.count("yaml") > 0;
    bool debugMode = vm.count("debug") > 0;
    std::string formatFilter = "";
    bool recursiveMode = false;

    if (xmlMode && yamlMode) {
        std::cerr << "Error: Cannot specify both --xml and --yaml at the same time." << std::endl;
        return 1;
    }

    std::string exportFormat = "none";
    if (xmlMode) {
        exportFormat = "xml";
    }
    else if (yamlMode) {
        exportFormat = "yaml";
    }

    if (vm.count("recursive")) {
        recursiveMode = true;
        if (vm["recursive"].as<std::string>() != "") {
            formatFilter = vm["recursive"].as<std::string>();
        }
    }

    bool logEnabled = vm.count("log") > 0;

    std::string outputDir = "";
    if (vm.count("output")) {
        outputDir = vm["output"].as<std::string>();
        if (!fs::exists(outputDir)) {
            try {
                fs::create_directories(outputDir);
                std::cout << "Created output directory: " << outputDir << std::endl;
            }
            catch (const fs::filesystem_error& e) {
                std::cerr << "Error creating output directory: " << e.what() << std::endl;
                return 1;
            }
        }
    }

    std::ofstream logFile;
    std::string logPath;
    if (logEnabled) {
        logPath = "parser_log.txt";

        logFile.open(logPath);
        if (!logFile.is_open()) {
            std::cerr << "Failed to open log file: " << logPath << std::endl;
            return 1;
        }
        std::cout << "Logging to: " << logPath << std::endl;
    }

    std::vector<std::string> failedFiles;

    std::streambuf* coutOriginal = nullptr;
    std::streambuf* cerrOriginal = nullptr;
    TeeStreamBuffer* teeStdout = nullptr;
    TeeStreamBuffer* teeStderr = nullptr;

    if (logEnabled) {
        coutOriginal = std::cout.rdbuf();
        cerrOriginal = std::cerr.rdbuf();

        teeStdout = new TeeStreamBuffer(coutOriginal, logFile.rdbuf());
        teeStderr = new TeeStreamBuffer(cerrOriginal, logFile.rdbuf());

        std::cout.rdbuf(teeStdout);
        std::cerr.rdbuf(teeStderr);
    }

    if (recursiveMode && fs::is_directory(inputPath)) {
        processDirectory(inputPath, outputDir, exportFormat, debugMode, logFile, failedFiles, formatFilter);
    }
    else if (fs::is_directory(inputPath)) {
        std::cerr << "Input path is a directory. Use --recursive to process all files." << std::endl;
        return 1;
    }
    else {
        processFile(inputPath, outputDir, exportFormat, debugMode, logFile, failedFiles);
    }

    if (!failedFiles.empty()) {
        std::cout << "\nThe following " << failedFiles.size() << " files failed to process:" << std::endl;

        for (const auto& file : failedFiles) {
            std::cout << "  - " << file << std::endl;
        }
    }

    if (logEnabled) {
        std::cout.rdbuf(coutOriginal);
        std::cerr.rdbuf(cerrOriginal);

        delete teeStdout;
        delete teeStderr;
    }

    if (logFile.is_open()) {
        logFile.close();
    }

    return failedFiles.empty() ? 0 : 1;
}