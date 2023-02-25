cmake_minimum_required(VERSION 3.16)

option(CPP_REST_SERVER_BUILD_EXAMPLES "Build examples" ON)

# Name of the project
project(Cpp-Rest-Server)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(HEADERS 
    src/3rd_party/refl.hpp src/3rd_party/color.hpp
    src/actions.hpp src/errors.hpp src/handler.hpp src/models.hpp src/routes.hpp src/utils.hpp 
    src/model/field.hpp src/model/constraint.hpp src/model/model.hpp
)

find_package(Boost REQUIRED COMPONENTS date_time)
find_package(fmt REQUIRED)
find_package(SOCI REQUIRED)
find_package(cpp-jwt REQUIRED)

set(SRC_LIST src/main.cpp)
include_directories(src)

add_executable(cpp-rest-server ${SRC_LIST} ${HEADERS})

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/db.sqlite
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(cpp-rest-server fmt::fmt SOCI::soci_core SOCI::soci_sqlite3 cpp-jwt::cpp-jwt http_parser)

if (CPP_REST_SERVER_BUILD_EXAMPLES)
    add_subdirectory(examples)
    install(TARGETS cpp-rest-server ${CPP_REST_SERVER_EXAMPLES} DESTINATION bin)
else()
    install(TARGETS cpp-rest-server DESTINATION bin)
endif()
