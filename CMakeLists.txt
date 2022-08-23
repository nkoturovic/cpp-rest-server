cmake_minimum_required(VERSION 3.16)

# Name of the project
project(Rest-Server)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(cmake/RunConan.cmake)
run_conan()

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)  # Includes the contents of the conanbuildinfo.cmake file.
conan_basic_setup()  # Prepares the CMakeList.txt for Conan.

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(HEADERS 
    src/3rd_party/refl.hpp src/3rd_party/color.hpp
    src/actions.hpp src/errors.hpp src/handler.hpp src/models.hpp src/routes.hpp src/utils.hpp 
    src/model/field.hpp src/model/constraint.hpp src/model/model.hpp
)

set(SRC_LIST src/main.cpp)
include_directories(src)

add_executable(rest-server ${SRC_LIST} ${HEADERS})

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/db.sqlite
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/bin)

target_link_libraries(rest-server ${CONAN_LIBS})

add_subdirectory(examples)
