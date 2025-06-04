# [[[header: include
set(ide_header_group "Header Files")
set(header-group-include-clingo-lpx
    "${CMAKE_CURRENT_SOURCE_DIR}/include/clingo-lpx/number.hh"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/clingo-lpx/number_flint.hh"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/clingo-lpx/number_imath.hh"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/clingo-lpx/parsing.hh"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/clingo-lpx/problem.hh"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/clingo-lpx/solving.hh"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/clingo-lpx/tableau.hh"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/clingo-lpx/util.hh")
source_group("${ide_header_group}\\include\\clingo-lpx" FILES ${header-group-include-clingo-lpx})
set(header
    ${header-group-include-clingo-lpx})
# ]]]
# [[[source: src
set(ide_source_group "Source Files")
set(source-group
    "${CMAKE_CURRENT_SOURCE_DIR}/src/parsing.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/problem.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/solving.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/tableau.cc")
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

add_library(clingo-lpx-solver STATIC)
clingolpx_target_properties(TARGETS clingo-lpx-solver FOLDER "lib" TYPE "extra")
target_sources(clingo-lpx-solver PRIVATE ${header} ${source})
target_include_directories(clingo-lpx-solver PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>")
target_link_libraries(clingo-lpx-solver PUBLIC clingo::clingo math::math)
if(CLINGOLPX_PROFILE)
    target_compile_definitions(clingo-lpx-solver PUBLIC CLINGOLPX_PROFILE)
    target_link_libraries(clingo-lpx-solver PUBLIC Gperftools::Profiler)
endif()

if(CLINGOLPX_BUILD_TESTS)
    add_library(clingo-lpx-solver_t STATIC)
    clingolpx_target_properties(TARGETS clingo-lpx-solver_t FOLDER "lib")
    target_sources(clingo-lpx-solver_t PRIVATE ${header} ${source})
    target_link_libraries(clingo-lpx-solver_t PUBLIC clingo::clingo math::math)
    target_include_directories(clingo-lpx-solver_t PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
    target_compile_definitions(clingo-lpx-solver_t PUBLIC CLINGOLPX_CROSSCHECK)

    add_executable(test_clingo-lpx-solver)
    clingolpx_target_properties(TARGETS test_clingo-lpx-solver FOLDER "test" SUBDIR "tests")
    target_sources(test_clingo-lpx-solver PRIVATE ${test})
    target_link_libraries(test_clingo-lpx-solver PRIVATE clingo-lpx-solver_t Catch2::Catch2WithMain)
    catch_discover_tests(test_clingo-lpx-solver DL_PATH "$<TARGET_FILE_DIR:clingo::clingo>")
endif()
