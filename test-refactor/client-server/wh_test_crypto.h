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
 * test-refactor/wh_test_crypto.h
 *
 * Basic crypto test suite (minimal SHA256 round-trip).
 */

#ifndef WH_TEST_CRYPTO_REFACTOR_H_
#define WH_TEST_CRYPTO_REFACTOR_H_

#include "wolfhsm/wh_settings.h"

#if !defined(WOLFHSM_CFG_NO_CRYPTO)

#include "wh_test_runner.h"

extern whTestSuite whTestSuite_Crypto;

#endif /* !WOLFHSM_CFG_NO_CRYPTO */

#endif /* WH_TEST_CRYPTO_REFACTOR_H_ */
