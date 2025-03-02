# Define the executable for this example
add_executable(ionExample)

file(GLOB EXAMPLE_SOURCES "*.cpp")
target_sources(ionExample PRIVATE ${EXAMPLE_SOURCES})

target_link_libraries(ionExample PRIVATE IonFlows)
target_include_directories(
    ionExample
    PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
           $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
           $<INSTALL_INTERFACE:include/ionflow>)

