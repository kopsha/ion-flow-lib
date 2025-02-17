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

if (ENABLE_COVERAGE)
    target_link_options(ionFlowTests PRIVATE -coverage)
    find_program(GCOVR gcovr)
    if(GCOVR)
        add_custom_target(coverage
            COMMAND ${GCOVR}
                -r ${CMAKE_SOURCE_DIR}
                --html --html-details
                -o ${CMAKE_BINARY_DIR}/coverage/index.html
                --exclude '.*gtest.*'
                --exclude '/usr/include/.*'
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMENT "Generating HTML coverage report..."
        )

    else()
        message(FATAL_ERROR "gcovr installation not found.")
    endif()
endif()

