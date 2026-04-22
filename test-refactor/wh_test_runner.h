/*
 * Copyright (C) 2026 wolfSSL Inc.
 *
 * This file is part of wolfHSM.
 *
 * wolfHSM is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * wolfHSM is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with wolfHSM.  If not, see <http://www.gnu.org/licenses/>.
 */
/*
 * test-refactor/wh_test_runner.h
 *
 * Test runner types and API. A suite is just a name plus a
 * NULL-terminated array of test functions; any fixture the
 * tests need is owned by the caller (the group function) and
 * passed through as the ctx argument. Generic -- knows
 * nothing about wolfHSM.
 */

#ifndef WH_TEST_RUNNER_H_
#define WH_TEST_RUNNER_H_

/* Test function: receives the per-suite context and returns 0
 * on success. */
typedef int (*whTestFn)(void* ctx);

typedef struct {
    const char* name;    /* suite name for output */
    whTestFn*   tests;   /* NULL-terminated array of tests */
} whTestSuite;


/*
 * Run a suite against a caller-provided context. Stops on the
 * first test failure. Returns 0 if every test succeeds.
 */
int whTestRunner_Run(const whTestSuite* suite, void* ctx);


/*
 * Convenience macro: run a suite and return from the calling
 * function on failure. Intended for use inside group entry
 * points.
 */
#define WH_TEST_SUITE_RUN(suite, ctx)                     \
    do {                                                  \
        int _wh_rc = whTestRunner_Run((suite), (ctx));    \
        if (_wh_rc != 0) {                                \
            return _wh_rc;                                \
        }                                                 \
    } while (0)


/*
 * Suite definition macro. Suites with their own fixtures
 * expose an init/cleanup pair and let the group function
 * drive them.
 */
#define WH_TEST_SUITE(sname, tfns)                        \
    {                                                     \
        .name  = (sname),                                 \
        .tests = (tfns),                                  \
    }


#endif /* WH_TEST_RUNNER_H_ */
