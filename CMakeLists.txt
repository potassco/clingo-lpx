cmake_minimum_required(VERSION 3.22.1)

file(STRINGS "lib/solver/include/clingo-lpx.h" CLINGOLPX_VERSION_LINE REGEX "#define CLINGOLPX_VERSION ")
string(REGEX REPLACE "#define CLINGOLPX_VERSION[ \t]+\"([^\"]+)\"" "\\1" CLINGOLPX_VERSION "${CLINGOLPX_VERSION_LINE}")

project(CLINGOLPX VERSION "${CLINGOLPX_VERSION}" LANGUAGES C CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_C_STANDARD_REQUIRED OFF)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED OFF)
set(CMAKE_CXX_EXTENSIONS OFF)

include(GNUInstallDirs)
include(CMakeDependentOption)
include(macros)

option(CLINGOLPX_BUILD_APP "build the app" ON)
option(CLINGOLPX_BUILD_TESTS "build tests" OFF)
option(CLINGOLPX_CROSSCHECK "enable additional state checking" OFF)
option(CLINGOLPX_USE_LOCAL_CATCH "use bundled catch." ON)
option(CLINGOLPX_PROFILE "enable profiling using gperftools" OFF)
option(CLINGOLPX_MANAGE_RPATH "manage rpath when installing" ON)

cmake_dependent_option(CLINGOLPX_BUILD_SHARED "build clingo-lpx shared library" ON "NOT EMSCRIPTEN" OFF)
cmake_dependent_option(CLINGOLPX_USE_VERSION_SCRIPT "use a version script to hide C++ symbols" ON "CLINGOLPX_BUILD_SHARED" OFF)
cmake_dependent_option(CLINGOLPX_USE_VISIBILITY "enable visibility attributes" ON "CLINGOLPX_BUILD_SHARED" OFF)

set(CLINGOLPX_MATH_LIBRARY "auto" CACHE STRING "math library: auto, imath, flint")

set(PYCLINGOLPX_INSTALL "prefix" CACHE STRING "where to install python module: user, system, prefix")
set(PYCLINGOLPX_INSTALL_DIR "" CACHE STRING "path where to install python module")

if (MSVC)
    set(VC_RELEASE_LINK_OPTIONS /LTCG)
    SET(CMAKE_EXE_LINKER_FLAGS_RELEASE    "${CMAKE_EXE_LINKER_FLAGS_RELEASE}    ${VC_RELEASE_LINK_OPTIONS}")
    SET(CMAKE_MODULE_LINKER_FLAGS_RELEASE "${CMAKE_MODULE_LINKER_FLAGS_RELEASE} ${VC_RELEASE_LINK_OPTIONS}")
    SET(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} ${VC_RELEASE_LINK_OPTIONS}")
    SET(CMAKE_STATIC_LINKER_FLAGS_RELEASE "${CMAKE_STATIC_LINKER_FLAGS_RELEASE} ${VC_RELEASE_LINK_OPTIONS}")
endif()

if(${CMAKE_VERSION} VERSION_LESS "3.26")
    set_property(GLOBAL PROPERTY USE_FOLDERS ON)
endif()

if (CLINGOLPX_USE_VISIBILITY)
    set(CMAKE_C_VISIBILITY_PRESET hidden)
    set(CMAKE_CXX_VISIBILITY_PRESET hidden)
    set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)
endif()

set(PYBIND11_NEWPYTHON ON)
set(PYBIND11_FINDPYTHON ON)
set(CLINGOLPX_INSTALL_DEFAULT OFF)
set(CLINGOLPX_INSTALL_EXTRA OFF)
set(CLINGOLPX_INSTALL_WHEEL OFF)

if (DEFINED SKBUILD)
    message(FATAL_ERROR "TODO: configuring for skbuild")
else()
    if(CLINGOLPX_MANAGE_RPATH)
        list(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${CMAKE_INSTALL_FULL_LIBDIR}" isSystemDir)
        if(isSystemDir EQUAL -1)
            set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_FULL_LIBDIR}")
        endif()
    endif()
    set(CLINGOLPX_INSTALL_DEFAULT ON)
    if (NOT CLINGOLPX_BUILD_SHARED)
        set(CLINGOLPX_INSTALL_EXTRA ON)
    endif()
    find_package(Python COMPONENTS Interpreter Development Development.Module)
    find_package(Clingo REQUIRED)
endif()

if (CLINGOLPX_BUILD_TESTS)
    enable_testing()
endif()

if (CLINGOLPX_PROFILE)
    find_package(Gperftools REQUIRED)
endif()

add_subdirectory(third_party)
add_subdirectory(lib libclingo-lpx)
if (CLINGOLPX_BUILD_APP)
    add_subdirectory(app)
endif()

if (CLINGOLPX_INSTALL_DEFAULT)
    configure_package_config_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/clingo-lpx-config.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/clingo-lpx-config.cmake"
        INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/clingo-lpx"
    )
    write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/clingo-lpx-config-version.cmake"
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY AnyNewerVersion
    )
    install(
        FILES
            "${CMAKE_CURRENT_BINARY_DIR}/clingo-lpx-config.cmake"
            "${CMAKE_CURRENT_BINARY_DIR}/clingo-lpx-config-version.cmake"
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/clingo-lpx"
    )
    install(EXPORT clingo-lpx-targets
        FILE clingo-lpx-targets.cmake
        NAMESPACE clingo-lpx::
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/clingo-lpx"
    )
endif()
