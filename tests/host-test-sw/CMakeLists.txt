cmake_minimum_required(VERSION 3.14)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# set include paths
set(REPO_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../")
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${REPO_SOURCE_DIR}/cmake")

# define project
project(host-test-sw)

# load rp2040-interp library
set(HAVE_RP2040_HARDWARE_INTERP OFF)
add_subdirectory(${REPO_SOURCE_DIR} rp2040-interp)

# add target
file(GLOB sources *.cpp)
add_executable(${PROJECT_NAME} ${sources})
target_link_libraries(${PROJECT_NAME} rp2040-interp)
