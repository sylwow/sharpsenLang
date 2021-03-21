#pragma once
#include <iostream>
#include <filesystem>

namespace api {
    class Api {
        private:
            std::filesystem::path _script;
            bool _ok = false;
        public:
            void validate(int argc, char** argv);
            bool isOk() { return _ok; }
            std::filesystem::path& getScriptFilePath() { return _script; }
        private:
            void validateFile(std::filesystem::path filename);
            void invalidFileExtension(std::filesystem::path extension);
            void printHelp();
            void invalidArgumentNmber(int argc);
            void filescriptDoesNotExists(std::filesystem::path script);
    };
}