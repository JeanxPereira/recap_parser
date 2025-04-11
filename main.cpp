#include <iostream>
#include <filesystem>
#include <fstream>
#include <vector>
#include <boost/program_options.hpp>
#include "catalog.h"

namespace po = boost::program_options;
namespace fs = std::filesystem;

std::string getFileExtension(const std::string& filepath) {
    std::string extension = fs::path(filepath).extension().string();
    for (auto& c : extension) {
        c = std::tolower(c);
    }
    return extension;
}

bool isSupportedFileType(const std::string& filepath, const Catalog& catalog) {
    std::string extension = getFileExtension(filepath);
    return catalog.getFileType(extension) != nullptr;
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
    bool xmlMode, bool debugMode, std::ofstream& logFile,
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
            Parser parser(catalog, filepath, debugMode, xmlMode);

            std::cout << "Parsing \"" << filename << "\"" << std::endl;

            if (!parser.parse()) {
                std::cerr << "Error parsing file: " << filepath << std::endl;
                failedFiles.push_back(filepath);
                success = false;
            }
            else if (xmlMode) {
                std::string originalExtension = fs::path(filepath).extension().string();
                std::string outputFilename = fs::path(filepath).stem().string() + originalExtension + ".xml";

                std::string outputPath;
                if (!outputDir.empty()) {
                    outputPath = (fs::path(outputDir) / outputFilename).string();
                }
                else {
                    outputPath = outputFilename;
                }

                parser.exportXml(outputPath);
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
    bool xmlMode, bool debugMode, std::ofstream& logFile,
    std::vector<std::string>& failedFiles) {

    Catalog tempCatalog; // Used only to check if file types are supported

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

    try {
        for (const auto& entry : fs::recursive_directory_iterator(dirPath)) {
            if (entry.is_regular_file() && isSupportedFileType(entry.path().string(), tempCatalog)) {
                processFile(entry.path().string(), outputDir, xmlMode, debugMode, logFile, failedFiles);
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
        ("help", "produce help message")
        ("file", po::value<std::string>(), "input file or directory to parse")
        ("xml", "export to XML")
        ("debug,d", "enable debug mode to show offsets")
        ("recursive,r", "process all supported files in directory recursively")
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
        std::cout << "Usage: parser [options] <file|directory>" << std::endl;
        std::cout << desc << std::endl;
        return 0;
    }

    std::string inputPath = vm["file"].as<std::string>();
    bool xmlMode = vm.count("xml") > 0;
    bool debugMode = vm.count("debug") > 0;
    bool recursiveMode = vm.count("recursive") > 0;
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
        processDirectory(inputPath, outputDir, xmlMode, debugMode, logFile, failedFiles);
    }
    else if (fs::is_directory(inputPath)) {
        std::cerr << "Input path is a directory. Use --recursive to process all files." << std::endl;
        return 1;
    }
    else {
        processFile(inputPath, outputDir, xmlMode, debugMode, logFile, failedFiles);
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