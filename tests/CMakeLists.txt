file(GLOB test-sources lib/src/*.cpp)
add_library(${PROJECT_NAME}-test ${test-sources})
set_property(TARGET ${PROJECT_NAME}-test PROPERTY CXX_STANDARD 17)
target_include_directories(${PROJECT_NAME}-test PUBLIC lib/include/)
target_link_libraries(${PROJECT_NAME}-test PUBLIC ${PROJECT_NAME})
