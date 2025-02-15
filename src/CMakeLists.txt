add_library(ionFlowLib STATIC)

# lets keep this tidy
find_program(CLANG_TIDY_CLI "clang-tidy")
if(CLANG_TIDY_CLI)
    message(STATUS "Using clang-tidy at: ${CLANG_TIDY_CLI}.")
    set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_CLI}" --extra-arg=-std=c++20)
else()
    message(WARN ": Cannot find clang-tidy.")
endif()

file(GLOB ION_SOURCES "*.cpp")
target_sources(ionFlowLib PRIVATE ${ION_SOURCES})
target_include_directories(
    ionFlowLib PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                      $<INSTALL_INTERFACE:include/ionflow>)

# Install rules for the library
install(
    TARGETS ionFlowLib
    EXPORT ionFlowTargets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin)

# Install the library's headers
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/ DESTINATION include/ionflow)

