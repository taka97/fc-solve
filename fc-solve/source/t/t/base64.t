#!/usr/bin/perl

use strict;
use warnings;

use Test::More tests => 1;

use FC_Solve::Base64;

# TEST:$c=0;
sub test_base64
{
    local $Test::Builder::Level = $Test::Builder::Level + 1;
    my ($init_buffer, $blurb_base) = @_;

    my $encoded = FC_Solve::Base64::base64_encode($init_buffer);

    # TEST:$c++;
    like(
        $encoded,
        qr/\A[A-Za-z0-9\+_]*=*\z/,
        "$blurb_base - encoded string matches Base64 regexp."
    );

}

# TEST:$test_base64=$c;

{
    # TEST*$test_base64
    test_base64('FooBar24', 'String FooBar24');
}

