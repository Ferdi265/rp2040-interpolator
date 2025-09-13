cmake_minimum_required(VERSION 3.14)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(rp2040-interp-c C)
option(RP2040_INTERP_GENERATION_RP2350 "default to RP2350 interpolator generation" OFF)

file(GLOB rp2040-interp-c-sources src/*.c)
add_library(${PROJECT_NAME} STATIC ${rp2040-interp-c-sources})
set_property(TARGET ${PROJECT_NAME} PROPERTY C_STANDARD 23)
target_include_directories(${PROJECT_NAME} PUBLIC include/)
if(${RP2040_INTERP_GENERATION_RP2350})
    target_compile_definitions(${PROJECT_NAME} PUBLIC INTERP_SW_GENERATION_DEFAULT_RP2350=1)
endif()
