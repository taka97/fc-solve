#!/usr/bin/perl

use strict;
use warnings;

use Test::More tests => 18;
use File::Spec;
use File::Temp qw( tempdir );
use Test::Trap
    qw( trap $trap :flow:stderr(systemsafe):stdout(systemsafe):warn );
use FC_Solve::Paths
    qw( bin_board bin_exe_raw is_without_dbm samp_board $FC_SOLVE__RAW );

my $MID24_BOARD = samp_board('24-mid.board');

{
    trap
    {
        system( $FC_SOLVE__RAW, "--reset_junk_at_end", $MID24_BOARD );
    };

    my $needle =
qq#Unknown option "--reset_junk_at_end". Type "$FC_SOLVE__RAW --help" for usage information.#;

    # TEST
    like( $trap->stderr(), qr/^\Q$needle\E$/ms,
        "Unknown option was recognized, for one that has a valid prefix.",
    );

    # TEST
    unlike( $trap->stdout(), qr/\S/,
        "Empty standard output due to unrecognized option.",
    );
}

{
    trap
    {
        system( $FC_SOLVE__RAW, '--read-from-file4,amateur-star.sh',
            '--stacks-num', '7', $MID24_BOARD );
    };

    my $needle =
qq#Unknown option "--read-from-file4,amateur-star.sh". Type "$FC_SOLVE__RAW --help" for usage information.#;

    # TEST
    like( $trap->stderr(), qr/^\Q$needle\E$/ms,
        "Option without space is not accepted." );
}

{
    trap
    {
        system( bin_exe_raw( ['freecell-solver-fc-pro-range-solve'] ),
            '1', '3', '1', '-l', 'as', );
    };

    my @lines = split( /\n/, $trap->stdout() );

    # TEST
    is_deeply(
        [
            grep {
                /\A\[\[Num FCS Moves\]\]=([0-9]+)\z/
                    ? ( not( $1 > 0 ) )
                    : ( die "Incorrect $_" );
                }
                grep { /\A\[\[Num FCS Moves\]\]/ } @lines
        ],
        [],
        "All FCS Move counts are valid",
    );

    # TEST
    is_deeply(
        [
            grep {
                /\A\[\[Num FCPro Moves\]\]=([0-9]+)\z/
                    ? ( not( $1 > 0 ) )
                    : ( die "Incorrect $_" );
                }
                grep { /\A\[\[Num FCPro Moves\]\]/ } @lines
        ],
        [],
        "All FCPro Move counts are valid.",
    );
}

{
    trap
    {
        system( bin_exe_raw( ['summary-fc-solve'] ), 1591, 1592, 1593,
            qw(-- --method random-dfs -to [0123456789] -sp r:tf -opt -opt-to 0123456789ABCDE -seed 24 -mi 10000)
        );
    };

    my $out = $trap->stdout();

    # TEST
    like(
        $out,
qr/^1591 = Verdict: Intractable.*?^1592 = Verdict: Solved.*?^1593 = Verdict: Solved/ms,
        "All deals in summary-fc-solve are either intractable or solved.",
    );

    # TEST
    unlike(
        $out,
        qr/Verdict: Unsolved/,
        'No deal is unsolved, because that makes no sense.',
    );
}

{
    my $status;

    trap
    {
        $status = system( $FC_SOLVE__RAW,
            qw#
                --flare-name prefix_of_a_long_name --method soft-dfs -to 0123456789 -sp r:tf -opt -opt-to 0123456789ABCDE
                -nf --flare-name another_long_name --method soft-dfs -to 0123467 -sp r:tf -opt -opt-to 0123456789ABCDE
                --flares-plan
                #, 'Run:100@prefix,Run:200@another_long_name',
            bin_board('24.board'),
        );
    };

    my $needle = q#Flares Plan: Unknown flare name.#;

    # TEST
    like(
        $trap->stderr(),
        qr/^\Q$needle\E$/ms,
q#Cannot use a prefix of a flare's name as the name in the flares plan.#,
    );

    # TEST
    ok( scalar( $status != 0 ), 'Exited with a non-zero exit code.', );

    # TEST
    is( $trap->stdout(), '', 'Empty standard output on flares plan error.' );
}

# TEST:$num_boards=2;
foreach my $board_fn (
    qw(24-with-stray-d-char.board
    24-with-stray-d-char-in-freecells.board)
    )
{
    trap
    {
        system( $FC_SOLVE__RAW, "-mi", 1000,
            samp_board('24-with-stray-d-char.board') );
    };

    my $needle = q#Not enough input.#;

    # TEST*$num_boards
    like( $trap->stderr(), qr/^\Q$needle\E$/ms, "Invalid card format.", );

    # TEST*$num_boards
    unlike( $trap->stdout(), qr/\S/,
        "Empty standard output due to invalid card format.",
    );
}

{
SKIP:
    {
        if ( is_without_dbm() )
        {
            Test::More::skip( "without the dbm fc_solvers", 3 );
        }
        my $status;
        trap
        {
            $status = system(
                bin_exe_raw( ['depth_dbm_fc_solver'] ),
                "--offload-dir-path",
                ( tempdir( CLEANUP => 1 ) . '/' ),
                bin_board('empty.board'),
            );
        };

        my $out = $trap->stdout();

        # TEST
        is( $out, '',
            "No output for depth_dbm_fc_solver on empty and invalid board",
        );

        # TEST
        like(
            $trap->stderr(),
            qr/\AInvalid input board/,
            "Correct stderr for depth_dbm_fc_solver on empty and invalid board",
        );

        # TEST
        ok( scalar( $status != 0 ), "Exit code is non-zero." );
    }
}

{
SKIP:
    {
        if ( is_without_dbm() )
        {
            Test::More::skip( "without the dbm fc_solvers", 1 );
        }
        my $status;
        trap
        {
            $status = system(
                bin_exe_raw( ['depth_dbm_fc_solver'] ),
                '--num-threads',
                3,
                '--batch-size',
                20,
                "--offload-dir-path",
                ( tempdir( CLEANUP => 1 ) . '/' ),
                bin_board('1107600547.board'),
            );
        };

        my $out = $trap->stdout();

        # TEST
        like(
            $out,
qr/\nCould not solve successfully\.\nhandle_and_destroy_instance_solution end\n?\z/,
            "1107600547 run finished.",
        );
    }
}

__END__

=head1 COPYRIGHT AND LICENSE

This file is part of Freecell Solver. It is subject to the license terms in
the COPYING.txt file found in the top-level directory of this distribution
and at http://fc-solve.shlomifish.org/docs/distro/COPYING.html . No part of
Freecell Solver, including this file, may be copied, modified, propagated,
or distributed except according to the terms contained in the COPYING file.

Copyright (c) 2008 Shlomi Fish

=cut

