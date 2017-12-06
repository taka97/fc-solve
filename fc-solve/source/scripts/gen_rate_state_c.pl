use strict;
use warnings;
use Path::Tiny qw/ path /;

my $TYPE_NAME  = 'fc_solve_seq_cards_power_type_t';
my $ARRAY_NAME = 'fc_solve_seqs_over_cards_lookup';
my $POWER      = 1.3;
my $TOP        = 2 * 13 * 4 + 1;
my $DECL       = "const $TYPE_NAME ${ARRAY_NAME}[$TOP]";

path("rate_state.h")->spew_utf8(<<"EOF");
// This file was generated by gen_rate_state_c.pl .
// Do not modify directly.
#pragma once

typedef double $TYPE_NAME;
extern $DECL;
#define FCS_SEQS_OVER_RENEGADE_POWER(n) ${ARRAY_NAME}[(n)]
EOF

path("rate_state.c")->spew_utf8(<<"EOF");
// This file was generated by gen_rate_state_c.pl .
// Do not modify directly.
#include "rate_state.h"

// This contains the exponents of the first few integers to the power
// of $POWER
$DECL =
{
@{[join ", ", map { $_**$POWER } ( 0 .. $TOP - 1 )]}
};
EOF
