# [[[header: .
set(ide_header_group "Header Files")
set(header-group
    "${CMAKE_CURRENT_SOURCE_DIR}/clingo-lpx.h")
source_group("${ide_header_group}" FILES ${header-group})
set(header
    ${header-group})
# ]]]
# [[[source: src
set(ide_source_group "Source Files")
set(source-group
    "${CMAKE_CURRENT_SOURCE_DIR}/src/clingo-lpx.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/number.hh"
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

if (PYCLINGOLPX_ENABLE STREQUAL "pip")
    set(clingolpx_build_interface ON)
else()
    set(clingolpx_build_interface OFF)
endif()

set(clingolpx_public_scope_ PUBLIC)
set(clingolpx_private_scope_ PRIVATE)
if (CLINGOLPX_BUILD_SHARED)
    set(clingolpx_lib_type SHARED)
    if (clingolpx_build_interface)
        set(clingolpx_lib_type INTERFACE)
        set(clingolpx_public_scope_ INTERFACE)
        set(clingolpx_private_scope_ INTERFACE)
    endif()
else()
    set(clingolpx_lib_type STATIC)
endif()

add_library(libclingo-lpx ${clingolpx_lib_type})
target_sources(libclingo-lpx ${clingolpx_private_scope_} ${header} ${source})
target_include_directories(libclingo-lpx ${clingolpx_public_scope_} "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>")
target_compile_definitions(libclingo-lpx ${clingolpx_private_scope_} CLINGOLPX_BUILD_LIBRARY)
if (NOT CLINGOLPX_HAS_SHARED_MUTEX)
    target_compile_definitions(libclingo-lpx ${clingolpx_private_scope_} CLINGOLPX_NO_SHARED_MUTEX)
endif()

if (CLINGOLPX_PROFILE) 
    target_compile_definitions(libclingo-lpx ${clingolpx_public_scope_} "CLINGOLPX_PROFILE")
    target_link_libraries(libclingo-lpx ${clingolpx_public_scope_} Gperftools::Profiler)
endif()
if (CLINGOLPX_CROSSCHECK)
    target_compile_definitions(libclingo-lpx ${clingolpx_public_scope_} "CLINGOLPX_CROSSCHECK")
endif()

if (NOT CLINGOLPX_BUILD_STATIC AND CLINGOLPX_BUILD_SHARED AND NOT clingolpx_build_interface)
    set_target_properties(libclingo-lpx PROPERTIES POSITION_INDEPENDENT_CODE ON)
endif()
target_link_libraries(libclingo-lpx ${clingolpx_public_scope_} libclingo MATH::MATH Threads::Threads)
if (NOT CLINGOLPX_BUILD_SHARED)
    target_compile_definitions(libclingo-lpx ${clingolpx_public_scope_}  CLINGOLPX_NO_VISIBILITY)
elseif(NOT clingolpx_build_interface)
    set_target_properties(libclingo-lpx PROPERTIES PUBLIC_HEADER clingo-lpx.h)
    install(TARGETS libclingo-lpx
        EXPORT ClingoLPXTargets
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

    install(EXPORT ClingoLPXTargets
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ClingoLPX)

    include(CMakePackageConfigHelpers)
    configure_package_config_file(
        ${PROJECT_SOURCE_DIR}/cmake/ClingoLPXConfig.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/ClingoLPXConfig.cmake
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ClingoLPX)
    write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/ClingoLPXConfigVersion.cmake
        COMPATIBILITY SameMajorVersion)
    install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/ClingoLPXConfig.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/ClingoLPXConfigVersion.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ClingoLPX)
endif()
if (NOT clingolpx_build_interface)
    set_target_properties(libclingo-lpx PROPERTIES
        OUTPUT_NAME clingo-lpx
        LIBRARY_OUTPUT_NAME clingo-lpx
        FOLDER lib)
endif()

if (CLINGOLPX_BUILD_TESTS)
    # NOTE: we build statically without hiding symbols
    add_library(libclingo-lpx_t STATIC ${header} ${source})
    target_include_directories(libclingo-lpx_t PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>" "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>")
    target_compile_definitions(libclingo-lpx_t PUBLIC CLINGOLPX_NO_VISIBILITY)
    target_link_libraries(libclingo-lpx_t PUBLIC libclingo MATH::MATH Threads::Threads)
    target_compile_definitions(libclingo-lpx_t PUBLIC "CLINGOLPX_CROSSCHECK")
    if (NOT CLINGOLPX_HAS_SHARED_MUTEX)
        target_compile_definitions(libclingo-lpx_t PUBLIC CLINGOLPX_NO_SHARED_MUTEX)
    endif()
    set_target_properties(libclingo-lpx_t PROPERTIES
        OUTPUT_NAME clingo-lpx_t
        LIBRARY_OUTPUT_NAME clingo-lpx_t
        FOLDER lib)
    add_subdirectory(tests)
endif()
