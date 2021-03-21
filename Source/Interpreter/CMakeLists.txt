
add_subdirectory(Analyzer)
add_subdirectory(ScriptFile)

add_library(Interpreter
    Interpreter.cpp)

target_link_libraries(Interpreter
    Analyzer
    ScriptFile)

target_include_directories(Interpreter PUBLIC 
    "h"
    "Analyzer/h"
    "ScriptFile/h")