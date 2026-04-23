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
#ifndef WH_TEST_LIST_H_
#define WH_TEST_LIST_H_

#include <stddef.h>

/*
 * Empty annotation macros. Test functions tag themselves with one
 * of these so the pre-processing script (gen_test_list.py) can
 * discover them and emit wh_test_list.c. The macros expand to
 * nothing at compile time; they exist purely as textual markers.
 */
#define WH_TEST_MISC
#define WH_TEST_CLIENT
#define WH_TEST_SERVER

/*
 * Portable weak-linkage attribute. The generated wh_test_list.c
 * emits a weak stub for every discovered test; the real test's
 * strong definition (compiled in when its feature gate is on)
 * overrides the stub at link time. When the gate is off the test
 * source compiles to nothing and the weak stub wins, returning
 * WH_TEST_SKIPPED at runtime.
 *
 * Usage:
 *   WH_TEST_WEAK(foo) int foo(void* ctx) { ... }
 *
 * The name argument is only used by toolchains whose weak-symbol
 * mechanism is a pragma rather than a prefix attribute; others
 * ignore it. Spellings covered:
 *   GCC, Clang, armclang, armcc, TI  -> __attribute__((weak))
 *   IAR                               -> __weak
 *   Renesas CC-RH / CC-RL / CC-RX    -> _Pragma("weak <name>")
 *
 * If a port's toolchain isn't covered here, add it rather than
 * silently falling back -- a no-op WH_TEST_WEAK makes the stub
 * strong and will cause a multiple-definition link error (or,
 * worse, the stub wins over the real test).
 */
#define WH_TEST_WEAK_STR_(s) #s
#if defined(__GNUC__) || defined(__clang__) || \
    defined(__ARMCC_VERSION) || defined(__CC_ARM) || \
    defined(__TI_COMPILER_VERSION__)
#define WH_TEST_WEAK(name) __attribute__((weak))
#elif defined(__IAR_SYSTEMS_ICC__)
#define WH_TEST_WEAK(name) __weak
#elif defined(__CCRH__) || defined(__CCRL__) || defined(__CCRX__)
#define WH_TEST_WEAK(name) _Pragma(WH_TEST_WEAK_STR_(weak name))
#else
#error "WH_TEST_WEAK: add a weak-symbol spelling for this toolchain"
#endif

/*
 * Sentinel returned by the weak skip stubs. Picked to be distinct
 * from any error code a real test would return and from 0 (pass).
 */
#define WH_TEST_SKIPPED (-777)

/*
 * One-shot declaration of a test's forward prototype AND its weak
 * skip implementation. The generator emits `WH_TEST_DECL(name);`
 * for every discovered test. If the real test's feature gate is
 * on, its strong definition in the test's .c file overrides this
 * stub at link time; otherwise the stub wins and the test
 * surfaces as SKIPPED.
 *
 * The trailing "struct ... whTest_decl_dummy_" lets callers end
 * the invocation with a ';' (a bare "};" after a function body
 * is ill-formed under strict C90); the dummy tag swallows it.
 */
#define WH_TEST_DECL(name)                                               \
    WH_TEST_WEAK(name) int name(void* ctx)                               \
    { (void)ctx; return WH_TEST_SKIPPED; }                               \
    struct whTest_decl_dummy_##name

typedef int (*whTestFn)(void* ctx);

typedef struct whTestCase {
    const char* name;
    whTestFn    fn;
} whTestCase;

/*
 * Per-group registries, populated by the generated wh_test_list.c.
 * Each group gets its own array so the runner walks only the
 * relevant tests and doesn't carry a group tag per row.
 */
extern const whTestCase whTestsMisc[];
extern const size_t     whTestsMiscCount;

extern const whTestCase whTestsServer[];
extern const size_t     whTestsServerCount;

extern const whTestCase whTestsClient[];
extern const size_t     whTestsClientCount;

#endif /* WH_TEST_LIST_H_ */
