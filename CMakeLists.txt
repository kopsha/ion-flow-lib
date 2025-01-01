cmake_minimum_required(VERSION 3.10)

project(IonFlow VERSION 1.0.0 LANGUAGES CXX)

add_library(IonFlow STATIC
    src/ionflow.cpp
)

target_include_directories(IonFlow PUBLIC
    ${PROJECT_SOURCE_DIR}/include
)

set_target_properties(IonFlow PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
)

option(BUILD_EXAMPLES "Build the examples" ON)
if(BUILD_EXAMPLES)
    add_executable(hello_ion examples/hello_ion.cpp)
    target_link_libraries(hello_ion PRIVATE IonFlow)
endif()

