/*
 * Copyright (c) 2015-2017, Intel Corporation
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

#ifndef SIMD_IMPL_HPP
#define SIMD_IMPL_HPP

#include <cstdint>
#include <cstdio>

#include "ue2common.h"
#include "util/arch.h"
#include "util/unaligned.h"
#include "util/supervector/supervector.hpp"

// 128-bit Powerpc64le implementation

template<>
really_inline SuperVector<16>::SuperVector(SuperVector const &other)
{
    u.v128[0] = other.u.v128[0];
}

template<>
really_inline SuperVector<16>::SuperVector(typename base_type::type const v)
{
    u.v128[0] = v;
};

template<>
template<>
really_inline SuperVector<16>::SuperVector<int8_t>(int8_t const other)
{
    //u.v128[0] = _mm_set1_epi8(other);
    u.v128[0] = vec_splat_s8(other);
}

template<>
template<>
really_inline SuperVector<16>::SuperVector<uint8_t>(uint8_t const other)
{
    //u.v128[0] = _mm_set1_epi8(static_cast<int8_t>(other));
    u.v128[0] = vec_splat_s8(static_cast<int8_t>(other));
}

template<>
template<>
really_inline SuperVector<16>::SuperVector<int16_t>(int16_t const other)
{
    //u.v128[0] = _mm_set1_epi16(other);
    u.v128[0] = vec_splat_s16(other);
}

template<>
template<>
really_inline SuperVector<16>::SuperVector<uint16_t>(uint16_t const other)
{
    //u.v128[0] = _mm_set1_epi16(static_cast<int16_t>(other));
    u.v128[0] = vec_splat_s16(static_cast<int8_t>(other));
}

template<>
template<>
really_inline SuperVector<16>::SuperVector<int32_t>(int32_t const other)
{
    //u.v128[0] = _mm_set1_epi32(other);
    u.v128[0] = vec_splat_s32(other);
}

template<>
template<>
really_inline SuperVector<16>::SuperVector<uint32_t>(uint32_t const other)
{
    //u.v128[0] = _mm_set1_epi32(static_cast<int32_t>(other));
    u.v128[0] = vec_splat_s32(static_cast<int8_t>(other));
}

template<>
template<>
really_inline SuperVector<16>::SuperVector<int64_t>(int64_t const other)
{
    //u.v128[0] = _mm_set1_epi64x(other);
    u.v128[0] = vec_splat_u64(other);
}

template<>
template<>
really_inline SuperVector<16>::SuperVector<uint64_t>(uint64_t const other)
{
    //u.v128[0] = _mm_set1_epi64x(static_cast<int64_t>(other));
    u.v128[0] = vec_splat_u32(static_cast<int8_t>(other));
}

// Constants
template<>
really_inline SuperVector<16> SuperVector<16>::Ones(void)
{
    //return {_mm_set1_epi8(0xFF)};
    return  {vec_splat_s8(0xFF)};
}

template<>
really_inline SuperVector<16> SuperVector<16>::Zeroes(void)
{
    //return {_mm_set1_epi8(0)};
    return  {vec_splat_s8(0)};
}

// Methods

template <>
really_inline void SuperVector<16>::operator=(SuperVector<16> const &other)
{
    u.v128[0] = other.u.v128[0];
}

template <>
really_inline SuperVector<16> SuperVector<16>::operator&(SuperVector<16> const &b) const
{
    //return {_mm_and_si128(u.v128[0], b.u.v128[0])};
    return {vec_and(u.v128[0], b.u.v128[0])};
}

template <>
really_inline SuperVector<16> SuperVector<16>::operator|(SuperVector<16> const &b) const
{
    //return {_mm_or_si128(u.v128[0], b.u.v128[0])};
    return  {vec_or(u.v128[0], b.u.v128[0]);}
}

template <>
really_inline SuperVector<16> SuperVector<16>::operator^(SuperVector<16> const &b) const
{
    //return {_mm_xor_si128(u.v128[0], b.u.v128[0])};
    return  {vec_xor(u.v128[0], b.u.v128[0]);}
}

template <>
really_inline SuperVector<16> SuperVector<16>::opandnot(SuperVector<16> const &b) const
{
    //return {_mm_andnot_si128(u.v128[0], b.u.v128[0])};
    #warning FIXME
}

template <>
really_inline SuperVector<16> SuperVector<16>::eq(SuperVector<16> const &b) const
{
    //return {_mm_cmpeq_epi8(u.v128[0], b.u.v128[0])};
    return { vec_all_eq(u.v128[0], b.u.v128[0])};
}

template <>
really_inline typename SuperVector<16>::movemask_type SuperVector<16>::movemask(void)const
{
    //return _mm_movemask_epi8(u.v128[0]);
    // Compute the mask from the input
    //uint64x2_t mask  = vpaddlq_u32(vpaddlq_u16(vpaddlq_u8(vandq_u8((uint8x16_t)u.v128[0], 0))));
    //uint64x2_t mask1 = (m128)vextq_s8(mask, Zeroes(), 7);
    //mask = vorrq_u8(mask, mask1);

    // Get the resulting bytes
    //uint16_t output;
    //vst1q_lane_u16((uint16_t*)&output, (uint16x8_t)mask, 0);
    //return output;
    #warning FIXME
}

template <>
really_inline typename SuperVector<16>::movemask_type SuperVector<16>::eqmask(SuperVector<16> const b) const
{
    return eq(b).movemask();
}

template <>
really_inline SuperVector<16> SuperVector<16>::rshift128_var(uint8_t const N) const
{
    switch(N) {
    case 1: return {vec_srl(u.v128[0], 1)}; break;
    case 2: return {vec_srl(u.v128[0], 2)}; break;
    case 3: return {vec_srl(u.v128[0], 3)}; break;
    case 4: return {vec_srl(u.v128[0], 4)}; break;
    case 5: return {vec_srl(u.v128[0], 5)}; break;
    case 6: return {vec_srl(u.v128[0], 6)}; break;
    case 7: return {vec_srl(u.v128[0], 7)}; break;
    case 8: return {vec_srl(u.v128[0], 8)}; break;
    case 9: return {vec_srl(u.v128[0], 9)}; break;
    case 10: return {vec_srl(u.v128[0], 10)}; break;
    case 11: return {vec_srl(u.v128[0], 11)}; break;
    case 12: return {vec_srl(u.v128[0], 12)}; break;
    case 13: return {vec_srl(u.v128[0], 13)}; break;
    case 14: return {vec_srl(u.v128[0], 14)}; break;
    case 15: return {vec_srl(u.v128[0], 15)}; break;
    case 16: return Zeroes(); break;
    default: break;
    }
    return *this;
}

#ifdef HS_OPTIMIZE
template <>
really_inline SuperVector<16> SuperVector<16>::operator>>(uint8_t const N) const
{
    return {vec_srl(u.v128[0], N)};
}
#else
template <>
really_inline SuperVector<16> SuperVector<16>::operator>>(uint8_t const N) const
{
    return rshift128_var(N);
}
#endif

template <>
really_inline SuperVector<16> SuperVector<16>::lshift128_var(uint8_t const N) const
{
    switch(N) {
    case 1: return {vec_sll(u.v128[0], 1)}; break;
    case 2: return {vec_sll(u.v128[0], 2)}; break;
    case 3: return {vec_sll(u.v128[0], 3)}; break;
    case 4: return {vec_sll(u.v128[0], 4)}; break;
    case 5: return {vec_sll(u.v128[0], 5)}; break;
    case 6: return {vec_sll(u.v128[0], 6)}; break;
    case 7: return {vec_sll(u.v128[0], 7)}; break;
    case 8: return {vec_sll(u.v128[0], 8)}; break;
    case 9: return {vec_sll(u.v128[0], 9)}; break;
    case 10: return {vec_sll(u.v128[0], 10)}; break;
    case 11: return {vec_sll(u.v128[0], 11)}; break;
    case 12: return {vec_sll(u.v128[0], 12)}; break;
    case 13: return {vec_sll(u.v128[0], 13)}; break;
    case 14: return {vec_sll(u.v128[0], 14)}; break;
    case 15: return {vec_sll(u.v128[0], 15)}; break;
    case 16: return Zeroes(); break;
    default: break;
    }
    return *this;
}

#ifdef HS_OPTIMIZE
template <>
really_inline SuperVector<16> SuperVector<16>::operator<<(uint8_t const N) const
{
    return {vec_sll(u.v128[0], N)};
}
#else
template <>
really_inline SuperVector<16> SuperVector<16>::operator<<(uint8_t const N) const
{
    return lshift128_var(N);
}
#endif

template <>
really_inline SuperVector<16> SuperVector<16>::loadu(void const *ptr)
{
    //return _mm_loadu_si128((const m128 *)ptr);
    #warning FIXME
}

template <>
really_inline SuperVector<16> SuperVector<16>::load(void const *ptr)
{
    //assert(ISALIGNED_N(ptr, alignof(SuperVector::size)));
    //ptr = assume_aligned(ptr, SuperVector::size);
    //return _mm_load_si128((const m128 *)ptr);
    //assert(ISALIGNED_N(ptr, alignof(m128)));
    //return vld1q_s32((const int32_t *)ptr);
    #warning FIXME
}

template <>
really_inline SuperVector<16> SuperVector<16>::loadu_maskz(void const *ptr, uint8_t const len)
{
    //SuperVector<16> mask = Ones().rshift128_var(16 -len);
    //mask.print8("mask");
    //SuperVector<16> v = vld1q_s32((const int32_t *)ptr);
    //v.print8("v");
    //return mask & v;
    #warning FIXME
}

#ifdef HS_OPTIMIZE
template<>
really_inline SuperVector<16> SuperVector<16>::alignr(SuperVector<16> &other, int8_t offset)
{
    //return {vextq_s8(u.v128[0], other.u.v128[0], offset)};
    #warning FIXME
}
#else
template<>
really_inline SuperVector<16> SuperVector<16>::alignr(SuperVector<16> &other, int8_t offset)
{   
    /*
    switch(offset) {
    case 0: return other; break;
    case 1: return {vextq_s8(u.v128[0], other.u.v128[0], 1)}; break;
    case 2: return {vextq_s8(u.v128[0], other.u.v128[0], 2)}; break;
    case 3: return {vextq_s8(u.v128[0], other.u.v128[0], 3)}; break;
    case 4: return {vextq_s8(u.v128[0], other.u.v128[0], 4)}; break;
    case 5: return {vextq_s8(u.v128[0], other.u.v128[0], 5)}; break;
    case 6: return {vextq_s8(u.v128[0], other.u.v128[0], 6)}; break;
    case 7: return {vextq_s8(u.v128[0], other.u.v128[0], 7)}; break;
    case 8: return {vextq_s8(u.v128[0], other.u.v128[0], 8)}; break;
    case 9: return {vextq_s8(u.v128[0], other.u.v128[0], 9)}; break;
    case 10: return {vextq_s8(u.v128[0], other.u.v128[0], 10)}; break;
    case 11: return {vextq_s8(u.v128[0], other.u.v128[0], 11)}; break;
    case 12: return {vextq_s8(u.v128[0], other.u.v128[0], 12)}; break;
    case 13: return {vextq_s8(u.v128[0], other.u.v128[0], 13)}; break;
    case 14: return {vextq_s8(u.v128[0], other.u.v128[0], 14)}; break;
    case 15: return {vextq_s8(u.v128[0], other.u.v128[0], 15)}; break;
    default: break;
    }
    return *this;
    */
   #warning FIXME
}
#endif

template<>
really_inline SuperVector<16> SuperVector<16>::pshufb(SuperVector<16> b)
{
    //return {_mm_shuffle_epi8(u.v128[0], b.u.v128[0])};
    //int8x16_t btranslated = vandq_s8((int8x16_t)b.u.v128[0],vdupq_n_s8(0x8f));
    //return (m128)vqtbl1q_s8((int8x16_t)u.v128[0], (uint8x16_t)btranslated);
    #warning FIXME
}

template<>
really_inline SuperVector<16> SuperVector<16>::pshufb_maskz(SuperVector<16> b, uint8_t const len)
{
    SuperVector<16> mask = Ones().rshift128_var(16 -len);
    return mask & pshufb(b);
}

#ifdef HS_OPTIMIZE
template<>
really_inline SuperVector<16> SuperVector<16>::lshift64(uint8_t const N)
{
    //return {vshlq_n_s64(u.v128[0], N)};
    return {vec_sldw((int64x2_t)u.v128[0], N, 8)};
}
#else
template<>
really_inline SuperVector<16> SuperVector<16>::lshift64(uint8_t const N)
{
    switch(N) {
    case 0: return *this; break;
    case 1: return {vec_sldw((int64x2_t)u.v128[0], 1, 8)}; break;
    case 2: return {vec_sldw((int64x2_t)u.v128[0], 2, 8)}; break;
    case 3: return {vec_sldw((int64x2_t)u.v128[0], 3, 8)}; break;
    case 4: return {vec_sldw((int64x2_t)u.v128[0], 4, 8)}; break;
    case 5: return {vec_sldw((int64x2_t)u.v128[0], 5, 8)}; break;
    case 6: return {vec_sldw((int64x2_t)u.v128[0], 6, 8)}; break;
    case 7: return {vec_sldw((int64x2_t)u.v128[0], 7, 8)}; break;
    case 8: return {vec_sldw((int64x2_t)u.v128[0], 8, 8)}; break;
    case 9: return {vec_sldw((int64x2_t)u.v128[0], 9, 8)}; break;
    case 10: return {vec_sldw((int64x2_t)u.v128[0], 10, 8)}; break;
    case 11: return {vec_sldw((int64x2_t)u.v128[0], 11, 8)}; break;
    case 12: return {vec_sldw((int64x2_t)u.v128[0], 12, 8)}; break;
    case 13: return {vec_sldw((int64x2_t)u.v128[0], 13, 8)}; break;
    case 14: return {vec_sldw((int64x2_t)u.v128[0], 14, 8)}; break;
    case 15: return {vec_sldw((int64x2_t)u.v128[0], 15, 8)}; break;
    case 16: return Zeroes();
    default: break;
    }
    return *this;
}
#endif

#ifdef HS_OPTIMIZE
template<>
really_inline SuperVector<16> SuperVector<16>::rshift64(uint8_t const N)
{
    //return {vshrq_n_s64(u.v128[0], N)};
    #warning FIXME
}
#else
template<>
really_inline SuperVector<16> SuperVector<16>::rshift64(uint8_t const N)
{   
    /*
    switch(N) {
    case 0: return {vshrq_n_s64(u.v128[0], 0)}; break;
    case 1: return {vshrq_n_s64(u.v128[0], 1)}; break;
    case 2: return {vshrq_n_s64(u.v128[0], 2)}; break;
    case 3: return {vshrq_n_s64(u.v128[0], 3)}; break;
    case 4: return {vshrq_n_s64(u.v128[0], 4)}; break;
    case 5: return {vshrq_n_s64(u.v128[0], 5)}; break;
    case 6: return {vshrq_n_s64(u.v128[0], 6)}; break;
    case 7: return {vshrq_n_s64(u.v128[0], 7)}; break;
    case 8: return {vshrq_n_s64(u.v128[0], 8)}; break;
    case 9: return {vshrq_n_s64(u.v128[0], 9)}; break;
    case 10: return {vshrq_n_s64(u.v128[0], 10)}; break;
    case 11: return {vshrq_n_s64(u.v128[0], 11)}; break;
    case 12: return {vshrq_n_s64(u.v128[0], 12)}; break;
    case 13: return {vshrq_n_s64(u.v128[0], 13)}; break;
    case 14: return {vshrq_n_s64(u.v128[0], 14)}; break;
    case 15: return {vshrq_n_s64(u.v128[0], 15)}; break;
        case 16: return Zeroes();
    default: break;
    }
    return *this;
    */
   #warning FIXME
}
#endif

template<>
really_inline SuperVector<16> SuperVector<16>::lshift128(uint8_t const N)
{
    return *this << N;
}

template<>
really_inline SuperVector<16> SuperVector<16>::rshift128(uint8_t const N)
{
    return *this >> N;
}
