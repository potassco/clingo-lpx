set(CMAKE_CXX_CLANG_TIDY "")

if (CLINGOLPX_BUILD_TESTS AND NOT TARGET Catch2::Catch2WithMain)
    if (CLINGOLPX_USE_LOCAL_CATCH)
        set(CATCH_BUILD_STATIC_LIBRARY ON CACHE INTERNAL "" FORCE)
        add_subdirectory(catch)
    else()
        find_package(Catch2 3.1 REQUIRED)
    endif()
endif()

if (_CLINGOLPX_MATH_LIBRARY STREQUAL "imath")
    add_library(libimath STATIC)
    target_sources(libimath PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/imath/imath.c
        ${CMAKE_CURRENT_SOURCE_DIR}/imath/imrat.c)
    target_include_directories(libimath PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/imath>")
    if (CLINGOLPX_BUILD_SHARED)
        set_target_properties(libimath PROPERTIES
            POSITION_INDEPENDENT_CODE ON)
    endif()
endif()
