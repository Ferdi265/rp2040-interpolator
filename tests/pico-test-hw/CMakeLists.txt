cmake_minimum_required(VERSION 3.14)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# set include paths
set(REPO_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../")
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${REPO_SOURCE_DIR}/cmake")

# load SDK
include(find_pico_sdk)
include(pico_sdk_import)

# define project
project(pico-test-hw)

# init SDK
pico_sdk_init()
include(fix_clangd_includes)

# load rp2040-interp library
set(HAVE_RP2040_HARDWARE_INTERP ON)
add_subdirectory(${REPO_SOURCE_DIR} rp2040-interp)

# add target
file(GLOB sources *.cpp)
add_executable(${PROJECT_NAME} ${sources})
target_link_libraries(${PROJECT_NAME} pico_stdlib hardware_interp rp2040-interp)
pico_add_extra_outputs(${PROJECT_NAME})
pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)
