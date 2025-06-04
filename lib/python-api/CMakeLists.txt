# [[[source: src
set(ide_source_group "Source Files")
set(source-group
    "${CMAKE_CURRENT_SOURCE_DIR}/src/clingolpx.cc")
source_group("${ide_source_group}" FILES ${source-group})
set(source
    ${source-group})
# ]]]

pybind11_add_module(pyclingo-lpx)
target_sources(pyclingo-lpx PRIVATE ${source})
clingolpx_target_properties(TARGETS pyclingo-lpx FOLDER "lib" SUBDIR "python")
set_target_properties(pyclingo-lpx PROPERTIES OUTPUT_NAME "clingolpx")
target_link_libraries(pyclingo-lpx PRIVATE clingo-lpx Python::Module)

if(CLINGOLPX_INSTALL_WHEEL)
    install(TARGETS pyclingo-lpx
        RUNTIME DESTINATION "."
        LIBRARY DESTINATION ".")
    if(CLINGOLPX_BUILD_SHARED)
        if(APPLE)
            set_target_properties(pyclingo-lpx
                PROPERTIES INSTALL_RPATH "@loader_path"
                BUILD_WITH_INSTALL_RPATH TRUE)
        elseif(UNIX)
            set_target_properties(pyclingo-lpx
                PROPERTIES INSTALL_RPATH "$ORIGIN"
                BUILD_WITH_INSTALL_RPATH TRUE)
        endif()
    endif()
elseif(CLINGOLPX_INSTALL_DEFAULT)
    if(PYCLINGOLPX_INSTALL_DIR STREQUAL "")
        if(PYCLINGOLPX_INSTALL STREQUAL "user")
            execute_process(
                COMMAND "${Python_EXECUTABLE}" "${CLINGOLPX_SOURCE_DIR}/cmake/python-site.py" target --user
                OUTPUT_VARIABLE _python_install_dir OUTPUT_STRIP_TRAILING_WHITESPACE)
        elseif(PYCLINGOLPX_INSTALL STREQUAL "prefix")
            execute_process(
                COMMAND "${Python_EXECUTABLE}" "${CLINGOLPX_SOURCE_DIR}/cmake/python-site.py" target --prefix "${CMAKE_INSTALL_PREFIX}"
                OUTPUT_VARIABLE _python_install_dir OUTPUT_STRIP_TRAILING_WHITESPACE)
        else()
            execute_process(
                COMMAND "${Python_EXECUTABLE}" "${CLINGOLPX_SOURCE_DIR}/cmake/python-site.py" target
                OUTPUT_VARIABLE _python_install_dir OUTPUT_STRIP_TRAILING_WHITESPACE)
        endif()
        file(TO_CMAKE_PATH "${_python_install_dir}" python_install_dir)
    else()
        set(python_install_dir "${PYCLINGOLPX_INSTALL_DIR}")
    endif()
    message(STATUS "Python module path: ${python_install_dir}")
    file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/INSTALLER" "pip\n")
    file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/METADATA" "Metadata-Version: 1.1\nName: clingo\nVersion: ${CLINGOLPX_VERSION}\n")
    install(
        FILES "${CMAKE_CURRENT_BINARY_DIR}/INSTALLER" "${CMAKE_CURRENT_BINARY_DIR}/METADATA"
        DESTINATION "${python_install_dir}/clingolpx-${CLINGOLPX_VERSION}.dist-info")
    install(
        TARGETS pyclingo-lpx
        RUNTIME DESTINATION "${python_install_dir}"
        LIBRARY DESTINATION "${python_install_dir}")
endif()

if(CLINGOLPX_BUILD_TESTS)
    if(WIN32)
        # NOTE: python from conda-forge with more than one element in the path fails for whatever reason
        set(python_subdir "${CMAKE_BINARY_DIR}/bin/python/")
        get_property(is_multi_config GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
        if(is_multi_config)
            set(python_subdir "${python_subdir}$<CONFIG>/")
        endif()
        add_custom_command(
            TARGET pyclingo-lpx POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory "${python_subdir}"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:pyclingo-lpx>" "${python_subdir}"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:clingo::pyclingo>" "${python_subdir}")
        pytest_discover_tests(
            test_pyclingo
            PYTHON_PATH_PREPEND "${python_subdir}"
            WORKING_DIRECTORY "tests"
            DEPENDS pyclingo-lpx)
    else()
        pytest_discover_tests(
            test_pyclingo
            PYTHON_PATH_PREPEND "$<TARGET_FILE_DIR:pyclingo-lpx>" "$<TARGET_FILE_DIR:clingo::pyclingo>"
            LIBRARY_PATH_PREPEND "$<TARGET_FILE_DIR:clingo-lpx>" "$<TARGET_FILE_DIR:clingo::clingo>"
            WORKING_DIRECTORY "tests"
            DEPENDS pyclingo-lpx)
    endif()
endif()
