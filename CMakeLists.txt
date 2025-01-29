cmake_minimum_required(VERSION 3.10)
project(IonFlow VERSION 1.0.0 LANGUAGES CXX)

include(CTest)
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.15.2
)
FetchContent_MakeAvailable(googletest)

add_library(IonFlow STATIC
    src/ionflow.cpp
)

target_include_directories(IonFlow PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_compile_options(IonFlow PRIVATE -coverage)
target_link_options(IonFlow PRIVATE -coverage)

set_target_properties(IonFlow PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
)

option(BUILD_EXAMPLES "Build the examples" OFF)
if(BUILD_EXAMPLES)
    add_executable(hello_ion examples/hello_ion.cpp)
    target_link_libraries(hello_ion PRIVATE IonFlow)
endif()

option(BUILD_TESTS "Build the unit tests" ON)
if(BUILD_TESTS)
    message("Building tests")
enable_testing()
    add_executable(ion_tests tests/ion_tests.cpp)
    target_compile_options(ion_tests PRIVATE -O0 -fprofile-generate -fno-exceptions -fno-inline -coverage)
    target_link_options(ion_tests PRIVATE -coverage)

    include(GoogleTest)
    gtest_discover_tests(ion_tests)
    target_link_libraries(ion_tests PRIVATE gtest_main IonFlow)

    # Add custom coverage report target
    find_program(GCOVR gcovr)
    if(GCOVR)
        add_custom_target(coverage
            COMMAND ${GCOVR}
                -r ${CMAKE_SOURCE_DIR}
                --html --html-details
                -o ${CMAKE_BINARY_DIR}/coverage/index.html
                --exclude '.*gtest.*'
                --exclude '/usr/include/.*'
                --exclude '${CMAKE_SOURCE_DIR}/tests/.*'
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMENT "Generating HTML coverage report..."
        )

    else()
        message(WARNING "gcovr not found, skipping coverage target")
    endif()
endif()

