# [[[header: include
set(ide_header_group "Header Files")
set(header-group-include
    "${CMAKE_CURRENT_SOURCE_DIR}/include/clingo-lpx.h")
source_group("${ide_header_group}\\include" FILES ${header-group-include})
set(header
    ${header-group-include})
# ]]]
# [[[source: src
set(ide_source_group "Source Files")
set(source-group
    "${CMAKE_CURRENT_SOURCE_DIR}/src/clingo-lpx.cc")
source_group("${ide_source_group}" FILES ${source-group})
set(source
    ${source-group})
# ]]]

if (CLINGOLPX_BUILD_SHARED)
    set(lib_type SHARED)
else()
    set(lib_type STATIC)
endif()

add_library(clingo-lpx "${lib_type}")
clingolpx_target_properties(TARGETS clingo-lpx FOLDER "lib" TYPE default)
target_sources(clingo-lpx PRIVATE ${header} ${source})
target_include_directories(clingo-lpx PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>")
target_link_libraries(clingo-lpx PRIVATE clingo-lpx-solver)
if (CLINGOLPX_USE_VISIBILITY)
    target_compile_definitions(clingo-lpx PRIVATE "$<BUILD_INTERFACE:CLINGOLPX_BUILD_LIBRARY>")
else()
    target_compile_definitions(clingo-lpx PUBLIC CLINGOLPX_NO_VISIBILITY)
endif()
