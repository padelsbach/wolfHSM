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
 * test-refactor/client-server/wh_test_crypto_sha3.c
 *
 * SHA3-224/256/384/512 and SHAKE128/256 offload.
 *
 * Every case hashes the same input twice, once with INVALID_DEVID so wolfCrypt
 * runs it locally and once on the server, and requires the two to agree. The
 * software path is the oracle: what is under test is the offload, not Keccak.
 */

#include "wolfhsm/wh_settings.h"

#if !defined(WOLFHSM_CFG_NO_CRYPTO)

#include <stdint.h>
#include <string.h>

#include "wolfssl/wolfcrypt/settings.h"
#include "wolfssl/wolfcrypt/types.h"
#include "wolfssl/wolfcrypt/sha3.h"
#include "wolfssl/wolfcrypt/error-crypt.h"

#include "wolfhsm/wh_error.h"
#include "wolfhsm/wh_client.h"
#include "wolfhsm/wh_client_crypto.h"

#include "wh_test_common.h"
#include "wh_test_list.h"

#if defined(WOLFSSL_SHA3) || defined(WOLFSSL_SHAKE128) || \
    defined(WOLFSSL_SHAKE256)

/* Long enough to span several comm-buffer messages at any supported size */
#define SHA3_TEST_MAX_IN 20000u
/* Larger than any response can carry, so a SHAKE this long must fall back to
 * software rather than be truncated */
#define SHA3_TEST_LONG_OUT (WOLFHSM_CFG_COMM_DATA_LEN + 1024u)

static uint8_t sha3TestIn[SHA3_TEST_MAX_IN];
static uint8_t sha3TestOutDev[SHA3_TEST_LONG_OUT];
static uint8_t sha3TestOutSw[SHA3_TEST_LONG_OUT];

static int _Sha3TestInit(wc_Sha3* sha, int hashType, int devId)
{
    switch (hashType) {
#ifdef WOLFSSL_SHA3
        case WC_HASH_TYPE_SHA3_224:
            return wc_InitSha3_224(sha, NULL, devId);
        case WC_HASH_TYPE_SHA3_256:
            return wc_InitSha3_256(sha, NULL, devId);
        case WC_HASH_TYPE_SHA3_384:
            return wc_InitSha3_384(sha, NULL, devId);
        case WC_HASH_TYPE_SHA3_512:
            return wc_InitSha3_512(sha, NULL, devId);
#endif
#ifdef WOLFSSL_SHAKE128
        case WC_HASH_TYPE_SHAKE128:
            return wc_InitShake128(sha, NULL, devId);
#endif
#ifdef WOLFSSL_SHAKE256
        case WC_HASH_TYPE_SHAKE256:
            return wc_InitShake256(sha, NULL, devId);
#endif
        default:
            return BAD_FUNC_ARG;
    }
}

static int _Sha3TestUpdate(wc_Sha3* sha, int hashType, const uint8_t* in,
                           uint32_t inSz)
{
    switch (hashType) {
#ifdef WOLFSSL_SHA3
        case WC_HASH_TYPE_SHA3_224:
            return wc_Sha3_224_Update(sha, in, inSz);
        case WC_HASH_TYPE_SHA3_256:
            return wc_Sha3_256_Update(sha, in, inSz);
        case WC_HASH_TYPE_SHA3_384:
            return wc_Sha3_384_Update(sha, in, inSz);
        case WC_HASH_TYPE_SHA3_512:
            return wc_Sha3_512_Update(sha, in, inSz);
#endif
#ifdef WOLFSSL_SHAKE128
        case WC_HASH_TYPE_SHAKE128:
            return wc_Shake128_Update(sha, in, inSz);
#endif
#ifdef WOLFSSL_SHAKE256
        case WC_HASH_TYPE_SHAKE256:
            return wc_Shake256_Update(sha, in, inSz);
#endif
        default:
            return BAD_FUNC_ARG;
    }
}

static int _Sha3TestFinal(wc_Sha3* sha, int hashType, uint8_t* out,
                          uint32_t outSz)
{
    /* Only a SHAKE consumes a length; the fixed-digest variants do not */
    (void)outSz;

    switch (hashType) {
#ifdef WOLFSSL_SHA3
        case WC_HASH_TYPE_SHA3_224:
            return wc_Sha3_224_Final(sha, out);
        case WC_HASH_TYPE_SHA3_256:
            return wc_Sha3_256_Final(sha, out);
        case WC_HASH_TYPE_SHA3_384:
            return wc_Sha3_384_Final(sha, out);
        case WC_HASH_TYPE_SHA3_512:
            return wc_Sha3_512_Final(sha, out);
#endif
#ifdef WOLFSSL_SHAKE128
        case WC_HASH_TYPE_SHAKE128:
            return wc_Shake128_Final(sha, out, outSz);
#endif
#ifdef WOLFSSL_SHAKE256
        case WC_HASH_TYPE_SHAKE256:
            return wc_Shake256_Final(sha, out, outSz);
#endif
        default:
            return BAD_FUNC_ARG;
    }
}

static uint32_t _Sha3TestRate(int hashType)
{
    switch (hashType) {
#ifdef WOLFSSL_SHA3
        case WC_HASH_TYPE_SHA3_224:
            return WC_SHA3_224_BLOCK_SIZE;
        case WC_HASH_TYPE_SHA3_256:
            return WC_SHA3_256_BLOCK_SIZE;
        case WC_HASH_TYPE_SHA3_384:
            return WC_SHA3_384_BLOCK_SIZE;
        case WC_HASH_TYPE_SHA3_512:
            return WC_SHA3_512_BLOCK_SIZE;
#endif
#ifdef WOLFSSL_SHAKE128
        case WC_HASH_TYPE_SHAKE128:
            return WC_SHA3_128_COUNT * 8u;
#endif
#ifdef WOLFSSL_SHAKE256
        case WC_HASH_TYPE_SHAKE256:
            return WC_SHA3_256_COUNT * 8u;
#endif
        default:
            return 0;
    }
}

static const char* _Sha3TestName(int hashType)
{
    switch (hashType) {
#ifdef WOLFSSL_SHA3
        case WC_HASH_TYPE_SHA3_224:
            return "SHA3-224";
        case WC_HASH_TYPE_SHA3_256:
            return "SHA3-256";
        case WC_HASH_TYPE_SHA3_384:
            return "SHA3-384";
        case WC_HASH_TYPE_SHA3_512:
            return "SHA3-512";
#endif
#ifdef WOLFSSL_SHAKE128
        case WC_HASH_TYPE_SHAKE128:
            return "SHAKE128";
#endif
#ifdef WOLFSSL_SHAKE256
        case WC_HASH_TYPE_SHAKE256:
            return "SHAKE256";
#endif
        default:
            return "?";
    }
}

/* Hash inLen bytes, feeding the update in chunks of chunkSz (0 = all at once)
 * so the multi-update path and the partial-block buffering are exercised. */
static int _Sha3TestHash(int devId, int hashType, const uint8_t* in,
                         uint32_t inLen, uint32_t chunkSz, uint8_t* out,
                         uint32_t outSz)
{
    wc_Sha3  sha[1];
    int      ret;
    uint32_t done = 0;

    ret = _Sha3TestInit(sha, hashType, devId);
    if (ret != 0) {
        return ret;
    }

    while ((ret == 0) && (done < inLen)) {
        uint32_t remaining = inLen - done;
        uint32_t chunk     = (chunkSz == 0) ? remaining : chunkSz;

        if (chunk > remaining) {
            chunk = remaining;
        }
        ret = _Sha3TestUpdate(sha, hashType, in + done, chunk);
        done += chunk;
    }

    if (ret == 0) {
        ret = _Sha3TestFinal(sha, hashType, out, outSz);
    }

    wc_Sha3_256_Free(sha);
    return ret;
}

/* Run one case on both paths and require them to agree. */
static int _Sha3TestCompare(int devId, int hashType, uint32_t inLen,
                            uint32_t chunkSz, uint32_t outSz)
{
    int ret;

    memset(sha3TestOutDev, 0, outSz);
    memset(sha3TestOutSw, 0xA5, outSz);

    ret = _Sha3TestHash(INVALID_DEVID, hashType, sha3TestIn, inLen, chunkSz,
                        sha3TestOutSw, outSz);
    if (ret != 0) {
        WH_ERROR_PRINT("%s software hash failed: %d\n", _Sha3TestName(hashType),
                       ret);
        return ret;
    }

    ret = _Sha3TestHash(devId, hashType, sha3TestIn, inLen, chunkSz,
                        sha3TestOutDev, outSz);
    if (ret != 0) {
        WH_ERROR_PRINT("%s device hash failed (in %u chunk %u out %u): %d\n",
                       _Sha3TestName(hashType), (unsigned)inLen,
                       (unsigned)chunkSz, (unsigned)outSz, ret);
        return ret;
    }

    if (memcmp(sha3TestOutDev, sha3TestOutSw, outSz) != 0) {
        WH_ERROR_PRINT("%s device and software differ (in %u chunk %u "
                       "out %u)\n",
                       _Sha3TestName(hashType), (unsigned)inLen,
                       (unsigned)chunkSz, (unsigned)outSz);
        return WH_ERROR_ABORTED;
    }
    return WH_ERROR_OK;
}

static int _Sha3TestVariant(whClientContext* ctx, int hashType)
{
    int      devId = WH_CLIENT_DEVID(ctx);
    uint32_t rate  = _Sha3TestRate(hashType);
    uint32_t digestSz;
    uint32_t i;
    uint32_t j;
    int      ret = WH_ERROR_OK;
    /* Sizes around the rate boundary, plus one long enough to need several
     * messages, so both the state-carrying and one-shot paths are hit. */
    const uint32_t inLens[] = {0u,          1u,          rate - 1u,
                               rate,        rate + 1u,   2u * rate,
                               2u * rate + 7u, SHA3_TEST_MAX_IN};
    /* All at once, then patterns that leave partial blocks buffered */
    const uint32_t chunks[] = {0u, 1u, 7u, rate, rate + 1u};

    switch (hashType) {
#ifdef WOLFSSL_SHAKE128
        case WC_HASH_TYPE_SHAKE128:
            digestSz = 32u;
            break;
#endif
#ifdef WOLFSSL_SHAKE256
        case WC_HASH_TYPE_SHAKE256:
            digestSz = 64u;
            break;
#endif
        default:
            digestSz = (uint32_t)wc_HashGetDigestSize((enum wc_HashType)
                                                          hashType);
            break;
    }

    const uint32_t inLenCnt = sizeof(inLens) / sizeof(inLens[0]);
    const uint32_t chunkCnt  = sizeof(chunks) / sizeof(chunks[0]);

    for (i = 0; (ret == WH_ERROR_OK) && (i < inLenCnt); i++) {
        for (j = 0; (ret == WH_ERROR_OK) && (j < chunkCnt); j++) {
            /* Chunking a 20000-byte input one byte at a time is a lot of
             * round trips for no extra coverage; the smaller inputs above
             * already exercise the same path */
            if ((inLens[i] > 4u * rate) && (chunks[j] != 0u) &&
                (chunks[j] < rate)) {
                continue;
            }
            ret = _Sha3TestCompare(devId, hashType, inLens[i], chunks[j],
                                   digestSz);
        }
    }

    if (ret == WH_ERROR_OK) {
        WH_TEST_PRINT("%s DEVID=0x%X SUCCESS\n", _Sha3TestName(hashType),
                      devId);
    }
    return ret;
}

/* The client entry points must reject bad arguments before going on the wire */
static int _Sha3TestBadArgs(whClientContext* ctx)
{
    wc_Sha3 sha[1];
    uint8_t buf[8];
    bool    sent     = false;
    int     hashType = WC_HASH_TYPE_SHA3_256;
    int     bad      = 0;

    if (wc_InitSha3_256(sha, NULL, WH_CLIENT_DEVID(ctx)) != 0) {
        return WH_ERROR_ABORTED;
    }

    bad = (wh_Client_Sha3(NULL, sha, hashType, buf, sizeof(buf), NULL, 0) !=
           WH_ERROR_BADARGS) ||
          (wh_Client_Sha3(ctx, NULL, hashType, buf, sizeof(buf), NULL, 0) !=
           WH_ERROR_BADARGS) ||
          /* A hash type that is not a Keccak variant at all */
          (wh_Client_Sha3(ctx, sha, WC_HASH_TYPE_SHA256, buf, sizeof(buf),
                          NULL, 0) != WH_ERROR_BADARGS) ||
          (wh_Client_Sha3UpdateRequest(ctx, sha, hashType, buf, sizeof(buf),
                                       NULL) != WH_ERROR_BADARGS) ||
          (wh_Client_Sha3UpdateRequest(ctx, sha, WC_HASH_TYPE_SHA256, buf,
                                       sizeof(buf), &sent) !=
           WH_ERROR_BADARGS) ||
          (wh_Client_Sha3UpdateRequest(ctx, NULL, hashType, buf, sizeof(buf),
                                       &sent) != WH_ERROR_BADARGS) ||
          (wh_Client_Sha3UpdateResponse(ctx, NULL, hashType) !=
           WH_ERROR_BADARGS) ||
          /* Finalizing needs a length, and a real variant to produce it */
          (wh_Client_Sha3FinalRequest(ctx, sha, hashType, 0) !=
           WH_ERROR_BADARGS) ||
          (wh_Client_Sha3FinalRequest(NULL, sha, hashType,
                                      WC_SHA3_256_DIGEST_SIZE) !=
           WH_ERROR_BADARGS) ||
          (wh_Client_Sha3FinalRequest(ctx, sha, WC_HASH_TYPE_SHA256,
                                      WC_SHA3_256_DIGEST_SIZE) !=
           WH_ERROR_BADARGS) ||
          (wh_Client_Sha3FinalResponse(ctx, sha, hashType, NULL,
                                       WC_SHA3_256_DIGEST_SIZE) !=
           WH_ERROR_BADARGS) ||
          (wh_Client_Sha3FinalResponse(ctx, NULL, hashType, buf,
                                       WC_SHA3_256_DIGEST_SIZE) !=
           WH_ERROR_BADARGS);

#if defined(WOLFSSL_SHAKE256)
    /* A SHAKE has no natural digest size, so finalizing without a length is
     * an error rather than something to fill in */
    bad = bad || (wh_Client_Sha3(ctx, sha, WC_HASH_TYPE_SHAKE256, NULL, 0, buf,
                                 0) != WH_ERROR_BADARGS);
#endif

    wc_Sha3_256_Free(sha);

    if (bad) {
        WH_ERROR_PRINT("A SHA3 client call accepted bad arguments\n");
        return WH_ERROR_ABORTED;
    }
    WH_TEST_PRINT("SHA3 bad-args SUCCESS\n");
    return WH_ERROR_OK;
}

#if defined(WOLFSSL_SHAKE128) || defined(WOLFSSL_SHAKE256)
/* A SHAKE asked for more output than a response can carry must still produce
 * the right answer, by declining the offload and letting software finish from
 * the sponge the client holds. */
static int _Sha3TestLongShakeOutput(whClientContext* ctx, int hashType)
{
    int devId = WH_CLIENT_DEVID(ctx);
    int ret;

    ret = _Sha3TestCompare(devId, hashType, 4096u, 0u, SHA3_TEST_LONG_OUT);
    if (ret == WH_ERROR_OK) {
        WH_TEST_PRINT("%s long output DEVID=0x%X SUCCESS\n",
                      _Sha3TestName(hashType), devId);
    }
    return ret;
}
#endif /* WOLFSSL_SHAKE128 || WOLFSSL_SHAKE256 */
#endif /* WOLFSSL_SHA3 || WOLFSSL_SHAKE128 || WOLFSSL_SHAKE256 */

int whTest_Crypto_Sha3(whClientContext* ctx)
{
#if defined(WOLFSSL_SHA3) || defined(WOLFSSL_SHAKE128) || \
    defined(WOLFSSL_SHAKE256)
    uint32_t i;

    for (i = 0; i < sizeof(sha3TestIn); i++) {
        sha3TestIn[i] = (uint8_t)(i * 31u + 7u);
    }

    WH_TEST_RETURN_ON_FAIL(_Sha3TestBadArgs(ctx));
#ifdef WOLFSSL_SHA3
    WH_TEST_RETURN_ON_FAIL(_Sha3TestVariant(ctx, WC_HASH_TYPE_SHA3_224));
    WH_TEST_RETURN_ON_FAIL(_Sha3TestVariant(ctx, WC_HASH_TYPE_SHA3_256));
    WH_TEST_RETURN_ON_FAIL(_Sha3TestVariant(ctx, WC_HASH_TYPE_SHA3_384));
    WH_TEST_RETURN_ON_FAIL(_Sha3TestVariant(ctx, WC_HASH_TYPE_SHA3_512));
#endif
#ifdef WOLFSSL_SHAKE128
    WH_TEST_RETURN_ON_FAIL(_Sha3TestVariant(ctx, WC_HASH_TYPE_SHAKE128));
    WH_TEST_RETURN_ON_FAIL(
        _Sha3TestLongShakeOutput(ctx, WC_HASH_TYPE_SHAKE128));
#endif
#ifdef WOLFSSL_SHAKE256
    WH_TEST_RETURN_ON_FAIL(_Sha3TestVariant(ctx, WC_HASH_TYPE_SHAKE256));
    WH_TEST_RETURN_ON_FAIL(
        _Sha3TestLongShakeOutput(ctx, WC_HASH_TYPE_SHAKE256));
#endif
#endif /* WOLFSSL_SHA3 || WOLFSSL_SHAKE128 || WOLFSSL_SHAKE256 */
    (void)ctx;
    return 0;
}

#endif /* !WOLFHSM_CFG_NO_CRYPTO */
