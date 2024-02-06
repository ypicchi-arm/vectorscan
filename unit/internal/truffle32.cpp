/*
 * Copyright (c) 2015-2017, Intel Corporation
 * Copyright (c) 2024, Arm Limited
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

#include "config.h"

#include "gtest/gtest.h"
#include "nfa/truffle.h"
#include "nfa/trufflecompile.h"
#include "util/charreach.h"
#include "util/simd_utils.h"

#include "util/arch.h"
#ifdef HAVE_SVE
using namespace ue2;

#define SKIP_IF_NARROW_SVE() \
    if(svcntb() < 32) {\
        std::cout << "[  SKIPPED ] System does not support 256b+ vectors" << std::endl;\
        return;\
    }

TEST(Truffle32, CompileDot) {
    SKIP_IF_NARROW_SVE()
    m256 mask;
    memset(&mask, 0, sizeof(mask));

    CharReach chars;

    chars.setall();

    truffleBuildMasks32(chars, (u8 *)&mask);

    CharReach out = truffle2cr32((u8 *)&mask);

    ASSERT_EQ(out, chars);

}

TEST(Truffle32, CompileChars) {
    SKIP_IF_NARROW_SVE()
    m256 mask;

    CharReach chars;

    // test one char at a time
    for (u32 c = 0; c < 256; ++c) {
        mask = zeroes256();
        chars.clear();
        chars.set((u8)c);
        truffleBuildMasks32(chars, (u8 *)&mask);
        CharReach out = truffle2cr32((u8 *)&mask);
        ASSERT_EQ(out, chars);
    }

    // set all chars up to dot
    for (u32 c = 0; c < 256; ++c) {
        mask = zeroes256();
        chars.set((u8)c);
        truffleBuildMasks32(chars, (u8 *)&mask);
        CharReach out = truffle2cr32((u8 *)&mask);
        ASSERT_EQ(out, chars);
    }

    // unset all chars from dot
    for (u32 c = 0; c < 256; ++c) {
        mask = zeroes256();
        chars.clear((u8)c);
        truffleBuildMasks32(chars, (u8 *)&mask);
        CharReach out = truffle2cr32((u8 *)&mask);
        ASSERT_EQ(out, chars);
    }

}

TEST(Truffle32, ExecNoMatch1) {
    SKIP_IF_NARROW_SVE()
    m256 mask;
    memset(&mask, 0, sizeof(mask));

    CharReach chars;

    chars.set('a');

    truffleBuildMasks32(chars, (u8 *)&mask);

    char t1[] = "bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb\xff";

    for (size_t i = 0; i < 16; i++) {
        const u8 *rv = truffleExec32(mask, (u8 *)t1 + i, (u8 *)t1 + strlen(t1));

        ASSERT_EQ((size_t)t1 + strlen(t1), (size_t)rv);
    }
}

TEST(Truffle32, ExecNoMatch2) {
    SKIP_IF_NARROW_SVE()
    m256 mask;

    CharReach chars;

    chars.set('a');
    chars.set('B');

    truffleBuildMasks32(chars, (u8 *)&mask);

    char t1[] = "bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb";

    for (size_t i = 0; i < 16; i++) {
        const u8 *rv = truffleExec32(mask, (u8 *)t1 + i, (u8 *)t1 + strlen(t1));

        ASSERT_EQ((size_t)t1 + strlen(t1), (size_t)rv);
    }
}

TEST(Truffle32, ExecNoMatch3) {
    SKIP_IF_NARROW_SVE()
    m256 mask;

    CharReach chars;

    chars.set('V'); /* V = 0x56, e = 0x65 */

    truffleBuildMasks32(chars, (u8 *)&mask);

    char t1[] = "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee";

    for (size_t i = 0; i < 16; i++) {
        const u8 *rv = truffleExec32(mask, (u8 *)t1 + i, (u8 *)t1 + strlen(t1));

        ASSERT_EQ((size_t)t1 + strlen(t1), (size_t)rv);
    }
}

TEST(Truffle32, ExecMiniMatch0) {
    SKIP_IF_NARROW_SVE()
    m256 mask;

    CharReach chars;
    chars.set('a');

    truffleBuildMasks32(chars, (u8 *)&mask);

    char t1[] = "a";

    const u8 *rv = truffleExec32(mask, (u8 *)t1, (u8 *)t1 + strlen(t1));

    ASSERT_EQ((size_t)t1, (size_t)rv);
}

TEST(Truffle32, ExecMiniMatch1) {
    SKIP_IF_NARROW_SVE()
    m256 mask;

    CharReach chars;
    chars.set('a');

    truffleBuildMasks32(chars, (u8 *)&mask);

    char t1[] = "bbbbbbbabbb";

    const u8 *rv = truffleExec32(mask, (u8 *)t1, (u8 *)t1 + strlen(t1));

    ASSERT_EQ((size_t)t1 + 7, (size_t)rv);
}

TEST(Truffle32, ExecMiniMatch2) {
    SKIP_IF_NARROW_SVE()
    m256 mask;

    CharReach chars;
    chars.set(0);

    truffleBuildMasks32(chars, (u8 *)&mask);

    char t1[] = "bbbbbbb\0bbb";

    const u8 *rv = truffleExec32(mask, (u8 *)t1, (u8 *)t1 + 11);

    ASSERT_EQ((size_t)t1 + 7, (size_t)rv);
}

TEST(Truffle32, ExecMiniMatch3) {
    SKIP_IF_NARROW_SVE()
    m256 mask;

    CharReach chars;
    chars.set('a');

    truffleBuildMasks32(chars, (u8 *)&mask);

    char t1[] = "\0\0\0\0\0\0\0a\0\0\0";

    const u8 *rv = truffleExec32(mask, (u8 *)t1, (u8 *)t1 + 11);

    ASSERT_EQ((size_t)t1 + 7, (size_t)rv);
}

TEST(Truffle32, ExecMatchBig) {
    SKIP_IF_NARROW_SVE()
    m256 mask;

    CharReach chars;
    chars.set('a');

    truffleBuildMasks32(chars, (u8 *)&mask);

    std::array<u8, 400> t1;
    t1.fill('b');
    t1[120] = 'a';

    for (size_t i = 0; i < 16; i++) {
        const u8 *rv = truffleExec32(mask, (u8 *)t1.data() + i, (u8 *)t1.data() + 399);

        ASSERT_LE(((size_t)t1.data() + 120) & ~0xf, (size_t)rv);
    }
}

TEST(Truffle32, ExecMatch1) {
    SKIP_IF_NARROW_SVE()
    m256 mask;

    CharReach chars;

    chars.set('a');

    truffleBuildMasks32(chars, (u8 *)&mask);

    /*          0123456789012345678901234567890 */
    char t1[] = "bbbbbbbbbbbbbbbbbabbbbbbbbbbbbbbbbbbbbbbbbbbbbbbabbbbbbbbbbbb";

    for (size_t i = 0; i < 16; i++) {
        const u8 *rv = truffleExec32(mask, (u8 *)t1 + i, (u8 *)t1 + strlen(t1));

        ASSERT_EQ((size_t)t1 + 17, (size_t)rv);
    }
}

TEST(Truffle32, ExecMatch2) {
    SKIP_IF_NARROW_SVE()
    m256 mask;

    CharReach chars;

    chars.set('a');

    truffleBuildMasks32(chars, (u8 *)&mask);

    /*          0123456789012345678901234567890 */
    char t1[] = "bbbbbbbbbbbbbbbbbaaaaaaaaaaaaaaaabbbbbbbbbbbbbbbabbbbbbbbbbbb";

    for (size_t i = 0; i < 16; i++) {
        const u8 *rv = truffleExec32(mask, (u8 *)t1 + i, (u8 *)t1 + strlen(t1));

        ASSERT_EQ((size_t)t1 + 17, (size_t)rv);
    }
}

TEST(Truffle32, ExecMatch3) {
    SKIP_IF_NARROW_SVE()
    m256 mask;

    CharReach chars;

    chars.set('a');
    chars.set('B');

    truffleBuildMasks32(chars, (u8 *)&mask);

    /*          0123456789012345678901234567890 */
    char t1[] = "bbbbbbbbbbbbbbbbbBaaaaaaaaaaaaaaabbbbbbbbbbbbbbbabbbbbbbbbbbb";

    for (size_t i = 0; i < 16; i++) {
        const u8 *rv = truffleExec32(mask, (u8 *)t1 + i, (u8 *)t1 + strlen(t1));

        ASSERT_EQ((size_t)t1 + 17, (size_t)rv);
    }
}

TEST(Truffle32, ExecMatch4) {
    SKIP_IF_NARROW_SVE()
    m256 mask;

    CharReach chars;

    chars.set('a');
    chars.set('C');
    chars.set('A');
    chars.set('c');

    truffleBuildMasks32(chars, (u8 *)&mask);

    /*          0123456789012345678901234567890 */
    char t1[] = "bbbbbbbbbbbbbbbbbAaaaaaaaaaaaaaaabbbbbbbbbbbbbbbabbbbbbbbbbbb";
    char t2[] = "bbbbbbbbbbbbbbbbbCaaaaaaaaaaaaaaabbbbbbbbbbbbbbbabbbbbbbbbbbb";
    char t3[] = "bbbbbbbbbbbbbbbbbcaaaaaaaaaaaaaaabbbbbbbbbbbbbbbabbbbbbbbbbbb";
    char t4[] = "bbbbbbbbbbbbbbbbbaaaaaaaaaaaaaaaabbbbbbbbbbbbbbbabbbbbbbbbbbb";

    for (size_t i = 0; i < 16; i++) {
        const u8 *rv = truffleExec32(mask, (u8 *)t1 + i, (u8 *)t1 + strlen(t1));

        ASSERT_EQ((size_t)t1 + 17, (size_t)rv);

        rv = truffleExec32(mask, (u8 *)t2 + i, (u8 *)t2 + strlen(t1));

        ASSERT_EQ((size_t)t2 + 17, (size_t)rv);

        rv = truffleExec32(mask, (u8 *)t3 + i, (u8 *)t3 + strlen(t3));

        ASSERT_EQ((size_t)t3 + 17, (size_t)rv);

        rv = truffleExec32(mask, (u8 *)t4 + i, (u8 *)t4 + strlen(t4));

        ASSERT_EQ((size_t)t4 + 17, (size_t)rv);
    }
}

TEST(Truffle32, ExecMatch5) {
    SKIP_IF_NARROW_SVE()
    m256 mask;

    CharReach chars;

    chars.set('a');

    truffleBuildMasks32(chars, (u8 *)&mask);

    char t1[] = "bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb";

    for (size_t i = 0; i < 31; i++) {
        t1[48 - i] = 'a';
        const u8 *rv = truffleExec32(mask, (u8 *)t1, (u8 *)t1 + strlen(t1));

        ASSERT_EQ((size_t)&t1[48 - i], (size_t)rv);
    }
}

TEST(Truffle32, ExecMatch6) {
    SKIP_IF_NARROW_SVE()
    m256 mask;

    CharReach chars;

    // [0-Z] - includes some graph chars
    chars.setRange('0', 'Z');

    truffleBuildMasks32(chars, (u8 *)&mask);

    std::array<u8, 128> t1;
    t1.fill('*'); // it's full of stars!

    for (u8 c = '0'; c <= 'Z'; c++) {
        t1[17] = c;
        const u8 *rv = truffleExec32(mask, (u8 *)t1.data(), (u8 *)t1.data() + 128);

        ASSERT_EQ((size_t)t1.data() + 17, (size_t)rv);
    }
}

TEST(Truffle32, ExecMatch7) {
    SKIP_IF_NARROW_SVE()
    m256 mask;

    CharReach chars;

    // hi bits
    chars.setRange(127, 255);

    truffleBuildMasks32(chars, (u8 *)&mask);

    std::array<u8, 128> t1;
    t1.fill('*'); // it's full of stars!

    for (unsigned int c = 127; c <= 255; c++) {
        t1[40] = (u8)c;
        const u8 *rv = truffleExec32(mask, (u8 *)t1.data(), (u8 *)t1.data() + 128);

        ASSERT_EQ((size_t)t1.data() + 40, (size_t)rv);
    }
}

TEST(ReverseTruffle32, ExecNoMatch1) {
    SKIP_IF_NARROW_SVE()
    m256 mask;

    CharReach chars;
    chars.set('a');

    truffleBuildMasks32(chars, (u8 *)&mask);

    char t[] = " bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb";
    char *t1 = t + 1;
    size_t len = strlen(t1);

    for (size_t i = 0; i < 16; i++) {
        const u8 *rv = rtruffleExec32(mask, (u8 *)t1, (u8 *)t1 + len - i);
        ASSERT_EQ((const u8 *)t, rv);
    }
}

TEST(ReverseTruffle32, ExecNoMatch2) {
    SKIP_IF_NARROW_SVE()
    m256 mask;

    CharReach chars;

    chars.set('a');
    chars.set('B');

    truffleBuildMasks32(chars, (u8 *)&mask);

    char t[] = " bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb";
    char *t1 = t + 1;
    size_t len = strlen(t1);

    for (size_t i = 0; i < 16; i++) {
        const u8 *rv = rtruffleExec32(mask, (u8 *)t1, (u8 *)t1 + len - i);
        ASSERT_EQ((const u8 *)t, rv);
    }
}

TEST(ReverseTruffle32, ExecNoMatch3) {
    SKIP_IF_NARROW_SVE()
    m256 mask;

    CharReach chars;
    chars.set('V'); /* V = 0x56, e = 0x65 */

    truffleBuildMasks32(chars, (u8 *)&mask);

    char t[] = "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee";
    char *t1 = t + 1;
    size_t len = strlen(t1);

    for (size_t i = 0; i < 16; i++) {
        const u8 *rv = rtruffleExec32(mask, (u8 *)t1, (u8 *)t1 + len - i);
        ASSERT_EQ((const u8 *)t, rv);
    }
}

TEST(ReverseTruffle32, ExecMiniMatch0) {
    SKIP_IF_NARROW_SVE()
    m256 mask;

    CharReach chars;
    chars.set('a');

    truffleBuildMasks32(chars, (u8 *)&mask);

    char t1[] = "a";

    const u8 *rv = rtruffleExec32(mask, (u8 *)t1, (u8 *)t1 + strlen(t1));

    ASSERT_EQ((size_t)t1, (size_t)rv);
}

TEST(ReverseTruffle32, ExecMiniMatch1) {
    SKIP_IF_NARROW_SVE()
    m256 mask;

    CharReach chars;
    chars.set('a');

    truffleBuildMasks32(chars, (u8 *)&mask);

    /*          0123456789012345678901234567890 */
    char t1[] = "bbbbbbbabbbb";
    size_t len = strlen(t1);

    const u8 *rv = rtruffleExec32(mask, (u8 *)t1, (u8 *)t1 + len);
    ASSERT_NE((const u8 *)t1 - 1, rv); // not found
    EXPECT_EQ('a', (char)*rv);
    ASSERT_EQ((const u8 *)t1 + 7, rv);
}

TEST(ReverseTruffle32, ExecMiniMatch2) {
    SKIP_IF_NARROW_SVE()
    m256 mask;

    CharReach chars;
    chars.set('a');

    truffleBuildMasks32(chars, (u8 *)&mask);

    /*          0123456789012345678901234567890 */
    char t1[] = "babbbbbabbbb";
    size_t len = strlen(t1);

    const u8 *rv = rtruffleExec32(mask, (u8 *)t1, (u8 *)t1 + len);
    ASSERT_NE((const u8 *)t1 - 1, rv); // not found
    EXPECT_EQ('a', (char)*rv);
    ASSERT_EQ((const u8 *)t1 + 7, rv);
}


TEST(ReverseTruffle32, ExecMatch1) {
    SKIP_IF_NARROW_SVE()
    m256 mask;

    CharReach chars;
    chars.set('a');

    truffleBuildMasks32(chars, (u8 *)&mask);

    /*          0123456789012345678901234567890 */
    char t1[] = "bbbbbbabbbbbbbbbbabbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb";
    size_t len = strlen(t1);

    for (size_t i = 0; i < 16; i++) {
        const u8 *rv = rtruffleExec32(mask, (u8 *)t1, (u8 *)t1 + len - i);
        ASSERT_NE((const u8 *)t1 - 1, rv); // not found
        EXPECT_EQ('a', (char)*rv);
        ASSERT_EQ((const u8 *)t1 + 17, rv);
    }
}

TEST(ReverseTruffle32, ExecMatch2) {
    SKIP_IF_NARROW_SVE()
    m256 mask;

    CharReach chars;
    chars.set('a');

    truffleBuildMasks32(chars, (u8 *)&mask);

    /*          0123456789012345678901234567890 */
    char t1[] = "bbbbabbbbbbbbbbbbaaaaaaaaaaaaaaaabbbbbbbbbbbbbbbbbbbbbbbbbbbb";
    size_t len = strlen(t1);

    for (size_t i = 0; i < 16; i++) {
        const u8 *rv = rtruffleExec32(mask, (u8 *)t1, (u8 *)t1 + len - i);
        ASSERT_NE((const u8 *)t1 - 1, rv); // not found
        EXPECT_EQ('a', (char)*rv);
        ASSERT_EQ((const u8 *)t1 + 32, rv);
    }
}

TEST(ReverseTruffle32, ExecMatch3) {
    SKIP_IF_NARROW_SVE()
    m256 mask;

    CharReach chars;
    chars.set('a');
    chars.set('B');

    truffleBuildMasks32(chars, (u8 *)&mask);

    /*          0123456789012345678901234567890 */
    char t1[] = "bbbbbbbbbbbbbbbbbaaaaaaaaaaaaaaaBbbbbbbbbbbbbbbbbbbbbbbbbbbbb";
    size_t len = strlen(t1);

    for (size_t i = 0; i < 16; i++) {
        const u8 *rv = rtruffleExec32(mask, (u8 *)t1, (u8 *)t1 + len - i);
        ASSERT_NE((const u8 *)t1 - 1, rv); // not found
        EXPECT_EQ('B', (char)*rv);
        ASSERT_EQ((const u8 *)t1 + 32, rv);
    }

    // check that we match the 'a' bytes as well.
    ASSERT_EQ('B', t1[32]);
    t1[32] = 'b';
    for (size_t i = 0; i < 16; i++) {
        const u8 *rv = rtruffleExec32(mask, (u8 *)t1, (u8 *)t1 + len - i);
        ASSERT_NE((const u8 *)t1 - 1, rv); // not found
        EXPECT_EQ('a', (char)*rv);
        ASSERT_EQ((const u8 *)t1 + 31, rv);
    }
}

TEST(ReverseTruffle32, ExecMatch4) {
    SKIP_IF_NARROW_SVE()
    m256 mask;

    CharReach chars;
    chars.set('a');
    chars.set('C');
    chars.set('A');
    chars.set('c');

    truffleBuildMasks32(chars, (u8 *)&mask);

    /*          0123456789012345678901234567890 */
    char t1[] = "bbbbbbbbbbbbbbbbbaaaaaaaaaaaaaaaAbbbbbbbbbbbbbbbbbbbbbbbbbbbb";
    char t2[] = "bbbbbbbbbbbbbbbbbaaaaaaaaaaaaaaaCbbbbbbbbbbbbbbbbbbbbbbbbbbbb";
    char t3[] = "bbbbbbbbbbbbbbbbbaaaaaaaaaaaaaaacbbbbbbbbbbbbbbbbbbbbbbbbbbbb";
    char t4[] = "bbbbbbbbbbbbbbbbbaaaaaaaaaaaaaaaabbbbbbbbbbbbbbbbbbbbbbbbbbbb";
    size_t len = strlen(t1);

    for (size_t i = 0; i < 16; i++) {
        const u8 *rv = rtruffleExec32(mask, (u8 *)t1, (u8 *)t1 + len - i);
        EXPECT_EQ('A', (char)*rv);
        ASSERT_EQ((const u8 *)t1 + 32, rv);

        rv = rtruffleExec32(mask, (u8 *)t2, (u8 *)t2 + len - i);
        EXPECT_EQ('C', (char)*rv);
        ASSERT_EQ((const u8 *)t2 + 32, rv);

        rv = rtruffleExec32(mask, (u8 *)t3, (u8 *)t3 + len - i);
        EXPECT_EQ('c', (char)*rv);
        ASSERT_EQ((const u8 *)t3 + 32, rv);

        rv = rtruffleExec32(mask, (u8 *)t4, (u8 *)t4 + len - i);
        EXPECT_EQ('a', (char)*rv);
        ASSERT_EQ((const u8 *)t4 + 32, rv);
    }
}

TEST(ReverseTruffle32, ExecMatch5) {
    SKIP_IF_NARROW_SVE()
    m256 mask;

    CharReach chars;
    chars.set('a');

    truffleBuildMasks32(chars, (u8 *)&mask);

    char t1[] = "bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb";
    size_t len = strlen(t1);

    for (size_t i = 0; i < len; i++) {
        t1[i] = 'a';
        const u8 *rv = rtruffleExec32(mask, (u8 *)t1, (u8 *)t1 + len);

        ASSERT_EQ((const u8 *)t1 + i, rv);
    }
}
#endif
