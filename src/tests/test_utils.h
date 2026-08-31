#pragma once

#include <cstdio>
#include <cmath>

/******************************************************************************
 * test_utils.h : Shared helper functions for unit test executables.
 *
 * Each test_*.cpp file is compiled into its own standalone executable, so
 * including this header in multiple test files does not cause any ODR
 * (One Definition Rule) violation at link time.
 *****************************************************************************/

/*
 * Global flag tracking whether every check() call in the current test
 * executable has passed so far.
 */
inline bool g_all_passed = true;

/******************************************************************************
 * Print the result of a single test condition and update g_all_passed.
 *****************************************************************************/
inline void check(bool cond, const char *name) {
    printf("[%s] %s\n", cond ? "PASS" : "FAIL", name);

    if (!cond)
        g_all_passed = false;
}

/******************************************************************************
 * Compare two floating-point values within a given absolute tolerance.
 *****************************************************************************/
inline bool almost_equal(double a, double b, double eps = 1e-9) {
    return std::abs(a - b) < eps;
}

