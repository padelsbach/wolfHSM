# test-refactor

Prototype of the refactored wolfHSM test infrastructure.

## Key differences from test/

- **Runner** (`wh_test_runner.h/c`): generic suite executor.
  Each suite is a name + NULL-terminated test array, run
  either via `whTestRunner_Run` (suite owns its setup/cleanup)
  or `whTestRunner_RunWithCtx` (caller provides the live
  context). Group functions use the latter.
- **App-owned init**: the port's main() brings up the server
  and client once at startup (mirroring real firmware boot)
  and hands the live contexts to the group functions. Suites
  no longer stand up their own fixtures.
- **Port helpers** (`wh_test_helpers_server_<port>.h/c`,
  `wh_test_helpers_client_<port>.h/c`): per-port files that
  stand in for what a real target does at boot -- configure
  flash, init NVM/crypto, wire up a transport, bring up the
  server or client context.
- **Groups** (`wh_test_groups.h/c`): three portable entry
  points (Misc/Server/Client) that main invokes. Each runs
  its gated suites and calls the caller-implemented reset
  hook between them.
- **Threaded driver**: the POSIX port's main runs the server
  and client on separate threads. The server thread runs the
  server-only group first, then enters a `HandleRequestMessage`
  loop; the client thread runs the client-only group against
  the live server. Ports that already split server and client
  onto different cores/tasks do the same thing natively.
- **Platform split**: platform-specific code is isolated in
  `wh_test_helpers_server_<port>.c`,
  `wh_test_helpers_client_<port>.c`, and
  `wh_test_main_<port>.c`. Test modules and groups are
  identical on all platforms.

## Suites implemented so far

| Suite | Group | Description |
|-------|-------|-------------|
| Flash RamSim | misc | Write-lock, erase, program, verify, blank-check |
| NVM Flash | misc | Flash unit ops, NVM add/overwrite/destroy/reclaim |
| Cert | server | Server-side cert add/verify/chain/erase |
| ClientServer | client-server | Echo round-trip, server info query |
| ThreadSafe Stress | client-server | Phased multi-thread contention (unchanged internals) |

## Remaining tests to port

| Suite | Group | Description |
|-------|-------|-------------|
| Comm | client-server | Transport layer (mem, TCP, SHM) |
| Crypto | client-server | AES, RSA, ECC, CMAC, curve25519, ed25519, etc. |
| Crypto Affinity | client-server | Device ID operation routing |
| SHE | client-server | Secure Hardware Extension key load, crypto, secure boot |
| Keywrap | client-server | Key wrap/unwrap operations |
| Log | misc | Logging frontend, ringbuf, POSIX file backends |
| Lock | misc | Lock primitives with POSIX backend |
| DMA | misc | DMA address translation and allow-list |
| Server Img Mgr | server | Image manager verify/install/erase |
| Timeout | client-server | POSIX timeout enforcement |
| wolfCrypt Test | client-server | wolfCrypt test suite via wolfHSM transport |
| MultiClient | client-server | 2 CS pairs, shared NVM, global/local key isolation |

## Platforms requiring update

Each platform with test infrastructure needs its own
`wh_test_helpers_server_<port>.c`,
`wh_test_helpers_client_<port>.c`, and
`wh_test_main_<port>.c` (see "Porting" below).

| Platform | Vendor | Test files |
|----------|--------|------------|
| POSIX | wolfSSL | `test-refactor/posix/` (done) |
| Bernina | STMicro | `bernina-server/src/bh_test.c` |
| SR6 | STMicro | (no test files found) |
| TC3xx | Infineon | `port/client/wolfhsm_tests.c`, `port/server/ccb_tests.c` |
| RH850 F1KM | Renesas | `rh850_test2_1/`, `rh850_test2_2/` |
| PIC32CZ | Microchip | `czhsm-client/tests/`, `czhsm-server/` |
| TDA4VH | TI | (no test files found) |
| New Eagle | Customer | (no test files found) |

## File layout

```
Portable (ships in wolfHSM):
  wh_test_runner.h/c         - suite runner
  wh_test_groups.h/c         - Misc/Server/Client entry points
  server/wh_test_*.c/h       - server-only test modules
  client-server/wh_test_*.c/h - client-server test modules
  misc/wh_test_*.c/h         - standalone test modules

Platform-specific (one directory per platform, e.g. posix/):
  <port>/wh_test_helpers_misc_<port>.h/c   - misc fixtures
  <port>/wh_test_helpers_server_<port>.h/c - server bringup
  <port>/wh_test_helpers_client_<port>.h/c - client bringup
  <port>/wh_test_main_<port>.c             - init, group
                                             dispatch, reset
                                             hooks
  <port>/Makefile                          - build rules
```

## Porting to other platforms

1. Implement the init helpers for the side(s) the target
   needs. These stand in for what your firmware's normal
   boot flow already does -- if it's simpler to call your
   existing init code directly from main, that works too:
   - `whTestHelperPosix_Server_Init/Cleanup` (reference):
     bring up flash/NVM/crypto/transport/server.
   - `whTestHelperPosix_Client_Init/Cleanup` (reference):
     bring up client comm + handshake. On single-process
     targets, the server runs in its own thread and pumps
     `HandleRequestMessage` itself.
2. Provide a `main()` that:
   - Calls `whTestGroup_Misc()` for standalone tests.
   - Brings up the server/client contexts once.
   - Calls `whTestGroup_Server(&server)` and/or
     `whTestGroup_Client(&client)` with the live handles.
   - Tears the contexts down.
   - Implements `whTestGroup_ResetServer` and/or
     `whTestGroup_ResetClient` -- called between suites to
     scrub persistent state.
3. Add the portable `.c` files and your port files to your
   build system.

See `wh_test_main_posix.c` and the two `*_posix.c` helpers as
a reference implementation.

## Build and run (POSIX)

```
cd posix
make run
make run DEBUG=1
make run THREADSAFE=1    # enables stress test gate
```
