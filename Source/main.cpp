#include <iostream>
#include "IInterpreter.hpp"
#include "Api.hpp"

int main(int argc, char** argv) {
    api::Api validator;
    validator.validate(argc, argv);
    if (!validator.isOk()) {
        return 0;
    }
    intrp::IInterpreter* interpreter = intrp::createNewInterpreter();
    interpreter->load(validator.getScriptFilePath());
    interpreter->run();
    return 0;
}

