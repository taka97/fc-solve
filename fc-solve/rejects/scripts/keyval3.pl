#!/usr/bin/perl

use strict;
use warnings;

sub process
{
    my ($ident, $field) = @_;

    if ($field eq "s")
    {
        return $ident . "_key";
    }
    else
    {
        return $ident . "_val->" . $field;
    }
}

while (<>)
{
    chomp;
    # An fcs_state_t function parameter
    if (m{\A\s+fcs_state_t\s*\*\s*(\w+)_key\s*,\s*\z})
    {
        my $ident = $1;
        print "$_\n";
        PARAM_LOOP:
        while(<>)
        {
            chomp;
            if (m{\A\{})
            {
                print "$_\n";
                FUNC_LOOP:
                while (<>)
                {
                    chomp;
                    if (m{\A\}})
                    {
                        print "$_\n";
                        last FUNC_LOOP;
                    }
                    else
                    {
                        s[$ident->(\w+)][process($ident, $1)]ge;
                        print "$_\n";
                    }
                }
                last PARAM_LOOP;
            }
            else
            {
                print "$_\n";
            }
        }
    }
    else
    {
        print "$_\n";
    }
}

__END__

=head1 COPYRIGHT AND LICENSE

This file is part of Freecell Solver. It is subject to the license terms in
the COPYING.txt file found in the top-level directory of this distribution
and at http://fc-solve.shlomifish.org/docs/distro/COPYING.html . No part of
Freecell Solver, including this file, may be copied, modified, propagated,
or distributed except according to the terms contained in the COPYING file.

Copyright (c) 2000 Shlomi Fish

=cut
