add_executable(soci_example soci_example.cpp)
add_executable(json_example json_example.cpp)
add_executable(constraint_example constraint_example.cpp)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/../db.sqlite
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

include_directories(../src)

target_link_libraries(soci_example PRIVATE pthread CONAN_PKG::fmt CONAN_PKG::soci)
target_link_libraries(json_example PRIVATE pthread CONAN_PKG::fmt)
target_link_libraries(constraint_example PRIVATE pthread CONAN_PKG::fmt)
