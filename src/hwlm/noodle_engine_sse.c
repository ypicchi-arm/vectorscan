/*
 * Copyright (c) 2015-2017, Intel Corporation
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

/* noodle scan parts for SSE */

static really_inline m128 getMask(u8 c, bool noCase) {
    u8 k = caseClear8(c, noCase);
    return set1_16x8(k);
}

static really_inline m128 getCaseMask(void) {
    return set1_16x8(0xdf);
}

static really_inline
hwlm_error_t scanSingleUnaligned(const struct noodTable *n, const u8 *buf,
                                 size_t len, size_t offset, m128 caseMask, m128 mask1,
                                 const struct cb_info *cbi, size_t start,
                                 size_t end) {
    const u8 *d = buf + offset;
    DEBUG_PRINTF("start %zu end %zu offset %zu\n", start, end, offset);
    const size_t l = end - start;

    m128 v = and128(loadu128(d), caseMask);

    u32 buf_off = start - offset;
    u32 mask = ((1 << l) - 1) << buf_off;
    u32 z = mask & movemask128(eq128(mask1, v));
    DEBUG_PRINTF("mask 0x%08x z 0x%08x\n", mask, z);

    return single_zscan(n, d, buf, &z, len, cbi);
}

static really_inline
hwlm_error_t scanDoubleUnaligned(const struct noodTable *n, const u8 *buf,
                                 size_t len, size_t offset,
                                 m128 caseMask, m128 mask1, m128 mask2,
                                 const struct cb_info *cbi, size_t start,
                                 size_t end) {
    const u8 *d = buf + offset;
    DEBUG_PRINTF("start %zu end %zu offset %zu\n", start, end, offset);
    size_t l = end - start;
    u32 buf_off = start - offset;

    m128 v = and128(loadu128(d), caseMask);

    // mask out where we can't match
    u32 mask = ((1 << l) - 1) << buf_off;
    u32 z = mask & movemask128(and128(lshiftbyte_m128(eq128(mask1, v), 1), eq128(mask2, v)));
    DEBUG_PRINTF("mask 0x%08x z 0x%08x\n", mask, z);

    return double_zscan(n, d, buf, &z, len, cbi);
}

static really_inline
hwlm_error_t scanSingleFast(const struct noodTable *n, const u8 *buf,
                            size_t len, m128 caseMask, m128 mask1,
                            const struct cb_info *cbi, size_t start,
                            size_t end) {
    const u8 *d = buf + start, *e = buf + end;
    assert(d < e);

    const u8 *base = ROUNDDOWN_PTR(d, 64);
    for (; d < e; d += 16) {
        m128 v = and128(load128(d), caseMask);
        u32 z = movemask128(eq128(mask1, v));

        // On large packet buffers, this prefetch appears to get us about 2%.
        __builtin_prefetch(base + 128);
        DEBUG_PRINTF("z 0x%08x\n", z);

        hwlm_error_t result = single_zscan(n, d, buf, &z, len, cbi);
        if (unlikely(result != HWLM_SUCCESS))
	    return result;
    }
    return HWLM_SUCCESS;
}

static really_inline
hwlm_error_t scanDoubleFast(const struct noodTable *n, const u8 *buf,
                            size_t len, m128 caseMask, m128 mask1,
                            m128 mask2, const struct cb_info *cbi, size_t start,
                            size_t end) {
    const u8 *d = buf + start, *e = buf + end;
    assert(d < e);
    m128 lastz1 = zeroes128();

    const u8 *base = ROUNDDOWN_PTR(d, 64);
    for (; d < e; d += 16) {
        m128 v = and128(load128(d), caseMask);
        m128 z1 = eq128(mask1, v);
        m128 z2 = eq128(mask2, v);
        u32 z = movemask128(and128(palignr(z1, lastz1, 15), z2));
        lastz1 = z1;

        // On large packet buffers, this prefetch appears to get us about 2%.
        __builtin_prefetch(base + 128);
        DEBUG_PRINTF("z 0x%08x\n", z);

        hwlm_error_t result = double_zscan(n, d, buf, &z, len, cbi);
        if (unlikely(result != HWLM_SUCCESS))
	    return result;

    }
    return HWLM_SUCCESS;
}
