#include "Api.hpp"
#include <filesystem>

namespace api {

    void Api::validate(int argc, char** argv) {
        switch (argc)
        {
        case 1:
            printHelp();
            break;
        case 2:
            validateFile(argv[1]);
            break;
        default:
            invalidArgumentNmber(argc);
            printHelp();
            break;
        }
    }
    
    void Api::validateFile(std::filesystem::path filename) {
        if (!filename.has_extension() || filename.extension() != ".srp") {
            invalidFileExtension(filename.extension());
            return;
        }
        _ok = true;
        _script = filename;
    }

    void Api::printHelp() {
        std::cout << "To run program pass one argument wchich is a path to .srp script" << std::endl;
    }

    void Api::invalidArgumentNmber(int argc) {
        std::cout << "Wrong arguments number: " << argc << " should be 1" << std::endl;
    }

    void Api::invalidFileExtension(std::filesystem::path extension) {
        std::cout << "Invalid script file extension: " << extension << " should be .srp file" << std::endl;
    }
}