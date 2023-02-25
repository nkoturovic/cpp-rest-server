add_executable(soci_example soci_example.cpp)
add_executable(json_example json_example.cpp)
add_executable(constraint_example constraint_example.cpp)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/../db.sqlite
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

include_directories(../src)

target_link_libraries(soci_example PRIVATE pthread fmt::fmt SOCI::soci_core SOCI::soci_sqlite3)
target_link_libraries(json_example PRIVATE pthread fmt::fmt)
target_link_libraries(constraint_example PRIVATE pthread fmt::fmt)

set(CPP_REST_SERVER_EXAMPLES soci_example json_example constraint_example PARENT_SCOPE)
