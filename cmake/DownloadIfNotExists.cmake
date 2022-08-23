function(download_if_not_exists ARG_FPATH ARG_LINK)
    if(NOT EXISTS ${ARG_FPATH})
       message(STATUS "Downloading ${ARG_FPATH} from ${ARG_LINK}")
       file(DOWNLOAD ${ARG_LINK} ${ARG_FPATH} STATUS DOWNLOAD_STATUS)
        list(GET DOWNLOAD_STATUS 0 STATUS_CODE)
        list(GET DOWNLOAD_STATUS 1 ERROR_MESSAGE)
        if(${STATUS_CODE} EQUAL 0)
            message(STATUS "${ARG_FPATH} downloaded successfully!")
        else()
            file(REMOVE ${ARG_FPATH})
            message(FATAL_ERROR 
                "Error downloading conan.cmake: ${ERROR_MESSAGE}, "  
                "Please check your Internet Connection!"
            )
        endif()
    endif()
endfunction()
