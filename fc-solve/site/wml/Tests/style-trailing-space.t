#!/usr/bin/perl

use strict;
use warnings;

use Test::TrailingSpace 0.03;
use Test::More tests => 1;

my $lib = qr#libfreecell-solver\.js(?:\.mem)?#;
my $finder = Test::TrailingSpace->new(
    {
        root => '.',
        filename_regex => qr/./,
        abs_path_prune_re => qr#(?:\A(?:\./)?(?:lib/fc-solve-for-javascript|dest(?:-prod)?/(?:(?:js/$lib)|(?:js-fc-solve/(?:text|automated-tests)/$lib\z))))|(?:\.(?:diff|patch|png|ts|woff|xz|zip)\z)#,
    }
);

# TEST
$finder->no_trailing_space("No trailing whitespace was found.")
