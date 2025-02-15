cmake_minimum_required(VERSION 3.30)
project(
    IonFlow
    VERSION 1.0.0
    LANGUAGES CXX)

message(
    STATUS
        "Building -=[ ${CMAKE_PROJECT_NAME} v${CMAKE_PROJECT_VERSION}-${CMAKE_BUILD_TYPE} (${TARGET_PLATFORM}-${TARGET_ABI}) ]=-"
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_EXTENSIONS OFF)

option(BUILD_EXAMPLES "Build the examples" OFF)
option(BUILD_TESTS "Build unit tests" OFF)

add_subdirectory(src)

if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests/)
endif()
