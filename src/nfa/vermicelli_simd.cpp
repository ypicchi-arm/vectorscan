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
const u8 *vermicelliBlock(SuperVector<S> data, SuperVector<S> chars, SuperVector<S> casemask, const u8 *buf) {

    SuperVector<S> mask = chars.eq(casemask & data);
    return first_non_zero_match<S>(buf, mask);
}


template <uint16_t S>
static really_inline
const u8 *vermicelliBlockNeg(SuperVector<S> data, SuperVector<S> chars, SuperVector<S> casemask, const u8 *buf) {

    SuperVector<S> mask = chars.eq(casemask & data);
    return first_zero_match_inverted<S>(buf, mask);
}

template <uint16_t S>
static really_inline
const u8 *rvermicelliBlock(SuperVector<S> data, SuperVector<S> chars, SuperVector<S> casemask, const u8 *buf) {

    SuperVector<S> mask = chars.eq(casemask & data);
    return last_non_zero_match<S>(buf, mask);
}


template <uint16_t S>
static really_inline
const u8 *rvermicelliBlockNeg(SuperVector<S> data, SuperVector<S> chars, SuperVector<S> casemask, const u8 *buf) {

    SuperVector<S> mask = chars.eq(casemask & data);
    return last_zero_match_inverted<S>(buf, mask);
}

template <uint16_t S>
static really_inline
const u8 *vermicelliDoubleBlock(SuperVector<S> data, SuperVector<S> chars1, SuperVector<S> chars2, SuperVector<S> casemask,
                                u8 const c1, u8 const c2, u8 const casechar, const u8 *buf) {

    SuperVector<S> v = casemask & data;
    SuperVector<S> mask1 = chars1.eq(v);
    SuperVector<S> mask2 = chars2.eq(v);
    SuperVector<S> mask = mask1 & (mask2 >> 1);

    DEBUG_PRINTF("rv[0] = %02hhx, rv[-1] = %02hhx\n", buf[0], buf[-1]);
    bool partial_match = (((buf[0] & casechar) == c2) && ((buf[-1] & casechar) == c1));
    DEBUG_PRINTF("partial = %d\n", partial_match);
    if (partial_match) return buf - 1;

    return first_non_zero_match<S>(buf, mask);
}

template <uint16_t S>
static really_inline
const u8 *rvermicelliDoubleBlock(SuperVector<S> data, SuperVector<S> chars1, SuperVector<S> chars2, SuperVector<S> casemask,
                                 u8 const c1, u8 const c2, u8 const casechar, const u8 *buf) {

    SuperVector<S> v = casemask & data;
    SuperVector<S> mask1 = chars1.eq(v);
    SuperVector<S> mask2 = chars2.eq(v);
    SuperVector<S> mask = (mask1 << 1)& mask2;

    DEBUG_PRINTF("buf[0] = %02hhx, buf[-1] = %02hhx\n", buf[0], buf[-1]);
    bool partial_match = (((buf[0] & casechar) == c2) && ((buf[-1] & casechar) == c1));
    DEBUG_PRINTF("partial = %d\n", partial_match);
    if (partial_match) {
        mask = mask | (SuperVector<S>::Ones() >> (S-1));
    }

    return last_non_zero_match<S>(buf, mask);
}

template <uint16_t S>
static really_inline
const u8 *vermicelliDoubleMaskedBlock(SuperVector<S> data, SuperVector<S> chars1, SuperVector<S> chars2,
                                      SuperVector<S> mask1, SuperVector<S> mask2,
                                      u8 const c1, u8 const c2, u8 const m1, u8 const m2, const u8 *buf) {

    SuperVector<S> v1 = chars1.eq(data & mask1);
    SuperVector<S> v2 = chars2.eq(data & mask2);
    SuperVector<S> mask = v1 & (v2 >> 1);

    DEBUG_PRINTF("rv[0] = %02hhx, rv[-1] = %02hhx\n", buf[0], buf[-1]);
    bool partial_match = (((buf[0] & m1) == c2) && ((buf[-1] & m2) == c1));
    DEBUG_PRINTF("partial = %d\n", partial_match);
    if (partial_match) return buf - 1;

    return first_non_zero_match<S>(buf, mask);
}

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
            rv = vermicelliBlock(data, chars, casemask, d);
            if (rv) return rv;
            d = ROUNDUP_PTR(d, S);
        }

        while(d + S <= buf_end) {
            __builtin_prefetch(d + 64);
            DEBUG_PRINTF("d %p \n", d);
            SuperVector<S> data = SuperVector<S>::load(d);
            rv = vermicelliBlock(data, chars, casemask, d);
            if (rv) return rv;
            d += S;
        }
    }

    DEBUG_PRINTF("d %p e %p \n", d, buf_end);
    // finish off tail

    if (d != buf_end) {
        SuperVector<S> data = SuperVector<S>::loadu_maskz(d, buf_end - d);
        rv = vermicelliBlock(data, chars, casemask, d);
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
            rv = vermicelliBlockNeg(data, chars, casemask, d);
            if (rv) return rv;
            d = ROUNDUP_PTR(d, S);
        }

        while(d + S <= buf_end) {
            __builtin_prefetch(d + 64);
            DEBUG_PRINTF("d %p \n", d);
            SuperVector<S> data = SuperVector<S>::load(d);
            rv = vermicelliBlockNeg(data, chars, casemask, d);
            if (rv) return rv;
            d += S;
        }
    }

    DEBUG_PRINTF("d %p e %p \n", d, buf_end);
    // finish off tail

    if (d != buf_end) {
        SuperVector<S> data = SuperVector<S>::loadu_maskz(d, buf_end - d);
        rv = vermicelliBlockNeg(data, chars, casemask, d);
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
            rv = rvermicelliBlock(data, chars, casemask, d - S);
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
            rv = rvermicelliBlock(data, chars, casemask, d);
            if (rv) return rv;
        }
    }

    DEBUG_PRINTF("tail d %p e %p \n", buf, d);
    // finish off head

    if (d != buf) {
        SuperVector<S> data = SuperVector<S>::loadu(buf);
        rv = rvermicelliBlock(data, chars, casemask, buf);
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
            rv = rvermicelliBlockNeg(data, chars, casemask, d - S);
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
            rv = rvermicelliBlockNeg(data, chars, casemask, d);
            if (rv) return rv;
        }
    }

    DEBUG_PRINTF("tail d %p e %p \n", buf, d);
    // finish off head

    if (d != buf) {
        SuperVector<S> data = SuperVector<S>::loadu(buf);
        rv = rvermicelliBlockNeg(data, chars, casemask, buf);
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
            rv = vermicelliDoubleBlock(data, chars1, chars2, casemask, c1, c2, casechar, d);
            if (rv) return rv;
            d = ROUNDUP_PTR(d, S);
        }

        while(d + S <= buf_end) {
            __builtin_prefetch(d + 64);
            DEBUG_PRINTF("d %p \n", d);
            SuperVector<S> data = SuperVector<S>::load(d);
            rv = vermicelliDoubleBlock(data, chars1, chars2, casemask, c1, c2, casechar, d);
            if (rv) return rv;
            d += S;
        }
    }

    DEBUG_PRINTF("tail d %p e %p \n", d, buf_end);
    // finish off tail

    if (d != buf_end) {
        SuperVector<S> data = SuperVector<S>::loadu_maskz(d, buf_end - d);
        rv = vermicelliDoubleBlock(data, chars1, chars2, casemask, c1, c2, casechar, d);
        DEBUG_PRINTF("rv %p \n", rv);
        if (rv && rv < buf_end) return rv;
    }

    DEBUG_PRINTF("real tail d %p e %p \n", d, buf_end);
    /* check for partial match at end */
    u8 mask = casemask.u.u8[0];
    if ((buf_end[-1] & mask) == (u8)c1) {
        DEBUG_PRINTF("partial!!!\n");
        return buf_end - 1;
    }

    return buf_end;
}

// /* returns highest offset of c2 (NOTE: not c1) */
template <uint16_t S>
const u8 *rvermicelliDoubleExecReal(char c1, char c2, SuperVector<S> const casemask, const u8 *buf, const u8 *buf_end) {
    assert(buf && buf_end);
    assert(buf < buf_end);
    DEBUG_PRINTF("rverm %p len %zu\n", buf, buf_end - buf);
    DEBUG_PRINTF("b %s\n", buf);
    char s[255];
    snprintf(s, buf_end - buf + 1, "%s", buf);
    DEBUG_PRINTF("b %s\n", s);

    const u8 *d = buf_end;
    const u8 *rv;
    const SuperVector<VECTORSIZE> chars1 = SuperVector<VECTORSIZE>::dup_u8(c1);
    const SuperVector<VECTORSIZE> chars2 = SuperVector<VECTORSIZE>::dup_u8(c2);
    const u8 casechar = casemask.u.u8[0];

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
            rv = rvermicelliDoubleBlock(data, chars1, chars2, casemask, c1, c2, casechar, d - S);
            DEBUG_PRINTF("rv %p \n", rv);
            if (rv && rv < buf_end) return rv;
            d = ROUNDDOWN_PTR(d, S);
        }

        while (d - S >= buf) {
            DEBUG_PRINTF("aligned %p \n", d);
            // On large packet buffers, this prefetch appears to get us about 2%.
            __builtin_prefetch(d - 64);

            d -= S;
            SuperVector<S> data = SuperVector<S>::load(d);
            rv = rvermicelliDoubleBlock(data, chars1, chars2, casemask, c1, c2, casechar, d);
            if (rv) return rv;
        }
    }

    DEBUG_PRINTF("tail d %p e %p \n", buf, d);
    // finish off head

    if (d != buf) {
        SuperVector<S> data = SuperVector<S>::loadu(buf);
        rv = rvermicelliDoubleBlock(data, chars1, chars2, casemask, c1, c2, casechar, buf);
        DEBUG_PRINTF("rv %p \n", rv);
        if (rv && rv < buf_end) return rv;
    }

    return buf - 1;
}

template <uint16_t S>
static const u8 *vermicelliDoubleMaskedExecReal(u8 const c1, u8 const c2, u8 const m1, u8 const m2,
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
    const SuperVector<VECTORSIZE> mask1 = SuperVector<VECTORSIZE>::dup_u8(m1);
    const SuperVector<VECTORSIZE> mask2 = SuperVector<VECTORSIZE>::dup_u8(m2);

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
            rv = vermicelliDoubleMaskedBlock(data, chars1, chars2, mask1, mask2, c1, c2, m1, m2, d);
            if (rv) return rv;
            d = ROUNDUP_PTR(d, S);
        }

        while(d + S <= buf_end) {
            __builtin_prefetch(d + 64);
            DEBUG_PRINTF("d %p \n", d);
            SuperVector<S> data = SuperVector<S>::load(d);
            rv = vermicelliDoubleMaskedBlock(data, chars1, chars2, mask1, mask2, c1, c2, m1, m2, d);
            if (rv) return rv;
            d += S;
        }
    }

    DEBUG_PRINTF("tail d %p e %p \n", d, buf_end);
    // finish off tail

    if (d != buf_end) {
        SuperVector<S> data = SuperVector<S>::loadu_maskz(d, buf_end - d);
        rv = vermicelliDoubleMaskedBlock(data, chars1, chars2, mask1, mask2, c1, c2, m1, m2, d);
        DEBUG_PRINTF("rv %p \n", rv);
        if (rv && rv < buf_end) return rv;
    }

    DEBUG_PRINTF("real tail d %p e %p \n", d, buf_end);
    /* check for partial match at end */
    if ((buf_end[-1] & m1) == (u8)c1) {
        DEBUG_PRINTF("partial!!!\n");
        return buf_end - 1;
    }

    return buf_end;
}

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

extern "C" const u8 *rvermicelliDoubleExec(char c1, char c2, char nocase, const u8 *buf, const u8 *buf_end) {
    DEBUG_PRINTF("rev double verm scan %s\\x%02hhx%02hhx over %zu bytes\n",
                 nocase ? "nocase " : "", c1, c2, (size_t)(buf_end - buf));
    assert(buf < buf_end);

    const SuperVector<VECTORSIZE> casemask{nocase ? getCaseMask<VECTORSIZE>() : SuperVector<VECTORSIZE>::Ones()};

    return rvermicelliDoubleExecReal<VECTORSIZE>(c1, c2, casemask, buf, buf_end);
}

extern "C" const u8 *vermicelliDoubleMaskedExec(char c1, char c2, char m1, char m2,
                                     const u8 *buf, const u8 *buf_end) {
    DEBUG_PRINTF("double verm scan (\\x%02hhx&\\x%02hhx)(\\x%02hhx&\\x%02hhx) "
                 "over %zu bytes\n", c1, m1, c2, m2, (size_t)(buf_end - buf));
    assert(buf < buf_end);

    return vermicelliDoubleMaskedExecReal<VECTORSIZE>(c1, c2, m1, m2, buf, buf_end);
}