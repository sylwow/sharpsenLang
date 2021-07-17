
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS *.cpp)

add_library(SharpsenLangLib ${SOURCES})

target_include_directories(SharpsenLangLib PUBLIC
  h
)
