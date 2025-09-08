cmake_minimum_required(VERSION 3.14)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_COLOR_DIAGNOSTICS ON)

# set include paths
set(REPO_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../")
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${REPO_SOURCE_DIR}/tests/cmake")

# load SDK
set(PICO_CXX_ENABLE_EXCEPTIONS 1)
include(find_sdk)
include(pico_sdk_import)

# define project
project(pico-auto-test)

# init SDK
pico_sdk_init()
include(fix_clangd_includes)

# load rp2040-interp library
set(RP2040_INTERP_WITH_HARDWARE ON)
set(RP2040_INTERP_WITH_TESTS ON)
if(${PICO_BOARD} STREQUAL pico2)
    set(RP2040_INTERP_GENERATION_RP2350 ON)
endif()
add_subdirectory(${REPO_SOURCE_DIR} rp2040-interp)

# add target
file(GLOB sources *.cpp)
add_executable(${PROJECT_NAME} ${sources})
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 23)
target_compile_options(${PROJECT_NAME} PUBLIC -Wall -Wextra)
target_link_libraries(${PROJECT_NAME} pico_stdlib pico_rand pico_time rp2040-interp-test)
pico_add_extra_outputs(${PROJECT_NAME})
pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)
