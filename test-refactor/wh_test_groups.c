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
 * test-refactor/wh_test_groups.c
 *
 * Portable group entry points. Each walks its slice of the
 * generated test registry (see wh_test_list.c) -- one array per
 * group. Tests whose feature gate is off are resolved by the
 * linker to the weak skip stub and surface as SKIPPED at
 * runtime.
 *
 * Output format follows wolfCrypt convention:
 *     whTest_Foo                    test passed!
 *     whTest_Bar                    test skipped
 *     whTest_Baz                    test FAILED (rc=-5)
 * with a final whTestGroup_Summary() tally.
 */

#include <string.h>

#include "wolfhsm/wh_settings.h"

#include "wh_test_common.h"
#include "wh_test_groups.h"
#include "wh_test_list.h"

#ifndef WH_TEST_RUNNER_PRINTF
#include <stdio.h>
#define WH_TEST_RUNNER_PRINTF printf
#endif

/* Column at which "test passed!" / "test skipped" / "test FAILED"
 * starts. Pad the test name with spaces to line these up. Pick
 * something a bit wider than the longest current name so future
 * tests don't force a reformat. */
#define WH_TEST_NAME_COL 40

/* Per-process tallies. The POSIX port runs the groups
 * sequentially (misc inline, then server, then client), so no
 * locking is needed. Ports that invoke groups from concurrent
 * threads must serialize the calls or add their own lock. */
static int whTestPassed  = 0;
static int whTestSkipped = 0;
static int whTestFailed  = 0;

static void whTest_PrintResult(const char* name, int rc)
{
    int pad = WH_TEST_NAME_COL - (int)strlen(name);
    if (pad < 1) {
        pad = 1;
    }
    WH_TEST_RUNNER_PRINTF("%s%*s", name, pad, "");

    if (rc == 0) {
        WH_TEST_RUNNER_PRINTF("test passed!\n");
        whTestPassed++;
    }
    else if (rc == WH_TEST_SKIPPED) {
        WH_TEST_RUNNER_PRINTF("test skipped\n");
        whTestSkipped++;
    }
    else {
        WH_TEST_RUNNER_PRINTF("test FAILED (rc=%d)\n", rc);
        whTestFailed++;
    }
}

static int whTest_Run(const whTestCase* tests, size_t count, void* ctx)
{
    int    overall = 0;
    size_t i;

    for (i = 0; i < count; i++) {
        int rc = tests[i].fn(ctx);
        whTest_PrintResult(tests[i].name, rc);

        if (rc != 0 && rc != WH_TEST_SKIPPED && overall == 0) {
            overall = rc;
        }
    }

    return overall;
}


int whTestGroup_Misc(void)
{
    return whTest_Run(whTestsMisc, whTestsMiscCount, NULL);
}

int whTestGroup_Server(whServerContext* server)
{
    return whTest_Run(whTestsServer, whTestsServerCount, server);
}

int whTestGroup_Client(whClientContext* client)
{
    return whTest_Run(whTestsClient, whTestsClientCount, client);
}

int whTestGroup_Summary(void)
{
    int total = whTestPassed + whTestSkipped + whTestFailed;

    if (whTestFailed == 0 && whTestSkipped == 0) {
        WH_TEST_RUNNER_PRINTF("All %d tests passed!\n", total);
    }
    else {
        WH_TEST_RUNNER_PRINTF(
            "%d passed, %d skipped, %d failed of %d tests\n",
            whTestPassed, whTestSkipped, whTestFailed, total);
    }

    return whTestFailed == 0 ? 0 : -1;
}
