/*
 * Copyright (c) 2024-2025, Arm ltd
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

#include "direct_API/common.h"

#include "hwlm/noodle_internal.h"

#define COMPILE_SHORT_LITERAL(in_pattern, in_pattern_len)                      \
    size_t pattern_len = (in_pattern_len);                                     \
    const char *pattern = (in_pattern);                                        \
    hs_short_literal_compiled_pattern_t *database = nullptr;                   \
    hs_error_t compile_ret =                                                   \
        hs_compile_short_literal_search(pattern, pattern_len, &database);      \
    hs_error_t ret = 0;                                                        \
    (void)ret; /* suppress a cppcheck warning when SEARCH is not called */     \
    const char *buffer = nullptr;                                              \
    (void)buffer;                                                              \
    context_t context = {};                                                    \
    (void) context;

// expected match array here is the index of the start of match.
#define SEARCH_SHORT_LITERAL(in_buffer, in_buffer_len, in_expected_match,      \
                             in_expected_start_array)                          \
    {                                                                          \
        buffer = (in_buffer);                                                  \
        const size_t buffer_len = (in_buffer_len);                             \
        const size_t expected_match = (in_expected_match);                     \
        size_t expected_start_array[expected_match] =                          \
            BRACED_INIT_LIST in_expected_start_array;                          \
        size_t expected_end_array[expected_match] =                            \
            BRACED_INIT_LIST in_expected_start_array;                          \
        size_t expected_id_array[expected_match];                              \
        for (size_t i = 0; i < expected_match; i++) {                          \
            expected_end_array[i] += pattern_len;                              \
            expected_id_array[i] = 0;                                          \
        }                                                                      \
        context.expected_start_array = expected_start_array;                   \
        context.expected_end_array = expected_end_array;                       \
        context.expected_id_array = expected_id_array;                         \
        context.array_size = expected_match;                                   \
        context.number_matched = 0;                                            \
        context.number_wrong = 0;                                              \
                                                                               \
        ret = hs_short_literal_search(database, buffer, buffer_len, callback,  \
                                      &context);                               \
    }

static_assert(HS_SHORT_PATTERN_THRESHOLD == 8,
              "changing the threshold for short/long literal require changing "
              "the tests to still test the threshold behavior");

// ------------------------free tests-------------------------------------------

/*
hs_free_short_literal_pattern
    nullptr
    general
*/

TEST(short_literal_free, nullptr) {
    hs_short_literal_compiled_pattern_t *database = nullptr;
    hs_free_short_literal_pattern(database);
}

TEST(short_literal_free, general) {
    SETUP_MEM_LEAK_TEST();
    noodTable *clear_database =
        reinterpret_cast<noodTable *>(test_malloc(sizeof(noodTable)));
            hs_short_literal_compiled_pattern_t *database =
                reinterpret_cast<hs_short_literal_compiled_pattern_t *>(
                    clear_database);

    hs_free_short_literal_pattern(database);
    EXPECT_MEMORY_CLEAN();
    UNSET_MEM_LEAK_TEST();
}

// ------------------------compile tests----------------------------------------

/*
hs_compile_short_literal_search
    single char
    general
    8 char
    >8 char
    valid pattern including null char
    empty expression
    nullptr expression
    nullptr output
*/

TEST(short_literal_compile, single_char) {
    COMPILE_SHORT_LITERAL(PATTERN_1_CHAR, 1);
    EXPECT_COMPILE_SUCCESS("test_compile_short_literal_single_char");
    hs_free_short_literal_pattern(database);
}

TEST(short_literal_compile, general) {
    COMPILE_SHORT_LITERAL(PATTERN_5_CHAR, 5);
    EXPECT_COMPILE_SUCCESS("test_compile_short_literal_general");
    hs_free_short_literal_pattern(database);
}

TEST(short_literal_compile, max_length) {
    COMPILE_SHORT_LITERAL(PATTERN_8_CHAR, 8);
    EXPECT_COMPILE_SUCCESS("test_compile_short_literal_max_len");
    hs_free_short_literal_pattern(database);
}

TEST(short_literal_compile, too_long) {
    COMPILE_SHORT_LITERAL(PATTERN_10_CHAR, 10);
    EXPECT_COMPILE_FAILURE("test_compile_short_literal_too_long");
    hs_free_short_literal_pattern(database);
}

TEST(short_literal_compile, null_char) {
    COMPILE_SHORT_LITERAL(PATTERN_5_WITH_NULL, 5);
    EXPECT_COMPILE_SUCCESS("test_compile_short_literal_null_char");
    hs_free_short_literal_pattern(database);
}

#if !defined(RELEASE_BUILD)
// test asserts

TEST(short_literal_compile, empty_pattern) {
    hs_short_literal_compiled_pattern_t *database = nullptr;
    EXPECT_DEATH(
        hs_compile_short_literal_search(PATTERN_0_CHAR, 0, &database),
        "called with an empty pattern");
}

TEST(short_literal_compile, nullptr_pattern) {
    hs_short_literal_compiled_pattern_t *database = nullptr;
    EXPECT_DEATH(hs_compile_short_literal_search(nullptr, 5, &database),
                 "called with nullptr");
}

TEST(short_literal_compile, nullptr_database) {
    EXPECT_DEATH(hs_compile_short_literal_search(PATTERN_5_CHAR, 5, nullptr),
                 "called with nullptr");
}

#endif

// ------------------------search tests-----------------------------------------

/*
hs_short_literal_search
    general pattern
        match at start
        match middle (general)
        match index 15 (noodle cross over vector)
        match at end
        match the full pattern, not just the first pair
        match past end (2 char ok, then end, so missing some chars)
        bad caseness
        search several times
    single char pattern
        general match
        match at end
        no match
    buffer containing null char
        pattern with null char
        general pattern
    buff size 0
    nullptr pattern
    nullptr buffer
    nullptr callback
*/

TEST(short_literal_search, start) {
    COMPILE_SHORT_LITERAL(PATTERN_5_CHAR, 5);
    ASSERT_COMPILE_SUCCESS("test_short_literal_search_start");
    SEARCH_SHORT_LITERAL(EXPR_NOISE_0, EXPR_NOISE_LEN, 1, (0));
    EXPECT_SEARCH_SUCCESS("hs_short_literal_search", pattern, buffer);
    hs_free_short_literal_pattern(database);
}

TEST(short_literal_search, general) {
    SETUP_MEM_LEAK_TEST();
    COMPILE_SHORT_LITERAL(PATTERN_5_CHAR, 5);
    ASSERT_COMPILE_SUCCESS("test_short_literal_search_general");
    SEARCH_SHORT_LITERAL(EXPR_NOISE_5, EXPR_NOISE_LEN, 1, (5));
    EXPECT_SEARCH_SUCCESS("hs_short_literal_search", pattern, buffer);
    hs_free_short_literal_pattern(database);
    EXPECT_MEMORY_CLEAN();
    UNSET_MEM_LEAK_TEST();
}

TEST(short_literal_search, cross_vector) {
    COMPILE_SHORT_LITERAL(PATTERN_5_CHAR, 5);
    ASSERT_COMPILE_SUCCESS("test_short_literal_search_cross_vector");
    SEARCH_SHORT_LITERAL(EXPR_NOISE_5_15, EXPR_NOISE_LEN, 2, (5, 15));
    EXPECT_SEARCH_SUCCESS("hs_short_literal_search", pattern, buffer);
    hs_free_short_literal_pattern(database);
}

TEST(short_literal_search, end) {
    COMPILE_SHORT_LITERAL(PATTERN_5_CHAR, 5);
    ASSERT_COMPILE_SUCCESS("test_short_literal_search_end");
    SEARCH_SHORT_LITERAL(EXPR_NOISE_ABCDE_END_27, EXPR_NOISE_LEN, 1, (27));
    EXPECT_SEARCH_SUCCESS("hs_short_literal_search", pattern, buffer);
    hs_free_short_literal_pattern(database);
}

TEST(short_literal_search, past_end) {
    COMPILE_SHORT_LITERAL(PATTERN_5_CHAR, 5);
    ASSERT_COMPILE_SUCCESS("test_short_literal_search_past_end");
    // cppcheck-suppress unsignedLessThanZero
    SEARCH_SHORT_LITERAL(EXPR_NOISE_ABCDE_END_27, EXPR_NOISE_LEN - 3, 0, ());
    EXPECT_SEARCH_SUCCESS("hs_short_literal_search", pattern, buffer);
    hs_free_short_literal_pattern(database);
}

TEST(short_literal_search, short_no_match) {
    COMPILE_SHORT_LITERAL(PATTERN_5_CHAR, 5);
    ASSERT_COMPILE_SUCCESS("test_short_literal_search_short_no_match");
    // cppcheck-suppress unsignedLessThanZero
    SEARCH_SHORT_LITERAL(EXPR_NOISE_5_AB, EXPR_NOISE_LEN, 0, ());
    EXPECT_SEARCH_SUCCESS("hs_short_literal_search", pattern, buffer);
    hs_free_short_literal_pattern(database);
}

TEST(short_literal_search, bad_case) {
    COMPILE_SHORT_LITERAL(PATTERN_5_CHAR, 5);
    ASSERT_COMPILE_SUCCESS("test_short_literal_search_bad_case");
    // cppcheck-suppress unsignedLessThanZero
    SEARCH_SHORT_LITERAL(EXPR_NOISE_5_15_BAD_CASE, EXPR_NOISE_LEN, 0, ());
    EXPECT_SEARCH_SUCCESS("hs_short_literal_search", pattern, buffer);
    hs_free_short_literal_pattern(database);
}

TEST(short_literal_search, several_search) {
    COMPILE_SHORT_LITERAL(PATTERN_5_CHAR, 5);
    ASSERT_COMPILE_SUCCESS("test_short_literal_search_several_search");
    SEARCH_SHORT_LITERAL(EXPR_NOISE_5, EXPR_NOISE_LEN, 1, (5));
    EXPECT_SEARCH_SUCCESS("hs_short_literal_search", pattern, buffer);
    // cppcheck-suppress redundantAssignment
    SEARCH_SHORT_LITERAL(EXPR_NOISE_5_15, EXPR_NOISE_LEN, 2, (5, 15));
    EXPECT_SEARCH_SUCCESS("hs_short_literal_search", pattern, buffer);
    hs_free_short_literal_pattern(database);
}

TEST(short_literal_search, single_char) {
    COMPILE_SHORT_LITERAL(PATTERN_1_CHAR, 1);
    ASSERT_COMPILE_SUCCESS("test_short_literal_search_single_char");
    SEARCH_SHORT_LITERAL(EXPR_NOISE_5_15, EXPR_NOISE_LEN, 2, (5, 15));
    EXPECT_SEARCH_SUCCESS("hs_short_literal_search", pattern, buffer);
    hs_free_short_literal_pattern(database);
}

TEST(short_literal_search, single_char_end) {
    COMPILE_SHORT_LITERAL(PATTERN_1_CHAR, 1);
    ASSERT_COMPILE_SUCCESS("test_short_literal_search_single_char_end");
    SEARCH_SHORT_LITERAL(EXPR_NOISE_AB_END_30, EXPR_NOISE_LEN - 1, 1, (30));
    EXPECT_SEARCH_SUCCESS("hs_short_literal_search", pattern, buffer);
    hs_free_short_literal_pattern(database);
}

TEST(short_literal_search, single_char_no_match) {
    COMPILE_SHORT_LITERAL(PATTERN_1_CHAR, 1);
    ASSERT_COMPILE_SUCCESS("test_short_literal_search_single_char_no_match");
    // cppcheck-suppress unsignedLessThanZero
    SEARCH_SHORT_LITERAL(EXPR_NOISE, EXPR_NOISE_LEN, 0, ());
    EXPECT_SEARCH_SUCCESS("hs_short_literal_search", pattern, buffer);
    hs_free_short_literal_pattern(database);
}

TEST(short_literal_search, null_char_buff_and_pattern) {
    COMPILE_SHORT_LITERAL(PATTERN_5_WITH_NULL, 5);
    ASSERT_COMPILE_SUCCESS(
        "test_short_literal_search_null_char_buff_and_pattern");
    SEARCH_SHORT_LITERAL(EXPR_NOISE_5_NULL, EXPR_NOISE_LEN, 1, (5));
    EXPECT_SEARCH_SUCCESS("hs_short_literal_search", pattern, buffer);
    hs_free_short_literal_pattern(database);
}

TEST(short_literal_search, null_char_buff) {
    COMPILE_SHORT_LITERAL(PATTERN_5_CHAR, 5);
    ASSERT_COMPILE_SUCCESS("test_short_literal_search_null_char_buff");
    // cppcheck-suppress unsignedLessThanZero
    SEARCH_SHORT_LITERAL(EXPR_NOISE_5_NULL, EXPR_NOISE_LEN, 0, ());
    EXPECT_SEARCH_SUCCESS("hs_short_literal_search", pattern, buffer);
    hs_free_short_literal_pattern(database);
}

TEST(short_literal_search, empty_buff) {
    COMPILE_SHORT_LITERAL(PATTERN_5_CHAR, 5);
    ASSERT_COMPILE_SUCCESS("test_short_literal_search_empty_buff");
    // cppcheck-suppress unsignedLessThanZero
    SEARCH_SHORT_LITERAL("", 0, 0, ());
    EXPECT_SEARCH_SUCCESS("hs_short_literal_search", pattern, buffer);
    hs_free_short_literal_pattern(database);
}

#if !defined(RELEASE_BUILD)
// test asserts

TEST(short_literal_search, nullptr_pattern) {
    const hs_short_literal_compiled_pattern_t *database = nullptr;
    context_t context;
    EXPECT_DEATH(
        { 
            const char *buffer;
            hs_error_t ret;
            size_t pattern_len = 5;
            // cppcheck-suppress unsignedLessThanZero
            // cppcheck-suppress unreadVariable
            SEARCH_SHORT_LITERAL(EXPR_NOISE_5, EXPR_NOISE_LEN, 0, ());
        },
        "called with nullptr database");
}

TEST(short_literal_search, nullptr_buffer) {
    COMPILE_SHORT_LITERAL(PATTERN_5_CHAR, 5);
    ASSERT_COMPILE_SUCCESS("test_short_literal_search_nullptr_buffer");
    EXPECT_DEATH(
        { 
            // cppcheck-suppress unsignedLessThanZero
            // cppcheck-suppress unreadVariable
            SEARCH_SHORT_LITERAL(nullptr, EXPR_NOISE_LEN, 0, ());
        },
        "called with nullptr buffer");
}

TEST(short_literal_search, nullptr_callback) {
    COMPILE_SHORT_LITERAL(PATTERN_5_CHAR, 5);
    ASSERT_COMPILE_SUCCESS("test_short_literal_search_nullptr_callback");

    buffer = EXPR_NOISE_5;
    const size_t buffer_len = EXPR_NOISE_LEN;
    const size_t expected_match = 1;
    size_t expected_start_array[expected_match] = {5};
    size_t expected_end_array[expected_match] = {5};
    for (size_t i = 0; i < expected_match; i++) {
        expected_end_array[i] += pattern_len;
    }
    context.expected_start_array = expected_start_array;
    context.expected_end_array = expected_end_array;
    context.array_size = expected_match;
    context.number_matched = 0;
    context.number_wrong = 0;

    EXPECT_DEATH(
        {
            hs_short_literal_search(database, buffer, buffer_len, nullptr,
                                    &context);
        },
        "called with nullptr callback");
}

#endif
