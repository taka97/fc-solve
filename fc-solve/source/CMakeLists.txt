ADD_LIBRARY (freecell-solver
    SHARED
    alloc.c app_str.c caas.c card.c cl_chop.c cmd_line.c fcs_dm.c fcs_hash.c fcs_isa.c freecell.c intrface.c lib.c lookup2.c move.c pqueue.c preset.c rand.c scans.c simpsim.c state.c
    )

ADD_EXECUTABLE (fc-solve
    main.c
)

TARGET_LINK_LIBRARIES (fc-solve freecell-solver)

