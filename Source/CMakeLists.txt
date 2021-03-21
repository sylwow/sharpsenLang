add_subdirectory(Types)
add_subdirectory(Api)
add_subdirectory(Interpreter)

add_executable(sharpcore main.cpp)

target_link_libraries(sharpcore
    Interpreter
    Api
    Types)

target_include_directories(sharpcore PUBLIC 
    "Interpreter/h"
    "Api/h")