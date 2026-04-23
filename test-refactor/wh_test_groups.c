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
 * Portable group entry points. Each function runs every
 * suite that belongs to its group, gated by the applicable
 * compile-time config flags. Ports call these from main.
 */

#include "wolfhsm/wh_settings.h"

#include "wh_test_common.h"
#include "wh_test_groups.h"

#if 0
/* Misc group */
#include "wh_test_flash_ramsim.h"
#include "wh_test_nvm_flash.h"

/* Server group */
#if defined(WOLFHSM_CFG_CERTIFICATE_MANAGER) \
    && !defined(WOLFHSM_CFG_NO_CRYPTO)
#include "wh_test_cert.h"
#endif

/* Client group */
#include "wh_test_echo.h"
#include "wh_test_server_info.h"

#if !defined(WOLFHSM_CFG_NO_CRYPTO)
#include "wh_test_crypto.h"
#endif
#endif

/*
 * Allow the build to redirect output for embedded targets that
 * lack stdout. Define WH_TEST_RUNNER_PRINTF before including
 * this file to override.
 */
#ifndef WH_TEST_RUNNER_PRINTF
#include <stdio.h>
#define WH_TEST_RUNNER_PRINTF printf
#endif

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#endif

typedef enum {
    WH_TEST_GROUP_MISC,
    WH_TEST_GROUP_SERVER,
    WH_TEST_GROUP_CLIENT,
} whTestGroup;

typedef int (*whTestFn)(void* ctx);
typedef struct whTestCase {
    const char*     name;
    whTestFn        fn;
    whTestGroup     group;
} whTestCase;

#define WH_TEST_MISC_TEST(fn)   { #fn, fn, WH_TEST_GROUP_MISC }
#define WH_TEST_SERVER_TEST(fn) { #fn, (whTestFn)fn, WH_TEST_GROUP_SERVER }
#define WH_TEST_CLIENT_TEST(fn) { #fn, (whTestFn)fn, WH_TEST_GROUP_CLIENT }

/* Pull in the list of tests (whTests) */
#define INCLUDE_WH_TEST_LIST
#include "wh_test_list.c"

static const size_t whTestCount = ARRAY_SIZE(whTests);
static int whTestResults[ARRAY_SIZE(whTests)] = {0};

static int whTest_Run(const whTestCase* tests, size_t test_count, whTestGroup group, void* ctx)
{
    const char* test_name = NULL;
    size_t i = 0;

    if (tests == NULL) {
        return -1;
    }

    /* Print test group name */
    switch (group) {
        case WH_TEST_GROUP_MISC:
            WH_TEST_RUNNER_PRINTF("[test group] MISC\n");
            break;
        case WH_TEST_GROUP_SERVER:
            WH_TEST_RUNNER_PRINTF("[test group] SERVER\n");
            break;
        case WH_TEST_GROUP_CLIENT:
            WH_TEST_RUNNER_PRINTF("[test group] CLIENT\n");
            break;
        default:
            WH_TEST_RUNNER_PRINTF("[test group] UNKNOWN\n");
            break;
    }

    for (i = 0; i < test_count; i++) {
        if (tests[i].group != group) {
            continue;
        }

        test_name = tests[i].name != NULL ? tests[i].name : "(unnamed)";

        WH_TEST_RUNNER_PRINTF("[test case] %s\n",
            test_name);

        whTestResults[i] = tests[i].fn(ctx);

        WH_TEST_RUNNER_PRINTF("[test case] %s: %s\n",
            test_name, whTestResults[i] == 0 ? "PASSED" : "FAILED");
    }

    return 0;
}


int whTestGroup_Misc(void)
{
    whTest_Run(whTests, whTestCount, WH_TEST_GROUP_MISC, NULL);
    return 0;
}

int whTestGroup_Server(whServerContext* server)
{
    whTest_Run(whTests, whTestCount, WH_TEST_GROUP_SERVER, server);
    return 0;
}

int whTestGroup_Client(whClientContext* client)
{
    whTest_Run(whTests, whTestCount, WH_TEST_GROUP_CLIENT, client);
    return 0;
}


#if 0 
int whTestGroup_Server(whServerContext* server)
{
#if defined(WOLFHSM_CFG_CERTIFICATE_MANAGER) \
    && !defined(WOLFHSM_CFG_NO_CRYPTO)
    WH_TEST_SUITE_RUN(&whTestSuite_Cert, server);
    WH_TEST_RETURN_ON_FAIL(whTestGroup_ResetServer(server));
#else
    (void)server;
#endif
    return 0;
}


int whTestGroup_Client(whClientContext* client)
{
    WH_TEST_SUITE_RUN(&whTestSuite_Echo, client);
    WH_TEST_RETURN_ON_FAIL(whTestGroup_ResetClient(client));

    WH_TEST_SUITE_RUN(&whTestSuite_ServerInfo, client);
    WH_TEST_RETURN_ON_FAIL(whTestGroup_ResetClient(client));

#if !defined(WOLFHSM_CFG_NO_CRYPTO)
    WH_TEST_SUITE_RUN(&whTestSuite_Crypto, client);
    WH_TEST_RETURN_ON_FAIL(whTestGroup_ResetClient(client));
#endif
    return 0;
}
#endif