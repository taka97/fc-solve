ADD_EXECUTABLE (
    make-gnome-freecell-board
    make_gnome_freecell_board.c
)

ADD_EXECUTABLE(
    make-aisleriot-freecell-board
    make_aisleriot_freecell_board.c
)

ADD_EXECUTABLE(
    make-microsoft-freecell-board
    make_microsoft_freecell_board.c
)

ADD_EXECUTABLE(
    pi-make-microsoft-freecell-board
    pi_make_microsoft_freecell_board.c
)

ADD_CUSTOM_COMMAND(
    OUTPUT fc-solve-board_gen.6
    COMMAND cat $< | sed 's/FCS_VERSION/@VERSION@/g' > fc-solve.pod 
    COMMAND pod2man --section=6 --center="board_gen" --date="2001/07/07" --release="board_gen @VERSION@" fc-solve.pod > $@ 
    COMMAND rm -f fc-solve.pod
    DEPENDS fc-solve-board_gen.pod ../ver.txt
)
