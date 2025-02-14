include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.15.2)
FetchContent_MakeAvailable(googletest)

include(CTest)
include(GoogleTest)

# Find all .cpp files in the current directory
file(GLOB TEST_SOURCES "*.cpp")
add_executable(flowTests ${TEST_SOURCES})
target_link_libraries(flowTests PRIVATE gtest_main ionFlowLib)
target_include_directories(flowTests
                           PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../src/include)

add_test(
    NAME flowTests
    COMMAND flowTests
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/tests)
set_tests_properties(flowTests PROPERTIES LABELS "unit")
gtest_discover_tests(flowTests)

