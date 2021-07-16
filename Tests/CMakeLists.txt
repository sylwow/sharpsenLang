
add_executable(Test
    RunTests.cpp
    TestTemplate.cpp
)

target_link_libraries(Test 
    SharpsenLangLib
    CONAN_PKG::gtest
)

gtest_discover_tests(Test Tests PROPERTIES TIMEOUT 10)