if (CLINGOLPX_USE_IMATH)
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

if (CLINGOLPX_BUILD_TESTS)
    cmake_policy(SET CMP0077 NEW)
    set(CATCH_BUILD_STATIC_LIBRARY On)
    add_subdirectory(catch)
endif()
