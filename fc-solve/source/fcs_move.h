/*
 * This file is part of Freecell Solver. It is subject to the license terms in
 * the COPYING.txt file found in the top-level directory of this distribution
 * and at http://fc-solve.shlomifish.org/docs/distro/COPYING.html . No part of
 * Freecell Solver, including this file, may be copied, modified, propagated,
 * or distributed except according to the terms contained in the COPYING file.
 *
 * Copyright (c) 2000 Shlomi Fish
 */
// fcs_move.h - header file for the move structure and enums of
// Freecell Solver. This file is common to the main code and to the
// library headers.

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

enum
{
    FCS_MOVE_TYPE_STACK_TO_STACK,
    FCS_MOVE_TYPE_STACK_TO_FREECELL,
    FCS_MOVE_TYPE_FREECELL_TO_STACK,
    FCS_MOVE_TYPE_FREECELL_TO_FREECELL,
    FCS_MOVE_TYPE_STACK_TO_FOUNDATION,
    FCS_MOVE_TYPE_FREECELL_TO_FOUNDATION,
    FCS_MOVE_TYPE_FLIP_CARD,
    FCS_MOVE_TYPE_DEAL_GYPSY_TALON,
    FCS_MOVE_TYPE_KLONDIKE_TALON_TO_STACK,
    FCS_MOVE_TYPE_KLONDIKE_FLIP_TALON,
    FCS_MOVE_TYPE_KLONDIKE_REDEAL_TALON,
    FCS_MOVE_TYPE_SEQ_TO_FOUNDATION,
    FCS_MOVE_TYPE_CANONIZE,
    FCS_MOVE_TYPE_SEPARATOR,
    FCS_MOVE_TYPE_NULL
};

typedef struct
{
    unsigned char c[4];
} fcs_move_t;

#define FCS_MOVE_TYPE 0
#define FCS_MOVE_SRC 1
#define FCS_MOVE_DEST 2
#define FCS_MOVE_NUM_CARDS_IN_SEQ 3
#define FCS_MOVE_NUM_CARDS_FLIPPED 3

#define fcs_move_set_src_stack(move, value)                                    \
    (move).c[FCS_MOVE_SRC] = ((unsigned char)(value));
#define fcs_move_set_src_freecell(move, value)                                 \
    (move).c[FCS_MOVE_SRC] = ((unsigned char)(value));
#define fcs_move_set_dest_stack(move, value)                                   \
    (move).c[FCS_MOVE_DEST] = ((unsigned char)(value));
#define fcs_move_set_dest_freecell(move, value)                                \
    (move).c[FCS_MOVE_DEST] = ((unsigned char)(value));
#define fcs_move_set_foundation(move, value)                                   \
    (move).c[FCS_MOVE_DEST] = ((unsigned char)(value));
#define fcs_move_set_type(move, value)                                         \
    (move).c[FCS_MOVE_TYPE] = ((unsigned char)(value));
#define fcs_move_set_num_cards_in_seq(move, value)                             \
    (move).c[FCS_MOVE_NUM_CARDS_IN_SEQ] = ((unsigned char)(value));
#define fcs_move_set_num_cards_flipped(move, value)                            \
    (move).c[FCS_MOVE_NUM_CARDS_FLIPPED] = ((unsigned char)(value));

#define fcs_move_get_src_stack(move) ((move).c[FCS_MOVE_SRC])
#define fcs_move_get_src_freecell(move) ((move).c[FCS_MOVE_SRC])
#define fcs_move_get_dest_stack(move) ((move).c[FCS_MOVE_DEST])
#define fcs_move_get_dest_freecell(move) ((move).c[FCS_MOVE_DEST])
#define fcs_move_get_foundation(move) ((move).c[FCS_MOVE_DEST])
#define fcs_move_get_type(move) ((move).c[FCS_MOVE_TYPE])
#define fcs_move_get_num_cards_in_seq(move)                                    \
    ((move).c[FCS_MOVE_NUM_CARDS_IN_SEQ])

typedef struct
{
    int num_moves;
    fcs_move_t *moves;
} fcs_moves_sequence_t;

#ifdef __cplusplus
}
#endif
