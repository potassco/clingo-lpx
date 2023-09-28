# [[[source: .
set(ide_source_group "Source Files")
set(source-group
    "${CMAKE_CURRENT_SOURCE_DIR}/_clingolpx.c")
source_group("${ide_source_group}" FILES ${source-group})
set(source
    ${source-group})
# ]]]

if (NOT PYCLINGOLPX_INSTALL_DIR AND Python_EXECUTABLE)
    if (PYCLINGOLPX_INSTALL STREQUAL "user")
        execute_process(COMMAND ${Python_EXECUTABLE} ${CLINGOLPX_SOURCE_DIR}/cmake/python-site.py target --user OUTPUT_VARIABLE PYCLINGOLPX_INSTALL_DIR OUTPUT_STRIP_TRAILING_WHITESPACE)
    elseif(PYCLINGOLPX_INSTALL STREQUAL "prefix")
        execute_process(COMMAND ${Python_EXECUTABLE} ${CLINGOLPX_SOURCE_DIR}/cmake/python-site.py target --prefix "${CMAKE_INSTALL_PREFIX}" OUTPUT_VARIABLE PYCLINGOLPX_INSTALL_DIR OUTPUT_STRIP_TRAILING_WHITESPACE)
    else()
        execute_process(COMMAND ${Python_EXECUTABLE} ${CLINGOLPX_SOURCE_DIR}/cmake/python-site.py target OUTPUT_VARIABLE PYCLINGOLPX_INSTALL_DIR OUTPUT_STRIP_TRAILING_WHITESPACE)
    endif()
endif()

if (NOT PYCLINGOLPX_SUFFIX)
    if (Python_EXECUTABLE)
        execute_process(COMMAND ${Python_EXECUTABLE} ${CLINGOLPX_SOURCE_DIR}/cmake/python-site.py suffix OUTPUT_VARIABLE PYCLINGOLPX_SUFFIX OUTPUT_STRIP_TRAILING_WHITESPACE)
    else()
        if (CYGWIN)
            set(PYCLINGOLPX_SUFFIX ".dll")
        elseif (UNIX)
            set(PYCLINGOLPX_SUFFIX ".so")
        else()
            set(PYCLINGOLPX_SUFFIX ".pyd")
        endif()
    endif()
endif()

# Note: For the pip package, we actually not just want to load this library but
# also link agaist it. This is mainly for Windows to ensure that the import
# library is installed.
if (PYCLINGOLPX_ENABLE STREQUAL "pip")
    add_library(pyclingo-lpx SHARED ${source})
else()
    add_library(pyclingo-lpx MODULE ${source})
endif()
target_link_libraries(pyclingo-lpx PRIVATE libclingo-lpx Python::Module)
target_compile_definitions(pyclingo-lpx PRIVATE _CFFI_NO_LIMITED_API)
set_target_properties(pyclingo-lpx PROPERTIES
    FOLDER lib
    OUTPUT_NAME _clingolpx
    SUFFIX ${PYCLINGOLPX_SUFFIX}
    PREFIX "")
if (CMAKE_RUNTIME_OUTPUT_DIRECTORY)
    set_target_properties(pyclingo-lpx PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/python)
endif()
if (CMAKE_LIBRARY_OUTPUT_DIRECTORY)
    set_target_properties(pyclingo-lpx PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/python)
endif()
if (CMAKE_ARCHIVE_OUTPUT_DIRECTORY)
    set_target_properties(pyclingo-lpx PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}/python)
endif()

if (PYCLINGOLPX_INSTALL_DIR)
    file(TO_CMAKE_PATH "${PYCLINGOLPX_INSTALL_DIR}" _PYCLINGOLPX_INSTALL_DIR)
    install(TARGETS pyclingo-lpx
        RUNTIME DESTINATION ${_PYCLINGOLPX_INSTALL_DIR}/clingolpx
        LIBRARY DESTINATION ${_PYCLINGOLPX_INSTALL_DIR}/clingolpx
        ARCHIVE DESTINATION ${_PYCLINGOLPX_INSTALL_DIR}/clingolpx)
    install(DIRECTORY ${CMAKE_SOURCE_DIR}/libpyclingo-lpx/clingolpx/
        DESTINATION ${_PYCLINGOLPX_INSTALL_DIR}/clingolpx
        FILES_MATCHING
        PATTERN "*.py"
        PATTERN "*.typed"
        PATTERN "tests" EXCLUDE)
    if (NOT PYCLINGOLPX_ENABLE STREQUAL "pip")
        file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/INSTALLER "pip\n")
        file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/METADATA "Metadata-Version: 1.1\nName: clingo-lpx\nVersion: ${CLINGOLPX_VERSION}\n")
        install(FILES
            ${CMAKE_CURRENT_BINARY_DIR}/INSTALLER
            ${CMAKE_CURRENT_BINARY_DIR}/METADATA
            DESTINATION ${_PYCLINGOLPX_INSTALL_DIR}/clingolpx-${CLINGOLPX_VERSION}.dist-info)
    else()
        install(FILES ${CMAKE_SOURCE_DIR}/libclingo-lpx/clingo-lpx.h
            DESTINATION ${_PYCLINGOLPX_INSTALL_DIR}/clingolpx)
    endif()
endif()
