cmake_minimum_required(VERSION 3.16)

project(Examples)

set(CMAKE_CXX_STANDARD 20)
add_executable(soci_example soci_example.cpp)
add_executable(json_example json_example.cpp)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/../db.sqlite
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

include_directories(../src)

target_link_libraries(soci_example PRIVATE pthread fmt soci_core soci_sqlite3)
target_link_libraries(json_example PRIVATE pthread fmt)
