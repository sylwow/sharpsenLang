
add_subdirectory(Compiler)
add_subdirectory(Runtime)
add_subdirectory(ScriptFile)

add_library(Interpreter
    Interpreter.cpp)

target_link_libraries(Interpreter
    Compiler
    Runtime
    ScriptFile)

target_include_directories(Interpreter PUBLIC 
    "h"
    "Compiler/h"
    "ScriptFile/h")