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
 * test-refactor/wh_test_runner.c
 *
 * Test runner implementation. See wh_test_runner.h for API docs.
 */

#include <stdio.h>
#include <stddef.h>

#include "wh_test_runner.h"

/*
 * Allow the build to redirect output for embedded targets that
 * lack stdout. Define WH_TEST_RUNNER_PRINTF before including
 * this file to override.
 */
#ifndef WH_TEST_RUNNER_PRINTF
#define WH_TEST_RUNNER_PRINTF printf
#endif


int whTestRunner_Run(const whTestSuite* suite, void* ctx)
{
    int ret = 0;
    int i   = 0;

    if (suite == NULL || suite->tests == NULL) {
        return -1;
    }

    WH_TEST_RUNNER_PRINTF("[SUITE] %s\n",
        suite->name != NULL ? suite->name : "(unnamed)");

    for (i = 0; suite->tests[i] != NULL; i++) {
        ret = suite->tests[i](ctx);
        if (ret != 0) {
            WH_TEST_RUNNER_PRINTF("[SUITE] %s: test %d FAILED"
                " (%d)\n", suite->name, i, ret);
            return ret;
        }
    }

    WH_TEST_RUNNER_PRINTF("[SUITE] %s: %d test(s) passed\n",
        suite->name, i);
    return 0;
}
