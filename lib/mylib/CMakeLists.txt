set(LIB_MYLIB_HEADERS
    example.hpp
)

set(LIB_MYLIB_SOURCES
    example.cpp
)

add_library(mylib STATIC 
    ${LIB_MYLIB_SOURCES}
    ${LIB_MYLIB_HEADERS}
)

target_link_libraries(mylib 
    PRIVATE project_options project_warnings 
    CONAN_PKG::fmt CONAN_PKG::spdlog
)
