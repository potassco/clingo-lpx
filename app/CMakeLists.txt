# [[[source: src
set(ide_source_group "Source Files")
set(source-group
    "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cc")
source_group("${ide_source_group}" FILES ${source-group})
set(source
    ${source-group})
# ]]]

add_executable(clingo-lpx-app)
target_sources(clingo-lpx-app PRIVATE ${source})
clingolpx_target_properties(TARGETS clingo-lpx-app FOLDER exe TYPE default)
target_link_libraries(clingo-lpx-app PRIVATE clingo::clingo clingo-lpx)
set_target_properties(clingo-lpx-app PROPERTIES OUTPUT_NAME clingo-lpx)
