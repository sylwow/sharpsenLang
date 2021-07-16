
add_subdirectory(Library)

add_executable(SharpsenLang
  main.cpp
)

target_link_libraries(SharpsenLang 
  SharpsenLangLib
)