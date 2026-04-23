# test-refactor

Prototype of the refactored wolfHSM test infrastructure.

## Key differences from `test/`

- **Groups** (`wh_test_groups.h/c`): three portable entry
  points (`whTestGroup_Misc` / `whTestGroup_Server` /
  `whTestGroup_Client`) that `main()` invokes. Each test
  receives the corresponding context pointer
  (`whClientContext*`, `whServerContext*`, or `NULL` for
  misc), pre-populated by the port.
- **Flat test registry**: tests are individual functions
  tagged with an annotation macro rather than gathered into
  suite structs. A pre-processing script
  (`gen_test_list.py`) scans the sources and emits
  `wh_test_list.c` with one registry array per group and a
  weak skip stub per test. No runner header, no per-suite
  tables, no hand-maintained test list.
- **App-owned init**: the port's `main()` brings up the server
  and client once at startup (mirroring real firmware boot) and
  hands the live contexts to the group functions. Tests no
  longer stand up their own fixtures.
- **Concurrent server + client during the client group**: the
  POSIX port runs server and client on separate threads. The
  server thread runs the server group first, then enters a
  `HandleRequestMessage` loop; the client thread runs the
  client group against that live server. No per-test
  sequencing/fan-in; tests just issue client calls and the
  server pumps them.
- **Separation of port and tests**: the multi-threaded POSIX
  port lives alongside the test code but is cleanly separated
  from it, serving as the reference implementation for other
  platforms/targets.

## Build and run (POSIX)

```
cd posix
make run
make run DEBUG=1
make run THREADSAFE=1    # enables stress test gate
```

The `check` (or `run`) target sends the full test output (including any
verbose prints from individual tests) to `test-suite.log` in
the POSIX build dir, and shows only the per-test result lines
and the final tally on the terminal -- mirroring the
`make check` convention in autotools-based wolfssl. Look at
`test-suite.log` for the verbose output on failure. The log
file is gitignored.

## Writing a test

Tag the function with one of `WH_TEST_MISC`, `WH_TEST_SERVER`,
or `WH_TEST_CLIENT`, and declare it with the context type that
matches its group -- no internal cast required:

```c
#include "wh_test_list.h"

WH_TEST_SERVER int whTest_CertVerify(whServerContext* server)
{
    /* ...use `server` directly... */
    return 0;
}

WH_TEST_CLIENT int whTest_Echo(whClientContext* client) { ... }

WH_TEST_MISC   int whTest_FlashWriteLock(void* ctx)
{
    (void)ctx;   /* misc group has no context; ctx is NULL */
    /* ... */
    return 0;
}
```

Why this works: the generator emits the weak skip stub as
`int name(void* ctx)` in `wh_test_list.c`, while the real
definition lives in a separate TU with its typed signature.
The compiler never sees both together, so there's no
conflicting-prototype error. The linker matches symbols by
name (strong overrides weak); the runner calls through
`int (*)(void*)`, and pointer arguments use the same ABI
regardless of pointee type, so the typed function sees the
context it expects.

Return `0` on success, any other non-zero value on failure.
Don't return `WH_TEST_SKIPPED` yourself -- that sentinel is
reserved for the weak stub that replaces a test whose feature
gate is off.

Name the function `whTest_CamelCase` to match the convention
in `test/` and the other tests in this directory.

After adding, renaming, or removing a test, regenerate the
registry:

```
cd test-refactor
python3 gen_test_list.py --output wh_test_list.c \
    misc server client-server
```

The POSIX Makefile regenerates automatically on build. CI
verifies the committed `wh_test_list.c` is in sync with the
annotations; see the `Verify wh_test_list.c is up to date`
step in `.github/workflows/build-and-test.yml`.

### Generated code and gen_test_list.py

The framework supports bottom-up test declaration: each test
associates itself with a group (client, server, or misc) via
its own annotation tag (e.g. `WH_TEST_CLIENT`), without
changing any upper layer. No header, no registration call,
no central switch statement -- the tag alone is enough.

To support this across toolchains without requiring link-time
tricks (ELF sections, `__start_`/`__stop_` symbols, etc.)
that aren't universally available, a small pre-processing
script scans the test sources for the tags and emits
`wh_test_list.c`. That file defines three registry arrays --
`whTestsMisc[]`, `whTestsServer[]`, `whTestsClient[]` -- plus
their counts, which the group runners in `wh_test_groups.c`
iterate directly.

**The generated file is checked in.** Two reasons: first,
some embedded toolchains don't have a convenient Python
runtime on the build host, and baking the generator into
the build would either force an extra dependency or a
hand-maintained fallback; committing the output sidesteps
both. Second, a committed registry makes test additions
reviewable in the diff -- a PR that adds a new test also
shows the corresponding registry change, which catches
forgotten regenerations and makes the CI "is this file in
sync" check pass by construction once the author runs the
script.

**Conditional compilation (`#ifdef`s) is handled by
link-time override of weak symbols, not by mirroring guards
in the registry.** Every discovered test gets an
unconditional entry in the registry and a weak stub that
returns `WH_TEST_SKIPPED`. When a test's feature gate is on,
the test source compiles to a strong symbol that overrides
the stub at link time and the real test runs. When the gate
is off, the test source compiles to an empty translation
unit, the stub wins, and the test surfaces as "test skipped"
at runtime. The generator never looks at `#if` context; all
the gating lives in the test's own source file, exactly once.

**Re-run the script whenever the set of tagged tests
changes:** adding a test, removing a test, or renaming a
tagged function. Changing a test's body or tweaking its
feature gate does *not* require regeneration -- only the
set of function names and their groups is captured in
`wh_test_list.c`. If the committed file and the annotations
drift, CI will flag it.

## Feature gating

Wrap the body of the test source in the feature's `#if` as
usual:

```c
#if defined(WOLFHSM_CFG_CERTIFICATE_MANAGER) \
    && !defined(WOLFHSM_CFG_NO_CRYPTO)

WH_TEST_SERVER int whTest_CertVerify(void* ctx) { ... }

#endif
```

The generator ignores preprocessor context and always emits
an entry and a weak stub. If the gate is off, the real test's
TU is empty, the stub's `WH_TEST_SKIPPED` propagates at
runtime, and the test shows up as `test skipped` in the
output rather than silently vanishing.

## Tests implemented so far

| Test                             | Group          | Description                                                                     |
|----------------------------------|----------------|---------------------------------------------------------------------------------|
| `whTest_FlashWriteLock`          | misc           | Flash write-lock behavior                                                       |
| `whTest_FlashEraseProgramVerify` | misc           | Flash erase, program, verify, blank-check                                       |
| `whTest_FlashUnitOps`            | misc           | Flash unit ops                                                                  |
| `whTest_NvmAddOverwriteDestroy`  | misc           | NVM add / overwrite / destroy / reclaim                                         |
| `whTest_CertVerify`              | server         | Server-side cert add / verify / chain / erase                                   |
| `whTest_Echo`                    | client         | Echo round-trip                                                                 |
| `whTest_ServerInfo`              | client         | Server info query                                                               |
| `whTest_CryptoSha256`            | client         | SHA256 via server                                                               |
| `whTest_CryptoAes`               | client         | AES-CBC via server                                                              |
| `whTest_CryptoEcc256`            | client         | ECC256 via server                                                               |
| ThreadSafe Stress (POSIX only)   | --             | Phased multi-thread contention, invoked directly by the POSIX port (not in the registry) |

## Remaining tests to port

| Test              | Group          | Description                                                     |
|-------------------|----------------|-----------------------------------------------------------------|
| Comm              | client-server  | Transport layer (mem, TCP, SHM)                                 |
| Crypto (rest)     | client-server  | RSA, CMAC, curve25519, ed25519, etc.                            |
| Crypto Affinity   | client-server  | Device ID operation routing                                     |
| SHE               | client-server  | SHE key load, crypto, secure boot                               |
| Keywrap           | client-server  | Key wrap/unwrap operations                                      |
| Log               | misc           | Logging frontend, ringbuf, POSIX file backends                  |
| Lock              | misc           | Lock primitives with POSIX backend                              |
| DMA               | misc           | DMA address translation and allow-list                          |
| Server Img Mgr    | server         | Image manager verify/install/erase                              |
| Timeout           | client-server  | POSIX timeout enforcement                                       |
| wolfCrypt Test    | client-server  | wolfCrypt test suite via wolfHSM transport                      |
| MultiClient       | client-server  | 2 CS pairs, shared NVM, global/local key isolation              |

## Platforms requiring update

Each platform with test infrastructure needs its own
`wh_test_helpers_server_<port>.c`,
`wh_test_helpers_client_<port>.c`, and
`wh_test_main_<port>.c` (see "Porting" below).

| Platform    | Vendor    | Test files                                                     |
|-------------|-----------|----------------------------------------------------------------|
| POSIX       | wolfSSL   | `test-refactor/posix/` (done)                                  |
| Bernina     | STMicro   | `bernina-server/src/bh_test.c`                                 |
| SR6         | STMicro   | (no test files found)                                          |
| TC3xx       | Infineon  | `port/client/wolfhsm_tests.c`, `port/server/ccb_tests.c`       |
| RH850 F1KM  | Renesas   | `rh850_test2_1/`, `rh850_test2_2/`                             |
| PIC32CZ     | Microchip | `czhsm-client/tests/`, `czhsm-server/`                         |
| TDA4VH      | TI        | (no test files found)                                          |
| New Eagle   | Customer  | (no test files found)                                          |

## File layout

```
Portable (ships in wolfHSM):
  wh_test_list.h              - annotation markers, whTestCase,
                                WH_TEST_WEAK, WH_TEST_DECL,
                                WH_TEST_SKIPPED, extern decls for
                                whTestsMisc/Server/Client[]
  wh_test_list.c              - GENERATED registry (three per-group
                                arrays + weak skip stubs). Run
                                gen_test_list.py to rebuild.
  wh_test_groups.h/c          - Misc/Server/Client entry points,
                                runner, pass/skip/fail counters,
                                whTestGroup_Summary()
  gen_test_list.py            - registry generator
  server/wh_test_*.c          - server-only test modules
  client-server/wh_test_*.c   - client-server test modules
  misc/wh_test_*.c            - standalone test modules

Platform-specific (one directory per platform, e.g. posix/):
  <port>/wh_test_helpers_server_<port>.h/c - server bringup
  <port>/wh_test_helpers_client_<port>.h/c - client bringup
  <port>/wh_test_main_<port>.c             - init, group
                                             dispatch, reset
                                             hooks, summary
  <port>/Makefile                          - build rules
```

## Toolchain support

The weak-symbol attribute in `WH_TEST_WEAK(name)` is mapped
per toolchain in `wh_test_list.h`:

| Toolchain(s)                          | Spelling                         |
|---------------------------------------|----------------------------------|
| GCC, Clang, armclang, armcc, TI       | `__attribute__((weak))`          |
| IAR                                   | `__weak`                         |
| Renesas CC-RH / CC-RL / CC-RX         | `_Pragma("weak <name>")`         |

Other toolchains trigger a `#error` -- add a case rather than
falling back silently, since a no-op expansion would make the
weak stub strong and either cause a multiple-definition link
error or (worse) let the stub win over the real test.

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
   - Calls `whTestGroup_Summary()` at the end to print the
     final wolfCrypt-style tally and return a non-zero exit
     code on failure.
   - Implements `whTestGroup_ResetServer` and/or
     `whTestGroup_ResetClient` -- called between tests to
     scrub persistent state.
3. Add the portable `.c` files and your port files to your
   build system. Either regenerate `wh_test_list.c` from the
   build (`python3 gen_test_list.py ...`) or rely on the
   committed copy.

See `wh_test_main_posix.c` and the two `*_posix.c` helpers as
a reference implementation.
