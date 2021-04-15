cmake_minimum_required(VERSION 3.15)

project(MyCppProject)
include(cmake/StandardProjectSettings.cmake)
include(cmake/PreventInSourceBuilds.cmake)

add_library(project_options INTERFACE)
target_compile_features(project_options INTERFACE cxx_std_20)
target_include_directories(project_options INTERFACE lib)

if(CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
  option(ENABLE_BUILD_WITH_TIME_TRACE "Enable -ftime-trace to generate time tracing .json files on clang" OFF)
  if(ENABLE_BUILD_WITH_TIME_TRACE)
    target_compile_options(project_options INTERFACE -ftime-trace)
  endif()
endif()

# Link this 'library' to use the warnings specified in CompilerWarnings.cmake
add_library(project_warnings INTERFACE)
include(cmake/CompilerWarnings.cmake)
set_project_warnings(project_warnings)

# sanitizer options if supported by compiler
include(cmake/Sanitizers.cmake)
enable_sanitizers(project_options)

include(cmake/StaticAnalyzers.cmake)

option(BUILD_SHARED_LIBS "Enable compilation of shared libraries" OFF)

include(cmake/RunConan.cmake)
run_conan()

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)  # Includes the contents of the conanbuildinfo.cmake file.
conan_basic_setup()  # Prepares the CMakeList.txt for Conan.

# Disabled options
include(cmake/Doxygen.cmake) # OFF by default
enable_doxygen()
option(ENABLE_TESTING "Enable Test Builds" OFF)
option(ENABLE_FUZZING "Enable Fuzzing Builds" OFF)

if(ENABLE_TESTING)
  enable_testing()
  message("Building Tests. Be sure to check out test/constexpr_tests for constexpr testing")
  add_subdirectory(test)
endif()

if(ENABLE_FUZZING)
  message("Building Fuzz Tests, using fuzzing sanitizer https://www.llvm.org/docs/LibFuzzer.html")
  add_subdirectory(fuzz_test)
endif()

option(BUILD_LIB_MYLIB "Enable Building of Lib 'MyLib'" ON)
if(BUILD_LIB_MYLIB)
    message("Building Library mylib enabled")
    add_subdirectory(lib/mylib)
endif()

option(BUILD_SRC "Enable Building of 'prog_rest_server'" ON)
if(BUILD_SRC)
    message("Building of myprog enabled")
    add_subdirectory(prog/rest_server)
endif()

option(BUILD_DEV_EXAMPLES "Enable Building of 'dev_examples'" ON)
if(BUILD_DEV_EXAMPLES)
    message("Building dev examples enabled")
    add_subdirectory(dev_example)
endif()

option(BUILD_DEMOS "Enable Building of 'demos'" ON)
if(BUILD_DEMOS)
    message("Building demos enabled")
    add_subdirectory(demo)
endif()

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/db.sqlite
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/bin)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/db.sqlite
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
