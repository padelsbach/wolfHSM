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
#include "wh_test_runner.h"
#include "wh_test_groups.h"

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


int whTestGroup_Misc(void)
{
    WH_TEST_SUITE_RUN(&whTestSuite_FlashRamSim, NULL);
    WH_TEST_SUITE_RUN(&whTestSuite_NvmFlash, NULL);
    return 0;
}


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
