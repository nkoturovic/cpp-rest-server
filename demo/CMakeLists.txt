add_executable(demo_use_mylib use_mylib.cpp)
target_include_directories(demo_use_mylib SYSTEM PRIVATE ../lib)
target_link_libraries(demo_use_mylib
    PRIVATE project_options project_warnings mylib)

add_executable(demo_json json.cpp)
target_include_directories(demo_json SYSTEM PRIVATE ../prog/rest_server ../prog/rest_server/3rd_party)
target_link_libraries(demo_json
    PRIVATE project_options project_warnings CONAN_PKG::fmt CONAN_PKG::nlohmann_json)

add_executable(demo_soci soci.cpp)
target_include_directories(demo_soci SYSTEM PRIVATE ../prog/rest_server ../prog/rest_server/3rd_party)
target_link_libraries(demo_soci
    PRIVATE project_options project_warnings CONAN_PKG::fmt CONAN_PKG::SOCI)

add_executable(demo_constraint constraint.cpp)
target_include_directories(demo_constraint SYSTEM PRIVATE ../prog/rest_server ../prog/rest_server/3rd_party)
target_link_libraries(demo_constraint
    PRIVATE project_options project_warnings CONAN_PKG::fmt CONAN_PKG::nlohmann_json)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/../db.sqlite
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
