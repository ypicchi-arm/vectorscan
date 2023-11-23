/*
 * Copyright (c) 2015-2020, Intel Corporation
 * Copyright (c) 2020-2021, VectorCamp PC
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
 * \brief SIMD types and primitive operations.
 */

#ifndef ARCH_SIMDE_SIMD_UTILS_H
#define ARCH_SIMDE_SIMD_UTILS_H

#include "ue2common.h"
#include "util/simd_types.h"
#include "util/unaligned.h"
#include "util/intrinsics.h"

#include <string.h> // for memcpy

#define ZEROES_8 0, 0, 0, 0, 0, 0, 0, 0
#define ZEROES_31 ZEROES_8, ZEROES_8, ZEROES_8, 0, 0, 0, 0, 0, 0, 0
#define ZEROES_32 ZEROES_8, ZEROES_8, ZEROES_8, ZEROES_8

/** \brief LUT for the mask1bit functions. */
ALIGN_CL_DIRECTIVE static const u8 simd_onebit_masks[] = {
    ZEROES_32, ZEROES_32,
    ZEROES_31, 0x01, ZEROES_32,
    ZEROES_31, 0x02, ZEROES_32,
    ZEROES_31, 0x04, ZEROES_32,
    ZEROES_31, 0x08, ZEROES_32,
    ZEROES_31, 0x10, ZEROES_32,
    ZEROES_31, 0x20, ZEROES_32,
    ZEROES_31, 0x40, ZEROES_32,
    ZEROES_31, 0x80, ZEROES_32,
    ZEROES_32, ZEROES_32,
};

static really_inline m128 ones128(void) {
    return (m128) _mm_set1_epi8(0xFF);
}

static really_inline m128 zeroes128(void) {
    return (m128) _mm_setzero_si128();
}

/** \brief Bitwise not for m128*/
static really_inline m128 not128(m128 a) {
    return (m128) _mm_xor_si128(a, ones128());
}

/** \brief Return 1 if a and b are different otherwise 0 */
static really_inline int diff128(m128 a, m128 b) {
    return (_mm_movemask_epi8(_mm_cmpeq_epi8(a, b)) ^ 0xffff);
}

static really_inline int isnonzero128(m128 a) {
    return !!diff128(a, zeroes128());
}

/**
 * "Rich" version of diff128(). Takes two vectors a and b and returns a 4-bit
 * mask indicating which 32-bit words contain differences.
 */
static really_inline u32 diffrich128(m128 a, m128 b) {
    a = _mm_cmpeq_epi32(a, b);
    return ~(_mm_movemask_ps(_mm_castsi128_ps(a))) & 0xf;
}

/**
 * "Rich" version of diff128(), 64-bit variant. Takes two vectors a and b and
 * returns a 4-bit mask indicating which 64-bit words contain differences.
 */
static really_inline u32 diffrich64_128(m128 a, m128 b) {
    a = _mm_cmpeq_epi64(a, b);
    return ~(_mm_movemask_ps(_mm_castsi128_ps(a))) & 0x5;
}

static really_really_inline
m128 add_2x64(m128 a, m128 b) {
    return (m128) _mm_add_epi64(a, b);
}

static really_really_inline
m128 sub_2x64(m128 a, m128 b) {
    return (m128) _mm_sub_epi64(a, b);
}

static really_really_inline
m128 lshift64_m128(m128 a, unsigned b) {
    return _mm_slli_epi64(a, b);
}

#define rshift64_m128(a, b) _mm_srli_epi64((a), (b))
#define eq128(a, b)         _mm_cmpeq_epi8((a), (b))
#define eq64_m128(a, b)     _mm_cmpeq_epi64((a), (b))
#define movemask128(a)      ((u32)_mm_movemask_epi8((a)))

static really_inline m128 set1_16x8(u8 c) {
    return _mm_set1_epi8(c);
}

static really_inline m128 set1_4x32(u32 c) {
    return _mm_set1_epi32(c);
}

static really_inline m128 set1_2x64(u64a c) {
    return _mm_set1_epi64x(c);
}

static really_inline u32 movd(const m128 in) {
    return _mm_cvtsi128_si32(in);
}

static really_inline u64a movq(const m128 in) {
    return _mm_cvtsi128_si64(in);
}

/* another form of movq */
static really_inline
m128 load_m128_from_u64a(const u64a *p) {
    return _mm_set_epi64x(0LL, *p);
}

#define CASE_RSHIFT_VECTOR(a, count)  case count: return _mm_srli_si128((m128)(a), (count)); break;

static really_inline
m128 rshiftbyte_m128(const m128 a, int count_immed) {
    switch (count_immed) {
    case 0: return a; break;
    CASE_RSHIFT_VECTOR(a, 1);
    CASE_RSHIFT_VECTOR(a, 2);
    CASE_RSHIFT_VECTOR(a, 3);
    CASE_RSHIFT_VECTOR(a, 4);
    CASE_RSHIFT_VECTOR(a, 5);
    CASE_RSHIFT_VECTOR(a, 6);
    CASE_RSHIFT_VECTOR(a, 7);
    CASE_RSHIFT_VECTOR(a, 8);
    CASE_RSHIFT_VECTOR(a, 9);
    CASE_RSHIFT_VECTOR(a, 10);
    CASE_RSHIFT_VECTOR(a, 11);
    CASE_RSHIFT_VECTOR(a, 12);
    CASE_RSHIFT_VECTOR(a, 13);
    CASE_RSHIFT_VECTOR(a, 14);
    CASE_RSHIFT_VECTOR(a, 15);
    default: return zeroes128(); break;
    }
}
#undef CASE_RSHIFT_VECTOR

#define CASE_LSHIFT_VECTOR(a, count)  case count: return _mm_slli_si128((m128)(a), (count)); break;

static really_inline
m128 lshiftbyte_m128(const m128 a, int count_immed) {
    switch (count_immed) {
    case 0: return a; break;
    CASE_LSHIFT_VECTOR(a, 1);
    CASE_LSHIFT_VECTOR(a, 2);
    CASE_LSHIFT_VECTOR(a, 3);
    CASE_LSHIFT_VECTOR(a, 4);
    CASE_LSHIFT_VECTOR(a, 5);
    CASE_LSHIFT_VECTOR(a, 6);
    CASE_LSHIFT_VECTOR(a, 7);
    CASE_LSHIFT_VECTOR(a, 8);
    CASE_LSHIFT_VECTOR(a, 9);
    CASE_LSHIFT_VECTOR(a, 10);
    CASE_LSHIFT_VECTOR(a, 11);
    CASE_LSHIFT_VECTOR(a, 12);
    CASE_LSHIFT_VECTOR(a, 13);
    CASE_LSHIFT_VECTOR(a, 14);
    CASE_LSHIFT_VECTOR(a, 15);
    default: return zeroes128(); break;
    }
}
#undef CASE_LSHIFT_VECTOR

#define extract32from128(a, imm) _mm_extract_epi32(a, imm)
#define extract64from128(a, imm) _mm_extract_epi64(a, imm)

static really_inline m128 add128(m128 a, m128 b) {
    return _mm_add_epi64(a, b);
}

static really_inline m128 and128(m128 a, m128 b) {
    return _mm_and_si128(a,b);
}

static really_inline m128 xor128(m128 a, m128 b) {
    return _mm_xor_si128(a,b);
}

static really_inline m128 or128(m128 a, m128 b) {
    return _mm_or_si128(a,b);
}

static really_inline m128 andnot128(m128 a, m128 b) {
    return _mm_andnot_si128(a, b);
}

// aligned load
static really_inline m128 load128(const void *ptr) {
    assert(ISALIGNED_N(ptr, alignof(m128)));
    ptr = vectorscan_assume_aligned(ptr, 16);
    return _mm_load_si128((const m128 *)ptr);
}

// aligned store
static really_inline void store128(void *ptr, m128 a) {
    assert(ISALIGNED_N(ptr, alignof(m128)));
    ptr = vectorscan_assume_aligned(ptr, 16);
    *(m128 *)ptr = a;
}

// unaligned load
static really_inline m128 loadu128(const void *ptr) {
    return _mm_loadu_si128((const m128 *)ptr);
}

// unaligned store
static really_inline void storeu128(void *ptr, m128 a) {
    _mm_storeu_si128 ((m128 *)ptr, a);
}

// packed unaligned store of first N bytes
static really_inline
void storebytes128(void *ptr, m128 a, unsigned int n) {
    assert(n <= sizeof(a));
    memcpy(ptr, &a, n);
}

// packed unaligned load of first N bytes, pad with zero
static really_inline
m128 loadbytes128(const void *ptr, unsigned int n) {
    m128 a = zeroes128();
    assert(n <= sizeof(a));
    memcpy(&a, ptr, n);
    return a;
}

static really_inline
m128 mask1bit128(unsigned int n) {
    assert(n < sizeof(m128) * 8);
    u32 mask_idx = ((n % 8) * 64) + 95;
    mask_idx -= n / 8;
    return loadu128(&simd_onebit_masks[mask_idx]);
}

// switches on bit N in the given vector.
static really_inline
void setbit128(m128 *ptr, unsigned int n) {
    *ptr = or128(mask1bit128(n), *ptr);
}

// switches off bit N in the given vector.
static really_inline
void clearbit128(m128 *ptr, unsigned int n) {
    *ptr = andnot128(mask1bit128(n), *ptr);
}

// tests bit N in the given vector.
static really_inline
char testbit128(m128 val, unsigned int n) {
    const m128 mask = mask1bit128(n);
#if defined(HAVE_SSE41)
    return !_mm_testz_si128(mask, val);
#else
    return isnonzero128(and128(mask, val));
#endif
}

// offset must be an immediate
#define palignr_imm(r, l, offset) _mm_alignr_epi8(r, l, offset)

static really_inline
m128 pshufb_m128(m128 a, m128 b) {
    return _mm_shuffle_epi8(a, b);
}

#define CASE_ALIGN_VECTORS(a, b, offset)  case offset: return palignr_imm((m128)(a), (m128)(b), (offset)); break;

static really_really_inline
m128 palignr_sw(m128 r, m128 l, int offset) {
    switch (offset) {
    case 0: return l; break;
    CASE_ALIGN_VECTORS(r, l, 1);
    CASE_ALIGN_VECTORS(r, l, 2);
    CASE_ALIGN_VECTORS(r, l, 3);
    CASE_ALIGN_VECTORS(r, l, 4);
    CASE_ALIGN_VECTORS(r, l, 5);
    CASE_ALIGN_VECTORS(r, l, 6);
    CASE_ALIGN_VECTORS(r, l, 7);
    CASE_ALIGN_VECTORS(r, l, 8);
    CASE_ALIGN_VECTORS(r, l, 9);
    CASE_ALIGN_VECTORS(r, l, 10);
    CASE_ALIGN_VECTORS(r, l, 11);
    CASE_ALIGN_VECTORS(r, l, 12);
    CASE_ALIGN_VECTORS(r, l, 13);
    CASE_ALIGN_VECTORS(r, l, 14);
    CASE_ALIGN_VECTORS(r, l, 15);
    case 16: return r; break;
    default:
	    return zeroes128();
	    break;
    }
}
#undef CASE_ALIGN_VECTORS

static really_really_inline
m128 palignr(m128 r, m128 l, int offset) {
#if defined(HAVE__BUILTIN_CONSTANT_P)
    if (__builtin_constant_p(offset)) {
       return palignr_imm(r, l, offset);
    }
#endif
    return palignr_sw(r, l, offset);
}

static really_inline
m128 variable_byte_shift_m128(m128 in, s32 amount) {
    assert(amount >= -16 && amount <= 16);
    if (amount < 0) {
        return palignr(zeroes128(), in, -amount);
    } else {
        return palignr(in, zeroes128(), 16 - amount);
    }
}
/*
static really_inline
m128 variable_byte_shift_m128(m128 in, s32 amount) {
    assert(amount >= -16 && amount <= 16);
    m128 shift_mask = loadu128(vbs_mask_data + 16 - amount);
    return pshufb_m128(in, shift_mask);
}*/

static really_inline
m128 max_u8_m128(m128 a, m128 b) {
    return _mm_max_epu8(a, b);
}

static really_inline
m128 min_u8_m128(m128 a, m128 b) {
    return _mm_min_epu8(a, b);
}

static really_inline
m128 sadd_u8_m128(m128 a, m128 b) {
    return _mm_adds_epu8(a, b);
}

static really_inline
m128 sub_u8_m128(m128 a, m128 b) {
    return _mm_sub_epi8(a, b);
}

static really_inline
m128 set4x32(u32 x3, u32 x2, u32 x1, u32 x0) {
    return _mm_set_epi32(x3, x2, x1, x0);
}

static really_inline
m128 set2x64(u64a hi, u64a lo) {
    return _mm_set_epi64x(hi, lo);
}

#endif // ARCH_SIMDE_SIMD_UTILS_H
