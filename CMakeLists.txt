cmake_minimum_required(VERSION 3.18)

project(CLINGOLP VERSION 1.0 LANGUAGES CXX)

list(APPEND CMAKE_MODULE_PATH "cmake")

option(CLINGOLP_PROFILE "Enable profiling using gperftools" OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

find_package(Clingo REQUIRED)
find_package(GMPXX REQUIRED)
if (CLINGOLP_PROFILE)
    find_package(Gperftools REQUIRED)
endif()

# [[[src: src
set(ide_src_group "Src Files")
set(src-group
    "${CMAKE_CURRENT_SOURCE_DIR}/src/parsing.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/parsing.hh"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/problem.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/problem.hh"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/solving.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/solving.hh"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/util.hh")
source_group("${ide_src_group}" FILES ${src-group})
set(src
    ${src-group})
# ]]]
# [[[app: app
set(ide_app_group "App Files")
set(app-group-app
    "${CMAKE_CURRENT_SOURCE_DIR}/app/main.cc")
source_group("${ide_app_group}\\app" FILES ${app-group-app})
set(app
    ${app-group-app})
# ]]]
# [[[test: test
set(ide_test_group "Test Files")
set(test-group-test
    "${CMAKE_CURRENT_SOURCE_DIR}/test/catch.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/test/main.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/test/parsing.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/test/solving.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/test/util.cc")
source_group("${ide_test_group}\\test" FILES ${test-group-test})
set(test
    ${test-group-test})
# ]]]

set(libs libclingo GMPXX::GMPXX)
set(defs)
if (CLINGOLP_PROFILE)
    LIST(APPEND libs Gperftools::Gperftools)
    LIST(APPEND defs CLINGOLP_PROFILE)
endif()

add_library(libclingolp STATIC ${src})
target_link_libraries(libclingolp PUBLIC ${libs})
target_include_directories(libclingolp PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
)
target_compile_definitions(libclingolp PUBLIC ${defs})
set_target_properties(libclingolp PROPERTIES FOLDER src)

add_executable(clingolp ${app})
target_link_libraries(clingolp PRIVATE libclingolp)
target_include_directories(clingolp PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/app>
)
set_target_properties(clingolp PROPERTIES FOLDER app)

enable_testing()
add_executable(test_clingolp ${test})
target_link_libraries(test_clingolp PRIVATE libclingolp)
target_include_directories(test_clingolp PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/test>
)
set_target_properties(test_clingolp PROPERTIES FOLDER test)

add_test(NAME test_clingolp COMMAND test_clingolp)
