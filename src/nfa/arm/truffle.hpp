/*
 * Copyright (c) 2015-2017, Intel Corporation
 * Copyright (c) 2020-2021, VectorCamp PC
 * Copyright (c) 2023, Arm Limited
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
 * \brief Truffle: character class acceleration.
 *
 */

/*
 * blockSingleMask takes in a character set (as masks) and a string and return for each character
 * of the string wether or not it is part of the set.
 *
 * 'shuf_mask_lo_highclear' and 'shuf_mask_lo_highset' are 128-bit masks where each bit
 * represents whether or not a character is in the character set. The 'highclear' and
 * 'highset' in the name refers to the MSb of the byte of the character (allowing two
 * 128-bit masks to cover all 256 values).
 *
 * The masks are arrays of 16 bytes each and are encoded this way:
 * Let C be a character in the set. The bit describing that character is at byte[C%16] and
 * within that byte, it's at bit[C/16]
 * As example, 'a' = 0x61, so the resulting mask will be: 0x00 0x40 0x00 0x00 0x00 ...
 */
static really_inline
uint8x16_t blockSingleMaskNEON(uint8x16_t shuf_mask_lo_highclear, uint8x16_t shuf_mask_lo_highset, uint8x16_t chars) {

    const uint8x16_t highconst = vdupq_n_u8(0x80);
    const uint8x16_t pshub_mask = vdupq_n_u8(0x8f);
    const uint8x16_t unique_bit_per_lane_mask = (uint8x16_t)(vdupq_n_u64(0x8040201008040201));

    /*
     * svtbl does a table lookup. Each byte in the second argument indexes into the array of bytes
     * in shuf_mask_lo_highclear and saves the result in the corresponding byte of byte_select_low.
     * We mask the chars so that we are using the low nibble of char as the index but we keep the
     * MSb so that high characters (not represented by the highclear mask) become an index out of
     * bounds and result in a 0.
     */
    uint8x16_t byte_select_low = vqtbl1q_u8(shuf_mask_lo_highclear, vandq_u8(chars, pshub_mask));

    /*
     * We flip the MSb of the chars and do the same table lookup with the highset mask.
     * This way it's the characters with MSb cleared that will result in out of bands indexes.
     * This allows us to cover the full range (0-127 and 128-255)
     */
    uint8x16_t char_high_flipped = veorq_u8(chars, highconst);
    uint8x16_t byte_select_high = vqtbl1q_u8(shuf_mask_lo_highset, vandq_u8(char_high_flipped, pshub_mask));

    /*
     * We now have selected the byte that contain the bit corresponding to the char. We need to
     * further filter it, otherwise we'd get a match for any character % 16 to a searched character
     *
     * The low nibble was used previously to select the byte out of the mask. The high nibble is
     * used to select the bit out of the byte. So we shift everything right by 4.
     *
     * Using svtbl, we can make an array where each element is a different bit. Using the high
     * nibble we can get a mask selecting only the bit out of a byte that may have the relevant
     * charset char.
     */
    uint8x16_t char_high_nibble = vshrq_n_u8(chars, 4);
    uint8x16_t bit_select = vqtbl1q_u8(unique_bit_per_lane_mask, char_high_nibble);

    /*
     * For every lane, only one of the byte selected may have a value, so we can OR them. We
     * then apply the bit_select mask. What is left is the bit in the charset encoding the
     * character in char. A non zero value means the char was in the charset
     *
     * The _x suffix only works if we process a full char vector. If we were to use a partial
     * vector, then _z and a mask would be required on this svand only. Otherwise, the disabled
     * lanes may have arbitrary values
     */
    uint8x16_t res = vandq_u8(vorrq_u8(byte_select_low, byte_select_high), bit_select);

    return res;
}

template <uint16_t S>
static really_inline
const SuperVector<S> blockSingleMask(SuperVector<S> shuf_mask_lo_highclear, SuperVector<S> shuf_mask_lo_highset, SuperVector<S> chars) {
    return blockSingleMaskNEON(*shuf_mask_lo_highclear.u.u8x16, *shuf_mask_lo_highset.u.u8x16, *chars.u.u8x16);
}

#ifdef HAVE_SVE

/*
 * blockSingleMask takes in a character set (as masks) and a string and return for each character
 * of the string wether or not it is part of the set.
 *
 * 'shuf_mask_lo_highclear' and 'shuf_mask_lo_highset' are 128-bit masks where each bit
 * represents whether or not a character is in the character set. The 'highclear' and
 * 'highset' in the name refers to the MSb of the byte of the character (allowing two
 * 128-bit masks to cover all 256 values).
 *
 * The masks are arrays of 16 bytes each and are encoded this way:
 * Let C be a character in the set. The bit describing that character is at byte[C%16] and
 * within that byte, it's at bit[C/16]
 * As example, 'a' = 0x61, so the resulting mask will be: 0x00 0x40 0x00 0x00 0x00 ...
 *
 * Assume both mask are 128b wide. If they are larger, the additional bits must be zero
 */
static really_inline
svuint8_t blockSingleMaskSVE(svuint8_t shuf_mask_lo_highclear, svuint8_t shuf_mask_lo_highset, svuint8_t chars) {

    const svuint8_t highconst = svdup_u8(0x80);
    const svuint8_t pshub_mask = svdup_u8(0x8f);
    const svuint8_t unique_bit_per_lane_mask = svreinterpret_u8(svdup_u64(0x8040201008040201));

    /*
     * svtbl does a table lookup. Each byte in the second argument indexes into the array of bytes
     * in shuf_mask_lo_highclear and saves the result in the corresponding byte of byte_select_low.
     * We mask the chars so that we are using the low nibble of char as the index but we keep the
     * MSb so that high characters (not represented by the highclear mask) become an index out of
     * bounds and result in a 0.
     */
    svuint8_t byte_select_low = svtbl(shuf_mask_lo_highclear, svand_x(svptrue_b8(), chars, pshub_mask));

    /*
     * We flip the MSb of the chars and do the same table lookup with the highset mask.
     * This way it's the characters with MSb cleared that will result in out of bands indexes.
     * This allows us to cover the full range (0-127 and 128-255)
     */
    svuint8_t char_high_flipped = sveor_x(svptrue_b8(), chars, highconst);
    svuint8_t byte_select_high = svtbl(shuf_mask_lo_highset, svand_x(svptrue_b8(), char_high_flipped, pshub_mask));

    /*
     * We now have selected the byte that contain the bit corresponding to the char. We need to
     * further filter it, otherwise we'd get a match for any character % 16 to a searched character
     *
     * The low nibble was used previously to select the byte out of the mask. The high nibble is
     * used to select the bit out of the byte. So we shift everything right by 4.
     *
     * Using svtbl, we can make an array where each element is a different bit. Using the high
     * nibble we can get a mask selecting only the bit out of a byte that may have the relevant
     * charset char.
     */
    svuint8_t char_high_nibble = svlsr_x(svptrue_b8(), chars, 4);
    svuint8_t bit_select = svtbl(unique_bit_per_lane_mask, char_high_nibble);
    /*
     * For every lane, only one of the byte selected may have a value, so we can OR them. We
     * then apply the bit_select mask. What is left is the bit in the charset encoding the
     * character in char. A non zero value means the char was in the charset
     *
     * The _x suffix only works if we process a full char vector. If we were to use a partial
     * vector, then _z and a mask would be required on this svand only. Otherwise, the disabled
     * lanes may have arbitrary values
     */
    return svand_x(svptrue_b8(), svorr_x(svptrue_b8(), byte_select_low, byte_select_high), bit_select);
}

#ifdef __ARM_NEON_SVE_BRIDGE
#include <arm_neon_sve_bridge.h> // Available from Clang 14 and gcc 14
static really_inline
svuint8_t blockSingleMask(svuint8_t shuf_mask_lo_highclear, svuint8_t shuf_mask_lo_highset, svuint8_t chars) {
    return svset_neonq(svundef_u8(), blockSingleMaskNEON(svget_neonq(shuf_mask_lo_highclear), svget_neonq(shuf_mask_lo_highset), svget_neonq(chars)));
}
#else
static really_inline
svuint8_t blockSingleMask(svuint8_t shuf_mask_lo_highclear, svuint8_t shuf_mask_lo_highset, svuint8_t chars) {
    return blockSingleMaskSVE(shuf_mask_lo_highclear, shuf_mask_lo_highset, chars);
}
#endif /* __ARM_NEON_SVE_BRIDGE */

#endif //HAVE_SVE