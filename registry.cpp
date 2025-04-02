#include "catalog.h"
#include <iostream>
#include <memory>
#include <string>

void printUsage() {
    std::cout << "Usage: darkspore_parser path [recursive] [xml] [-d|--debug]" << std::endl;
    std::cout << "  path: path to file or directory" << std::endl;
    std::cout << "  recursive: parse all files in path recursively" << std::endl;
    std::cout << "  xml: export parsed files as xml" << std::endl;
    std::cout << "  -d, --debug: show offset information for each parsed field" << std::endl;
    std::cout << std::endl;
    std::cout << "Supported file types (by extension):" << std::endl;
    std::cout << "  .phase, .characteranimation, .aidefinition, .classattributes," << std::endl;
    std::cout << "  .npcaffix, .playerclass, .nonplayerclass, .noun, .markerset, .catalog" << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printUsage();
        return 1;
    }

    try {
        // Parse command line arguments
        AppConfig config = AppConfig::parseCommandLine(argc, argv);

        // Create processor with configuration
        DarksporeProcessor processor(config);

        // Process input path
        bool success = processor.processPath(config.inputPath);

        return success ? 0 : 1;
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}