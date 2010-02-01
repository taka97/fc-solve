### This is to set the RPATH correctly, so when installed under a prefix
### the executables will find the libraries.
### 
### See:
###
### http://www.cmake.org/Wiki/CMake_RPATH_handling
###
### (Taken from that wiki page)

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

SET (_docfiles "AUTHORS.txt" "USAGE.txt" )

SET (_pdf_targets)

FOREACH(_file ${_docfiles})
    GET_FILENAME_COMPONENT(_file_we ${_file} NAME_WE)
    SET(_in "${_file_we}")
    SET(_full_file "${CMAKE_CURRENT_SOURCE_DIR}/${_file}")
    SET(_docbook_out "${CMAKE_CURRENT_SOURCE_DIR}/${_file_we}.xml")

    SET (_pdf "${_file_we}.pdf")

    SET (target "${_file_we}_build")

    ADD_CUSTOM_COMMAND(
        OUTPUT "${_pdf}"
        COMMAND docmake -v --make -o ${_pdf} pdf ${_docbook_out}
        DEPENDS ${_docbook_out}
        COMMENT "docmake to PDF"
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

# ENDIF(ASCIIDOC_FOUND)

