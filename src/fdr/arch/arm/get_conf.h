/*
 * Copyright (c) 2015-2017, Intel Corporation
 * Copyright (c) 2025, Arm Limited
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

static really_inline
void get_conf_stride_1(const u8 *itPtr, UNUSED const u8 *start_ptr,
                       UNUSED const u8 *end_ptr, u32 domain_mask_32,
                       const u64a *ft, u64a *conf0, u64a *conf8, m128 *s) {
    /* +1: the zones ensure that we can read the byte at z->end */
    assert(itPtr >= start_ptr && itPtr + ITER_BYTES <= end_ptr);
    u64a domain_mask = domain_mask_32;
#if defined(HAVE_NEON)
    uint8x16_t input = vld1q_u8(itPtr);
    uint8x16_t shifted_input = vextq_u8(input, vdupq_n_u8(0), 1);

    uint16x8_t even = vreinterpretq_u16_u8(input);
    uint16x8_t odd = vreinterpretq_u16_u8(shifted_input);
    //between those two we have 15 values. The last one will still be scalar.

    uint16x8_t vect_domain_mask = vdupq_n_u16(domain_mask);
    even = vandq_u16(vect_domain_mask, even);
    odd = vandq_u16(vect_domain_mask, odd);

    uint16_t reach0  = vgetq_lane_u16(even, 0);
    uint16_t reach1  = vgetq_lane_u16(odd, 0);
    uint16_t reach2  = vgetq_lane_u16(even, 1);
    uint16_t reach3  = vgetq_lane_u16(odd, 1);
    uint16_t reach4  = vgetq_lane_u16(even, 2);
    uint16_t reach5  = vgetq_lane_u16(odd, 2);
    uint16_t reach6  = vgetq_lane_u16(even, 3);
    uint16_t reach7  = vgetq_lane_u16(odd, 3);
    uint16_t reach8  = vgetq_lane_u16(even, 4);
    uint16_t reach9  = vgetq_lane_u16(odd, 4);
    uint16_t reach10 = vgetq_lane_u16(even, 5);
    uint16_t reach11 = vgetq_lane_u16(odd, 5);
    uint16_t reach12 = vgetq_lane_u16(even, 6);
    uint16_t reach13 = vgetq_lane_u16(odd, 6);
    uint16_t reach14 = vgetq_lane_u16(even, 7);
    uint16_t reach15 = domain_mask & unaligned_load_u16(itPtr + 15);
#else
    u64a it_hi = *(const u64a *)itPtr;
    u64a it_lo = *(const u64a *)(itPtr + 8);
    u64a reach0  = domain_mask & it_hi;
    u64a reach1  = domain_mask & (it_hi >> 8);
    u64a reach2  = domain_mask & (it_hi >> 16);
    u64a reach3  = domain_mask & (it_hi >> 24);
    u64a reach4  = domain_mask & (it_hi >> 32);
    u64a reach5  = domain_mask & (it_hi >> 40);
    u64a reach6  = domain_mask & (it_hi >> 48);
    u64a reach7  = domain_mask & ((it_hi >> 56) | (it_lo << 8));
    u64a reach8  = domain_mask & it_lo;
    u64a reach9  = domain_mask & (it_lo >> 8);
    u64a reach10 = domain_mask & (it_lo >> 16);
    u64a reach11 = domain_mask & (it_lo >> 24);
    u64a reach12 = domain_mask & (it_lo >> 32);
    u64a reach13 = domain_mask & (it_lo >> 40);
    u64a reach14 = domain_mask & (it_lo >> 48);
    u64a reach15 = domain_mask & unaligned_load_u32(itPtr + 15);
#endif

    m128 st0  = load_m128_from_u64a(ft + reach0);
    m128 st1  = lshiftbyte_m128(load_m128_from_u64a(ft + reach1), 1);
    m128 st2  = lshiftbyte_m128(load_m128_from_u64a(ft + reach2), 2);
    m128 st3  = lshiftbyte_m128(load_m128_from_u64a(ft + reach3), 3);
    m128 st4  = lshiftbyte_m128(load_m128_from_u64a(ft + reach4), 4);
    m128 st5  = lshiftbyte_m128(load_m128_from_u64a(ft + reach5), 5);
    m128 st6  = lshiftbyte_m128(load_m128_from_u64a(ft + reach6), 6);
    m128 st7  = lshiftbyte_m128(load_m128_from_u64a(ft + reach7), 7);
    m128 st8  = load_m128_from_u64a(ft + reach8);
    m128 st9  = lshiftbyte_m128(load_m128_from_u64a(ft + reach9), 1);
    m128 st10 = lshiftbyte_m128(load_m128_from_u64a(ft + reach10), 2);
    m128 st11 = lshiftbyte_m128(load_m128_from_u64a(ft + reach11), 3);
    m128 st12 = lshiftbyte_m128(load_m128_from_u64a(ft + reach12), 4);
    m128 st13 = lshiftbyte_m128(load_m128_from_u64a(ft + reach13), 5);
    m128 st14 = lshiftbyte_m128(load_m128_from_u64a(ft + reach14), 6);
    m128 st15 = lshiftbyte_m128(load_m128_from_u64a(ft + reach15), 7);

    st0 = or128(st0, st1);
    st2 = or128(st2, st3);
    st4 = or128(st4, st5);
    st6 = or128(st6, st7);
    st0 = or128(st0, st2);
    st4 = or128(st4, st6);
    st0 = or128(st0, st4);

    st8 = or128(st8, st9);
    st10 = or128(st10, st11);
    st12 = or128(st12, st13);
    st14 = or128(st14, st15);
    st8 = or128(st8, st10);
    st12 = or128(st12, st14);
    st8 = or128(st8, st12);

    m128 st = or128(*s, st0);
    *conf0 = movq(st) ^ ~0ULL;
    st = rshiftbyte_m128(st, 8);
    st = or128(st, st8);

    *conf8 = movq(st) ^ ~0ULL;
    *s = rshiftbyte_m128(st, 8);
}

static really_inline
void get_conf_stride_2(const u8 *itPtr, UNUSED const u8 *start_ptr,
                       UNUSED const u8 *end_ptr, u32 domain_mask,
                       const u64a *ft, u64a *conf0, u64a *conf8, m128 *s) {
    assert(itPtr >= start_ptr && itPtr + ITER_BYTES <= end_ptr);

#if defined(HAVE_NEON)
    uint8x16_t input = vld1q_u8(itPtr);
    uint16x8_t even = vreinterpretq_u16_u8(input);

    uint16x8_t vect_domain_mask = vdupq_n_u16(domain_mask);
    even = vandq_u16(vect_domain_mask, even);

    uint16_t reach0  = vgetq_lane_u16(even, 0);
    uint16_t reach2  = vgetq_lane_u16(even, 1);
    uint16_t reach4  = vgetq_lane_u16(even, 2);
    uint16_t reach6  = vgetq_lane_u16(even, 3);

    m128 st0 = load_m128_from_u64a(ft + reach0);
    m128 st2 = load_m128_from_u64a(ft + reach2);
    m128 st4 = load_m128_from_u64a(ft + reach4);
    m128 st6 = load_m128_from_u64a(ft + reach6);

    uint16_t reach8  = vgetq_lane_u16(even, 4);
    uint16_t reach10 = vgetq_lane_u16(even, 5);
    uint16_t reach12 = vgetq_lane_u16(even, 6);
    uint16_t reach14 = vgetq_lane_u16(even, 7);

    m128 st8 = load_m128_from_u64a(ft + reach8);
    m128 st10 = load_m128_from_u64a(ft + reach10);
    m128 st12 = load_m128_from_u64a(ft + reach12);
    m128 st14 = load_m128_from_u64a(ft + reach14);
#else
    u64a reach0 = domain_mask & unaligned_load_u32(itPtr);
    u64a reach2 = domain_mask & unaligned_load_u32(itPtr + 2);
    u64a reach4 = domain_mask & unaligned_load_u32(itPtr + 4);
    u64a reach6 = domain_mask & unaligned_load_u32(itPtr + 6);

    m128 st0 = load_m128_from_u64a(ft + reach0);
    m128 st2 = load_m128_from_u64a(ft + reach2);
    m128 st4 = load_m128_from_u64a(ft + reach4);
    m128 st6 = load_m128_from_u64a(ft + reach6);

    u64a reach8 = domain_mask & unaligned_load_u32(itPtr + 8);
    u64a reach10 = domain_mask & unaligned_load_u32(itPtr + 10);
    u64a reach12 = domain_mask & unaligned_load_u32(itPtr + 12);
    u64a reach14 = domain_mask & unaligned_load_u32(itPtr + 14);

    m128 st8 = load_m128_from_u64a(ft + reach8);
    m128 st10 = load_m128_from_u64a(ft + reach10);
    m128 st12 = load_m128_from_u64a(ft + reach12);
    m128 st14 = load_m128_from_u64a(ft + reach14);
#endif

    st2  = lshiftbyte_m128(st2, 2);
    st4  = lshiftbyte_m128(st4, 4);
    st6  = lshiftbyte_m128(st6, 6);

    *s = or128(*s, st0);
    *s = or128(*s, st2);
    *s = or128(*s, st4);
    *s = or128(*s, st6);

    *conf0 = movq(*s);
    *s = rshiftbyte_m128(*s, 8);
    *conf0 ^= ~0ULL;

    st10 = lshiftbyte_m128(st10, 2);
    st12 = lshiftbyte_m128(st12, 4);
    st14 = lshiftbyte_m128(st14, 6);

    *s = or128(*s, st8);
    *s = or128(*s, st10);
    *s = or128(*s, st12);
    *s = or128(*s, st14);

    *conf8 = movq(*s);
    *s = rshiftbyte_m128(*s, 8);
    *conf8 ^= ~0ULL;
}

static really_inline
void get_conf_stride_4(const u8 *itPtr, UNUSED const u8 *start_ptr,
                       UNUSED const u8 *end_ptr, u32 domain_mask,
                       const u64a *ft, u64a *conf0, u64a *conf8, m128 *s) {
    assert(itPtr >= start_ptr && itPtr + ITER_BYTES <= end_ptr);

    u64a reach0 = domain_mask & unaligned_load_u32(itPtr);
    u64a reach4 = domain_mask & unaligned_load_u32(itPtr + 4);
    u64a reach8 = domain_mask & unaligned_load_u32(itPtr + 8);
    u64a reach12 = domain_mask & unaligned_load_u32(itPtr + 12);

    m128 st0 = load_m128_from_u64a(ft + reach0);
    m128 st4 = load_m128_from_u64a(ft + reach4);
    m128 st8 = load_m128_from_u64a(ft + reach8);
    m128 st12 = load_m128_from_u64a(ft + reach12);

    st4 = lshiftbyte_m128(st4, 4);
    st12 = lshiftbyte_m128(st12, 4);

    *s = or128(*s, st0);
    *s = or128(*s, st4);
    *conf0 = movq(*s);
    *s = rshiftbyte_m128(*s, 8);
    *conf0 ^= ~0ULL;

    *s = or128(*s, st8);
    *s = or128(*s, st12);
    *conf8 = movq(*s);
    *s = rshiftbyte_m128(*s, 8);
    *conf8 ^= ~0ULL;
}