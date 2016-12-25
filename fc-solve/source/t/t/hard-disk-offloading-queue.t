#!/usr/bin/perl

use strict;
use warnings;

use Test::More tests => 1;
use File::Spec ();
use File::Path qw(mkpath);

package FC_Solve::QueueInC;

use Config;
use Inline;

my $IS_WIN = ( $^O eq "MSWin32" );

sub load
{
    my ($self) = @_;

    my $src = <<'EOF';
/*
 * offloading_queue.h - header file for the offloading-to-hard-disk
 * queue.
 */

#include <string.h>
#include <limits.h>
#include <stdio.h>
#include <stdbool.h>

typedef const unsigned char *fcs_offloading_queue_item_t;

typedef struct
{
    long page_index;
    size_t write_to_idx;
    size_t read_from_idx;
    unsigned char *data;
} fcs_offloading_queue_page_t;

static inline void fcs_offloading_queue_page__init(
    fcs_offloading_queue_page_t *const page,
    const long page_index)
{
    fcs_offloading_queue_page_t new_page = {
        .page_index = page_index,
        .data =
            malloc(sizeof(fcs_offloading_queue_item_t) * 100)};
    *page = new_page;
    page->write_to_idx = 0;
    page->read_from_idx = 0;

    return;
}

static inline void fcs_offloading_queue_page__destroy(
    fcs_offloading_queue_page_t *const page)
{
    free(page->data);
    page->data = NULL;
}

typedef struct
{
    const char *offload_dir_path;
    fcs_offloading_queue_page_t pages[2];
} fcs_offloading_queue_t;

const size_t NUM_ITEMS_PER_PAGE = (128 * 1024);
static inline void fcs_offloading_queue__init(
    fcs_offloading_queue_t *const queue, const char *const offload_dir_path
    )
{
    queue->offload_dir_path = offload_dir_path;

    fcs_offloading_queue_page__init(
        &(queue->pages[0]), 0);
    fcs_offloading_queue_page__init(
        &(queue->pages[1]), 0);
}

static inline void fcs_offloading_queue__destroy(fcs_offloading_queue_t *queue)
{
    fcs_offloading_queue_page__destroy(&(queue->pages[0]));
    fcs_offloading_queue_page__destroy(&(queue->pages[1]));
}

typedef struct
{
    fcs_offloading_queue_t q;
} QueueInC;

SV* _proto_new(const char * offload_dir_path) {
        QueueInC * s;

        New(42, s, 1, QueueInC);

        fcs_offloading_queue__init(&(s->q), strdup(offload_dir_path));
        SV*      obj_ref = newSViv(0);
        SV*      obj = newSVrv(obj_ref, "FC_Solve::QueueInC");
        sv_setiv(obj, (IV)s);
        SvREADONLY_on(obj);
        return obj_ref;
}

static inline QueueInC * deref(SV * const obj) {
    return (QueueInC*)SvIV(SvRV(obj));
}

void DESTROY(SV* obj) {
  QueueInC * s = deref(obj);
  free(s->q.offload_dir_path);
  fcs_offloading_queue__destroy(&s->q);
  Safefree(s);
}

EOF

    my $pkg = 'FC_Solve::QueueInC';

    my @workaround_for_a_heisenbug =
        ( $IS_WIN ? ( optimize => '-g' ) : () );

    my $ccflags = "$Config{ccflags} -std=gnu99";
    if ($IS_WIN)
    {
        $ccflags =~ s#(^|\s)-[Of][a-zA-Z0-9_\-]*#$1#gms;
    }

    Inline->bind(
        C                 => $src,
        name              => $pkg,
        NAME              => $pkg,
        CCFLAGS           => $ccflags,
        CLEAN_AFTER_BUILD => 0,
        @workaround_for_a_heisenbug,
    );

    return;
}

__PACKAGE__->load;

sub new
{
    my ( $class, $args ) = @_;

    return FC_Solve::QueueInC::_proto_new(
        $args->{offload_dir_path},
    );
}

package main;

my $queue_offload_dir_path =
    File::Spec->catdir( File::Spec->curdir(), "queue-offload-dir" );
mkpath($queue_offload_dir_path);

# TEST:$c=0;
sub run_queue_tests
{
    my ( $blurb_base, $class_name ) = @_;

    {
        my $queue = $class_name->new(
            {
                offload_dir_path   => $queue_offload_dir_path,
            }
        );
    }

    {
        my $queue = $class_name->new(
            {
                offload_dir_path   => $queue_offload_dir_path,
            }
        );

        my $map_idx_to_item = sub { my ($idx) = @_; return $idx * 3 + 1; };

    }

    # TEST:$c++
    pass("Placeholder test");

    return;
}

# TEST:$run_queue_tests=$c;

# TEST*$run_queue_tests
run_queue_tests( 'C queue', 'FC_Solve::QueueInC' );

=head1 COPYRIGHT & LICENSE

Copyright 2016 by Shlomi Fish

This program is distributed under the MIT/Expat License:
L<http://www.opensource.org/licenses/mit-license.php>

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

=cut
