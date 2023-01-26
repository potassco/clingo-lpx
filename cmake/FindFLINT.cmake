set(gmp_library_search_hints)
set(flint_include_search_hints)
set(flint_library_search_hints)

if(DEFINED GMP_DIR)
    list(APPEND gmp_library_search_hints ${GMP_DIR})
    list(APPEND gmp_library_search_hints ${GMP_DIR}/lib)
endif()

if(DEFINED ENV{GMP_DIR})
    list(APPEND gmp_library_search_hints $ENV{FLINT_DIR})
    list(APPEND gmp_library_search_hints $ENV{FLINT_DIR}/lib)
endif()

if(DEFINED FLINT_DIR)
    list(APPEND flint_library_search_hints ${FLINT_DIR})
    list(APPEND flint_library_search_hints ${FLINT_DIR}/lib)
    list(APPEND flint_include_search_hints ${FLINT_DIR})
    list(APPEND flint_include_search_hints ${FLINT_DIR}/include)
endif()

if(DEFINED ENV{FLINT_DIR})
    list(APPEND flint_library_search_hints $ENV{FLINT_DIR})
    list(APPEND flint_library_search_hints $ENV{FLINT_DIR}/lib)
    list(APPEND flint_include_search_hints $ENV{FLINT_DIR})
    list(APPEND flint_include_search_hints $ENV{FLINT_DIR}/include)
endif()

find_library(
    GMP_LIBRARY 
    NAMES gmp
    HINTS ${gmp_library_search_hints})

find_path(
    FLINT_INCLUDE 
    NAMES flint/flint.h flint.h
    HINTS ${flint_include_search_hints})

find_library(
    FLINT_LIBRARY 
    NAMES flint 
    HINTS ${flint_library_search_hints})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(FLINT DEFAULT_MSG FLINT_LIBRARY GMP_LIBRARY FLINT_INCLUDE)
mark_as_advanced(GMP_LIBRARY FLINT_INCLUDE FLINT_LIBRARY)

set(FLINT_LIBRARIES ${FLINT_LIBRARY} ${GMP_LIBRARY})
set(FLINT_INCLUDE_DIRS ${FLINT_INCLUDE})

add_library(FLINT::FLINT INTERFACE IMPORTED)
set_property(TARGET FLINT::FLINT PROPERTY INTERFACE_INCLUDE_DIRECTORIES "${FLINT_INCLUDE_DIRS}")
set_property(TARGET FLINT::FLINT PROPERTY INTERFACE_LINK_LIBRARIES "${FLINT_LIBRARIES}")
