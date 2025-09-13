cmake_minimum_required(VERSION 3.14)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(rp2040-interp C CXX)
option(RP2040_INTERP_WITH_C "use c library" ON)
option(RP2040_INTERP_WITH_CPP "use c++ library" ON)
option(RP2040_INTERP_WITH_TESTS "use common test library" OFF)

if(${RP2040_INTERP_WITH_C})
    add_subdirectory(c)
endif()

if(${RP2040_INTERP_WITH_CPP})
    add_subdirectory(cpp)
endif()

if(${RP2040_INTERP_WITH_TESTS})
    add_subdirectory(tests)
endif()
