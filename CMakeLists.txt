cmake_minimum_required(VERSION 3.10)
project(IonFlow VERSION 1.0.0 LANGUAGES CXX)

add_library(IonFlow STATIC
    src/ionflow.cpp
    src/sticky_socket.cpp
)
set_target_properties(IonFlow PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED YES
)

target_include_directories(IonFlow PUBLIC ${PROJECT_SOURCE_DIR}/include)

option(BUILD_EXAMPLES "Build the examples" ON)
if(BUILD_EXAMPLES)
    add_executable(hello_ion examples/hello_ion.cpp)
    set_target_properties(hello_ion PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED YES
    )
    target_link_libraries(hello_ion PRIVATE IonFlow)
endif()

