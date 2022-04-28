cmake_minimum_required(VERSION 3.14)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(rp2040-interpolator CXX)

option(HAVE_RP2040_HARDWARE_INTERP "use RP2040 hardware interpolator" OFF)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(rp2040-interp INTERFACE)
target_include_directories(rp2040-interp INTERFACE include/)
if(${HAVE_RP2040_HARDWARE_INTERP})
    target_compile_definitions(rp2040-interp INTERFACE HAVE_RP2040_HARDWARE_INTERP=1)
    target_link_libraries(rp2040-interp INTERFACE hardware_interp)
endif()
