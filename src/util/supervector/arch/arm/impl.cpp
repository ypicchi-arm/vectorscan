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

#include "ue2common.h"
#include "util/supervector/supervector.hpp"

// 128-bit NEON implementation

template<>
really_inline SuperVector<16>::SuperVector(typename base_type::type const v)
{
    u.v128[0] = v;
}

template<>
template<>
really_inline SuperVector<16>::SuperVector<int8x16_t>(int8x16_t const other)
{
    u.v128[0] = static_cast<m128>(other);
}

template<>
template<>
really_inline SuperVector<16>::SuperVector<uint8x16_t>(uint8x16_t const other)
{
    u.v128[0] = static_cast<m128>(other);
}

template<>
template<>
really_inline SuperVector<16>::SuperVector<int8_t>(int8_t const other)
{
    u.v128[0] = vdupq_n_s8(other);
}

template<>
template<>
really_inline SuperVector<16>::SuperVector<uint8_t>(uint8_t const other)
{
    u.v128[0] = vdupq_n_u8(other);
}

template<>
template<>
really_inline SuperVector<16>::SuperVector<int16_t>(int16_t const other)
{
    u.v128[0] = vdupq_n_s16(other);
}

template<>
template<>
really_inline SuperVector<16>::SuperVector<uint16_t>(uint16_t const other)
{
    u.v128[0] = vdupq_n_u16(other);
}

template<>
template<>
really_inline SuperVector<16>::SuperVector<int32_t>(int32_t const other)
{
    u.v128[0] = vdupq_n_s32(other);
}

template<>
template<>
really_inline SuperVector<16>::SuperVector<uint32_t>(uint32_t const other)
{
    u.v128[0] = vdupq_n_u32(other);
}

template<>
template<>
really_inline SuperVector<16>::SuperVector<int64_t>(int64_t const other)
{
    u.v128[0] = vdupq_n_s64(other);
}

template<>
template<>
really_inline SuperVector<16>::SuperVector<uint64_t>(uint64_t const other)
{
    u.v128[0] = vdupq_n_u64(other);
}

// Constants
template<>
really_inline SuperVector<16> SuperVector<16>::Ones(void)
{
    return {vdupq_n_u8(0xFF)};
}

template<>
really_inline SuperVector<16> SuperVector<16>::Zeroes(void)
{
    return {vdupq_n_u8(0)};
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
    return {vandq_s8(u.v128[0], b.u.v128[0])};
}

template <>
really_inline SuperVector<16> SuperVector<16>::operator|(SuperVector<16> const &b) const
{
    return {vorrq_s8(u.v128[0], b.u.v128[0])};
}

template <>
really_inline SuperVector<16> SuperVector<16>::operator^(SuperVector<16> const &b) const
{
    return {veorq_s8(u.v128[0], b.u.v128[0])};
}

template <>
really_inline SuperVector<16> SuperVector<16>::operator!() const
{
    return {vmvnq_s8(u.v128[0])};
}

template <>
really_inline SuperVector<16> SuperVector<16>::opandnot(SuperVector<16> const &b) const
{
    return {vandq_s8(vmvnq_s8(u.v128[0]), b.u.v128[0])};
}

template <>
really_inline SuperVector<16> SuperVector<16>::operator==(SuperVector<16> const &b) const
{
    return {vceqq_s8((int16x8_t)u.v128[0], (int16x8_t)b.u.v128[0])};
}

template <>
really_inline SuperVector<16> SuperVector<16>::operator!=(SuperVector<16> const &b) const
{
    return !(*this == b);
}

template <>
really_inline SuperVector<16> SuperVector<16>::operator>(SuperVector<16> const &b) const
{
    return {vcgtq_s8((int16x8_t)u.v128[0], (int16x8_t)b.u.v128[0])};
}

template <>
really_inline SuperVector<16> SuperVector<16>::operator>=(SuperVector<16> const &b) const
{
    return {vcgeq_s8((int16x8_t)u.v128[0], (int16x8_t)b.u.v128[0])};
}

template <>
really_inline SuperVector<16> SuperVector<16>::operator<(SuperVector<16> const &b) const
{
    return {vcltq_s8((int16x8_t)u.v128[0], (int16x8_t)b.u.v128[0])};
}

template <>
really_inline SuperVector<16> SuperVector<16>::operator<=(SuperVector<16> const &b) const
{
    return {vcgeq_s8((int16x8_t)u.v128[0], (int16x8_t)b.u.v128[0])};
}

template <>
really_inline SuperVector<16> SuperVector<16>::eq(SuperVector<16> const &b) const
{
    return (*this == b);
}

template <>
really_inline typename SuperVector<16>::movemask_type SuperVector<16>::movemask(void) const
{
    SuperVector powers{0x8040201008040201UL};

    // Compute the mask from the input
    uint64x2_t mask  = vpaddlq_u32(vpaddlq_u16(vpaddlq_u8(vandq_u8((uint16x8_t)u.v128[0], powers.u.v128[0]))));
    uint64x2_t mask1 = (m128)vextq_s8(mask, vdupq_n_u8(0), 7);
    mask = vorrq_u8(mask, mask1);

    // Get the resulting bytes
    uint16_t output;
    vst1q_lane_u16(&output, (uint16x8_t)mask, 0);
    return static_cast<typename SuperVector<16>::movemask_type>(output);
}

template <>
really_inline typename SuperVector<16>::movemask_type SuperVector<16>::eqmask(SuperVector<16> const b) const
{
    return eq(b).movemask();
}

template <>
template<uint8_t N>
really_inline SuperVector<16> SuperVector<16>::vshl_8_imm() const
{
    return {(m128)vshlq_n_s8(u.v128[0], N)};
}

template <>
template<uint8_t N>
really_inline SuperVector<16> SuperVector<16>::vshl_16_imm() const
{
    return {(m128)vshlq_n_s16(u.v128[0], N)};
}

template <>
template<uint8_t N>
really_inline SuperVector<16> SuperVector<16>::vshl_32_imm() const
{
    return {(m128)vshlq_n_s32(u.v128[0], N)};
}

template <>
template<uint8_t N>
really_inline SuperVector<16> SuperVector<16>::vshl_64_imm() const
{
    return {(m128)vshlq_n_s64(u.v128[0], N)};
}

template <>
template<uint8_t N>
really_inline SuperVector<16> SuperVector<16>::vshl_128_imm() const
{
    return {vextq_s8(vdupq_n_u8(0), (int16x8_t)u.v128[0], 16 - N)};
}

template <>
template<uint8_t N>
really_inline SuperVector<16> SuperVector<16>::vshl_imm() const
{
    return vshl_128_imm<N>();
}

template <>
template<uint8_t N>
really_inline SuperVector<16> SuperVector<16>::vshr_8_imm() const
{
    return {(m128)vshrq_n_s8(u.v128[0], N)};
}

template <>
template<uint8_t N>
really_inline SuperVector<16> SuperVector<16>::vshr_16_imm() const
{
    return {(m128)vshrq_n_s16(u.v128[0], N)};
}

template <>
template<uint8_t N>
really_inline SuperVector<16> SuperVector<16>::vshr_32_imm() const
{
    return {(m128)vshrq_n_s32(u.v128[0], N)};
}

template <>
template<uint8_t N>
really_inline SuperVector<16> SuperVector<16>::vshr_64_imm() const
{
    return {(m128)vshrq_n_s64(u.v128[0], N)};
}

template <>
template<uint8_t N>
really_inline SuperVector<16> SuperVector<16>::vshr_128_imm() const
{
    return {vextq_s8((int16x8_t)u.v128[0], vdupq_n_u8(0), N)};
}

template <>
template<uint8_t N>
really_inline SuperVector<16> SuperVector<16>::vshr_imm() const
{
    return vshr_128_imm<N>();
}

#if !defined(HS_OPTIMIZE)
template SuperVector<16> SuperVector<16>::vshl_8_imm<4>() const;
template SuperVector<16> SuperVector<16>::vshl_16_imm<1>() const;
template SuperVector<16> SuperVector<16>::vshl_64_imm<1>() const;
template SuperVector<16> SuperVector<16>::vshl_64_imm<4>() const;
template SuperVector<16> SuperVector<16>::vshl_128_imm<1>() const;
template SuperVector<16> SuperVector<16>::vshl_128_imm<4>() const;
template SuperVector<16> SuperVector<16>::vshr_8_imm<1>() const;
template SuperVector<16> SuperVector<16>::vshr_8_imm<4>() const;
template SuperVector<16> SuperVector<16>::vshr_16_imm<1>() const;
template SuperVector<16> SuperVector<16>::vshr_64_imm<1>() const;
template SuperVector<16> SuperVector<16>::vshr_64_imm<4>() const;
template SuperVector<16> SuperVector<16>::vshr_128_imm<1>() const;
template SuperVector<16> SuperVector<16>::vshr_128_imm<4>() const;
#endif

template <>
really_inline SuperVector<16> SuperVector<16>::vshl_8  (uint8_t const N) const
{
    if (N == 0) return *this;
    if (N == 16) return Zeroes();
    SuperVector result;
    Unroller<1, 16>::iterator([&,v=this](auto const i) { constexpr uint8_t n = i.value; if (N == n) result = {(m128)vshlq_n_s8(u.v128[0], n)}; });
    return result;
}

template <>
really_inline SuperVector<16> SuperVector<16>::vshl_16 (uint8_t const N) const
{
    if (N == 0) return *this;
    if (N == 16) return Zeroes();
    SuperVector result;
    Unroller<1, 16>::iterator([&,v=this](auto const i) { constexpr uint8_t n = i.value; if (N == n) result = {(m128)vshlq_n_s16(u.v128[0], n)}; });
    return result;
}

template <>
really_inline SuperVector<16> SuperVector<16>::vshl_32 (uint8_t const N) const
{
    if (N == 0) return *this;
    if (N == 16) return Zeroes();
    SuperVector result;
    Unroller<1, 16>::iterator([&,v=this](auto const i) { constexpr uint8_t n = i.value; if (N == n) result = {(m128)vshlq_n_s32(u.v128[0], n)}; });
    return result;
}

template <>
really_inline SuperVector<16> SuperVector<16>::vshl_64 (uint8_t const N) const
{
    if (N == 0) return *this;
    if (N == 16) return Zeroes();
    SuperVector result;
    Unroller<1, 16>::iterator([&,v=this](auto const i) { constexpr uint8_t n = i.value; if (N == n) result = {(m128)vshlq_n_s64(u.v128[0], n)}; });
    return result;
}

template <>
really_inline SuperVector<16> SuperVector<16>::vshl_128(uint8_t const N) const
{
    if (N == 0) return *this;
    if (N == 16) return Zeroes();
    SuperVector result;
    Unroller<1, 16>::iterator([&,v=this](auto const i) { constexpr uint8_t n = i.value; if (N == n) result = {vextq_s8(vdupq_n_u8(0), (int16x8_t)u.v128[0], 16 - n)}; });
    return result;
}

template <>
really_inline SuperVector<16> SuperVector<16>::vshl(uint8_t const N) const
{
    return vshl_128(N);
}

template <>
really_inline SuperVector<16> SuperVector<16>::vshr_8  (uint8_t const N) const
{
    if (N == 0) return *this;
    if (N == 16) return Zeroes();
    SuperVector result;
    Unroller<1, 16>::iterator([&,v=this](auto const i) { constexpr uint8_t n = i.value; if (N == n) result = {(m128)vshrq_n_s8(u.v128[0], n)}; });
    return result;
}

template <>
really_inline SuperVector<16> SuperVector<16>::vshr_16 (uint8_t const N) const
{
    if (N == 0) return *this;
    if (N == 16) return Zeroes();
    SuperVector result;
    Unroller<1, 16>::iterator([&,v=this](auto const i) { constexpr uint8_t n = i.value; if (N == n) result = {(m128)vshrq_n_s16(u.v128[0], n)}; });
    return result;
}

template <>
really_inline SuperVector<16> SuperVector<16>::vshr_32 (uint8_t const N) const
{
    if (N == 0) return *this;
    if (N == 16) return Zeroes();
    SuperVector result;
    Unroller<1, 16>::iterator([&,v=this](auto const i) { constexpr uint8_t n = i.value; if (N == n) result = {(m128)vshrq_n_s32(u.v128[0], n)}; });
    return result;
}

template <>
really_inline SuperVector<16> SuperVector<16>::vshr_64 (uint8_t const N) const
{
    if (N == 0) return *this;
    if (N == 16) return Zeroes();
    SuperVector result;
    Unroller<1, 16>::iterator([&,v=this](auto const i) { constexpr uint8_t n = i.value; if (N == n) result = {(m128)vshrq_n_s64(u.v128[0], n)}; });
    return result;
}

template <>
really_inline SuperVector<16> SuperVector<16>::vshr_128(uint8_t const N) const
{
    if (N == 0) return *this;
    if (N == 16) return Zeroes();
    SuperVector result;
    Unroller<1, 16>::iterator([&,v=this](auto const i) { constexpr uint8_t n = i.value; if (N == n) result = {vextq_s8((int16x8_t)u.v128[0], vdupq_n_u8(0), n)}; });
    return result;
}

template <>
really_inline SuperVector<16> SuperVector<16>::vshr(uint8_t const N) const
{
    return vshr_128(N);
}

#ifdef HS_OPTIMIZE
template <>
really_inline SuperVector<16> SuperVector<16>::operator>>(uint8_t const N) const
{
    return {vextq_s8((int16x8_t)u.v128[0], vdupq_n_u8(0), N)};
}
#else
template <>
really_inline SuperVector<16> SuperVector<16>::operator>>(uint8_t const N) const
{
    return vshr_128(N);
}
#endif

#ifdef HS_OPTIMIZE
template <>
really_inline SuperVector<16> SuperVector<16>::operator<<(uint8_t const N) const
{
    return {vextq_s8(vdupq_n_u8(0), (int16x8_t)u.v128[0], 16 - N)};
}
#else
template <>
really_inline SuperVector<16> SuperVector<16>::operator<<(uint8_t const N) const
{
    return vshl_128(N);
}
#endif


template<>
really_inline SuperVector<16> SuperVector<16>::Ones_vshr(uint8_t const N)
{
    return Ones().vshr_128(N);
}

template<>
really_inline SuperVector<16> SuperVector<16>::Ones_vshl(uint8_t const N)
{
    return Ones().vshl_128(N);
}

template <>
really_inline SuperVector<16> SuperVector<16>::loadu(void const *ptr)
{
    return {vld1q_s32((const int32_t *)ptr)};
}

template <>
really_inline SuperVector<16> SuperVector<16>::load(void const *ptr)
{
    assert(ISALIGNED_N(ptr, alignof(SuperVector::size)));
    ptr = assume_aligned(ptr, SuperVector::size);
    return {vld1q_s32((const int32_t *)ptr)};
}

template <>
really_inline SuperVector<16> SuperVector<16>::loadu_maskz(void const *ptr, uint8_t const len)
{
    SuperVector mask = Ones_vshr(16 -len);
    //mask.print8("mask");
    SuperVector<16> v = loadu(ptr);
    //v.print8("v");
    return mask & v;
}

#ifdef HS_OPTIMIZE
template<>
really_inline SuperVector<16> SuperVector<16>::alignr(SuperVector<16> &other, int8_t offset)
{
    if (offset == 16) {
        return *this;
    } else {
        return {vextq_s8((int16x8_t)other.u.v128[0], (int16x8_t)u.v128[0], offset)};
    }
}
#else
template<>
really_inline SuperVector<16> SuperVector<16>::alignr(SuperVector<16> &other, int8_t offset)
{
    switch(offset) {
    case 0: return other; break;
    case 1: return {vextq_s8((int16x8_t) other.u.v128[0], (int16x8_t) u.v128[0], 1)}; break;
    case 2: return {vextq_s8((int16x8_t) other.u.v128[0], (int16x8_t) u.v128[0], 2)}; break;
    case 3: return {vextq_s8((int16x8_t) other.u.v128[0], (int16x8_t) u.v128[0], 3)}; break;
    case 4: return {vextq_s8((int16x8_t) other.u.v128[0], (int16x8_t) u.v128[0], 4)}; break;
    case 5: return {vextq_s8((int16x8_t) other.u.v128[0], (int16x8_t) u.v128[0], 5)}; break;
    case 6: return {vextq_s8((int16x8_t) other.u.v128[0], (int16x8_t) u.v128[0], 6)}; break;
    case 7: return {vextq_s8((int16x8_t) other.u.v128[0], (int16x8_t) u.v128[0], 7)}; break;
    case 8: return {vextq_s8((int16x8_t) other.u.v128[0], (int16x8_t) u.v128[0], 8)}; break;
    case 9: return {vextq_s8((int16x8_t) other.u.v128[0], (int16x8_t) u.v128[0], 9)}; break;
    case 10: return {vextq_s8((int16x8_t) other.u.v128[0], (int16x8_t) u.v128[0], 10)}; break;
    case 11: return {vextq_s8((int16x8_t) other.u.v128[0], (int16x8_t) u.v128[0], 11)}; break;
    case 12: return {vextq_s8((int16x8_t) other.u.v128[0], (int16x8_t) u.v128[0], 12)}; break;
    case 13: return {vextq_s8((int16x8_t) other.u.v128[0], (int16x8_t) u.v128[0], 13)}; break;
    case 14: return {vextq_s8((int16x8_t) other.u.v128[0], (int16x8_t) u.v128[0], 14)}; break;
    case 15: return {vextq_s8((int16x8_t) other.u.v128[0], (int16x8_t) u.v128[0], 15)}; break;
    case 16: return *this; break;
    default: break;
    }
    return *this;
}
#endif

template<>
template<>
really_inline SuperVector<16> SuperVector<16>::pshufb<false>(SuperVector<16> b)
{
    return {vqtbl1q_s8((int8x16_t)u.v128[0], (uint8x16_t)b.u.v128[0])};
}

template<>
template<>
really_inline SuperVector<16> SuperVector<16>::pshufb<true>(SuperVector<16> b)
{
    /* On Intel, if bit 0x80 is set, then result is zero, otherwise which the lane it is &0xf.
       In NEON, if >=16, then the result is zero, otherwise it is that lane.
       btranslated is the version that is converted from Intel to NEON.  */
    SuperVector<16> btranslated = b & SuperVector<16>::dup_s8(0x8f);
    return pshufb<false>(btranslated);
}

template<>
really_inline SuperVector<16> SuperVector<16>::pshufb_maskz(SuperVector<16> b, uint8_t const len)
{
    SuperVector mask = Ones_vshr(16 -len);
    return mask & pshufb<true>(b);
}

#endif // SIMD_IMPL_HPP
