cmake_minimum_required(VERSION 3.14)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(rp2040-interp CXX)

option(RP2040_INTERP_WITH_HARDWARE "use RP2040 hardware interpolator" OFF)
option(RP2040_INTERP_GENERATION_RP2350 "default to RP2350 interpolator generation" OFF)
option(RP2040_INTERP_WITH_TESTS "use common test library" OFF)

add_library(rp2040-interp INTERFACE)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)
target_include_directories(${PROJECT_NAME} INTERFACE include/)
if(${RP2040_INTERP_WITH_HARDWARE})
    target_compile_definitions(${PROJECT_NAME} INTERFACE RP2040_INTERP_WITH_HARDWARE=1)
    target_link_libraries(${PROJECT_NAME} INTERFACE hardware_interp)
endif()
if(${RP2040_INTERP_GENERATION_RP2350})
    target_compile_definitions(${PROJECT_NAME} INTERFACE RP2040_INTERP_GENERATION_RP2350=1)
endif()

if(${RP2040_INTERP_WITH_TESTS})
    add_subdirectory(tests)
endif()
