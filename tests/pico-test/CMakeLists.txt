cmake_minimum_required(VERSION 3.14)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_COLOR_DIAGNOSTICS ON)

# set include paths
set(REPO_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../")
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${REPO_SOURCE_DIR}/tests/cmake")

# load SDK
include(find_sdk)
include(pico_sdk_import)

# define project
project(pico-test)

# init SDK
pico_sdk_init()
include(fix_clangd_includes)

# load rp2040-interp library
set(RP2040_INTERP_WITH_HARDWARE ON)
set(RP2040_INTERP_WITH_TESTS ON)
if(${PICO_PLATFORM} MATCHES ^rp2350)
    set(RP2040_INTERP_GENERATION_RP2350 ON)
endif()
add_subdirectory(${REPO_SOURCE_DIR} rp2040-interp)

# add target
file(GLOB sources *.cpp)
foreach(type sw hw)
    set(target ${PROJECT_NAME}-${type})

    add_executable(${target} ${sources})
    set_property(TARGET ${target} PROPERTY CXX_STANDARD 23)
    target_compile_options(${target} PUBLIC -Wall -Wextra)
    target_link_libraries(${target} pico_platform pico_stdlib rp2040-interp-test)
    pico_add_extra_outputs(${target})
    pico_enable_stdio_usb(${target} 1)
    pico_enable_stdio_uart(${target} 0)

    string(TOUPPER ${type} type-upper)
    target_compile_definitions(${target} PUBLIC INTERP_TYPE=Interp${type-upper})
endforeach()
