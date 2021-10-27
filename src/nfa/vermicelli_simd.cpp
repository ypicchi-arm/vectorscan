/*
 * Copyright (c) 2015-2020, Intel Corporation
 * Copyright (c) 2020-2021, VectorCamp PC
 * Copyright (c) 2021, Arm Limited
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *  * Neither the name of Intel Corporation nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/** \file
 * \brief Vermicelli: single-byte and double-byte acceleration.
 */

#include "util/bitutils.h"
#include "util/simd_utils.h"

#include "vermicelli.hpp"
#include "util/supervector/casemask.hpp"
#include "util/match.hpp"

template <uint16_t S>
static really_inline
const u8 *vermicelliSingleBlock(SuperVector<S> data, SuperVector<S> chars, SuperVector<S> casemask, const u8 *buf) {

    SuperVector<S> mask = chars.eq(casemask & data);
    return first_non_zero_match<S>(buf, mask);
}

template <uint16_t S>
static really_inline
const u8 *rvermicelliSingleBlock(SuperVector<S> data, SuperVector<S> chars, SuperVector<S> casemask, const u8 *buf) {

    SuperVector<S> mask = chars.eq(casemask & data);
    return last_non_zero_match<S>(buf, mask);
}

template <uint16_t S>
static really_inline
const u8 *vermicelliDoubleBlock(SuperVector<S> data, SuperVector<S> chars1, SuperVector<S> chars2, SuperVector<S> casemask,
                                const u8 *buf/*, SuperVector<S> *lastmask1, size_t len = S*/) {

    // lastmask1->print8("lastmask1");
    data.print8("data");
    chars1.print8("chars1");
    chars2.print8("chars2");
    casemask.print8("casemask");
    SuperVector<S> v = casemask & data;
    v.print8("v");
    SuperVector<S> mask1 = chars1.eq(v);
    mask1.print8("mask1");
    SuperVector<S> mask2 = chars2.eq(v);
    mask2.print8("mask2");
    SuperVector<S> mask = (mask1 & (mask2 >> 1));
    mask.print8("mask");
    DEBUG_PRINTF("len = %ld\n", len);
    // *lastmask1 = mask1 >> (len -1);
    // lastmask1->print8("lastmask1");

    return first_non_zero_match<S>(buf, mask);
}

template <uint16_t S>
static really_inline
const u8 *vermicelliSingleBlockNeg(SuperVector<S> data, SuperVector<S> chars, SuperVector<S> casemask, const u8 *buf) {

    SuperVector<S> mask = chars.eq(casemask & data);
    return first_zero_match_inverted<S>(buf, mask);
}

template <uint16_t S>
static really_inline
const u8 *rvermicelliSingleBlockNeg(SuperVector<S> data, SuperVector<S> chars, SuperVector<S> casemask, const u8 *buf) {

    SuperVector<S> mask = chars.eq(casemask & data);
    return last_zero_match_inverted<S>(buf, mask);
}
/*
template <uint16_t S>
static really_inline
const u8 *vermicelliDoubleBlockNeg(SuperVector<S> data, SuperVector<S> chars1, SuperVector<S> chars2, SuperVector<S> casemask,
                                const u8 *buf, size_t len = S) {

    // lastmask1.print8("lastmask1");
    data.print8("data");
    chars1.print8("chars1");
    chars2.print8("chars2");
    casemask.print8("casemask");
    SuperVector<S> v = casemask & data;
    v.print8("v");
    SuperVector<S> mask1 = chars1.eq(v);
    mask1.print8("mask1");
    SuperVector<S> mask2 = chars2.eq(v);
    mask2.print8("mask2");
    SuperVector<S> mask = (mask1 & (mask2 >> 1));// | lastmask1;
    mask.print8("mask");
    DEBUG_PRINTF("len = %ld\n", len);
    // lastmask1 = mask << (len -1);
    // lastmask1.print8("lastmask1");

    return last_zero_match_inverted<S>(buf, mask);
}*/

template <uint16_t S>
static const u8 *vermicelliExecReal(SuperVector<S> const chars, SuperVector<S> const casemask, const u8 *buf, const u8 *buf_end) {
    assert(buf && buf_end);
    assert(buf < buf_end);
    DEBUG_PRINTF("verm %p len %zu\n", buf, buf_end - buf);
    DEBUG_PRINTF("b %s\n", buf);

    const u8 *d = buf;
    const u8 *rv;

    __builtin_prefetch(d +   64);
    __builtin_prefetch(d + 2*64);
    __builtin_prefetch(d + 3*64);
    __builtin_prefetch(d + 4*64);
    DEBUG_PRINTF("start %p end %p \n", d, buf_end);
    assert(d < buf_end);
    if (d + S <= buf_end) {
        // Reach vector aligned boundaries
        DEBUG_PRINTF("until aligned %p \n", ROUNDUP_PTR(d, S));
        if (!ISALIGNED_N(d, S)) {
            SuperVector<S> data = SuperVector<S>::loadu(d);
            rv = vermicelliSingleBlock(data, chars, casemask, d);
            if (rv) return rv;
            d = ROUNDUP_PTR(d, S);
        }

        while(d + S <= buf_end) {
            __builtin_prefetch(d + 64);
            DEBUG_PRINTF("d %p \n", d);
            SuperVector<S> data = SuperVector<S>::load(d);
            rv = vermicelliSingleBlock(data, chars, casemask, d);
            if (rv) return rv;
            d += S;
        }
    }

    DEBUG_PRINTF("d %p e %p \n", d, buf_end);
    // finish off tail

    if (d != buf_end) {
        SuperVector<S> data = SuperVector<S>::loadu_maskz(d, buf_end - d);
        rv = vermicelliSingleBlock(data, chars, casemask, d);
        DEBUG_PRINTF("rv %p \n", rv);
        if (rv && rv < buf_end) return rv;
    }

    return buf_end;
}

template <uint16_t S>
static const u8 *nvermicelliExecReal(SuperVector<S> const chars, SuperVector<S> const casemask, const u8 *buf, const u8 *buf_end) {
    assert(buf && buf_end);
    assert(buf < buf_end);
    DEBUG_PRINTF("verm %p len %zu\n", buf, buf_end - buf);
    DEBUG_PRINTF("b %s\n", buf);

    const u8 *d = buf;
    const u8 *rv;

    

    __builtin_prefetch(d +   64);
    __builtin_prefetch(d + 2*64);
    __builtin_prefetch(d + 3*64);
    __builtin_prefetch(d + 4*64);
    DEBUG_PRINTF("start %p end %p \n", d, buf_end);
    assert(d < buf_end);
    if (d + S <= buf_end) {
        // Reach vector aligned boundaries
        DEBUG_PRINTF("until aligned %p \n", ROUNDUP_PTR(d, S));
        if (!ISALIGNED_N(d, S)) {
            SuperVector<S> data = SuperVector<S>::loadu(d);
            rv = vermicelliSingleBlockNeg(data, chars, casemask, d);
            if (rv) return rv;
            d = ROUNDUP_PTR(d, S);
        }

        while(d + S <= buf_end) {
            __builtin_prefetch(d + 64);
            DEBUG_PRINTF("d %p \n", d);
            SuperVector<S> data = SuperVector<S>::load(d);
            rv = vermicelliSingleBlockNeg(data, chars, casemask, d);
            if (rv) return rv;
            d += S;
        }
    }

    DEBUG_PRINTF("d %p e %p \n", d, buf_end);
    // finish off tail

    if (d != buf_end) {
        SuperVector<S> data = SuperVector<S>::loadu_maskz(d, buf_end - d);
        rv = vermicelliSingleBlockNeg(data, chars, casemask, d);
        DEBUG_PRINTF("rv %p \n", rv);
        if (rv && rv < buf_end) return rv;
    }

    return buf_end;
}

// Reverse vermicelli scan. Provides exact semantics and returns (buf - 1) if
// character not found.
template <uint16_t S>
const u8 *rvermicelliExecReal(SuperVector<S> const chars, SuperVector<S> const casemask, const u8 *buf, const u8 *buf_end) {
    assert(buf && buf_end);
    assert(buf < buf_end);
    DEBUG_PRINTF("rverm %p len %zu\n", buf, buf_end - buf);
    DEBUG_PRINTF("b %s\n", buf);

    const u8 *d = buf_end;
    const u8 *rv;

    __builtin_prefetch(d -   64);
    __builtin_prefetch(d - 2*64);
    __builtin_prefetch(d - 3*64);
    __builtin_prefetch(d - 4*64);
    DEBUG_PRINTF("start %p end %p \n", buf, d);
    assert(d > buf);
    if (d - S >= buf) {
        // Reach vector aligned boundaries
        DEBUG_PRINTF("until aligned %p \n", ROUNDDOWN_PTR(d, S));
        if (!ISALIGNED_N(d, S)) {
            SuperVector<S> data = SuperVector<S>::loadu(d - S);
            rv = rvermicelliSingleBlock(data, chars, casemask, d - S);
            DEBUG_PRINTF("rv %p \n", rv);
            if (rv) return rv;
            d = ROUNDDOWN_PTR(d, S);
        }

        while (d - S >= buf) {
            DEBUG_PRINTF("aligned %p \n", d);
            // On large packet buffers, this prefetch appears to get us about 2%.
            __builtin_prefetch(d - 64);

            d -= S;
            SuperVector<S> data = SuperVector<S>::load(d);
            rv = rvermicelliSingleBlock(data, chars, casemask, d);
            if (rv) return rv;
        }
    }

    DEBUG_PRINTF("tail d %p e %p \n", buf, d);
    // finish off head

    if (d != buf) {
        SuperVector<S> data = SuperVector<S>::loadu(buf);
        rv = rvermicelliSingleBlock(data, chars, casemask, buf);
        DEBUG_PRINTF("rv %p \n", rv);
        if (rv && rv < buf_end) return rv;
    }

    return buf - 1;
}

// Reverse vermicelli scan. Provides exact semantics and returns (buf - 1) if
// character not found.
template <uint16_t S>
const u8 *rnvermicelliExecReal(SuperVector<S> const chars, SuperVector<S> const casemask, const u8 *buf, const u8 *buf_end) {
    assert(buf && buf_end);
    assert(buf < buf_end);
    DEBUG_PRINTF("rverm %p len %zu\n", buf, buf_end - buf);
    DEBUG_PRINTF("b %s\n", buf);

    const u8 *d = buf_end;
    const u8 *rv;

    __builtin_prefetch(d -   64);
    __builtin_prefetch(d - 2*64);
    __builtin_prefetch(d - 3*64);
    __builtin_prefetch(d - 4*64);
    DEBUG_PRINTF("start %p end %p \n", buf, d);
    assert(d > buf);
    if (d - S >= buf) {
        // Reach vector aligned boundaries
        DEBUG_PRINTF("until aligned %p \n", ROUNDDOWN_PTR(d, S));
        if (!ISALIGNED_N(d, S)) {
            SuperVector<S> data = SuperVector<S>::loadu(d - S);
            rv = rvermicelliSingleBlockNeg(data, chars, casemask, d - S);
            DEBUG_PRINTF("rv %p \n", rv);
            if (rv) return rv;
            d = ROUNDDOWN_PTR(d, S);
        }

        while (d - S >= buf) {
            DEBUG_PRINTF("aligned %p \n", d);
            // On large packet buffers, this prefetch appears to get us about 2%.
            __builtin_prefetch(d - 64);

            d -= S;
            SuperVector<S> data = SuperVector<S>::load(d);
            rv = rvermicelliSingleBlockNeg(data, chars, casemask, d);
            if (rv) return rv;
        }
    }

    DEBUG_PRINTF("tail d %p e %p \n", buf, d);
    // finish off head

    if (d != buf) {
        SuperVector<S> data = SuperVector<S>::loadu(buf);
        rv = rvermicelliSingleBlockNeg(data, chars, casemask, buf);
        DEBUG_PRINTF("rv %p \n", rv);
        if (rv && rv < buf_end) return rv;
    }

    return buf - 1;
}

template <uint16_t S>
static const u8 *vermicelliDoubleExecReal(u8 const c1, u8 const c2, SuperVector<S> const casemask,
                                          const u8 *buf, const u8 *buf_end) {
    assert(buf && buf_end);
    assert(buf < buf_end);
    DEBUG_PRINTF("verm %p len %zu\n", buf, buf_end - buf);
    DEBUG_PRINTF("b %s\n", buf);

    const u8 *d = buf;
    const u8 *rv;
    // SuperVector<S> lastmask1{0};
    const SuperVector<VECTORSIZE> chars1 = SuperVector<VECTORSIZE>::dup_u8(c1);
    const SuperVector<VECTORSIZE> chars2 = SuperVector<VECTORSIZE>::dup_u8(c2);
    const u8 casechar = casemask.u.u8[0];

    __builtin_prefetch(d +   64);
    __builtin_prefetch(d + 2*64);
    __builtin_prefetch(d + 3*64);
    __builtin_prefetch(d + 4*64);
    DEBUG_PRINTF("start %p end %p \n", d, buf_end);
    assert(d < buf_end);
    if (d + S <= buf_end) {
        // Reach vector aligned boundaries
        DEBUG_PRINTF("until aligned %p \n", ROUNDUP_PTR(d, S));
        if (!ISALIGNED_N(d, S)) {
            SuperVector<S> data = SuperVector<S>::loadu(d);
            rv = vermicelliDoubleBlock(data, chars1, chars2, casemask, d);//, &lastmask1);
            if (rv) return rv;
            d = ROUNDUP_PTR(d, S);
        }

        while(d + S <= buf_end) {
            __builtin_prefetch(d + 64);
            DEBUG_PRINTF("d %p \n", d);
            SuperVector<S> data = SuperVector<S>::load(d);
            rv = vermicelliDoubleBlock(data, chars1, chars2, casemask, d);//, &lastmask1);
            if (rv) {
                bool partial_match = (((rv[0] & casechar) == c2) && ((rv[-1] & casechar) == c1));
                return rv - partial_match;
            }
            d += S;
        }
    }

    DEBUG_PRINTF("tail d %p e %p \n", d, buf_end);
    // finish off tail

    if (d != buf_end) {
        SuperVector<S> data = SuperVector<S>::loadu_maskz(d, buf_end - d);
        rv = vermicelliDoubleBlock(data, chars1, chars2, casemask, d);//, buf_end - d);
        DEBUG_PRINTF("rv %p \n", rv);
        if (rv && rv < buf_end) return rv;
    }

    DEBUG_PRINTF("real tail d %p e %p \n", d, buf_end);
    /* check for partial match at end */
    u8 mask = casemask.u.u8[0];
    // u8 c1 = chars1.u.u8[0];
    if ((buf_end[-1] & mask) == (u8)c1) {
        DEBUG_PRINTF("partial!!!\n");
        return buf_end - 1;
    }

    return buf_end;
}

// /* returns highest offset of c2 (NOTE: not c1) */
// static really_inline
// const u8 *rvermicelliDoubleExec(char c1, char c2, char nocase, const u8 *buf,
//                                 const u8 *buf_end) {
//     DEBUG_PRINTF("rev double verm scan %s\\x%02hhx%02hhx over %zu bytes\n",
//                  nocase ? "nocase " : "", c1, c2, (size_t)(buf_end - buf));
//     assert(buf < buf_end);

//     VERM_TYPE chars1 = VERM_SET_FN(c1); /* nocase already uppercase */
//     VERM_TYPE chars2 = VERM_SET_FN(c2); /* nocase already uppercase */

// #ifdef HAVE_AVX512
//     if (buf_end - buf <= VERM_BOUNDARY) {
//         const u8 *ptr = nocase
//                       ? rdvermMiniNocase(chars1, chars2, buf, buf_end)
//                       : rdvermMini(chars1, chars2, buf, buf_end);

//         if (ptr) {
//             return ptr;
//         }

//         // check for partial match at end ???
//         return buf - 1;
//     }
// #endif

//     assert((buf_end - buf) >= VERM_BOUNDARY);
//     size_t min = (size_t)buf_end % VERM_BOUNDARY;
//     if (min) {
//         // input not aligned, so we need to run one iteration with an unaligned
//         // load, then skip buf forward to the next aligned address. There's
//         // some small overlap here, but we don't mind scanning it twice if we
//         // can do it quickly, do we?
//         const u8 *ptr = nocase ? rdvermPreconditionNocase(chars1, chars2,
//                                                           buf_end - VERM_BOUNDARY)
//                                : rdvermPrecondition(chars1, chars2,
//                                                     buf_end - VERM_BOUNDARY);

//         if (ptr) {
//             return ptr;
//         }

//         buf_end -= min;
//         if (buf >= buf_end) {
//             return buf_end;
//         }
//     }

//     // Aligned loops from here on in
//     if (nocase) {
//         return rdvermSearchAlignedNocase(chars1, chars2, c1, c2, buf, buf_end);
//     } else {
//         return rdvermSearchAligned(chars1, chars2, c1, c2, buf, buf_end);
//     }
// }

extern "C" const u8 *vermicelliExec(char c, char nocase, const u8 *buf, const u8 *buf_end) {
    DEBUG_PRINTF("verm scan %s\\x%02hhx over %zu bytes\n",
                 nocase ? "nocase " : "", c, (size_t)(buf_end - buf));
    assert(buf < buf_end);

    const SuperVector<VECTORSIZE> chars = SuperVector<VECTORSIZE>::dup_u8(c);
    const SuperVector<VECTORSIZE> casemask{nocase ? getCaseMask<VECTORSIZE>() : SuperVector<VECTORSIZE>::Ones()};

    return vermicelliExecReal<VECTORSIZE>(chars, casemask, buf, buf_end);
}

/* like vermicelliExec except returns the address of the first character which
 * is not c */
extern "C" const u8 *nvermicelliExec(char c, char nocase, const u8 *buf, const u8 *buf_end) {
    DEBUG_PRINTF("nverm scan %s\\x%02hhx over %zu bytes\n",
                 nocase ? "nocase " : "", c, (size_t)(buf_end - buf));
    assert(buf < buf_end);

    const SuperVector<VECTORSIZE> chars = SuperVector<VECTORSIZE>::dup_u8(c);
    const SuperVector<VECTORSIZE> casemask{nocase ? getCaseMask<VECTORSIZE>() : SuperVector<VECTORSIZE>::Ones()};

    return nvermicelliExecReal<VECTORSIZE>(chars, casemask, buf, buf_end);
}

extern "C" const u8 *rvermicelliExec(char c, char nocase, const u8 *buf, const u8 *buf_end) {
    DEBUG_PRINTF("rev verm scan %s\\x%02hhx over %zu bytes\n",
                 nocase ? "nocase " : "", c, (size_t)(buf_end - buf));
    assert(buf < buf_end);

    const SuperVector<VECTORSIZE> chars = SuperVector<VECTORSIZE>::dup_u8(c);
    const SuperVector<VECTORSIZE> casemask{nocase ? getCaseMask<VECTORSIZE>() : SuperVector<VECTORSIZE>::Ones()};

    return rvermicelliExecReal<VECTORSIZE>(chars, casemask, buf, buf_end);
}

extern "C" const u8 *rnvermicelliExec(char c, char nocase, const u8 *buf, const u8 *buf_end) {
     DEBUG_PRINTF("rev verm scan %s\\x%02hhx over %zu bytes\n",
                  nocase ? "nocase " : "", c, (size_t)(buf_end - buf));
    assert(buf < buf_end);

    const SuperVector<VECTORSIZE> chars = SuperVector<VECTORSIZE>::dup_u8(c);
    const SuperVector<VECTORSIZE> casemask{nocase ? getCaseMask<VECTORSIZE>() : SuperVector<VECTORSIZE>::Ones()};

    return rnvermicelliExecReal<VECTORSIZE>(chars, casemask, buf, buf_end);
}

extern "C" const u8 *vermicelliDoubleExec(char c1, char c2, char nocase, const u8 *buf, const u8 *buf_end) {
    DEBUG_PRINTF("double verm scan %s\\x%02hhx%02hhx over %zu bytes\n",
                 nocase ? "nocase " : "", c1, c2, (size_t)(buf_end - buf));
    assert(buf < buf_end);

    const SuperVector<VECTORSIZE> casemask{nocase ? getCaseMask<VECTORSIZE>() : SuperVector<VECTORSIZE>::Ones()};

    return vermicelliDoubleExecReal<VECTORSIZE>(c1, c2, casemask, buf, buf_end);
}