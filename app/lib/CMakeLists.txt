# [[[header: clingo-lpx-app
set(ide_header_group "Header Files")
set(header-group-clingo-lpx-app
    "${CMAKE_CURRENT_SOURCE_DIR}/clingo-lpx-app/app.hh")
source_group("${ide_header_group}\\clingo-lpx-app" FILES ${header-group-clingo-lpx-app})
set(header
    ${header-group-clingo-lpx-app})
# ]]]
# [[[source: src
set(ide_source_group "Source Files")
set(source-group
    "${CMAKE_CURRENT_SOURCE_DIR}/src/app.cc")
source_group("${ide_source_group}" FILES ${source-group})
set(source
    ${source-group})
# ]]]

add_library(libclingo-lpx-app STATIC)
target_sources(libclingo-lpx-app PRIVATE ${header} ${source})
target_include_directories(libclingo-lpx-app PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>")

if (NOT CLINGOLPX_BUILD_STATIC AND CLINGOLPX_BUILD_SHARED AND NOT clingolpx_build_interface)
    set_target_properties(libclingo-lpx-app PROPERTIES POSITION_INDEPENDENT_CODE ON)
endif()
target_link_libraries(libclingo-lpx-app PUBLIC libclingo-lpx)

set_target_properties(libclingo-lpx-app PROPERTIES
    OUTPUT_NAME clingo-lpx-app
    LIBRARY_OUTPUT_NAME clingo-lpx-app
    FOLDER lib)

if (CLINGOLPX_BUILD_TESTS)
    add_library(libclingo-lpx-app_t STATIC ${header} ${source})
    target_include_directories(libclingo-lpx-app_t PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>")
    target_link_libraries(libclingo-lpx-app_t PUBLIC libclingo-lpx_t)
    set_target_properties(libclingo-lpx-app_t PROPERTIES
        OUTPUT_NAME clingo-lpx-app_t
        LIBRARY_OUTPUT_NAME clingo-lpx-app_t
        FOLDER lib)
endif()
