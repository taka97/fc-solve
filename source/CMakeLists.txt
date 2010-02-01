INCLUDE( "${CMAKE_CURRENT_SOURCE_DIR}/Common.cmake" )

LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# This is the equivalent to perform a "make dist"/"make distdir" etc.
SET(CPACK_PACKAGE_NAME "freecell-solver")

# Process and extract the version number.
FILE( READ "ver.txt" VERSION)

CHOMP (VERSION)

STRING (REGEX MATCHALL "([0-9]+)" VERSION_DIGITS "${VERSION}")

LIST(GET VERSION_DIGITS 0 CPACK_PACKAGE_VERSION_MAJOR)
LIST(GET VERSION_DIGITS 1 CPACK_PACKAGE_VERSION_MINOR)
LIST(GET VERSION_DIGITS 2 CPACK_PACKAGE_VERSION_PATCH)

SET(CPACK_PACKAGE_INSTALL_DIRECTORY "${CPACK_PACKAGE_DESCRIPTION_SUMMARY} ${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")

SET(CPACK_SOURCE_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")

IF(NOT (WIN32 AND NOT UNIX))
  SET(CPACK_STRIP_FILES "fc-solve")
  SET(CPACK_SOURCE_STRIP_FILES "")
ENDIF(NOT (WIN32 AND NOT UNIX))

SET(CPACK_PACKAGE_EXECUTABLES
    "fc-solve" "Single-Game Solver"
)

### This is to set the RPATH correctly, so when installed under a prefix
### the executables will find the libraries.
### 
### See:
###
### http://www.cmake.org/Wiki/CMake_RPATH_handling
###
### (Taken from that wiki page)

# use, i.e. don't skip the full RPATH for the build tree
SET(CMAKE_SKIP_BUILD_RPATH  FALSE)

# when building, don't use the install RPATH already
# (but later on when installing)
SET(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE) 

# the RPATH to be used when installing
SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")

# add the automatically determined parts of the RPATH
# which point to directories outside the build tree to the install RPATH
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

INCLUDE(CPack)

include(CheckFunctionExists)
INCLUDE(CheckCCompilerFlag)
INCLUDE(FindThreads)

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

SET(COMPILER_FLAGS_TO_CHECK 
    "-Wall" "-Werror=implicit-function-declaration" "-fvisibility=hidden"
)

IF (CPU_ARCH)
    LIST(APPEND COMPILER_FLAGS_TO_CHECK "-march=${CPU_ARCH}")
ENDIF(CPU_ARCH)

IF (OPTIMIZATION_OMIT_FRAME_POINTER)
    LIST(APPEND COMPILER_FLAGS_TO_CHECK "-fomit-frame-pointer")
ENDIF(OPTIMIZATION_OMIT_FRAME_POINTER)

IF (CMAKE_BUILD_TYPE STREQUAL debug)
    LIST(APPEND COMPILER_FLAGS_TO_CHECK "-DFCS_COMPILE_DEBUG_FUNCTIONS=1")
ENDIF (CMAKE_BUILD_TYPE STREQUAL debug)

# So it can find config.h
INCLUDE_DIRECTORIES(BEFORE ${CMAKE_CURRENT_BINARY_DIR})

INCLUDE (CheckTypeSize)
CHECK_TYPE_SIZE("int" INT_SIZE_IN_BYTES)

MATH(EXPR INT_SIZE_IN_BITS "8 * ${INT_SIZE_IN_BYTES}")

SET(TEMP_SIZE 1)
SET(FCS_INT_BIT_SIZE_LOG2 0)

WHILE ( NOT ("${TEMP_SIZE}" STREQUAL "${INT_SIZE_IN_BITS}") )

    MATH(EXPR TEMP2 "${TEMP_SIZE} << 1")
    SET(TEMP_SIZE "${TEMP2}")

    MATH(EXPR TEMP2 "${FCS_INT_BIT_SIZE_LOG2} + 1")
    SET(FCS_INT_BIT_SIZE_LOG2 "${TEMP2}")

ENDWHILE ( NOT ("${TEMP_SIZE}" STREQUAL "${INT_SIZE_IN_BITS}") )

FIND_PACKAGE(Asciidoc)

SET (_docfiles "AUTHORS.txt" "COPYING.txt" "HACKING.txt" "INSTALL.txt" 
    "NEWS.txt" "README.txt" "TODO.txt" "USAGE.txt" 
)

SET (docs_to_install)
FOREACH(_file ${_docfiles})
    GET_FILENAME_COMPONENT(_file_we ${_file} NAME_WE)
    SET(_full_file "${CMAKE_CURRENT_BINARY_DIR}/${_file_we}")
    LIST(APPEND docs_to_install ${_full_file})
ENDFOREACH(_file)

IF(ASCIIDOC_FOUND)

    SET (_pdf_targets)

    FOREACH(_file ${_docfiles})
        GET_FILENAME_COMPONENT(_file_we ${_file} NAME_WE)
        SET(_in "${_file_we}")
        SET(_html_out "${CMAKE_CURRENT_SOURCE_DIR}/${_file_we}.html")
        SET(_full_file "${CMAKE_CURRENT_SOURCE_DIR}/${_file}")
        SET(_docbook_out "${CMAKE_CURRENT_SOURCE_DIR}/${_file_we}.xml")
        # MESSAGE(STATUS "_in == <<${_in}>> _html_out == <<${_html_out}>>")

        # -a toc 
        ADD_CUSTOM_COMMAND(
            OUTPUT "${_html_out}"
            COMMAND ${ASCIIDOC_EXECUTABLE} 
                -o ${_html_out} "${_full_file}"
            DEPENDS "${_full_file}"
            COMMENT "Asciidoc ${_in}"
            )


        ADD_CUSTOM_COMMAND(
            OUTPUT "${_docbook_out}"
            COMMAND ${ASCIIDOC_EXECUTABLE} 
                --backend=docbook -o ${_docbook_out} "${_full_file}"
            DEPENDS "${_full_file}"
            COMMENT "Asciidoc->DocBook ${_in}"
            )

        SET (_pdf "${_file_we}.pdf")

        SET (target "${_file_we}_build")

        ADD_CUSTOM_COMMAND(
            OUTPUT "${_pdf}"
            COMMAND docmake -v --make -o ${_pdf} pdf ${_docbook_out}
            DEPENDS ${_docbook_out}
            COMMENT "docmake to PDF"
        )

        ADD_CUSTOM_COMMAND(
            OUTPUT ${_file_we}
            COMMAND cp -f "${_full_file}" ${_file_we}
            DEPENDS "${_file_we}.txt"
            COMMENT "copy AsciiDoc to non-\".txt\" file"
        )
    
        ADD_CUSTOM_TARGET(${target} ALL echo -n
            DEPENDS "${_html_out}" "${_docbook_out}" "${_file_we}"
        )

        ADD_CUSTOM_TARGET(${_in}_pdf echo -n
            DEPENDS "${_pdf}"
        )

        LIST(APPEND _pdf_targets ${_in}_pdf)

    ENDFOREACH(_file)

    # MESSAGE ("PDF_TARGETS == ${_pdf_targets}")
    ADD_CUSTOM_TARGET("pdfs" echo -n
        DEPENDS ${_pdf_targets}
    )

ENDIF(ASCIIDOC_FOUND)

