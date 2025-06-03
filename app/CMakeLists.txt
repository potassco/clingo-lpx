# [[[source: src
set(ide_source_group "Source Files")
set(source-group
    "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cc")
source_group("${ide_source_group}" FILES ${source-group})
set(source
    ${source-group})
# ]]]

add_executable(clingo-lpx-app ${source})
target_link_libraries(clingo-lpx-app PRIVATE libclingo clingo-lpx)
set_target_properties(clingo-lpx-app PROPERTIES
    OUTPUT_NAME clingo-lpx
    FOLDER exe)

install(TARGETS clingo-lpx-app
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
