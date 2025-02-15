include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.15.2
)
FetchContent_MakeAvailable(googletest)
include(GoogleTest)

file(GLOB TEST_SOURCES "*.cpp")
add_executable(ionFlowTests ${TEST_SOURCES})
target_link_libraries(ionFlowTests PRIVATE gtest_main ionFlowLib)
target_include_directories(ionFlowTests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../src/include)
gtest_discover_tests(ionFlowTests)

include(CTest)
add_test(NAME IonFlowTest COMMAND ionFlowTests)
set_tests_properties(IonFlowTest PROPERTIES LABELS "unit")

