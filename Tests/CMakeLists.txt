
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS *.cpp)

add_executable(Test
    ${SOURCES}
)

target_link_libraries(Test 
    SharpsenLangLib
    CONAN_PKG::gtest
)

target_include_directories(Test PRIVATE
    h
)

gtest_discover_tests(Test Tests PROPERTIES TIMEOUT 10)