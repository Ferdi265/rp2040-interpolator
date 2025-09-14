cmake_minimum_required(VERSION 3.14)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_COLOR_DIAGNOSTICS ON)

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
foreach(type sw swc)
    set(target ${PROJECT_NAME}-${type})

    add_executable(${target} ${sources})
    set_property(TARGET ${target} PROPERTY CXX_STANDARD 23)
    target_compile_options(${target} PUBLIC -Wall -Wextra)
    target_link_libraries(${target} PUBLIC rp2040-interp-test)

    string(TOUPPER ${type} type-upper)
    target_compile_definitions(${target} PUBLIC INTERP_TYPE=Interp${type-upper})
endforeach()
