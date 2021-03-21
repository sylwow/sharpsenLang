
add_library(Analyzer
Analyzer.cpp)

target_include_directories(Analyzer PUBLIC 
    "h"
    "../ScriptFile/h")