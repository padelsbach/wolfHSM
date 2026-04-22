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
 * test-refactor/wh_test_helpers_client_posix.h
 *
 * POSIX client-side init helper. Stands in for the normal
 * boot-time transport+client init a real firmware would do.
 * Wires the client onto the mem-transport buffers exposed by
 * the server helper and performs the CommInit handshake. The
 * port's main is responsible for running a server thread that
 * pumps HandleRequestMessage -- this helper does not touch the
 * server context.
 */

#ifndef WH_TEST_HELPERS_CLIENT_POSIX_H_
#define WH_TEST_HELPERS_CLIENT_POSIX_H_

#include "wolfhsm/wh_client.h"

/*
 * Initialize the client context plus transport state and
 * perform the CommInit handshake. The server helper must have
 * been initialized first (its transport config supplies the
 * shared request/response buffers) and a server thread must be
 * actively processing requests when this is called.
 */
int whTestHelperPosix_Client_Init(whClientContext* client);

int whTestHelperPosix_Client_Cleanup(whClientContext* client);

#endif /* WH_TEST_HELPERS_CLIENT_POSIX_H_ */
