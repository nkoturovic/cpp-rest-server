add_executable(dev_hello_example hello_example.cpp)
target_link_libraries(dev_hello_example
    PRIVATE project_options project_warnings CONAN_PKG::fmt)

