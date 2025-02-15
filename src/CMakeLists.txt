add_library(ionFlowLib STATIC)

file(GLOB ION_SOURCES "*.cpp")
target_sources(ionFlowLib PRIVATE ${ION_SOURCES})
target_include_directories(
    ionFlowLib PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                      $<INSTALL_INTERFACE:include/ionflow>)
