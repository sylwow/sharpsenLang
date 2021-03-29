
add_library(Compiler
Compiler.cpp)

target_include_directories(Compiler PUBLIC 
    "h"
    "../h"
    "../ScriptFile/h")