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
 * test-refactor/wh_test_nvm_flash.h
 *
 * NVM flash test suite. Standalone -- owns a ramsim-backed
 * flash + NVM stack internally. Callers just invoke Run.
 */

#ifndef WH_TEST_NVM_FLASH_REFACTOR_H_
#define WH_TEST_NVM_FLASH_REFACTOR_H_

#include "wh_test_runner.h"

extern whTestSuite whTestSuite_NvmFlash;

#endif /* WH_TEST_NVM_FLASH_REFACTOR_H_ */
