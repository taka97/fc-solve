SUBDIRS( presets )

CONFIGURE_FILE(
    ${CMAKE_CURRENT_SOURCE_DIR}/presetrc.proto.in
    ${CMAKE_CURRENT_SOURCE_DIR}/presetrc.proto
)

