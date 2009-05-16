/* Copyright (c) 2000 Shlomi Fish
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */
/*
 * TODO : Add a description of this file.
 */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include "fcs_cl.h"

struct fc_solve_display_information_context_struct
{
    int debug_iter_state_output;
    int freecells_num;
    int stacks_num;
    int decks_num;
    int parseable_output;
    int canonized_order_output;
    int display_10_as_t;
    int display_parent_iter_num;
    int debug_iter_output_on;
    int display_moves;
    int display_states;
    int standard_notation;
};

typedef struct fc_solve_display_information_context_struct fc_solve_display_information_context_t;

static void init_debug_context(
    fc_solve_display_information_context_t * dc
    )
{
    dc->debug_iter_state_output = 0;
    dc->parseable_output = 0;
    dc->canonized_order_output = 0;
    dc->display_10_as_t = 0;
    dc->display_parent_iter_num = 0;
    dc->display_moves = 0;
    dc->display_states = 1;
    dc->standard_notation = 0;
}



static void my_iter_handler(
    void * user_instance,
    int iter_num,
    int depth,
    void * ptr_state,
    int parent_iter_num,
    void * lp_context
    )
{
    fc_solve_display_information_context_t * context;
    context = (fc_solve_display_information_context_t*)lp_context;

    fprintf(stdout, "Iteration: %i\n", iter_num);
    fprintf(stdout, "Depth: %i\n", depth);
    fprintf(stdout, "Stored-States: %i\n",
        freecell_solver_user_get_num_states_in_collection(user_instance)
        );
    if (context->display_parent_iter_num)
    {
        fprintf(stdout, "Parent Iteration: %i\n", parent_iter_num);
    }
    fprintf(stdout, "\n");


    if (context->debug_iter_state_output)
    {
        char * state_string =
            freecell_solver_user_iter_state_as_string(
                user_instance,
                ptr_state,
                context->parseable_output,
                context->canonized_order_output,
                context->display_10_as_t
                );
        printf("%s\n---------------\n\n\n", state_string);
        free((void*)state_string);
    }

#ifdef MYDEBUG
    {
        fcs_card_t card;
        int ret;
        char card_str[10];

        ret = fcs_check_state_validity(
            ptr_state_with_locations,
            context->freecells_num,
            context->stacks_num,
            context->decks_num,
            &card
            );

        if (ret != 0)
        {

            fcs_card_perl2user(card, card_str, context->display_10_as_t);
            if (ret == 3)
            {
                fprintf(stdout, "%s\n",
                    "There's an empty slot in one of the stacks."
                    );
            }
            else
            {
                fprintf(stdout, "%s%s.\n",
                    ((ret == 2)? "There's an extra card: " : "There's a missing card: "),
                    card_str
                );
            }
            exit(-1);
        }
    }
#endif
}

struct help_screen_struct
{
    char * key;
    char * screen;
};

typedef struct help_screen_struct help_screen_t;

help_screen_t help_screens[] = {
{
    "configs",
"These configurations are usually faster than the unmodified run:\n"
"\n"
"    fc-solve -l cool-jives\n"
"    fc-solve -l john-galt-line\n"
"\n"
"Or if you want an accurate verdict:\n"
"\n"
"    fc-solve -l fools-gold\n"
"\n"
"If you want to try constructing your own configurations refer to the\n"
"USAGE file in the Freecell Solver distribution\n"
},
{
    "options",
"fc-solve [options] board_file\n"
"\n"
},
{
    "problems",
"To be discussed.\n"
},
{
    "short-sol",
"The following configurations may produce shorter solutions:\n"
"\n"
"    fc-solve -opt\n"
"    fc-solve --method a-star -opt\n"
"    fc-solve --reparent-states -opt\n"
"    fc-solve --method a-star --reparent-states -opt\n"
"\n"
"If \"--method a-star\" is specified you can set the weights with\n"
"-asw {comma separated list of 5 numeric weights}, which may improve\n"
"the length of the solution. (refer to the USAGE file for more information)\n"
},
{
    "summary",
"fc-solve [flags] [board_file|-]\n"
"\n"
"Reads board from standard input by default or if a \"-\" is specified.\n"
"\n"
"- If it takes too long to finish, type \"fc-solve --help-configs\"\n"
"- If it erroneously reports a board as unsolvable, try adding the\n"
"  \"-to 01ABCDE\" flag\n"
"- If the solution is too long type \"fc-solve --help-short-sol\"\n"
"- To present the moves only try adding \"-m\" or \"-m -snx\"\n"
"- For a description of all options type \"fc-solve --help-options\"\n"
"- To deal with other problems type \"fc-solve --help-problems\"\n"
"- To turn --help into something more useful, type\n"
"  \"fc-solve --help-real-help\"\n"
"\n"
"Contact Shlomi Fish, http://www.shlomifish.org/ for more information.\n"
},
{
    NULL,
    NULL
}
}
;

enum MY_FCS_CMD_LINE_RET_VALUES
{
    EXIT_AND_RETURN_0 = FCS_CMD_LINE_USER
};

static void print_help_string(char * key)
{
    int i;
    for(i=0;help_screens[i].key != NULL ; i++)
    {
        if (!strcmp(key, help_screens[i].key))
        {
            printf("%s", help_screens[i].screen);
        }
    }
}

static int cmd_line_callback(
    void * instance,
    int argc,
    char * argv[],
    int arg,
    int * num_to_skip,
    int * ret,
    void * context
    )
{
    fc_solve_display_information_context_t * dc;
    *num_to_skip = 0;

    dc = (fc_solve_display_information_context_t * )context;

    if (!strcmp(argv[arg], "--version"))
    {
        printf(
            "fc-solve\nlibfreecell-solver version %s\n",
            freecell_solver_user_get_lib_version(instance)
        );
        *ret = EXIT_AND_RETURN_0;
        return FCS_CMD_LINE_STOP;
    }
    else if ((!strcmp(argv[arg], "-h")) || (!strcmp(argv[arg], "--help")))
    {
        char * help_key;

        help_key = getenv("FREECELL_SOLVER_DEFAULT_HELP");
        if (help_key == NULL)
        {
            help_key = "summary";
        }
        print_help_string(help_key);
        *ret = EXIT_AND_RETURN_0;
        return FCS_CMD_LINE_STOP;
    }
    else if (!strncmp(argv[arg], "--help-", 7))
    {
        print_help_string(argv[arg]+7);
        *ret = EXIT_AND_RETURN_0;
        return FCS_CMD_LINE_STOP;
    }
    else if ((!strcmp(argv[arg], "-i")) || (!strcmp(argv[arg], "--iter-output")))
    {
#define set_iter_handler() \
        freecell_solver_user_set_iter_handler(   \
            instance,   \
            my_iter_handler,   \
            dc    \
            );        \
        dc->debug_iter_output_on = 1;

        set_iter_handler();
    }
    else if ((!strcmp(argv[arg], "-s")) || (!strcmp(argv[arg], "--state-output")))
    {
        set_iter_handler();
        dc->debug_iter_state_output = 1;
#undef set_iter_handler
    }
    else if ((!strcmp(argv[arg], "-p")) || (!strcmp(argv[arg], "--parseable-output")))
    {
        dc->parseable_output = 1;
    }
    else if ((!strcmp(argv[arg], "-c")) || (!strcmp(argv[arg], "--canonized-order-output")))
    {
        dc->canonized_order_output = 1;
    }
    else if ((!strcmp(argv[arg], "-t")) || (!strcmp(argv[arg], "--display-10-as-t")))
    {
        dc->display_10_as_t = 1;
    }
    else if ((!strcmp(argv[arg], "-m")) || (!strcmp(argv[arg], "--display-moves")))
    {
        dc->display_moves = 1;
        dc->display_states = 0;
    }
    else if ((!strcmp(argv[arg], "-sn")) || (!strcmp(argv[arg], "--standard-notation")))
    {
        dc->standard_notation = 1;

    }
    else if ((!strcmp(argv[arg], "-snx")) || (!strcmp(argv[arg], "--standard-notation-extended")))
    {
        dc->standard_notation = 2;
    }
    else if ((!strcmp(argv[arg], "-sam")) || (!strcmp(argv[arg], "--display-states-and-moves")))
    {
        dc->display_moves = 1;
        dc->display_states = 1;
    }
    else if ((!strcmp(argv[arg], "-pi")) || (!strcmp(argv[arg], "--display-parent-iter")))
    {
        dc->display_parent_iter_num = 1;
    }
    else if ((!strcmp(argv[arg], "--reset")))
    {
        init_debug_context(dc);
        freecell_solver_user_set_iter_handler(
            instance,
            NULL,
            NULL
            );
        *num_to_skip = 0;
        return FCS_CMD_LINE_OK;
    }
    else
    {
        printf("Unimplemented option - \"%s\"!", argv[arg]);
        exit(-1);
    }
    *num_to_skip = 1;
    return FCS_CMD_LINE_SKIP;
}


static int command_num = 0;
static int debug_iter_output_on = 0;

static void select_signal_handler(int signal_num)
{
    command_num = (command_num+1)%3;
}

static void * current_instance;
static fc_solve_display_information_context_t * dc;


static void command_signal_handler(int signal_num)
{
    if (command_num == 0)
    {
        fprintf(
            stderr,
            "The number of iterations is %i\n",
            freecell_solver_user_get_num_times(current_instance)
            );
    }
    else if (command_num == 1)
    {
        if (debug_iter_output_on)
        {
            freecell_solver_user_set_iter_handler(
                current_instance,
                NULL,
                NULL
                );
            debug_iter_output_on = 0;
        }
        else
        {
            freecell_solver_user_set_iter_handler(
                current_instance,
                my_iter_handler,
                dc
                );
            debug_iter_output_on = 1;
        }
    }
    else if (command_num == 2)
    {
        dc->debug_iter_state_output = ! dc->debug_iter_state_output;
    }

    command_num = 0;
}


static char * known_parameters[] = {
    "-h", "--help",
        "--help-configs", "--help-options", "--help-problems",
        "--help-real-help", "--help-short-sol", "--help-summary",
    "-i", "--iter-output",
    "-s", "--state-output",
    "-p", "--parseable-output",
    "-c", "--canonized-order-output",
    "-t", "--display-10-as-t",
    "-m", "--display-moves",
    "-sn", "--standard-notation",
    "-snx", "--standard-notation-extended",
    "-sam", "--display-states-and-moves",
    "-pi", "--display-parent-iter",
    "--reset",
    "--version",
    NULL
    };

#define USER_STATE_SIZE 1024

int main(int argc, char * argv[])
{
    int parser_ret;
    void * instance;
    char * error_string;
    int arg;
    FILE * file;
    char user_state[USER_STATE_SIZE];
    int ret;

    fc_solve_display_information_context_t debug_context;

    init_debug_context(&debug_context);

    dc = &debug_context;

    instance = freecell_solver_user_alloc();

    current_instance = instance;


    parser_ret =
        freecell_solver_user_cmd_line_parse_args(
            instance,
            argc,
            argv,
            1,
            known_parameters,
            cmd_line_callback,
            &debug_context,
            &error_string,
            &arg
            );

    if (parser_ret == EXIT_AND_RETURN_0)
    {
        freecell_solver_user_free(instance);
        return 0;
    }
    else if (
        (parser_ret == FCS_CMD_LINE_PARAM_WITH_NO_ARG)
            )
    {
        fprintf(stderr, "The command line parameter \"%s\" requires an argument"
                " and was not supplied with one.\n", argv[arg]);
        return (-1);
    }
    else if (
        (parser_ret == FCS_CMD_LINE_ERROR_IN_ARG)
        )
    {
        if (error_string != NULL)
        {
            fprintf(stderr, "%s", error_string);
            free(error_string);
        }
        freecell_solver_user_free(instance);
        return -1;
    }

    if ((arg == argc) || (!strcmp(argv[arg], "-")))
    {
        file = stdin;
        if (!getenv("FREECELL_SOLVER_QUIET"))
        {
            fprintf(stderr, "%s",
                    "Reading the board from the standard input.\n"
                    "Type \"fc-solve --help\" for more usage information.\n"
                    "To cancel this message set the FREECELL_SOLVER_QUIET environment variable.\n"
                   );
        }
    }
    else if (argv[arg][0] == '-')
    {
        fprintf(stderr,
                "Unknown option \"%s\". "
                "Type \"%s --help\" for usage information.\n",
                argv[arg],
                argv[0]
                );
        freecell_solver_user_free(instance);

        return -1;
    }
    else
    {
        file = fopen(argv[arg], "r");
        if (file == NULL)
        {
            fprintf(stderr,
                "Could not open file \"%s\" for input. Exiting.\n",
                argv[arg]
                );
            freecell_solver_user_free(instance);

            return -1;
        }
    }
    memset(user_state, '\0', sizeof(user_state));
    fread(user_state, sizeof(user_state[0]), USER_STATE_SIZE-1, file);
    fclose(file);

    /* Win32 Does not have those signals */
#ifndef WIN32
#ifndef SIGUSR1
#define SIGUSR1 10
#endif
#ifndef SIGUSR2
#define SIGUSR2 12
#endif
    signal(SIGUSR1, command_signal_handler);
    signal(SIGUSR2, select_signal_handler);
#endif

#if 0
    {
        int limit = 500;
        freecell_solver_user_limit_iterations(instance, limit);
        ret = freecell_solver_user_solve_board(instance, user_state);
        while (ret == FCS_STATE_SUSPEND_PROCESS)
        {
            limit += 500;
            freecell_solver_user_limit_iterations(instance, limit);
            ret = freecell_solver_user_resume_solution(instance);
        }
    }
#else
    ret = freecell_solver_user_solve_board(instance, user_state);
#endif

    if (ret == FCS_STATE_INVALID_STATE)
    {
        char * error_string;
        error_string =
            freecell_solver_user_get_invalid_state_error_string(
                instance,
                debug_context.display_10_as_t
                );
        printf("%s\n", error_string);
        free(error_string);
    }
    else
    {
        if (ret == FCS_STATE_WAS_SOLVED)
        {
            printf("-=-=-=-=-=-=-=-=-=-=-=-\n\n");
            {
                fcs_move_t move;
                FILE * move_dump;
                char * as_string;
                int move_num = 0;

                move_dump = stdout;


                if (debug_context.display_states)
                {
                    as_string =
                        freecell_solver_user_current_state_as_string(
                            instance,
                            debug_context.parseable_output,
                            debug_context.canonized_order_output,
                            debug_context.display_10_as_t
                            );

                    fprintf(move_dump, "%s\n", as_string);

                    free(as_string);

                    fprintf(move_dump, "%s", "\n====================\n\n");
                }

                while (
                        freecell_solver_user_get_next_move(
                            instance,
                            &move
                            ) == 0
                        )
                {
                    if (debug_context.display_moves)
                    {
                        as_string =
                            freecell_solver_user_move_to_string_w_state(
                                instance,
                                move,
                                debug_context.standard_notation
                                );

                        if (debug_context.display_states && debug_context.standard_notation)
                        {
                            fprintf(move_dump, "Move: ");
                        }

                        fprintf(
                            move_dump,
                            (debug_context.standard_notation ?
                                "%s " :
                                "%s\n"
                            ),
                            as_string
                            );
                        move_num++;
                        if (debug_context.standard_notation)
                        {
                            if ((move_num % 10 == 0) || debug_context.display_states)
                            {
                                fprintf(move_dump, "\n");
                            }
                        }
                        if (debug_context.display_states)
                        {
                            fprintf(move_dump, "\n");
                        }
                        fflush(move_dump);
                        free(as_string);
                    }

                    if (debug_context.display_states)
                    {
                        as_string =
                            freecell_solver_user_current_state_as_string(
                                instance,
                                debug_context.parseable_output,
                                debug_context.canonized_order_output,
                                debug_context.display_10_as_t
                                );

                        fprintf(move_dump, "%s\n", as_string);

                        free(as_string);
                    }

                    if (debug_context.display_states || (!debug_context.standard_notation))
                    {
                        fprintf(move_dump, "%s", "\n====================\n\n");
                    }
                }

                if (debug_context.standard_notation && (!debug_context.display_states))
                {
                    fprintf(move_dump, "\n\n");
                }
            }

            printf("This game is solveable.\n");
        }
        else
        {
            printf ("I could not solve this game.\n");
        }

        printf(
            "Total number of states checked is %i.\n",
            freecell_solver_user_get_num_times(instance)
            );
#if 1
        printf(
            "This scan generated %i states.\n",
            freecell_solver_user_get_num_states_in_collection(instance)
            );
#endif
    }

    freecell_solver_user_free(instance);

    return 0;
}

