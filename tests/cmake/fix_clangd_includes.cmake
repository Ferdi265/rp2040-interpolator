if(${FIX_CLANGD_INCLUDES})
    set(fix-clangd-includes "")

    if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
        execute_process(
            COMMAND "${CMAKE_CXX_COMPILER}" -x c++ -Wp,-v -E -
            INPUT_FILE /dev/null
            ERROR_VARIABLE fix-clangd-includes-gxx-output
            OUTPUT_QUIET
        )
        string(REPLACE "\n" ";" fix-clangd-includes-gxx-lines "${fix-clangd-includes-gxx-output}")
        list(FILTER fix-clangd-includes-gxx-lines INCLUDE REGEX "^ .*")
        list(TRANSFORM fix-clangd-includes-gxx-lines STRIP)
        set(fix-clangd-includes "${fix-clangd-includes-gxx-lines}")
    elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
        # nothing to do
    else()
        message(WARNING "unsupported compiler vendor ${CMAKE_CXX_COMPILER_ID}")
    endif()

    include_directories("${fix-clangd-includes}")
    foreach(include ${fix-clangd-includes})
        message(STATUS "fix_clangd_includes: found default include ${include}")
    endforeach()
endif()
