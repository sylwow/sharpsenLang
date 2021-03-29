
add_library(Runtime
Runtime.cpp)

target_include_directories(Runtime PUBLIC 
    "h"
    "../h"
    "../ScriptFile/h")