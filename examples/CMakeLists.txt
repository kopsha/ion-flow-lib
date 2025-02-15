# Define the executable for this example
add_executable(ionExample)

file(GLOB EXAMPLE_SOURCES "*.cpp")
target_sources(ionExample PRIVATE ${EXAMPLE_SOURCES})

target_link_libraries(ionExample PRIVATE ionFlowLib)
target_include_directories(ionExample PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../src/include)

