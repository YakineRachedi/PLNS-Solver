#pragma once

#include <cstdio>
#include <cmath>

#include "logging.h"

/******************************************************************************
 * test_utils.h : Shared helper functions for unit test executables.
 *
 * Each test_*.cpp file is compiled into its own standalone executable, so
 * including this header in multiple test files does not cause any ODR
 * (One Definition Rule) violation at link time.
 *****************************************************************************/

/*
 * Automatically initialize the logging system before main() runs, so that
 * LOG_MSG() works out of the box in every test without requiring an explicit
 * log_init() call in each main(). Safe here because each test executable
 * has exactly one translation unit including this header.
 */
namespace detail {
struct LoggingInitializer {
    LoggingInitializer()  { log_init(nullptr); }
    ~LoggingInitializer() { log_fini(); }
};
inline LoggingInitializer g_logging_initializer;
}

/*
 * Global flag tracking whether every check() call in the current test
 * executable has passed so far.
 */
inline bool g_all_passed = true;

/******************************************************************************
 * Print the result of a single test condition and update g_all_passed.
 *
 * On failure, the message is also routed through LOG_MSG(), which prepends
 * the timestamp, source file, and line number of the check() call itself
 * (not of the failing assertion inside the tested code).
 *****************************************************************************/
inline void check(bool cond, const char *name) {
    if (cond) {
        LOG_MSG("[PASS] %s", name);
    } else {
        g_all_passed = false;
        LOG_MSG("[FAIL] %s", name);
    }
}

/******************************************************************************
 * Compare two floating-point values within a given absolute tolerance.
 *****************************************************************************/

inline bool almost_equal(double a, double b, double eps = 1e-9) {
    return std::abs(a - b) < eps;
}

