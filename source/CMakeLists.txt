CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

SET (_docfiles "AUTHORS.xml" "USAGE.xml" )

SET (_pdf_targets)

FOREACH(_file ${_docfiles})
    GET_FILENAME_COMPONENT(_file_wo_ext ${_file} NAME_WE)

    SET (_in "${CMAKE_CURRENT_SOURCE_DIR}/${_file}")
    SET (_pdf "${CMAKE_CURRENT_BINARY_DIR}/${_file_wo_ext}.pdf")

    SET (_target "${_file_wo_ext}_pdf")

    ADD_CUSTOM_COMMAND(
        OUTPUT "${_pdf}"
        COMMAND cp -f "${_in}" "${_pdf}"
        DEPENDS "${_in}"
        COMMENT "docmake to PDF"
    )

    ADD_CUSTOM_TARGET("${_target}" echo -n
        DEPENDS "${_pdf}"
    )

    LIST(APPEND _pdf_targets "${_target}")

ENDFOREACH(_file)

ADD_CUSTOM_TARGET("pdfs" echo -n)

ADD_DEPENDENCIES("pdfs" ${_pdf_targets})

