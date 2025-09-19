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
# [[[test: tests
set(ide_test_group "Test Files")
set(test-group-tests
    "${CMAKE_CURRENT_SOURCE_DIR}/tests/solve.cc")
source_group("${ide_test_group}\\tests" FILES ${test-group-tests})
set(test
    ${test-group-tests})
# ]]]

if(CLINGOLPX_BUILD_SHARED)
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
target_link_libraries(clingo-lpx PUBLIC clingo::clingo PRIVATE clingo-lpx-solver)
if(CLINGOLPX_USE_VISIBILITY)
    target_compile_definitions(clingo-lpx PRIVATE "$<BUILD_INTERFACE:CLINGOLPX_BUILD_LIBRARY>")
else()
    target_compile_definitions(clingo-lpx PUBLIC CLINGOLPX_NO_VISIBILITY)
endif()

if(CLINGOLPX_INSTALL_DEFAULT)
    install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR} FILES_MATCHING PATTERN "*.h")
elseif(CLINGOLPX_INSTALL_WHEEL AND CLINGOLPX_BUILD_SHARED)
    install(
        TARGETS clingo-lpx
        RUNTIME DESTINATION "."
        LIBRARY DESTINATION ".")
    if(CLINGOLPX_BUILD_SHARED)
        if(APPLE)
            set_target_properties(clingo-lpx PROPERTIES
                INSTALL_NAME_DIR "@loader_path")
        elseif(UNIX)
            set_target_properties(clingo-lpx
                PROPERTIES INSTALL_RPATH "$ORIGIN"
                BUILD_WITH_INSTALL_RPATH TRUE)
        endif()
    endif()
endif()

if(CLINGOLPX_BUILD_TESTS)
    if(WIN32)
        # NOTE: needed to get the test explorer working
        set(test_subdir "${CMAKE_BINARY_DIR}/bin/tests/")
        set(python_subdir "${CMAKE_BINARY_DIR}/bin/python/")
        get_property(is_multi_config GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
        if(is_multi_config)
            set(test_subdir "${test_subdir}$<CONFIG>/")
            set(python_subdir "${python_subdir}$<CONFIG>/")
        endif()
        if(CLINGOLPX_BUILD_SHARED)
            add_custom_command(
                TARGET clingo-lpx POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E make_directory "${python_subdir}"
                COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:clingo-lpx>" "${python_subdir}"
            )
        endif()
        add_custom_command(
            TARGET clingo-lpx POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory "${test_subdir}"
            COMMAND ${CMAKE_COMMAND} -E make_directory "${python_subdir}"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:clingo::clingo>" "${test_subdir}"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:clingo::clingo>" "${python_subdir}"
        )
    endif()

    add_library(clingo-lpx_t STATIC)
    clingolpx_target_properties(TARGETS clingo-lpx_t FOLDER "lib")
    target_sources(clingo-lpx_t PRIVATE ${header} ${source})
    target_link_libraries(clingo-lpx_t PUBLIC clingo-lpx-solver_t)
    target_include_directories(clingo-lpx_t PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>")
    target_compile_definitions(clingo-lpx_t PUBLIC CLINGOLPX_NO_VISIBILITY)

    add_executable(test_clingo-lpx)
    clingolpx_target_properties(TARGETS test_clingo-lpx FOLDER "test" SUBDIR "tests")
    target_sources(test_clingo-lpx PRIVATE ${test})
    target_link_libraries(test_clingo-lpx PRIVATE clingo-lpx_t Catch2::Catch2WithMain)
    catch_discover_tests(test_clingo-lpx DL_PATH "$<TARGET_FILE_DIR:clingo::clingo>")
endif()
