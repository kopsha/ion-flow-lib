cmake_minimum_required(VERSION 3.10)
project(
    IonFlow
    VERSION 1.0.0
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB ION_SOURCES "src/*.cpp")
add_library(ionFlowLib STATIC ${ION_SOURCES})

target_include_directories(
    ionFlowLib PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/include>
                      $<INSTALL_INTERFACE:include/ionflow>)

option(BUILD_EXAMPLES "Build the examples" ON)
if(BUILD_EXAMPLES)
    add_executable(hello_ion examples/hello_ion.cpp)
    target_link_libraries(hello_ion PRIVATE ionFlowLib)
endif()

option(BUILD_TESTS "Build unit tests" ON)
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
