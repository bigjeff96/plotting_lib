#include "rounding_nums.h"
#include <math.h>
#include <stdlib.h>
#include <values.h>

#define internal static

#define BETWEEN(a, b, c)                                                \
    ({                                                                  \
        __typeof__(a) __a = (a);                                        \
        __typeof__(b) __b = (b);                                        \
        __typeof__(c) __c = (c);                                        \
        (__a >= __b && __a <= __c) || (__a >= __c && __a <= __b);       \
    })


double round_number(double num, ROUND_NUMBER_TYPE TYPE, HOW_TO_ROUND ROUNDING) {
    // Returns the closest round number that is bigger than num
    double *round_nums = NULL;
    int total_round_nums;
    const int base_count      = 5;
    const int general_count   = 19;
    const int one_digit_count = 10;
    if (TYPE == BASE) {
        total_round_nums             = base_count;
        round_nums                   = (double *)malloc(total_round_nums * sizeof(double));
        double base_nums[base_count] = {0., 1., 2., 2.5, 5.};
        for (int i = 0; i < total_round_nums; ++i) {
            round_nums[i] = base_nums[i];
        }
    }
    if (TYPE == GENERAL) {
        total_round_nums                = general_count;
        round_nums                      = (double *)malloc(total_round_nums * sizeof(double));
        double base_nums[general_count] = {0.,  1., 1.5, 2., 2.5, 3., 3.5, 4., 4.5, 5.,
                                           5.5, 6., 6.5, 7., 7.5, 8., 8.5, 9., 9.5};
        for (int i = 0; i < total_round_nums; ++i) {
            round_nums[i] = base_nums[i];
        }
    }
    if (TYPE == ONE_DIGIT) {
        total_round_nums                  = one_digit_count;
        round_nums                        = (double *)malloc(total_round_nums * sizeof(double));
        double base_nums[one_digit_count] = {0., 1., 2., 3., 4., 5., 6., 7., 8., 9.};
        for (int i = 0; i < total_round_nums; ++i) {
            round_nums[i] = base_nums[i];
        }
    }
    
    int magnetude_of_num = 0;
    if (!BETWEEN(num, -1e-8, 1e-8))
      magnetude_of_num = (int)(floor(log10(abs(num))));
    
    double sign_of_num   = num < 0. ? -1. : 1.;

    double smallest_diffs_per_round_num[total_round_nums];
    int magnetudes_per_round_num[total_round_nums];

    for (int i = 0; i < total_round_nums; ++i) {
        double smallest_diff = MAXDOUBLE;
        double magnetude_of_smalles_diff;
        double current_diff, current_diff_abs;
        double round_num = round_nums[i];
        for (int range_of_magnetude = -1; range_of_magnetude <= 1; ++range_of_magnetude) {
            current_diff     = num - sign_of_num * round_num * pow(10., magnetude_of_num + range_of_magnetude);
            current_diff_abs = abs(current_diff);

            if (ROUNDING == UP) {
                if ((current_diff < 0.001 * pow(10., magnetude_of_num)) && (current_diff_abs < smallest_diff)) {
                    smallest_diff             = current_diff_abs;
                    magnetude_of_smalles_diff = range_of_magnetude + magnetude_of_num;
                }
            } else {
                if (current_diff < 0.)
                    continue;

                if (current_diff_abs < smallest_diff) {
                    smallest_diff             = current_diff_abs;
                    magnetude_of_smalles_diff = range_of_magnetude + magnetude_of_num;
                }
            }
        }
        smallest_diffs_per_round_num[i] = smallest_diff;
        magnetudes_per_round_num[i]     = magnetude_of_smalles_diff;
    }
    // sort the diffs
    int index_of_smallest_diff = 0;
    for (int i = 1; i < total_round_nums; ++i) {
        if (smallest_diffs_per_round_num[index_of_smallest_diff] > smallest_diffs_per_round_num[i])
            index_of_smallest_diff = i;
    }
    double best_round_num = round_nums[index_of_smallest_diff];
    free(round_nums);
    return sign_of_num * best_round_num * pow(10., (int)magnetudes_per_round_num[index_of_smallest_diff]);
}

double round_down(double num, ROUND_NUMBER_TYPE TYPE) {
    double result = round_number(num, TYPE, DOWN);
    return result;
}

double round_up(double num, ROUND_NUMBER_TYPE TYPE) {
    double result = round_number(num, TYPE, UP);
    return result;
}
