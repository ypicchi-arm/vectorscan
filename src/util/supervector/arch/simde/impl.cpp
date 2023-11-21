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
really_inline SuperVector<16>::SuperVector(int8_t const other)
{
    u.v128[0] = _mm_set1_epi8(other);
}

template<>
template<>
really_inline SuperVector<16>::SuperVector(uint8_t const other)
{
    u.v128[0] = _mm_set1_epi8(static_cast<int8_t>(other));
}

template<>
template<>
really_inline SuperVector<16>::SuperVector(int16_t const other)
{
    u.v128[0] = _mm_set1_epi16(other);
}

template<>
template<>
really_inline SuperVector<16>::SuperVector(uint16_t const other)
{
    u.v128[0] = _mm_set1_epi16(static_cast<int16_t>(other));
}

template<>
template<>
really_inline SuperVector<16>::SuperVector(int32_t const other)
{
    u.v128[0] = _mm_set1_epi32(other);
}

template<>
template<>
really_inline SuperVector<16>::SuperVector(uint32_t const other)
{
    u.v128[0] = _mm_set1_epi32(static_cast<int32_t>(other));
}

template<>
template<>
really_inline SuperVector<16>::SuperVector(int64_t const other)
{
    u.v128[0] = _mm_set1_epi64x(other);
}

template<>
template<>
really_inline SuperVector<16>::SuperVector(uint64_t const other)
{
    u.v128[0] = _mm_set1_epi64x(static_cast<int64_t>(other));
}

// Constants
template<>
really_inline SuperVector<16> SuperVector<16>::Ones()
{
    return {_mm_set1_epi8(0xFF)};
}

template<>
really_inline SuperVector<16> SuperVector<16>::Zeroes(void)
{
    return {_mm_set1_epi8(0)};
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
    return {_mm_and_si128(u.v128[0], b.u.v128[0])};
}

template <>
really_inline SuperVector<16> SuperVector<16>::operator|(SuperVector<16> const &b) const
{
    return {_mm_or_si128(u.v128[0], b.u.v128[0])};
}

template <>
really_inline SuperVector<16> SuperVector<16>::operator^(SuperVector<16> const &b) const
{
    return {_mm_xor_si128(u.v128[0], b.u.v128[0])};
}

template <>
really_inline SuperVector<16> SuperVector<16>::operator!() const
{
    return {_mm_xor_si128(u.v128[0], u.v128[0])};
}

template <>
really_inline SuperVector<16> SuperVector<16>::opandnot(SuperVector<16> const &b) const
{
    return {_mm_andnot_si128(u.v128[0], b.u.v128[0])};
}

template <>
really_inline SuperVector<16> SuperVector<16>::operator==(SuperVector<16> const &b) const
{
    return {_mm_cmpeq_epi8(u.v128[0], b.u.v128[0])};
}

template <>
really_inline SuperVector<16> SuperVector<16>::operator!=(SuperVector<16> const &b) const
{
    return !(*this == b);
}

template <>
really_inline SuperVector<16> SuperVector<16>::operator>(SuperVector<16> const &b) const
{
    return {_mm_cmpgt_epi8(u.v128[0], b.u.v128[0])};
}

template <>
really_inline SuperVector<16> SuperVector<16>::operator<(SuperVector<16> const &b) const
{
    return {_mm_cmplt_epi8(u.v128[0], b.u.v128[0])};
}

template <>
really_inline SuperVector<16> SuperVector<16>::operator>=(SuperVector<16> const &b) const
{
    return !(*this < b);
}

template <>
really_inline SuperVector<16> SuperVector<16>::operator<=(SuperVector<16> const &b) const
{
    return !(*this > b);
}

template <>
really_inline SuperVector<16> SuperVector<16>::eq(SuperVector<16> const &b) const
{
    return (*this == b);
}

template <>
really_inline typename SuperVector<16>::comparemask_type
SuperVector<16>::comparemask(void) const {
    return (u32)_mm_movemask_epi8(u.v128[0]);
}

template <>
really_inline typename SuperVector<16>::comparemask_type
SuperVector<16>::eqmask(SuperVector<16> const b) const {
    return eq(b).comparemask();
}

template <> really_inline u32 SuperVector<16>::mask_width() { return 1; }

template <>
really_inline typename SuperVector<16>::comparemask_type
SuperVector<16>::iteration_mask(
    typename SuperVector<16>::comparemask_type mask) {
    return mask;
}

// template <>
// template<uint8_t N>
// really_inline SuperVector<16> SuperVector<16>::vshl_8_imm() const
// {
//     const uint8_t i = N;
//     return {_mm_slli_epi8(u.v128[0], i)};
// }

template <>
template<uint8_t N>
really_inline SuperVector<16> SuperVector<16>::vshl_16_imm() const
{
    return {_mm_slli_epi16(u.v128[0], N)};
}

template <>
template<uint8_t N>
really_inline SuperVector<16> SuperVector<16>::vshl_32_imm() const
{
    return {_mm_slli_epi32(u.v128[0], N)};
}

template <>
template<uint8_t N>
really_inline SuperVector<16> SuperVector<16>::vshl_64_imm() const
{
    return {_mm_slli_epi64(u.v128[0], N)};
}

template <>
template<uint8_t N>
really_inline SuperVector<16> SuperVector<16>::vshl_128_imm() const
{
    return {_mm_slli_si128(u.v128[0], N)};
}

template <>
template<uint8_t N>
really_inline SuperVector<16> SuperVector<16>::vshl_imm() const
{
    return vshl_128_imm<N>();
}

// template <>
// template<uint8_t N>
// really_inline SuperVector<16> SuperVector<16>::vshr_8_imm() const
// {
//     return {_mm_srli_epi8(u.v128[0], N)};
// }

template <>
template<uint8_t N>
really_inline SuperVector<16> SuperVector<16>::vshr_16_imm() const
{
    return {_mm_srli_epi16(u.v128[0], N)};
}

template <>
template<uint8_t N>
really_inline SuperVector<16> SuperVector<16>::vshr_32_imm() const
{
    return {_mm_srli_epi32(u.v128[0], N)};
}
  
template <>
template<uint8_t N>
really_inline SuperVector<16> SuperVector<16>::vshr_64_imm() const
{
    return {_mm_srli_epi64(u.v128[0], N)};
}

template <>
template<uint8_t N>
really_inline SuperVector<16> SuperVector<16>::vshr_128_imm() const
{
    return {_mm_srli_si128(u.v128[0], N)};
}

template <>
template<uint8_t N>
really_inline SuperVector<16> SuperVector<16>::vshr_imm() const
{
    return vshr_128_imm<N>();
}

#if !defined(HS_OPTIMIZE)
template SuperVector<16> SuperVector<16>::vshl_16_imm<1>() const;
template SuperVector<16> SuperVector<16>::vshl_64_imm<1>() const;
template SuperVector<16> SuperVector<16>::vshl_64_imm<4>() const;
template SuperVector<16> SuperVector<16>::vshl_128_imm<1>() const;
template SuperVector<16> SuperVector<16>::vshl_128_imm<4>() const;
template SuperVector<16> SuperVector<16>::vshr_16_imm<1>() const;
template SuperVector<16> SuperVector<16>::vshr_64_imm<1>() const;
template SuperVector<16> SuperVector<16>::vshr_64_imm<4>() const;
template SuperVector<16> SuperVector<16>::vshr_128_imm<1>() const;
template SuperVector<16> SuperVector<16>::vshr_128_imm<4>() const;
#endif

// template <>
// really_inline SuperVector<16> SuperVector<16>::vshl_8  (uint8_t const N) const
// {
//     Unroller<0, 15>::iterator([&,v=this](int i) { if (N == i) return {_mm_slli_epi8(v->u.v128[0], i)}; });
//     if (N == 16) return Zeroes();
// }

template <>
really_inline SuperVector<16> SuperVector<16>::vshl_16 (uint8_t const N) const
{
#if defined(HAVE__BUILTIN_CONSTANT_P)
    if (__builtin_constant_p(N)) {
        return {_mm_slli_epi16(u.v128[0], N)};
    }
#endif
    if (N == 0) return *this;
    if (N == 16) return Zeroes();
    SuperVector result;
    Unroller<1, 16>::iterator([&,v=this](auto const i) { constexpr uint8_t n = i.value; if (N == n) result = {_mm_slli_epi16(v->u.v128[0], n)}; });
    return result;
}

template <>
really_inline SuperVector<16> SuperVector<16>::vshl_32 (uint8_t const N) const
{
    if (N == 0) return *this;
    if (N == 16) return Zeroes();
    SuperVector result;
    Unroller<1, 16>::iterator([&,v=this](auto const i) { constexpr uint8_t n = i.value; if (N == n) result = {_mm_slli_epi32(v->u.v128[0], n)}; });
    return result;
}

template <>
really_inline SuperVector<16> SuperVector<16>::vshl_64 (uint8_t const N) const
{
    if (N == 0) return *this;
    if (N == 16) return Zeroes();
    SuperVector result;
    Unroller<1, 16>::iterator([&,v=this](auto const i) { constexpr uint8_t n = i.value; if (N == n) result = {_mm_slli_epi64(v->u.v128[0], n)}; });
    return result;
}

template <>
really_inline SuperVector<16> SuperVector<16>::vshl_128(uint8_t const N) const
{
    if (N == 0) return *this;
    if (N == 16) return Zeroes();
    SuperVector result;
    Unroller<1, 16>::iterator([&,v=this](auto const i) { constexpr uint8_t n = i.value; if (N == n) result = {_mm_slli_si128(v->u.v128[0], n)}; });
    return result;
}

template <>
really_inline SuperVector<16> SuperVector<16>::vshl(uint8_t const N) const
{
    return vshl_128(N);
}

// template <>
// really_inline SuperVector<16> SuperVector<16>::vshr_8  (uint8_t const N) const
// {
//     SuperVector<16> result;
//     Unroller<0, 15>::iterator([&,v=this](uint8_t const i) { if (N == i) result = {_mm_srli_epi8(v->u.v128[0], i)}; });
//     if (N == 16) result = Zeroes();
//     return result;
// }

template <>
really_inline SuperVector<16> SuperVector<16>::vshr_16 (uint8_t const N) const
{
    if (N == 0) return *this;
    if (N == 16) return Zeroes();
    SuperVector result;
    Unroller<1, 16>::iterator([&,v=this](auto const i) { constexpr uint8_t n = i.value; if (N == n) result = {_mm_srli_epi16(v->u.v128[0], n)}; });
    return result;
}

template <>
really_inline SuperVector<16> SuperVector<16>::vshr_32 (uint8_t const N) const
{
    if (N == 0) return *this;
    if (N == 16) return Zeroes();
    SuperVector result;
    Unroller<1, 16>::iterator([&,v=this](auto const i) { constexpr uint8_t n = i.value; if (N == n) result = {_mm_srli_epi32(v->u.v128[0], n)}; });
    return result;
}

template <>
really_inline SuperVector<16> SuperVector<16>::vshr_64 (uint8_t const N) const
{
    if (N == 0) return *this;
    if (N == 16) return Zeroes();
    SuperVector result;
    Unroller<1, 16>::iterator([&,v=this](auto const i) { constexpr uint8_t n = i.value; if (N == n) result = {_mm_srli_epi64(v->u.v128[0], n)}; });
    return result;
}

template <>
really_inline SuperVector<16> SuperVector<16>::vshr_128(uint8_t const N) const
{
    if (N == 0) return *this;
    if (N == 16) return Zeroes();
    SuperVector result;
    Unroller<1, 16>::iterator([&,v=this](auto const i) { constexpr uint8_t n = i.value; if (N == n) result = {_mm_srli_si128(v->u.v128[0], n)}; });
    return result;
}

template <>
really_inline SuperVector<16> SuperVector<16>::vshr(uint8_t const N) const
{
    return vshr_128(N);
}

template <>
really_inline SuperVector<16> SuperVector<16>::operator>>(uint8_t const N) const
{
    return vshr_128(N);
}

template <>
really_inline SuperVector<16> SuperVector<16>::operator<<(uint8_t const N) const
{
    return vshl_128(N);
}

template<>
really_inline SuperVector<16> SuperVector<16>::Ones_vshr(uint8_t const N)
{
    if (N == 0) return Ones();
    else return Ones().vshr_128(N);
}

template<>
really_inline SuperVector<16> SuperVector<16>::Ones_vshl(uint8_t const N)
{
    if (N == 0) return Ones();
    else return Ones().vshr_128(N);
}

template <>
really_inline SuperVector<16> SuperVector<16>::loadu(void const *ptr)
{
    return _mm_loadu_si128((const m128 *)ptr);
}

template <>
really_inline SuperVector<16> SuperVector<16>::load(void const *ptr)
{
    assert(ISALIGNED_N(ptr, alignof(SuperVector::size)));
    ptr = vectorscan_assume_aligned(ptr, SuperVector::size);
    return _mm_load_si128((const m128 *)ptr);
}

template <>
really_inline SuperVector<16> SuperVector<16>::loadu_maskz(void const *ptr, uint8_t const len)
{
    SuperVector mask = Ones_vshr(16 -len);
    SuperVector v = _mm_loadu_si128((const m128 *)ptr);
    return mask & v;
}

template<>
really_inline SuperVector<16> SuperVector<16>::alignr(SuperVector<16> &other, int8_t offset)
{
#if defined(HAVE__BUILTIN_CONSTANT_P)
    if (__builtin_constant_p(offset)) {
        if (offset == 16) {
            return *this;
        } else {
            return {_mm_alignr_epi8(u.v128[0], other.u.v128[0], offset)};
        }
    }
#endif
    switch(offset) {
    case 0: return other; break;
    case 1: return {_mm_alignr_epi8(u.v128[0], other.u.v128[0], 1)}; break;
    case 2: return {_mm_alignr_epi8(u.v128[0], other.u.v128[0], 2)}; break;
    case 3: return {_mm_alignr_epi8(u.v128[0], other.u.v128[0], 3)}; break;
    case 4: return {_mm_alignr_epi8(u.v128[0], other.u.v128[0], 4)}; break;
    case 5: return {_mm_alignr_epi8(u.v128[0], other.u.v128[0], 5)}; break;
    case 6: return {_mm_alignr_epi8(u.v128[0], other.u.v128[0], 6)}; break;
    case 7: return {_mm_alignr_epi8(u.v128[0], other.u.v128[0], 7)}; break;
    case 8: return {_mm_alignr_epi8(u.v128[0], other.u.v128[0], 8)}; break;
    case 9: return {_mm_alignr_epi8(u.v128[0], other.u.v128[0], 9)}; break;
    case 10: return {_mm_alignr_epi8(u.v128[0], other.u.v128[0], 10)}; break;
    case 11: return {_mm_alignr_epi8(u.v128[0], other.u.v128[0], 11)}; break;
    case 12: return {_mm_alignr_epi8(u.v128[0], other.u.v128[0], 12)}; break;
    case 13: return {_mm_alignr_epi8(u.v128[0], other.u.v128[0], 13)}; break;
    case 14: return {_mm_alignr_epi8(u.v128[0], other.u.v128[0], 14)}; break;
    case 15: return {_mm_alignr_epi8(u.v128[0], other.u.v128[0], 15)}; break;
    default: break;
    }
    return *this;
}

template<>
template<>
really_inline SuperVector<16> SuperVector<16>::pshufb<true>(SuperVector<16> b)
{
    return {_mm_shuffle_epi8(u.v128[0], b.u.v128[0])};
}

template<>
really_inline SuperVector<16> SuperVector<16>::pshufb_maskz(SuperVector<16> b, uint8_t const len)
{
    SuperVector mask = Ones_vshr(16 -len);
    return mask & pshufb(b);
}

#endif // SIMD_IMPL_HPP
