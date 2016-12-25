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
    size_t num_items_per_page;
    long page_index, queue_id;
    size_t write_to_idx;
    size_t read_from_idx;
    unsigned char *data;
} fcs_offloading_queue_page_t;

static inline void fcs_offloading_queue_page__recycle(
    fcs_offloading_queue_page_t *const page)
{
    page->write_to_idx = 0;
    page->read_from_idx = 0;
}

static inline void fcs_offloading_queue_page__init(
    fcs_offloading_queue_page_t *const page, const size_t num_items_per_page,
    const long page_index, const long queue_id)
{
    fcs_offloading_queue_page_t new_page = {
        .num_items_per_page = num_items_per_page,
        .page_index = page_index,
        .queue_id = queue_id,
        .data =
            malloc(sizeof(fcs_offloading_queue_item_t) * num_items_per_page)};
    *page = new_page;
    fcs_offloading_queue_page__recycle(page);

    return;
}

static inline void fcs_offloading_queue_page__destroy(
    fcs_offloading_queue_page_t *const page)
{
    free(page->data);
    page->data = NULL;
}

static inline bool fcs_offloading_queue_page__can_extract(
    const fcs_offloading_queue_page_t *const page)
{
    return (page->read_from_idx < page->write_to_idx);
}

static inline bool fcs_offloading_queue_page__can_insert(
    const fcs_offloading_queue_page_t *const page)
{
    return (page->write_to_idx < page->num_items_per_page);
}

static inline const char *fcs_offloading_queue_page__calc_filename(
    fcs_offloading_queue_page_t *const page, char *const buffer,
    const char *const offload_dir_path)
{
    sprintf(buffer, "%s/fcs_queue%lXq_%020lX.page", offload_dir_path,
        page->queue_id, page->page_index);

    return buffer;
}

static inline void fcs_offloading_queue_page__start_after(
    fcs_offloading_queue_page_t *const page,
    const fcs_offloading_queue_page_t *const other_page)
{
    page->page_index = other_page->page_index + 1;
    fcs_offloading_queue_page__recycle(page);
}

static inline void fcs_offloading_queue_page__bump(
    fcs_offloading_queue_page_t *const page)
{
    fcs_offloading_queue_page__start_after(page, page);
}

static inline void fcs_offloading_queue_page__read_next_from_disk(
    fcs_offloading_queue_page_t *const page, const char *const offload_dir_path)
{
    fcs_offloading_queue_page__bump(page);
    char page_filename[PATH_MAX + 1];
    fcs_offloading_queue_page__calc_filename(
        page, page_filename, offload_dir_path);

    FILE *const f = fopen(page_filename, "rb");
    fread(page->data, sizeof(fcs_offloading_queue_item_t),
        page->num_items_per_page, f);
    fclose(f);

    /* We need to set this limit because it's a read-only page that we
     * retrieve from the disk and otherwise ->can_extract() will return
     * false for most items.
     * */
    page->write_to_idx = page->num_items_per_page;

    unlink(page_filename);
}

static inline void fcs_offloading_queue_page__offload(
    fcs_offloading_queue_page_t *const page, const char *const offload_dir_path)
{
    char page_filename[PATH_MAX + 1];

    fcs_offloading_queue_page__calc_filename(
        page, page_filename, offload_dir_path);

    FILE *const f = fopen(page_filename, "wb");
    fwrite(page->data, sizeof(fcs_offloading_queue_item_t),
        page->num_items_per_page, f);
    fclose(f);
}

typedef struct
{
    int num_items_per_page;
    const char *offload_dir_path;
    long num_inserted, num_items_in_queue, num_extracted;
    long id;
    /*
     * page_idx_to_write_to, page_idx_for_backup and page_idx_to_read_from
     * always point to the two "pages" below, but they can be swapped and
     * page_idx_for_backup may be NULL.
     */
    int page_idx_to_write_to, page_idx_for_backup, page_idx_to_read_from;
    fcs_offloading_queue_page_t pages[2];
} fcs_offloading_queue_t;

const size_t NUM_ITEMS_PER_PAGE = (128 * 1024);
static inline void fcs_offloading_queue__init(
    fcs_offloading_queue_t *const queue, const char *const offload_dir_path,
    const long id)
{
    queue->num_items_per_page = NUM_ITEMS_PER_PAGE;
    queue->offload_dir_path = offload_dir_path;
    queue->num_inserted = queue->num_items_in_queue = queue->num_extracted = 0;
    queue->id = id;

    fcs_offloading_queue_page__init(
        &(queue->pages[0]), NUM_ITEMS_PER_PAGE, 0, queue->id);
    fcs_offloading_queue_page__init(
        &(queue->pages[1]), NUM_ITEMS_PER_PAGE, 0, queue->id);

    queue->page_idx_to_read_from = queue->page_idx_to_write_to = 0;
    queue->page_idx_for_backup = 1;
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

SV* _proto_new(int num_items_per_page, const char * offload_dir_path, long queue_id) {
        QueueInC * s;

        New(42, s, 1, QueueInC);

        fcs_offloading_queue__init(&(s->q), strdup(offload_dir_path), queue_id);
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
        $args->{num_items_per_page},
        $args->{offload_dir_path},
        ( $args->{queue_id} || 0 )
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
                num_items_per_page => 10,
                offload_dir_path   => $queue_offload_dir_path,
            }
        );
    }

    {
        my $queue = $class_name->new(
            {
                num_items_per_page => 10,
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
