#include <iostream>
#include <filesystem>
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

int main(int argc, char* argv[]) {
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help", "produce help message")
        ("file", po::value<std::string>(), "input file to parse")
        ("xml", "export to XML")
        ("debug,d", "enable debug mode to show offsets");

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
        std::cout << "Usage: parser [options] <file>" << std::endl;
        std::cout << desc << std::endl;
        return 0;
    }

    std::string filepath = vm["file"].as<std::string>();
    bool xmlMode = vm.count("xml") > 0;
    bool debugMode = vm.count("debug") > 0;

    if (!fs::exists(filepath)) {
        std::cerr << "Error: File " << filepath << " does not exist" << std::endl;
        return 1;
    }

    std::string filename = fs::path(filepath).filename().string();
    std::string extension = getFileExtension(filepath);

    Catalog catalog;
    Parser parser(catalog, filepath, debugMode, xmlMode);

    std::cout << "parsing \"" << filename << "\"" << std::endl;

    if (!parser.parse()) {
        std::cerr << "Error parsing file" << std::endl;
        return 1;
    }

    if (xmlMode) {
        std::string outputFile = fs::path(filepath).stem().string() + ".xml";
        parser.exportXml(outputFile);
        std::cout << "Exported to " << outputFile << std::endl;
    }

    return 0;
}