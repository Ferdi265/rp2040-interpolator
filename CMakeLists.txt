cmake_minimum_required(VERSION 3.14)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(rp2040-interp CXX)

option(RP2040_INTERP_WITH_HARDWARE "use RP2040 hardware interpolator" OFF)
option(RP2040_INTERP_WITH_TESTS "use common test library" OFF)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(rp2040-interp INTERFACE)
target_include_directories(${PROJECT_NAME} INTERFACE include/)
if(${RP2040_INTERP_WITH_HARDWARE})
    target_compile_definitions(${PROJECT_NAME} INTERFACE HAVE_RP2040_HARDWARE_INTERP=1)
    target_link_libraries(${PROJECT_NAME} INTERFACE hardware_interp)
endif()

if(${RP2040_INTERP_WITH_TESTS})
    add_subdirectory(tests)
endif()
