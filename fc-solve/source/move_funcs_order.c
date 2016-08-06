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
 * move_funcs_order.c - contains the fc_solve_apply_tests_order function.
 */
#include "dll_thunk.h"
#include <string.h>
#include "config.h"
#include "fcs_back_compat.h"
#include "move_funcs_order.h"
#include "set_weights.h"

int fc_solve_apply_tests_order(fcs_tests_order_t *tests_order,
    const char *string FCS__PASS_ERR_STR(char *const error_string))
{
    char test_name[2] = {0};

    fc_solve_free_tests_order(tests_order);

    tests_order->groups = SMALLOC(tests_order->groups, TESTS_ORDER_GROW_BY);
    tests_order->groups[tests_order->num_groups].num = 0;
    tests_order->groups[tests_order->num_groups].order_group_tests =
        SMALLOC(tests_order->groups[tests_order->num_groups].order_group_tests,
            TESTS_ORDER_GROW_BY);
    tests_order->groups[tests_order->num_groups].shuffling_type =
        FCS_NO_SHUFFLING;

    tests_order->num_groups++;

    const size_t len = strlen(string);
    fcs_bool_t is_group = FALSE;
    fcs_bool_t is_start_group = FALSE;

    size_t i;
    for (i = 0; i < len; i++)
    {
        if ((string[i] == '(') || (string[i] == '['))
        {
            if (is_group)
            {
#ifdef FCS_WITH_ERROR_STRS
                strcpy(error_string, "There's a nested random group.");
#endif
                return 1;
            }
            is_group = TRUE;
            is_start_group = TRUE;
            if (tests_order->groups[tests_order->num_groups - 1].num)
            {
                if (!(tests_order->num_groups & (TESTS_ORDER_GROW_BY - 1)))
                {
                    tests_order->groups =
                        SREALLOC(tests_order->groups, TESTS_ORDER_GROW_BY);
                }

                tests_order->groups[tests_order->num_groups].num = 0;
                tests_order->groups[tests_order->num_groups].order_group_tests =
                    SMALLOC(tests_order->groups[tests_order->num_groups]
                                .order_group_tests,
                        TESTS_ORDER_GROW_BY);

                tests_order->num_groups++;
            }
            tests_order->groups[tests_order->num_groups - 1].shuffling_type =
                FCS_RAND;
            continue;
        }

        if ((string[i] == ')') || (string[i] == ']'))
        {
            if (is_start_group)
            {
#ifdef FCS_WITH_ERROR_STRS
                strcpy(error_string, "There's an empty group.");
#endif
                return 2;
            }
            if (!is_group)
            {
#ifdef FCS_WITH_ERROR_STRS
                strcpy(error_string,
                    "There's a renegade right parenthesis or bracket.");
#endif
                return 3;
            }
            /* Try to parse the ordering function. */
            if (string[i + 1] == '=')
            {
                i += 2;
                const char *const open_paren = strchr(string + i, '(');
                if (!open_paren)
                {
#ifdef FCS_WITH_ERROR_STRS
                    strcpy(error_string, "A = ordering function is missing its "
                                         "open parenthesis - (");
#endif
                    return 5;
                }
                if (string_starts_with(string + i, "rand", open_paren))
                {
                    tests_order->groups[tests_order->num_groups - 1]
                        .shuffling_type = FCS_RAND;
                }
                else if (string_starts_with(string + i, "asw", open_paren))
                {
                    tests_order->groups[tests_order->num_groups - 1]
                        .shuffling_type = FCS_WEIGHTING;
                }
                else
                {
#ifdef FCS_WITH_ERROR_STRS
                    strcpy(error_string, "Unknown = ordering function");
#endif
                    return 6;
                }
                const char *const aft_open_paren = open_paren + 1;
                const char *const close_paren = strchr(aft_open_paren, ')');
                if (!close_paren)
                {
#ifdef FCS_WITH_ERROR_STRS
                    strcpy(error_string,
                        "= ordering function not terminated with a ')'");
#endif
                    return 7;
                }
                if (tests_order->groups[tests_order->num_groups - 1]
                        .shuffling_type == FCS_WEIGHTING)
                {
                    fc_solve_set_weights(aft_open_paren, close_paren,
                        tests_order->groups[tests_order->num_groups - 1]
                            .weighting.befs_weights.weights);
                }
                else
                {
                    if (close_paren != aft_open_paren)
                    {
#ifdef FCS_WITH_ERROR_STRS
                        strcpy(
                            error_string, "=rand() arguments are not empty.");
#endif
                        return 8;
                    }
                }
                /*
                 * a will be incremented so it should be -1 here -
                 * at the end of the token/expression.
                 *
                 * */
                i = close_paren - string;
            }
            is_group = FALSE;
            is_start_group = FALSE;

            if (tests_order->groups[tests_order->num_groups - 1].num)
            {
                if (!(tests_order->num_groups & (TESTS_ORDER_GROW_BY - 1)))
                {
                    tests_order->groups = SREALLOC(tests_order->groups,
                        tests_order->num_groups + TESTS_ORDER_GROW_BY);
                }
                tests_order->groups[tests_order->num_groups].num = 0;
                tests_order->groups[tests_order->num_groups].order_group_tests =
                    SMALLOC(tests_order->groups[tests_order->num_groups]
                                .order_group_tests,
                        TESTS_ORDER_GROW_BY);

                tests_order->num_groups++;
            }
            tests_order->groups[tests_order->num_groups - 1].shuffling_type =
                FCS_NO_SHUFFLING;
            continue;
        }

        if (!(tests_order->groups[tests_order->num_groups - 1].num &
                (TESTS_ORDER_GROW_BY - 1)))
        {
            tests_order->groups[tests_order->num_groups - 1].order_group_tests =
                SREALLOC(tests_order->groups[tests_order->num_groups - 1]
                             .order_group_tests,
                    TESTS_ORDER_GROW_BY);
        }

        test_name[0] = string[i];
        tests_order->groups[tests_order->num_groups - 1].order_group_tests
            [tests_order->groups[tests_order->num_groups - 1].num++] =
            (fc_solve_string_to_test_num(test_name) % FCS_MOVE_FUNCS_NUM);

        is_start_group = FALSE;
    }
    if (i != len)
    {
#ifdef FCS_WITH_ERROR_STRS
        strcpy(error_string, "The Input string is too long.");
#endif
        return 4;
    }

    if (!tests_order->groups[tests_order->num_groups - 1].num)
    {
        tests_order->num_groups--;
        free(tests_order->groups[tests_order->num_groups].order_group_tests);
        tests_order->groups[tests_order->num_groups].order_group_tests = NULL;
    }

#ifdef FCS_WITH_ERROR_STRS
    error_string[0] = '\0';
#endif

    return 0;
}
