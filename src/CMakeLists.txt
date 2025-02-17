add_library(ionFlowLib STATIC)

# lets keep this tidy
find_program(CLANG_TIDY_CLI "clang-tidy")
if(CLANG_TIDY_CLI)
    message(STATUS "Using clang-tidy at: ${CLANG_TIDY_CLI}.")
    set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_CLI}" --extra-arg=-std=c++20)
else()
    message(WARN ": Cannot find clang-tidy.")
endif()

set(LIB_VERSION $ENV{VERSION})
configure_file("${CMAKE_SOURCE_DIR}/src/version.h.in"
               "${PROJECT_BINARY_DIR}/include/version.h" @ONLY)

file(GLOB ION_SOURCES "*.cpp")
target_sources(ionFlowLib PRIVATE ${ION_SOURCES})
target_include_directories(
    ionFlowLib
    PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
           $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
           $<INSTALL_INTERFACE:include/ionflow>)

# Coverage settings
if(ENABLE_COVERAGE)
    message(STATUS "Building for coverage...")
    target_compile_options(
        ionFlowLib PRIVATE -O0 -fprofile-generate -fno-exceptions -fno-inline
                           -coverage)
    target_link_options(ionFlowLib PRIVATE -coverage)
endif()

# Install rules for the library
install(
    TARGETS ionFlowLib
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin)
install(DIRECTORY ${PROJECT_SOURCE_DIR}/src/include/
        DESTINATION include/ionflow)
install(DIRECTORY ${PROJECT_BINARY_DIR}/include/ DESTINATION include/ionflow)
