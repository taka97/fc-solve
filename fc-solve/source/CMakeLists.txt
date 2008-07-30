include(CheckFunctionExists)

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

