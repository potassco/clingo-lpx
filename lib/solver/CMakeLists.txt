# [[[header: include
set(ide_header_group "Header Files")
set(header)
# ]]]
# [[[source: src
set(ide_source_group "Source Files")
set(source-group
    "${CMAKE_CURRENT_SOURCE_DIR}/src/clingo-lpx.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/number.hh"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/number_flint.hh"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/number_imath.hh"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/parsing.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/parsing.hh"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/problem.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/problem.hh"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/solving.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/solving.hh"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/tableau.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/tableau.hh"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/util.hh")
source_group("${ide_source_group}" FILES ${source-group})
set(source
    ${source-group})
# ]]]
# [[[test: tests
set(ide_test_group "Test Files")
set(test-group-tests
    "${CMAKE_CURRENT_SOURCE_DIR}/tests/number.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/tests/parsing.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/tests/solving.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/tests/tableau.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/tests/util.cc")
source_group("${ide_test_group}\\tests" FILES ${test-group-tests})
set(test
    ${test-group-tests})
# ]]]

if (CLINGOLPX_BUILD_SHARED)
    set(lib_type SHARED)
else()
    set(lib_type STATIC)
endif()

add_library(clingo-lpx "${lib_type}")
clingolpx_target_properties(TARGETS clingo-lpx FOLDER "lib" TYPE default)
target_sources(clingo-lpx PRIVATE ${header} ${source})
target_include_directories(clingo-lpx PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>")
target_compile_definitions(clingo-lpx PRIVATE "$<BUILD_INTERFACE:CLINGOLPX_BUILD_LIBRARY>")
target_link_libraries(clingo-lpx PRIVATE libclingo math::math)
if (CLINGOLPX_USE_VISIBILITY)
    target_compile_definitions(clingo-lpx PRIVATE "$<BUILD_INTERFACE:CLINGOLPX_BUILD_LIBRARY>")
else()
    target_compile_definitions(clingo-lpx PUBLIC CLINGOLPX_NO_VISIBILITY)
endif()
if (CLINGOLPX_PROFILE)
    target_compile_definitions(clingo-lpx PUBLIC CLINGOLPX_PROFILE)
    target_link_libraries(clingo-lpx PUBLIC Gperftools::Profiler)
endif()

if (CLINGOLPX_BUILD_TESTS)
    add_library(clingo-lpx_t STATIC)
    clingolpx_target_properties(TARGETS clingo-lpx_t FOLDER "lib")
    target_sources(clingo-lpx_t PRIVATE ${header} ${source})
    target_link_libraries(clingo-lpx_t PUBLIC libclingo math::math)
    target_include_directories(clingo-lpx_t PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>")
    target_include_directories(clingo-lpx_t PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>")
    target_compile_definitions(clingo-lpx_t PUBLIC "$<BUILD_INTERFACE:CLINGOLPX_NO_VISIBILITY>")
    target_compile_definitions(clingo-lpx_t PUBLIC CLINGOLPX_CROSSCHECK)

    add_executable(test_solver)
    clingolpx_target_properties(TARGETS test_solver FOLDER "test" SUBDIR "tests")
    target_sources(test_solver PRIVATE ${test})
    target_link_libraries(test_solver PRIVATE clingo-lpx_t Catch2::Catch2WithMain)
    catch_discover_tests(test_solver DL_PATH "$<TARGET_FILE_DIR:libclingo>")
endif()
