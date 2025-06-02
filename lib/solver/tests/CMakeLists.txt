# [[[source: .
set(ide_source_group "Source Files")
set(source-group
    "${CMAKE_CURRENT_SOURCE_DIR}/number.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/parsing.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/solving.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/tableau.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/util.cc")
source_group("${ide_source_group}" FILES ${source-group})
set(source
    ${source-group})
# ]]]

add_executable(test_clingo-lpx ${source})
target_link_libraries(test_clingo-lpx PRIVATE libclingo-lpx-app_t Catch2::Catch2WithMain)
target_include_directories(test_clingo-lpx PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)
set_target_properties(test_clingo-lpx PROPERTIES FOLDER test)
if(MSVC)
    target_compile_definitions(test_clingo-lpx PRIVATE _SCL_SECURE_NO_WARNINGS)
endif()

add_test(NAME test_clingo-lpx COMMAND test_clingo-lpx)
