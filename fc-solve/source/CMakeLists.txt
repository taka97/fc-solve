# This is the equivalent to perform a "make dist"/"make distdir" etc.
SET(CPACK_PACKAGE_NAME "freecell-solver")
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Freecell Solver")
SET(CPACK_PACKAGE_VENDOR "Shlomi Fish")
SET(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README")
SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/COPYING")
SET(CPACK_PACKAGE_VERSION_MAJOR "2")
SET(CPACK_PACKAGE_VERSION_MINOR "11")
SET(CPACK_PACKAGE_VERSION_PATCH "3")
SET(CPACK_PACKAGE_INSTALL_DIRECTORY "CMake ${CMake_VERSION_MAJOR}.${CMake_VERSION_MINOR}")

SET(CPACK_SOURCE_IGNORE_FILES
    "freecell-solver-.*\\\\.tar\\\\.(gz|bz2|Z|lzma)$"
    "/_CPack_Packages/"
    "^freecell-solver-.*Source$"
    "\\\\.o$"
    "~$"
    "^board_gen/(pi-make-microsoft-freecell-board|make-microsoft-freecell-board|make-gnome-freecell-board|make-aisleriot-freecell-board)$"
    "^fc-solve$"
    "\\\\.so$"
    "/.svn/"
)


IF(WIN32 AND NOT UNIX)
  # There is a bug in NSI that does not handle full unix paths properly. Make
  # sure there is at least one set of four (4) backlasshes.
  SET(CPACK_PACKAGE_ICON "${CMake_SOURCE_DIR}/Utilities/Release\\\\InstallIcon.bmp")
  SET(CPACK_NSIS_INSTALLED_ICON_NAME "bin\\\\MyExecutable.exe")
  SET(CPACK_NSIS_DISPLAY_NAME "${CPACK_PACKAGE_INSTALL_DIRECTORY} My Famous Project")
  SET(CPACK_NSIS_HELP_LINK "http:\\\\\\\\fc-solve.berlios.de")
  SET(CPACK_NSIS_URL_INFO_ABOUT "http:\\\\\\\\fc-solve.berlios.de")
  SET(CPACK_NSIS_CONTACT "shlomif@iglu.org.il")
  SET(CPACK_NSIS_MODIFY_PATH ON)
ELSE(WIN32 AND NOT UNIX)
  SET(CPACK_STRIP_FILES "fc-solve")
  SET(CPACK_SOURCE_STRIP_FILES "")
ENDIF(WIN32 AND NOT UNIX)

SET(CPACK_PACKAGE_EXECUTABLES 
    "fc-solve" "Single-Game Solver"
)

INCLUDE(CPack)

include(CheckFunctionExists)

SUBDIRS(
    Presets
)

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

ADD_LIBRARY (freecell-solver
    SHARED
    alloc.c app_str.c caas.c card.c cl_chop.c cmd_line.c fcs_dm.c fcs_hash.c fcs_isa.c freecell.c intrface.c lib.c lookup2.c move.c pqueue.c preset.c rand.c scans.c simpsim.c state.c
    )

TARGET_LINK_LIBRARIES (freecell-solver m)

ADD_EXECUTABLE (fc-solve
    main.c
)

CHECK_FUNCTION_EXISTS(pow HAVE_POW)

TARGET_LINK_LIBRARIES (fc-solve freecell-solver)

SET (INDIRECT_STACK_STATES 1)

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake 
    ${CMAKE_CURRENT_SOURCE_DIR}/config.h)

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/prefix.h.cmake 
    ${CMAKE_CURRENT_SOURCE_DIR}/prefix.h)

INSTALL(
    TARGETS "fc-solve"
    DESTINATION "bin"
)

INSTALL(
    TARGETS "freecell-solver"
    DESTINATION "lib"
)
