/*
 * This file is part of Freecell Solver. It is subject to the license terms in
 * the COPYING.txt file found in the top-level directory of this distribution
 * and at http://fc-solve.shlomifish.org/docs/distro/COPYING.html . No part of
 * Freecell Solver, including this file, may be copied, modified, propagated,
 * or distributed except according to the terms contained in the COPYING file.
 *
 * Copyright (c) 2000 Shlomi Fish
 */
/*
 * instance.h - header file of fc_solve_instance_t / fc_solve_hard_thread_t /
 * fc_solve_soft_thread_t .
 */
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <math.h>

#include "move.h"
#include "fcs_enums.h"

#include "rate_state.h"
#include "indirect_buffer.h"
#include "rand.h"
#include "game_type_params.h"

#if (FCS_STATE_STORAGE == FCS_STATE_STORAGE_LIBREDBLACK_TREE) ||               \
    (defined(INDIRECT_STACK_STATES) &&                                         \
        (FCS_STACK_STORAGE == FCS_STACK_STORAGE_LIBREDBLACK_TREE))
#include <redblack.h>
#endif

#if (FCS_STATE_STORAGE == FCS_STATE_STORAGE_LIBAVL2_TREE)
#include "fcs_libavl2_state_storage.h"
#endif

#if (FCS_STACK_STORAGE == FCS_STACK_STORAGE_LIBAVL2_TREE)
#include "fcs_libavl2_stack_storage.h"
#endif

#if (FCS_STATE_STORAGE == FCS_STATE_STORAGE_GLIB_TREE) ||                      \
    (FCS_STATE_STORAGE == FCS_STATE_STORAGE_GLIB_HASH) ||                      \
    (defined(INDIRECT_STACK_STATES) &&                                         \
        ((FCS_STACK_STORAGE == FCS_STACK_STORAGE_GLIB_TREE) ||                 \
            (FCS_STACK_STORAGE == FCS_STACK_STORAGE_GLIB_HASH)))
#include <glib.h>
#endif

#if ((defined(FCS_RCS_STATES) &&                                               \
         (FCS_RCS_CACHE_STORAGE == FCS_RCS_CACHE_STORAGE_JUDY)) ||             \
     (FCS_STATE_STORAGE == FCS_STATE_STORAGE_JUDY) ||                          \
     (defined(INDIRECT_STACK_STATES) &&                                        \
         (FCS_STACK_STORAGE == FCS_STACK_STORAGE_JUDY)))
#include <Judy.h>
#endif

#if ((FCS_STATE_STORAGE == FCS_STATE_STORAGE_INTERNAL_HASH) ||                 \
     (FCS_STACK_STORAGE == FCS_STACK_STORAGE_INTERNAL_HASH))
#include "fcs_hash.h"
#endif

#if (FCS_STATE_STORAGE == FCS_STATE_STORAGE_GOOGLE_DENSE_HASH) ||              \
    (FCS_STACK_STORAGE == FCS_STACK_STORAGE_GOOGLE_DENSE_HASH)
#include "google_hash.h"
#endif

#if ((FCS_STATE_STORAGE == FCS_STATE_STORAGE_KAZ_TREE) ||                      \
     (defined(FCS_RCS_STATES) &&                                               \
         (FCS_RCS_CACHE_STORAGE == FCS_RCS_CACHE_STORAGE_KAZ_TREE)))
#include "kaz_tree.h"
#endif

#if (FCS_STATE_STORAGE == FCS_STATE_STORAGE_DB_FILE)
#include <db.h>
#endif

#include "pqueue.h"

#include "meta_alloc.h"

#ifndef FCS_DISABLE_SIMPLE_SIMON
#define FCS_SS_POS_BY_RANK_WIDTH (13 + 1)
#define FCS_SS_POS_BY_RANK_LEN (FCS_SS_POS_BY_RANK_WIDTH * 4)
#endif

/* We need 2 chars per card - one for the column_idx and one
 * for the card_idx.
 *
 * We also need it times 13 for each of the ranks.
 *
 * We need (4*LOCAL_DECKS_NUM+1) slots to hold the cards plus a
 * (-1,-1) (= end) padding.
 * */
#define FCS_POS_BY_RANK_WIDTH (MAX_NUM_DECKS << 3)

/* We don't keep track of kings (rank == 13). */
#define NUM_POS_BY_RANK_SLOTS 13
#define FCS_POS_BY_RANK_LEN (NUM_POS_BY_RANK_SLOTS * FCS_POS_BY_RANK_WIDTH)
typedef struct
{
    char col, height;
} fcs_pos_by_rank_t;

#ifndef FCS_DISABLE_SIMPLE_SIMON
#define FCS_BOTH__POS_BY_RANK__SIZE                                            \
    (max(FCS_SS_POS_BY_RANK_LEN * sizeof(fcs_pos_by_rank_t),                   \
        FCS_POS_BY_RANK_LEN))
#else
#define FCS_BOTH__POS_BY_RANK__SIZE FCS_POS_BY_RANK_LEN
#endif

typedef int8_t fcs__positions_by_rank_t[FCS_BOTH__POS_BY_RANK__SIZE];

/*
 * This is a linked list item that is used to implement a queue for the BFS
 * scan.
 * */
typedef struct fcs_states_linked_list_item_struct
{
    fcs_collectible_state_t *s;
    struct fcs_states_linked_list_item_struct *next;
} fcs_states_linked_list_item_t;

/*
 * Declare these structures because they will be used within
 * fc_solve_instance, and they will contain a pointer to it.
 * */
struct fc_solve_hard_thread_struct;
struct fc_solve_soft_thread_struct;
struct fc_solve_instance_struct;

typedef void (*fc_solve_solve_for_state_move_func_t)(
    struct fc_solve_soft_thread_struct *, fcs_kv_state_t *,
    fcs_derived_states_list_t *);

#ifdef FCS_SINGLE_HARD_THREAD
typedef struct fc_solve_instance_struct fc_solve_hard_thread_t;
#else
typedef struct fc_solve_hard_thread_struct fc_solve_hard_thread_t;
#endif

extern fcs_bool_t fc_solve_check_and_add_state(
    fc_solve_hard_thread_t *, fcs_kv_state_t *, fcs_kv_state_t *);

#if (FCS_STATE_STORAGE == FCS_STATE_STORAGE_GLIB_HASH)
extern guint fc_solve_hash_function(gconstpointer key);
#endif
#include "move_funcs_maps.h"

/* HT_LOOP == hard threads' loop - macros to abstract it. */
#ifdef FCS_SINGLE_HARD_THREAD

#define HT_LOOP_START() fc_solve_hard_thread_t *const hard_thread = instance;

#else
#define HT_LOOP_START()                                                        \
    fc_solve_hard_thread_t *hard_thread = instance->hard_threads;              \
    fc_solve_hard_thread_t *const end_hard_thread =                            \
        hard_thread + instance->num_hard_threads;                              \
    for (; hard_thread < end_hard_thread; hard_thread++)
#endif

/* ST_LOOP == soft threads' loop - macros to abstract it. */
#define ST_LOOP_START()                                                        \
    fc_solve_soft_thread_t *const ht_soft_threads =                            \
        HT_FIELD(hard_thread, soft_threads);                                   \
    fc_solve_soft_thread_t *soft_thread = ht_soft_threads;                     \
    fc_solve_soft_thread_t *const end_soft_thread =                            \
        ht_soft_threads + HT_FIELD(hard_thread, num_soft_threads);             \
    for (; soft_thread < end_soft_thread; soft_thread++)

#define ST_LOOP__WAS_FINISHED() (soft_thread == end_soft_thread)

#define ST_LOOP__GET_INDEX() (soft_thread - ht_soft_threads)

#define MOVES_GROW_BY 16

typedef struct
{
    double weights[FCS_NUM_BEFS_WEIGHTS];
} fcs_default_weights_t;
typedef struct
{
    fcs_bool_t should_go_over_stacks;
    double max_sequence_move_factor, cards_under_sequences_factor,
        seqs_over_renegade_cards_factor, depth_factor,
        num_cards_not_on_parents_factor;

    double num_cards_out_lookup_table[14];
    /*
     * The BeFS weights of the different BeFS tests. Those
     * weights determine the commulative priority of the state.
     * */
    fcs_default_weights_t befs_weights;
} fc_solve_state_weighting_t;

typedef enum {
    FCS_NO_SHUFFLING,
    FCS_RAND,
    FCS_WEIGHTING,
} fcs_tests_group_type_t;

typedef struct
{
    size_t num;
    size_t *order_group_moves;
    fcs_tests_group_type_t shuffling_type;
    fc_solve_state_weighting_t weighting;
} fcs_tests_order_group_t;

typedef struct
{
    size_t num_groups;
    fcs_tests_order_group_t *groups;
} fcs_tests_order_t;

typedef struct
{
    ssize_t max_depth;
    fcs_tests_order_t moves_order;
} fcs_by_depth_tests_order_t;

typedef struct
{
    size_t num;
    fcs_by_depth_tests_order_t *by_depth_moves;
} fcs_by_depth_tests_order_array_t;

#define STRUCT_CLEAR_FLAG(instance, flag) (instance)->flag = FALSE
#define STRUCT_TURN_ON_FLAG(instance, flag) (instance)->flag = TRUE
#define STRUCT_QUERY_FLAG(instance, flag) ((instance)->flag)
#define STRUCT_SET_FLAG_TO(instance, flag, value) (instance)->flag = (value)

#ifdef FCS_RCS_STATES
struct fcs_cache_key_info_struct
{
    fcs_collectible_state_t *val_ptr;
    fcs_state_t key;
    /* lower_pri and higher_pri form a doubly linked list.
     *
     * pri == priority.
     * */
    struct fcs_cache_key_info_struct *lower_pri, *higher_pri;
};

typedef struct fcs_cache_key_info_struct fcs_cache_key_info_t;

typedef struct
{
#if (FCS_RCS_CACHE_STORAGE == FCS_RCS_CACHE_STORAGE_JUDY)
    Pvoid_t states_values_to_keys_map;
#elif (FCS_RCS_CACHE_STORAGE == FCS_RCS_CACHE_STORAGE_KAZ_TREE)
    dict_t *kaz_tree;
#else
#error Unknown FCS_RCS_CACHE_STORAGE
#endif
    fcs_compact_allocator_t states_values_to_keys_allocator;
    fcs_int_limit_t count_elements_in_cache, max_num_elements_in_cache;

    fcs_cache_key_info_t *lowest_pri, *highest_pri, *recycle_bin;
} fcs_lru_cache_t;

#endif

#ifndef FCS_WITHOUT_ITER_HANDLER
typedef void *fcs_instance_debug_iter_output_context_t;

typedef void (*fcs_instance_debug_iter_output_func_t)(
    fcs_instance_debug_iter_output_context_t, fcs_int_limit_t, int, void *,
    fcs_kv_state_t *, fcs_int_limit_t);
#endif

typedef struct fc_solve_soft_thread_struct fc_solve_soft_thread_t;

typedef struct fc_solve_instance_struct fc_solve_instance_t;

/***************************************************/

typedef struct
{
    size_t scan_idx;
    size_t quota;
} fcs_prelude_item_t;

struct fc_solve_hard_thread_struct
{
#ifndef FCS_SINGLE_HARD_THREAD
    fc_solve_instance_t *instance;
#endif

    struct fc_solve_soft_thread_struct *soft_threads;

#ifndef FCS_SINGLE_HARD_THREAD
    /*
     * The hard thread count of how many states he checked himself. The
     * instance num_checked_states can be confusing because other threads modify
     * it too.
     *
     * Thus, the soft thread switching should be done based on this variable
     * */
    fcs_int_limit_t ht__num_checked_states;

#endif
    /*
     * The maximal limit for num_checked_states.
     * */
    fcs_int_limit_t ht__max_num_checked_states;

    /*
     * The index for the soft-thread that is currently processed
     * */
    int_fast32_t st_idx;
    /*
     * This is the mechanism used to allocate memory for stacks, states
     * and move stacks.
     * */
    fcs_compact_allocator_t allocator;

#ifdef FCS_WITH_MOVES
    /*
     * This is a move stack that is used and re-used by the
     * tests functions of this hard thread
     * */
    fcs_move_stack_t reusable_move_stack;
#endif

    /*
     * This is a buffer used to temporarily store the stacks of the duplicated
     * state.
     * */
    DECLARE_IND_BUF_T(indirect_stacks_buffer)

    size_t prelude_num_items;
    size_t prelude_idx;
    fcs_prelude_item_t *prelude;

    fcs_bool_t allocated_from_list;
    int_fast32_t num_soft_threads;

    /*
     * A counter that determines how many of the soft threads that belong
     * to this hard thread have already finished. If it becomes num_soft_threads
     * this thread is skipped.
     * */
    int num_soft_threads_finished;

    char *prelude_as_string;
};

/********************************************/

typedef struct
{
    int idx;
    pq_rating_t rating;
} fcs_rating_with_index_t;

typedef struct
{
    fcs_collectible_state_t *state;
    fcs_derived_states_list_t derived_states_list;
    int_fast32_t move_func_list_idx;
    int current_state_index;
    int move_func_idx;
    int derived_states_random_indexes_max_size;
    fcs_rating_with_index_t *derived_states_random_indexes;
    fcs__positions_by_rank_t positions_by_rank;
    fcs_game_limit_t num_vacant_stacks;
    fcs_game_limit_t num_vacant_freecells;
} fcs_soft_dfs_stack_item_t;

typedef struct
{
    fc_solve_solve_for_state_move_func_t *move_funcs;
    /* num_move_funcs should be int instead of size_t for performance.*/
    int num_move_funcs;
    fcs_tests_group_type_t shuffling_type;
    fc_solve_state_weighting_t weighting;
} fcs_tests_list_t;

typedef struct
{
    uint_fast32_t num_lists;
    fcs_tests_list_t *lists;
} fcs_tests_list_of_lists;

typedef struct
{
    ssize_t max_depth;
    fcs_tests_list_of_lists move_funcs;
} fcs_tests_by_depth_unit_t;

typedef struct
{
    int num_units;
    fcs_tests_by_depth_unit_t *by_depth_units;
} fcs_tests_by_depth_array_t;

typedef enum {
    FCS_SUPER_METHOD_DFS,
    FCS_SUPER_METHOD_BEFS_BRFS,
#ifndef FCS_DISABLE_PATSOLVE
    FCS_SUPER_METHOD_PATSOLVE,
#endif
} fcs_super_method_type_t;

struct fc_solve__patsolve_thread_struct;
struct fc_solve_soft_thread_struct
{
    fc_solve_hard_thread_t *hard_thread;

    /*
     * The ID of the soft thread inside the instance.
     * Used for the state-specific flags.
     * */
    int id;

    // The moves' order indicates which move funcs to run.
    fcs_by_depth_tests_order_array_t by_depth_moves_order;

    /* The super-method type - can be  */
    fcs_super_method_type_t super_method_type;

    fc_solve_seq_cards_power_type_t initial_cards_under_sequences_value;

    struct
    {
        struct
        {
            /*
             * The (temporary) max depth of the Soft-DFS scans)
             * */
            ssize_t dfs_max_depth;

            /*
             * Soft-DFS uses a stack of fcs_soft_dfs_stack_item_t s.
             *
             * derived_states_list - a list of states to be checked next. Not
             * all of them will be checked because it is possible that future
             * states already visited them.
             *
             * current_state_index - the index of the last checked state
             * in depth i.
             *
             * move_func_list_idx - the index of the test list that is
             * performed. FCS performs each test separately, so
             * states_to_check and friends will not be overpopulated.
             *
             * num_vacant_stacks - the number of unoccpied stacks that
             * correspond
             * to solution_states.
             *
             * num_vacant_freecells - ditto for the freecells.
             *
             * */
            fcs_soft_dfs_stack_item_t *soft_dfs_info;

            /* The depth of the DFS stacks */
            ssize_t depth;

            /*
             * A pseudo-random number generator for use in the random-DFS scan
             * */
            fcs_rand_t rand_gen;

            /*
             * The initial seed of this random number generator
             * */
            int rand_seed;

            /*
             * The tests to be performed in a preprocessed form.
             * */
            fcs_tests_by_depth_array_t tests_by_depth_array;
        } soft_dfs;
        struct
        {
            fcs__positions_by_rank_t befs_positions_by_rank;
            fc_solve_solve_for_state_move_func_t *tests_list, *tests_list_end;
            struct
            {
                struct
                {
                    /*
                     * A linked list that serves as the queue for the BFS scan.
                     * */
                    fcs_states_linked_list_item_t *bfs_queue;
                    /*
                     * The last item in the linked list, so new items can be
                     * added at
                     * it, thus making it a queue.
                     * */
                    fcs_states_linked_list_item_t *bfs_queue_last_item;
                    /*
                     * A linked list of items that were freed from
                     * the queue and should be reused before allocating new
                     * items.
                     * */
                    fcs_states_linked_list_item_t *recycle_bin;
                } brfs;
                struct
                {
                    /*
                     * The priority queue of the BeFS scan
                     * */
                    pri_queue_t pqueue;
                    fc_solve_state_weighting_t weighting;
                } befs;
            } meth;
            /*
             * The first state to be checked by the scan. It is a kind of
             * bootstrap for the algorithm.
             * */
            fcs_collectible_state_t *first_state_to_check;
        } befs;
    } method_specific;

    fcs_bool_t FCS_SOFT_THREAD_IS_FINISHED, FCS_SOFT_THREAD_INITIALIZED,
        FCS_SOFT_THREAD_IS_A_COMPLETE_SCAN;

    /*
     * The number of vacant stacks in the current state - is read by
     * the move functions in freecell.c .
     * */
    fcs_game_limit_t num_vacant_stacks;

    /*
     * The number of vacant freecells in the current state - is read
     * from the test functions in freecell.c .
     * */
    fcs_game_limit_t num_vacant_freecells;

    /*
     * The number of iterations with which to process this scan
     * */
    fcs_int_limit_t checked_states_step;

    /*
     * A string that serves as an identification for the user.
     * */
    char name[FCS_MAX_IDENT_LEN];

#ifndef FCS_ENABLE_PRUNE__R_TF__UNCOND
    /*
     * Whether pruning should be done.
     * This variable is temporary - there should be a better pruning
     * abstraction with several optional prunes.
     * */
    fcs_bool_t enable_pruning;
#endif

#ifndef FCS_DISABLE_PATSOLVE
    /*
     * The patsolve soft_thread that is associated with this soft_thread.
     * */
    struct fc_solve__patsolve_thread_struct *pats_scan;
#endif
    /*
     * Differentiates between SOFT_DFS and RANDOM_DFS.
     * */
    fcs_bool_t master_to_randomize;
    fcs_bool_t is_befs
#ifdef FCS_WITH_MOVES
        ,
        is_optimize_scan
#endif
        ;
};

struct fc_solve_instance_struct
{
// The parameters of the game - see the declaration of fcs_game_type_params_t .
#ifndef FCS_FREECELL_ONLY
    fcs_game_type_params_t game_params;
#ifndef FCS_DISABLE_PATSOLVE
    fcs_card_t game_variant_suit_mask;
    fcs_card_t game_variant_desired_suit_value;
#endif
#endif

    /* The number of states that were checked by the solving algorithm. */
    fcs_int_limit_t i__num_checked_states;

#ifndef FCS_WITHOUT_MAX_NUM_STATES
    /*
     * Limit for the maximal number of checked states. max_num_checked_states
     * is useful because it can limit the amount of consumed memory (and time).
     *
     * This is the effective number that enables the process to work without
     * checking if it's zero.
     *
     * Normally should be used instead.
     * */
    fcs_int_limit_t effective_max_num_checked_states;
#endif
#ifndef FCS_DISABLE_NUM_STORED_STATES
    fcs_int_limit_t effective_max_num_states_in_collection;
#ifndef FCS_WITHOUT_TRIM_MAX_STORED_STATES
    fcs_int_limit_t effective_trim_states_in_collection_from;
#endif
#endif
/*
 * tree is the balanced binary tree that is used to store and index
 * the checked states.
 *
 * */
#if (FCS_STATE_STORAGE == FCS_STATE_STORAGE_LIBREDBLACK_TREE)
    struct rbtree *tree;
#elif (FCS_STATE_STORAGE == FCS_STATE_STORAGE_JUDY)
    Pvoid_t judy_array;
#elif (FCS_STATE_STORAGE == FCS_STATE_STORAGE_LIBAVL2_TREE)
    fcs_libavl2_states_tree_table_t *tree;
#elif (FCS_STATE_STORAGE == FCS_STATE_STORAGE_GLIB_TREE)
    GTree *tree;
#elif (FCS_STATE_STORAGE == FCS_STATE_STORAGE_KAZ_TREE)
    dict_t *tree;
#endif

/*
 * hash is the hash table that is used to store the previous
 * states of the scan.
 * */
#if (FCS_STATE_STORAGE == FCS_STATE_STORAGE_GLIB_HASH)
    GHashTable *hash;
#elif (FCS_STATE_STORAGE == FCS_STATE_STORAGE_INTERNAL_HASH)
    fc_solve_hash_t hash;
#elif (FCS_STATE_STORAGE == FCS_STATE_STORAGE_GOOGLE_DENSE_HASH)
    fcs_states_google_hash_handle_t hash;
#endif

#if defined(INDIRECT_STACK_STATES)
/*
 * The storage mechanism for the stacks assuming INDIRECT_STACK_STATES is
 * used.
 * */
#if (FCS_STACK_STORAGE == FCS_STACK_STORAGE_INTERNAL_HASH)
    fc_solve_hash_t stacks_hash;
#elif (FCS_STACK_STORAGE == FCS_STACK_STORAGE_LIBAVL2_TREE)
    fcs_libavl2_stacks_tree_table_t *stacks_tree;
#elif (FCS_STACK_STORAGE == FCS_STACK_STORAGE_LIBREDBLACK_TREE)
    struct rbtree *stacks_tree;
#elif (FCS_STACK_STORAGE == FCS_STACK_STORAGE_GLIB_TREE)
    GTree *stacks_tree;
#elif (FCS_STACK_STORAGE == FCS_STACK_STORAGE_GLIB_HASH)
    GHashTable *stacks_hash;
#elif (FCS_STACK_STORAGE == FCS_STACK_STORAGE_GOOGLE_DENSE_HASH)
    fcs_columns_google_hash_handle_t stacks_hash;
#elif (FCS_STACK_STORAGE == FCS_STACK_STORAGE_JUDY)
    Pvoid_t stacks_judy_array;
#else
#error FCS_STACK_STORAGE is not set to a good value.
#endif
#endif

    fcs_collectible_state_t *list_of_vacant_states;
/*
 * Storing using Berkeley DB is not operational for some reason so
 * pay no attention to it for the while
 * */
#if (FCS_STATE_STORAGE == FCS_STATE_STORAGE_DB_FILE)
    DB *db;
#endif

#ifndef FCS_HARD_CODE_CALC_REAL_DEPTH_AS_FALSE
    fcs_bool_t FCS_RUNTIME_CALC_REAL_DEPTH;
#endif
#ifndef FCS_HARD_CODE_REPARENT_STATES_AS_FALSE
    fcs_bool_t FCS_RUNTIME_TO_REPARENT_STATES_REAL;
#endif
#ifndef FCS_HARD_CODE_SCANS_SYNERGY_AS_TRUE
    fcs_bool_t FCS_RUNTIME_SCANS_SYNERGY;
#endif
#ifndef FCS_HARD_CODE_REPARENT_STATES_AS_FALSE
    fcs_bool_t FCS_RUNTIME_TO_REPARENT_STATES_PROTO;
#endif
    ;
#ifdef FCS_WITH_MOVES
    fcs_bool_t FCS_RUNTIME_OPTIMIZE_SOLUTION_PATH,
        FCS_RUNTIME_IN_OPTIMIZATION_THREAD, FCS_RUNTIME_OPT_TESTS_ORDER_WAS_SET;
#endif

/*
 * This is the number of states in the state collection.
 *
 * It gives a rough estimate of the memory occupied by the instance.
 * */
#ifndef FCS_DISABLE_NUM_STORED_STATES
#ifndef FCS_WITHOUT_TRIM_MAX_STORED_STATES
    fcs_int_limit_t active_num_states_in_collection;
#endif
    fcs_int_limit_t num_states_in_collection;
#endif

#ifdef FCS_SINGLE_HARD_THREAD
    struct fc_solve_hard_thread_struct hard_thread;
#ifdef FCS_WITH_MOVES
    fcs_bool_t is_optimization_st;
    struct fc_solve_soft_thread_struct optimization_soft_thread;
#endif
#else
    uint_fast32_t num_hard_threads;
    struct fc_solve_hard_thread_struct *hard_threads;
    /*
     * An iterator over the hard threads.
     * */
    fc_solve_hard_thread_t *current_hard_thread;

#ifdef FCS_WITH_MOVES
    /*
     * This is the hard-thread used for the optimization scan.
     * */
    struct fc_solve_hard_thread_struct *optimization_thread;
#endif
#endif

    /*
     * This is the master moves' funcs order. It is used to initialize all
     * the new Soft-Threads.
     * */
    fcs_tests_order_t instance_tests_order;

    /*
     * A counter that determines how many of the hard threads that belong
     * to this hard thread have already finished. If it becomes num_hard_threads
     * the instance terminates.
     * */
    int num_hard_threads_finished;

#ifdef FCS_WITH_MOVES
    /*
     * The tests order for the optimization scan as specified by the user.
     * */
    fcs_tests_order_t opt_tests_order;
#endif

#ifdef FCS_RCS_STATES
    fcs_lru_cache_t rcs_states_cache;

#if ((FCS_STATE_STORAGE == FCS_STATE_STORAGE_LIBAVL2_TREE) ||                  \
     (FCS_STATE_STORAGE == FCS_STATE_STORAGE_KAZ_TREE))
    fcs_state_t *tree_new_state_key;
    fcs_collectible_state_t *tree_new_state;
#endif

#endif

#ifndef FCS_WITHOUT_ITER_HANDLER
    /*
     * The debug_iter_output variables provide a programmer programmable way
     * to debug the algorithm while it is running. This works well for DFS
     * and Soft-DFS scans but at present support for BeFS and BFS is not
     * too good, as its hard to tell which state came from which parent state.
     *
     * debug_iter_output_func is a pointer to the function that performs the
     * debugging. If NULL, this feature is not used.
     *
     * debug_iter_output_context is a user-specified context for it, that
     * may include data that is not included in the instance structure.
     *
     * This feature is used by the "-s" and "-i" flags of fc-solve-debug.
     * */
    fcs_instance_debug_iter_output_func_t debug_iter_output_func;
    fcs_instance_debug_iter_output_context_t debug_iter_output_context;
#endif

    /*
     * The next ID to allocate for a soft-thread.
     * */
    int next_soft_thread_id;

    /* This is a place-holder for the initial state */
    fcs_state_keyval_pair_t *state_copy_ptr;

#ifdef FCS_WITH_MOVES
    /* This is the final state that the scan recommends to the
     * interface
     * */
    fcs_collectible_state_t *final_state;

    /*
     * A move stack that contains the moves leading to the solution.
     *
     * It is created only after the solution was found by swallowing
     * all the stacks of each depth.
     * */
    fcs_move_stack_t solution_moves;
#endif

    /*
     * The meta allocator - see meta_alloc.h.
     * */
    fcs_meta_compact_allocator_t *meta_alloc;

#if (defined(FCS_WITH_MOVES) && (!defined(FCS_DISABLE_PATSOLVE)))
    /*
     * The soft_thread that solved the state.
     *
     * Needed to trace the patsolve solutions.
     * */
    fc_solve_soft_thread_t *solving_soft_thread;
#endif
#ifndef FCS_DISABLE_PATSOLVE
    /*
     * This is intended to be used by the patsolve scan which is
     * sensitive to the ordering of the columns/stacks. This is an ugly hack
     * but hopefully it will work.
     * */
    fcs_state_keyval_pair_t *initial_non_canonized_state;
#endif

#ifndef FCS_DISABLE_SIMPLE_SIMON
    /*
     * Whether or not this is a Simple Simon-like game.
     * */
    fcs_bool_t is_simple_simon;
#endif
};

#ifdef FCS_SINGLE_HARD_THREAD
#define HT_FIELD(ht, field) (ht)->hard_thread.field
#define HT_INSTANCE(hard_thread) (hard_thread)
#define INST_HT0(instance) ((instance)->hard_thread)
#else
#define HT_FIELD(hard_thread, field) (hard_thread)->field
#define HT_INSTANCE(hard_thread) ((hard_thread)->instance)
#define INST_HT0(instance) ((instance)->hard_threads[0])
#endif
#define fcs_st_instance(soft_thread) HT_INSTANCE((soft_thread)->hard_thread)

#define DFS_VAR(soft_thread, var) (soft_thread)->method_specific.soft_dfs.var
#define BEFS_VAR(soft_thread, var)                                             \
    (soft_thread)->method_specific.befs.meth.befs.var
/* M is Methods-common. */
#define BEFS_M_VAR(soft_thread, var) (soft_thread)->method_specific.befs.var
#define BRFS_VAR(soft_thread, var)                                             \
    (soft_thread)->method_specific.befs.meth.brfs.var

/*
 * An enum that specifies the meaning of each BeFS weight.
 * */
#define FCS_BEFS_WEIGHT_CARDS_OUT 0
#define FCS_BEFS_WEIGHT_MAX_SEQUENCE_MOVE 1
#define FCS_BEFS_WEIGHT_CARDS_UNDER_SEQUENCES 2
#define FCS_BEFS_WEIGHT_SEQS_OVER_RENEGADE_CARDS 3
#define FCS_BEFS_WEIGHT_DEPTH 4
#define FCS_BEFS_WEIGHT_NUM_CARDS_NOT_ON_PARENTS 5

#ifndef FCS_DISABLE_PATSOLVE
#include "pat.h"
#endif

extern fc_solve_solve_process_ret_t fc_solve_befs_or_bfs_do_solve(
    fc_solve_soft_thread_t *const soft_thread);

extern void fc_solve_increase_dfs_max_depth(
    fc_solve_soft_thread_t *const soft_thread);

static inline void *memdup(const void *const src, const size_t my_size)
{
    void *const dest = malloc(my_size);
    if (dest == NULL)
    {
        return NULL;
    }

    memcpy(dest, src, my_size);

    return dest;
}

static inline int update_col_cards_under_sequences(
#ifndef FCS_FREECELL_ONLY
    const int sequences_are_built_by,
#endif
    const fcs_const_cards_column_t col,
    int d /* One less than cards_num of col. */
    )
{
    fcs_card_t this_card = fcs_col_get_card(col, d);
    fcs_card_t prev_card = fcs_col_get_card(col, d - 1);
    for (; (d > 0) && ({
             prev_card = fcs_col_get_card(col, d - 1);
             fcs_is_parent_card(this_card, prev_card);
         });
         d--, this_card = prev_card)
    {
    }
    return d;
}

static inline void fc_solve_soft_thread_update_initial_cards_val(
    fc_solve_soft_thread_t *const soft_thread)
{
    fc_solve_instance_t *const instance = fcs_st_instance(soft_thread);
#ifndef FCS_FREECELL_ONLY
    const int sequences_are_built_by =
        GET_INSTANCE_SEQUENCES_ARE_BUILT_BY(instance);
#endif
    /* We cannot use typeof here because clang complains about double
     * const.
     * */
    const fcs_state_t *const s = &(instance->state_copy_ptr->s);

    fc_solve_seq_cards_power_type_t cards_under_sequences = 0;
    for (int a = 0; a < INSTANCE_STACKS_NUM; a++)
    {
        const_AUTO(col, fcs_state_get_col(*s, a));
        const int cards_num = fcs_col_len(col);
        if (cards_num <= 1)
        {
            continue;
        }
        cards_under_sequences += FCS_SEQS_OVER_RENEGADE_POWER(
            (size_t)update_col_cards_under_sequences(
#ifndef FCS_FREECELL_ONLY
                sequences_are_built_by,
#endif
                col, cards_num - 1));
    }
    soft_thread->initial_cards_under_sequences_value = cards_under_sequences;
}

extern fcs_collectible_state_t *fc_solve_sfs_raymond_prune(
    fc_solve_soft_thread_t *const, fcs_kv_state_t *const);

#ifdef FCS_RCS_STATES
fcs_state_t *fc_solve_lookup_state_key_from_val(
    fc_solve_instance_t *const instance,
    fcs_collectible_state_t *const orig_ptr_state_val);

extern int fc_solve_compare_lru_cache_keys(const void *, const void *, void *);

#endif

extern void fc_solve_soft_thread_init_befs_or_bfs(
    fc_solve_soft_thread_t *const soft_thread);

extern void fc_solve_instance__init_hard_thread(
#ifndef FCS_SINGLE_HARD_THREAD
    fc_solve_instance_t *const instance,
#endif
    fc_solve_hard_thread_t *const hard_thread);

extern void fc_solve_free_soft_thread_by_depth_test_array(
    fc_solve_soft_thread_t *const soft_thread);

static inline fcs_tests_order_t tests_order_dup(fcs_tests_order_t *const orig)
{
    const_SLOT(num_groups, orig);
    fcs_tests_order_t ret = (fcs_tests_order_t){.num_groups = num_groups,
        .groups = memdup(
            orig->groups, sizeof(orig->groups[0]) *
                              ((num_groups & (size_t)(~(MOVES_GROW_BY - 1))) +
                                  MOVES_GROW_BY))};

    for (size_t i = 0; i < num_groups; ++i)
    {
        ret.groups[i].order_group_moves =
            memdup(ret.groups[i].order_group_moves,
                sizeof(ret.groups[i].order_group_moves[0]) *
                    ((ret.groups[i].num & (size_t)(~(MOVES_GROW_BY - 1))) +
                        MOVES_GROW_BY));
    }

    return ret;
}

extern fc_solve_soft_thread_t *fc_solve_new_soft_thread(
    fc_solve_hard_thread_t *const hard_thread);

/* This is the commmon code from fc_solve_instance__init_hard_thread() and
 * recycle_hard_thread() */
static inline void fc_solve_reset_hard_thread(
    fc_solve_hard_thread_t *const hard_thread)
{
#ifndef FCS_SINGLE_HARD_THREAD
    HT_FIELD(hard_thread, ht__num_checked_states) = 0;
#endif
    HT_FIELD(hard_thread, ht__max_num_checked_states) = FCS_INT_LIMIT_MAX;
    HT_FIELD(hard_thread, num_soft_threads_finished) = 0;
}

static inline void fc_solve_reset_soft_thread(
    fc_solve_soft_thread_t *const soft_thread)
{
    STRUCT_CLEAR_FLAG(soft_thread, FCS_SOFT_THREAD_IS_FINISHED);
    STRUCT_CLEAR_FLAG(soft_thread, FCS_SOFT_THREAD_INITIALIZED);
}

typedef enum {
    FOREACH_SOFT_THREAD_CLEAN_SOFT_DFS,
    FOREACH_SOFT_THREAD_FREE_INSTANCE,
    FOREACH_SOFT_THREAD_ACCUM_TESTS_ORDER,
    FOREACH_SOFT_THREAD_DETERMINE_SCAN_COMPLETENESS
} fcs_foreach_st_callback_choice_t;

extern void fc_solve_foreach_soft_thread(fc_solve_instance_t *const instance,
    const fcs_foreach_st_callback_choice_t callback_choice,
    void *const context);

/*
    This function is the last function that should be called in the
    sequence of operations on instance, and it is meant for de-allocating
    whatever memory was allocated by alloc_instance().
  */

static inline void fc_solve_free_tests_order(fcs_tests_order_t *moves_order)
{
    const_SLOT(groups, moves_order);
    const_SLOT(num_groups, moves_order);
    for (size_t group_idx = 0; group_idx < num_groups; ++group_idx)
    {
        free(groups[group_idx].order_group_moves);
    }
    free(groups);
    moves_order->groups = NULL;
    moves_order->num_groups = 0;
}

/***********************************************************/

#define DECLARE_MOVE_FUNCTION(name)                                            \
    extern void name(fc_solve_soft_thread_t *const soft_thread,                \
        fcs_kv_state_t *const raw_ptr_state_raw,                               \
        fcs_derived_states_list_t *const derived_states_list)

#ifdef FCS_SINGLE_HARD_THREAD
extern void fc_solve_init_soft_thread(fc_solve_hard_thread_t *const hard_thread,
    fc_solve_soft_thread_t *const soft_thread);
#endif

#ifndef FCS_HARD_CODE_CALC_REAL_DEPTH_AS_FALSE
static inline fcs_bool_t fcs_get_calc_real_depth(
    const fc_solve_instance_t *const instance)
{
    return STRUCT_QUERY_FLAG(instance, FCS_RUNTIME_CALC_REAL_DEPTH);
}
#endif

#ifdef FCS_WITH_MOVES
extern void fc_solve_trace_solution(fc_solve_instance_t *const instance);
#endif
extern void fc_solve_finish_instance(fc_solve_instance_t *const instance);

#ifdef __cplusplus
}
#endif

#ifdef FCS_SINGLE_HARD_THREAD
#define NUM_CHECKED_STATES (HT_INSTANCE(hard_thread)->i__num_checked_states)
#else
#define NUM_CHECKED_STATES HT_FIELD(hard_thread, ht__num_checked_states)
#endif
