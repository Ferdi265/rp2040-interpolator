cmake_minimum_required(VERSION 3.14)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# set include paths
set(REPO_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../")
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${REPO_SOURCE_DIR}/cmake")

# define project
project(host-test)

# load rp2040-interp library
set(RP2040_INTERP_WITH_HARDWARE OFF)
set(RP2040_INTERP_WITH_TESTS ON)
add_subdirectory(${REPO_SOURCE_DIR} rp2040-interp)

# add target
file(GLOB sources *.cpp)
add_executable(${PROJECT_NAME}-sw ${sources})
set_property(TARGET ${PROJECT_NAME}-sw PROPERTY CXX_STANDARD 17)
target_compile_options(${PROJECT_NAME}-sw PUBLIC -Wall -Wextra)
target_link_libraries(${PROJECT_NAME}-sw rp2040-interp-test)
