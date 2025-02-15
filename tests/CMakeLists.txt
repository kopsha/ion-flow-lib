cmake_minimum_required(VERSION 3.10)
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

# define library target
file(GLOB ION_SOURCES "src/*.cpp")
add_library(ionFlowLib STATIC ${ION_SOURCES})

target_include_directories(
    ionFlowLib PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/include>
                      $<INSTALL_INTERFACE:include/ionflow>)

if(BUILD_EXAMPLES)
    add_executable(hello_ion examples/hello_ion.cpp)
    target_link_libraries(hello_ion PRIVATE ionFlowLib)
endif()

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# working clean
find_program(CLANG_TIDY_CLI "clang-tidy")
if(CLANG_TIDY_CLI)
    message(STATUS "Using clang-tidy at: ${CLANG_TIDY_CLI}.")
    set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_CLI}")
else()
    message(WARN "clang-tidy not found. Static analysis will not be performed.")
endif()
