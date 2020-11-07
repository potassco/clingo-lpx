cmake_minimum_required(VERSION 3.18)

project(CLINGOLP VERSION 1.0 LANGUAGES CXX)

list(APPEND CMAKE_MODULE_PATH "cmake")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

find_package(Clingo REQUIRED)
find_package(GMPXX REQUIRED)

# [[[source: src
set(ide_source_group "Source Files")
set(source-group
    "${CMAKE_CURRENT_SOURCE_DIR}/src/catch.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/solver.cc")
source_group("${ide_source_group}" FILES ${source-group})
set(source
    ${source-group})
# ]]]

enable_testing()

add_executable(test_clingolp ${source})
target_link_libraries(test_clingolp PRIVATE libclingo GMPXX::GMPXX)
target_include_directories(test_clingolp PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)
set_target_properties(test_clingolp PROPERTIES FOLDER test)

add_test(NAME test_clingolp COMMAND test_clingolp)
