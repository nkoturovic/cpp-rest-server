macro(run_conan)
    set(CONAN_SYSTEM_INCLUDES "On") # use -isystem for conan includes instead of -I
    set(ENV{CONAN_SYSREQUIRES_MODE} "enabled")
    set(ENV{CONAN_SYSREQUIRES_SUDO} "1")

    include(cmake/DownloadIfNotExists.cmake)
    download_if_not_exists("${CMAKE_CURRENT_LIST_DIR}/cmake/conan.cmake" "https://raw.githubusercontent.com/conan-io/cmake-conan/master/conan.cmake")
    include(${CMAKE_CURRENT_LIST_DIR}/cmake/conan.cmake)

    conan_add_remote(
        NAME
        bincrafters
        URL
        https://api.bintray.com/conan/bincrafters/public-conan
    )

    conan_add_remote(
        NAME
        kotur
        URL
        https://api.bintray.com/conan/kotur/public-conan
    )

    conan_cmake_run (
        CONANFILE conanfile.txt
        ENV CONAN_SYSREQUIRES_MODE=enabled
        ENV CONAN_SYSREQUIRES_SUDO=1
        BASIC_SETUP CMAKE_TARGETS
        BUILD missing
    )

    include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)  # Includes the contents of the conanbuildinfo.cmake file.
    conan_basic_setup()  # Prepares the CMakeList.txt for Conan.
endmacro()
