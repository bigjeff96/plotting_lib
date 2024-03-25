#pragma once

enum HOW_TO_ROUND { UP, DOWN, COUND_HOW_TO_ROUND };
enum ROUND_NUMBER_TYPE { BASE, GENERAL, ONE_DIGIT, COUNT_ROUND_NUMBER_TYPE };
double round_number(double num, ROUND_NUMBER_TYPE TYPE, HOW_TO_ROUND ROUNDING);
double round_down(double num, ROUND_NUMBER_TYPE TYPE);
double round_up(double num, ROUND_NUMBER_TYPE TYPE);
