function(clingolpx_target_properties)
    set(options)
    set(single_values FOLDER TYPE SUBDIR)
    set(multi_values TARGETS)
    cmake_parse_arguments(clingolpx "${options}" "${single_values}" "${multi_values}" ${ARGV})

    set(binary_subdir "bin")
    set(library_subdir "lib")
    if(clingolpx_SUBDIR)
        set(binary_subdir "bin/${clingolpx_SUBDIR}")
        set(library_subdir "lib/${clingolpx_SUBDIR}")
    endif()

    get_property(is_multi_config GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
    if(is_multi_config)
        set(binary_subdir "${binary_subdir}/$<CONFIG>")
        set(library_subdir "${library_subdir}/$<CONFIG>")
    endif()

    message(STATUS "folder: ${clingolpx_TARGETS}, targets: ${clingolpx_TARGETS}")

    if(clingolpx_FOLDER)
        set_target_properties(${clingolpx_TARGETS} PROPERTIES
            FOLDER "${clingolpx_FOLDER}"
            POSITION_INDEPENDENT_CODE ON
            RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${binary_subdir}"
            LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${binary_subdir}"
            ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${library_subdir}"
            PDB_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${binary_subdir}")
    endif()

    if(clingolpx_TYPE STREQUAL "extra" AND CLINGOLPX_INSTALL_EXTRA)
        install(
            TARGETS ${clingolpx_TARGETS}
            EXPORT clingo-lpx-targets
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
            INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        )
    elseif((clingolpx_TYPE STREQUAL "default" OR clingolpx_TYPE STREQUAL "binary") AND CLINGOLPX_INSTALL_DEFAULT)
        install(
            TARGETS ${clingolpx_TARGETS}
            EXPORT clingo-lpx-targets
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
            INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        )
    endif()
endfunction()
