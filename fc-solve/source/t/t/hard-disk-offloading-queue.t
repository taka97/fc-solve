#!/usr/bin/perl

use strict;
use warnings;

use Test::More tests => 1131;
use File::Spec ();
use File::Path qw(mkpath);

package FC_Solve::QueueInC;

use Config;

use FC_Solve::InlineWrap (
    C => <<'EOF',
#define FCS_DBM_USE_OFFLOADING_QUEUE

#include "offloading_queue.h"

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

static inline fcs_offloading_queue_t * q(SV * const obj) {
    return &(deref(obj)->q);
}

void insert(SV* obj, int item_i) {
    fcs_offloading_queue_item_t item = (fcs_offloading_queue_item_t)item_i;
    fcs_offloading_queue__insert(q(obj), &item);
}

SV* extract(SV* obj) {
    fcs_offloading_queue_item_t item;

    return (fcs_offloading_queue__extract(q(obj), &item))
    ? newSViv((int)item)
    : &PL_sv_undef;
}

long get_num_inserted(SV* obj) {
    return q(obj)->num_inserted;
}

long get_num_items_in_queue(SV* obj) {
    return q(obj)->num_items_in_queue;
}

long get_num_extracted(SV* obj) {
    return q(obj)->num_extracted;
}

void DESTROY(SV* obj) {
  QueueInC * s = deref(obj);
  free(s->q.offload_dir_path);
  fcs_offloading_queue__destroy(&s->q);
  Safefree(s);
}

EOF
);

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

        # TEST:$c++;
        ok( $queue, "$blurb_base - Queue was initialized." );

        # TEST:$c++
        is( $queue->get_num_inserted(),
            0, "$blurb_base - No items were inserted yet." );

        # TEST:$c++
        is( $queue->get_num_items_in_queue(),
            0, "$blurb_base - No items in queue." );

        # TEST:$c++
        is( $queue->get_num_extracted(),
            0, "$blurb_base - no items extracted." );

        $queue->insert(1);

        # TEST:$c++
        is( $queue->get_num_inserted(), 1, "$blurb_base - 1 item." );

        # TEST:$c++
        is( $queue->get_num_items_in_queue(),
            1, "$blurb_base - 1 items in queue." );

        $queue->insert(200);

        # TEST:$c++
        is( $queue->get_num_inserted(), 2, "$blurb_base - 2 item." );

        # TEST:$c++
        is( $queue->get_num_items_in_queue(),
            2, "$blurb_base - 2 items in queue." );

        $queue->insert(33);

        # TEST:$c++
        is( $queue->get_num_inserted(), 3, "$blurb_base - 3 item." );

        # TEST:$c++
        is( $queue->get_num_items_in_queue(),
            3, "$blurb_base - 3 items in queue." );

        # TEST:$c++
        is( $queue->get_num_extracted(),
            0, "$blurb_base - no items extracted." );

        # TEST:$c++
        is( scalar( $queue->extract() ),
            1, "$blurb_base - Extracted 1 from queue." );

        # TEST:$c++;
        is( $queue->get_num_inserted(),
            3, "$blurb_base - 3 Items were inserted so far." );

        # TEST:$c++;
        is( $queue->get_num_items_in_queue(),
            2, "$blurb_base - 2 items in queue (after one extracted." );

        # TEST:$c++
        is( $queue->get_num_extracted(),
            1, "$blurb_base - 1 item was extracted." );

        # TEST:$c++
        is( scalar( $queue->extract() ),
            200, "$blurb_base - Extracted 1 from queue." );

        # TEST:$c++;
        is( $queue->get_num_inserted(),
            3, "$blurb_base - 3 Items were inserted so far." );

        # TEST:$c++;
        is( $queue->get_num_items_in_queue(),
            1, "$blurb_base - 1 items in queue (after two extracted.)" );

        # TEST:$c++
        is( $queue->get_num_extracted(),
            2, "$blurb_base - 2 items were extracted." );

        # Now trying to add an item after a few were extracted and see how
        # the statistics are affected.
        $queue->insert(4);

        # TEST:$c++;
        is( $queue->get_num_inserted(),
            4, "$blurb_base - 4 Items were inserted so far." );

        # TEST:$c++;
        is( $queue->get_num_items_in_queue(), 2,
"$blurb_base - 2 items in queue (after two extracted and one added.)"
        );

        # TEST:$c++
        is( $queue->get_num_extracted(),
            2, "$blurb_base - 2 items were extracted." );
    }

    {
        my $queue = $class_name->new(
            {
                num_items_per_page => 10,
                offload_dir_path   => $queue_offload_dir_path,
            }
        );

        my $map_idx_to_item = sub { my ($idx) = @_; return $idx * 3 + 1; };

        # TEST:$num_items=1000;
        foreach my $item_idx ( 1 .. 1_000 )
        {
            $queue->insert( $map_idx_to_item->($item_idx) );
        }

        foreach my $item_idx ( 1 .. 1_000 )
        {
            # TEST:$c=$c+$num_items;
            is(
                scalar( $queue->extract() ),
                $map_idx_to_item->($item_idx),
                "$blurb_base - Testing the extraction of item no. $item_idx"
            );
        }

        # Now let's test the final statistics.

        # TEST:$c++;
        is( $queue->get_num_inserted(),
            1_000, "$blurb_base - 1,000 items were inserted" );

        # TEST:$c++;
        is( $queue->get_num_items_in_queue(),
            0, "$blurb_base - 0 items in queue." );

        # TEST:$c++
        is( $queue->get_num_extracted(),
            1_000, "$blurb_base - 1,000 items were extracted in total." );

        # Now let's add more items after the queue is empty.

        # TEST:$num_items=100;
        foreach my $item_idx ( 1 .. 100 )
        {
            $queue->insert( $map_idx_to_item->($item_idx) );
        }

        # TEST:$c++;
        is( $queue->get_num_inserted(),
            1_100, "$blurb_base - 1,100 items were inserted" );

        # TEST:$c++;
        is( $queue->get_num_items_in_queue(),
            100, "$blurb_base - 100 items in queue." );

        # TEST:$c++
        is( $queue->get_num_extracted(),
            1_000, "$blurb_base - 1,000 items were extracted in total." );

        foreach my $item_idx ( 1 .. 100 )
        {
            # TEST:$c=$c+$num_items;
            is(
                scalar( $queue->extract() ),
                $map_idx_to_item->($item_idx),
                "$blurb_base - Testing the extraction of item no. $item_idx"
            );
        }

        # TEST:$c++;
        is( $queue->get_num_inserted(),
            1_100, "$blurb_base - 1,100 items were inserted" );

        # TEST:$c++;
        is( $queue->get_num_items_in_queue(),
            0, "$blurb_base - 100 items in queue." );

        # TEST:$c++
        is( $queue->get_num_extracted(),
            1_100, "$blurb_base - 1,100 items were extracted in total." );

    }

    return;
}

# TEST:$run_queue_tests=$c;

# TEST*$run_queue_tests
run_queue_tests( 'C queue', 'FC_Solve::QueueInC' );
