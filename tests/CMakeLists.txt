project(rp2040-interp-test)

file(GLOB test-sources lib/src/*.cpp)
add_library(${PROJECT_NAME} STATIC ${test-sources})
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 23)
target_include_directories(${PROJECT_NAME} PUBLIC lib/include/)
target_link_libraries(${PROJECT_NAME} PUBLIC rp2040-interp-cpp)
