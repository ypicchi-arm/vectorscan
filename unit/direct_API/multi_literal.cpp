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

#include "common.h"

#include "fdr/fdr_internal.h"

#define COMPILE_MULTI_LITERAL(in_pattern, in_pattern_count, in_pattern_len)    \
    const size_t pattern_count = (in_pattern_count);                           \
    size_t pattern_len[pattern_count] = BRACED_INIT_LIST in_pattern_len;       \
    const char *pattern_storage[] = in_pattern;                                \
    const char **pattern = pattern_storage;                                    \
    hs_multi_literal_compiled_pattern_t *database = nullptr;                   \
    hs_error_t compile_ret = hs_compile_multi_literal_search(                  \
        pattern, pattern_count, pattern_len, &database);                       \
    hs_error_t ret = 0;                                                        \
    (void)ret; /* suppress a cppcheck warning when SEARCH is not called */     \
    const char *buffer = nullptr;                                              \
    (void)buffer;                                                              \
    context_t context = {};                                                    \
    (void) context;

//  expected match array here is the index of the start of match, assuming it
//  match a pattern with the same length as pattern 0
#define SEARCH_MULTI_LITERAL(in_buffer, in_buffer_len, in_expected_match,      \
                             in_expected_start_array, in_expected_id_array)    \
    {                                                                          \
        buffer = (in_buffer);                                                  \
        const size_t buffer_len = (in_buffer_len);                             \
        const size_t expected_match = (in_expected_match);                     \
        size_t expected_start_array[expected_match] =                          \
            BRACED_INIT_LIST in_expected_start_array;                          \
        size_t expected_end_array[expected_match] =                            \
            BRACED_INIT_LIST in_expected_start_array;                          \
        size_t expected_id_array[expected_match] =                             \
            BRACED_INIT_LIST in_expected_id_array;                             \
        for (size_t i = 0; i < expected_match; i++) {                          \
            expected_end_array[i] += pattern_len[0];                           \
        }                                                                      \
        context.expected_start_array = expected_start_array;                   \
        context.expected_end_array = expected_end_array;                       \
        context.expected_id_array = expected_id_array;                         \
        context.array_size = expected_match;                                   \
        context.number_matched = 0;                                            \
        context.number_wrong = 0;                                              \
                                                                               \
        ret = hs_multi_literal_search(database, buffer, buffer_len, callback,  \
                                      &context);                               \
    }

// ------------------------free tests-------------------------------------------

/*
hs_free_multi_literal_pattern
    nullptr
    general
*/

TEST(multi_literal_free, nullptr) {
    hs_multi_literal_compiled_pattern_t *database = nullptr;
    hs_free_multi_literal_pattern(database);
}

TEST(multi_literal_free, general) {
    SETUP_MEM_LEAK_TEST();
    combined_fdr_database *clear_database =
        reinterpret_cast<combined_fdr_database *>(
            test_malloc(sizeof(combined_fdr_database)));

    hs_multi_literal_compiled_pattern_t *database =
        reinterpret_cast<hs_multi_literal_compiled_pattern_t*>(clear_database);

    hs_free_multi_literal_pattern(database);
    EXPECT_MEMORY_CLEAN();
    UNSET_MEM_LEAK_TEST();
}

// ------------------------compile tests----------------------------------------

/*
hs_compile_multi_literal_search
    single expression
    single char expression
    general (several expressions)
    pattern duplicate
    valid pattern including null char
    overlaping patterns (eg, "abba" and "bb")

    no expressions
    empty expression
    nullptr expression array
    one of the expression is nullptr
    nullptr output
*/

TEST(multi_literal_compile, single_pattern) {
    COMPILE_MULTI_LITERAL(PATTERN_ARRAY_SINGLE_PAT_5, 1, (5));
    EXPECT_COMPILE_SUCCESS("test_compile_multi_literal_single_pattern");
    hs_free_multi_literal_pattern(database);
}

TEST(multi_literal_compile, single_pattern_single_char) {
    COMPILE_MULTI_LITERAL(PATTERN_ARRAY_SINGLE_CHAR_PAT_1, 1, (1));
    EXPECT_COMPILE_SUCCESS(
        "test_compile_multi_literal_single_pattern_single_char");
    hs_free_multi_literal_pattern(database);
}

TEST(multi_literal_compile, general) {
    COMPILE_MULTI_LITERAL(PATTERN_ARRAY_GENERAL_5_5, 2, (5, 5));
    EXPECT_COMPILE_SUCCESS("test_compile_multi_literal_general");
    hs_free_multi_literal_pattern(database);
}

TEST(multi_literal_compile, duplicate) {
    COMPILE_MULTI_LITERAL(PATTERN_ARRAY_GENERAL_5_DUPLICATE, 2, (5, 5));
    EXPECT_COMPILE_SUCCESS("test_compile_multi_literal_duplicate");
    hs_free_multi_literal_pattern(database);
}

TEST(multi_literal_compile, with_null_char) {
    COMPILE_MULTI_LITERAL(PATTERN_ARRAY_WITH_NULL_5_5, 2, (5, 5));
    EXPECT_COMPILE_SUCCESS("test_compile_multi_literal_with_null_char");
    hs_free_multi_literal_pattern(database);
}

TEST(multi_literal_compile, overlapping_patterns) {
    COMPILE_MULTI_LITERAL(PATTERN_ARRAY_OVERLAP_5_8, 2, (5, 8));
    EXPECT_COMPILE_SUCCESS("test_compile_multi_literal_overlapping_patterns");
    hs_free_multi_literal_pattern(database);
}

#if !defined(RELEASE_BUILD)
// test asserts

TEST(multi_literal_compile, no_expression) {
    const size_t pattern_count = 0;
    const char *pattern_storage[] = PATTERN_ARRAY_GENERAL_5_5;
    const char **pattern = pattern_storage;
    hs_multi_literal_compiled_pattern_t *database = nullptr;
    EXPECT_DEATH(
        {
            size_t pattern_len[2];
            pattern_len[0] = 5;
            pattern_len[1] = 5;
            hs_compile_multi_literal_search(pattern, pattern_count, pattern_len,
                                            &database);
        },
        "called with no pattern");
}

TEST(multi_literal_compile, empty_expression) {
    const size_t pattern_count = 1;
    const size_t pattern_len[pattern_count] = {0};
    const char *pattern_storage[] = PATTERN_ARRAY_CONTAIN_EMPTY_0;
    const char **pattern = pattern_storage;
    hs_multi_literal_compiled_pattern_t *database = nullptr;
    EXPECT_DEATH(hs_compile_multi_literal_search(pattern, pattern_count,
                                                 pattern_len, &database),
                 "called with an empty pattern");
}

TEST(multi_literal_compile, nullptr_pattern_array) {
    const size_t pattern_count = 1;
    const size_t pattern_len[pattern_count] = {5};
    const char **pattern = nullptr;
    hs_multi_literal_compiled_pattern_t *database = nullptr;
    EXPECT_DEATH(hs_compile_multi_literal_search(pattern, pattern_count,
                                                 pattern_len, &database),
                 "called with nullptr");
}

TEST(multi_literal_compile, nullptr_pattern_in_array) {
    const size_t pattern_count = 2;
    const size_t pattern_len[pattern_count] = {5, 5};
    const char *pattern_storage[] = PATTERN_ARRAY_CONTAIN_NULLPTR_5_0;
    const char **pattern = pattern_storage;
    hs_multi_literal_compiled_pattern_t *database = nullptr;
    EXPECT_DEATH(hs_compile_multi_literal_search(pattern, pattern_count,
                                                 pattern_len, &database),
                 "called with an empty pattern");
}

TEST(multi_literal_compile, nullptr_database) {
    const size_t pattern_count = 2;
    const size_t pattern_len[pattern_count] = {5, 5};
    const char *pattern_storage[] = PATTERN_ARRAY_GENERAL_5_5;
    const char **pattern = pattern_storage;
    EXPECT_DEATH(hs_compile_multi_literal_search(pattern, pattern_count,
                                                 pattern_len, nullptr),
                 "called with nullptr");
}

#endif

// ------------------------search tests-----------------------------------------

/*
hs_multi_literal_search
    general pattern
        match at start
        match middle (general)
        match index 15 (cross over vector)
        match at end
        match past end (a few char ok, then end, so missing some chars)
        match long patterns
        long pattern but the buffer only have the short part of it
        bad caseness
        search several times
        match first pattern
        match last pattern
        match several pattern in the same search
        match overlapping patterns
        pattern mix (start with pattern A, finish with pattern B. Expect no
match)
        match a pattern duplicate
        single char pattern
        general match
        match at end
        no match
    buffer containing null char
        pattern with null char
        general pattern (no null char searched for)
    buff size 0
    nullptr pattern
    nullptr buffer
    nullptr callback
*/

TEST(multi_literal_search, start) {
    COMPILE_MULTI_LITERAL(PATTERN_ARRAY_GENERAL_5_5, 2, (5, 5));
    ASSERT_COMPILE_SUCCESS("test_multi_literal_search_start");
    SEARCH_MULTI_LITERAL(EXPR_NOISE_0, EXPR_NOISE_LEN, 1, (0), (0));
    EXPECT_SEARCH_SUCCESS("hs_multi_literal_search", pattern[0], buffer);
    hs_free_multi_literal_pattern(database);
}

TEST(multi_literal_search, general) {
    SETUP_MEM_LEAK_TEST();
    COMPILE_MULTI_LITERAL(PATTERN_ARRAY_GENERAL_5_5, 2, (5, 5));
    ASSERT_COMPILE_SUCCESS("test_multi_literal_search_general");
    SEARCH_MULTI_LITERAL(EXPR_NOISE_5, EXPR_NOISE_LEN, 1, (5), (0));
    EXPECT_SEARCH_SUCCESS("hs_multi_literal_search", pattern[0], buffer);
    hs_free_multi_literal_pattern(database);
    EXPECT_MEMORY_CLEAN();
    UNSET_MEM_LEAK_TEST();
}

TEST(multi_literal_search, cross_vector) {
    COMPILE_MULTI_LITERAL(PATTERN_ARRAY_GENERAL_5_5, 2, (5, 5));
    ASSERT_COMPILE_SUCCESS("test_multi_literal_search_cross_vector");
    SEARCH_MULTI_LITERAL(EXPR_NOISE_5_15, EXPR_NOISE_LEN, 2, (5, 15), (0, 0));
    EXPECT_SEARCH_SUCCESS("hs_multi_literal_search", pattern[0], buffer);
    hs_free_multi_literal_pattern(database);
}

TEST(multi_literal_search, end) {
    COMPILE_MULTI_LITERAL(PATTERN_ARRAY_GENERAL_5_5, 2, (5, 5));
    ASSERT_COMPILE_SUCCESS("test_multi_literal_search_end");
    SEARCH_MULTI_LITERAL(EXPR_NOISE_ABCDE_END_27, EXPR_NOISE_LEN, 1, (27), (0));
    EXPECT_SEARCH_SUCCESS("hs_multi_literal_search", pattern[0], buffer);
    hs_free_multi_literal_pattern(database);
}

TEST(multi_literal_search, past_end) {
    COMPILE_MULTI_LITERAL(PATTERN_ARRAY_GENERAL_5_5, 2, (5, 5));
    ASSERT_COMPILE_SUCCESS("test_multi_literal_search_past_end");
    // cppcheck-suppress unsignedLessThanZero
    SEARCH_MULTI_LITERAL(EXPR_NOISE_ABCDE_END_27, EXPR_NOISE_LEN - 3, 0, (), ());
    EXPECT_SEARCH_SUCCESS("hs_multi_literal_search", pattern[0], buffer);
    hs_free_multi_literal_pattern(database);
}

TEST(multi_literal_search, long_pattern) {
    COMPILE_MULTI_LITERAL(PATTERN_ARRAY_LONG_10_10, 2, (10, 10));
    ASSERT_COMPILE_SUCCESS("test_multi_literal_search_long_pattern");
    SEARCH_MULTI_LITERAL(EXPR_NOISE_5, EXPR_NOISE_LEN, 2, (5, 22), (0, 1));
    EXPECT_SEARCH_SUCCESS("hs_multi_literal_search", pattern[0], buffer);
    hs_free_multi_literal_pattern(database);
}

TEST(multi_literal_search, short_but_negative_long) {
    COMPILE_MULTI_LITERAL(PATTERN_ARRAY_LONG_10_10, 2, (10, 10));
    ASSERT_COMPILE_SUCCESS("test_multi_literal_search_short_but_negative_long");
    SEARCH_MULTI_LITERAL(EXPR_NOISE_SHORT_ONLY_5, EXPR_NOISE_LEN, 1, (22), (1));
    EXPECT_SEARCH_SUCCESS("hs_multi_literal_search", pattern[1], buffer);
    hs_free_multi_literal_pattern(database);
}

TEST(multi_literal_search, bad_case) {
    COMPILE_MULTI_LITERAL(PATTERN_ARRAY_GENERAL_5_5, 2, (5, 5));
    ASSERT_COMPILE_SUCCESS("test_multi_literal_search_bad_case");
    // cppcheck-suppress unsignedLessThanZero
    SEARCH_MULTI_LITERAL(EXPR_NOISE_5_15_BAD_CASE, EXPR_NOISE_LEN, 0, (), ());
    EXPECT_SEARCH_SUCCESS("hs_multi_literal_search", pattern[0], buffer);
    hs_free_multi_literal_pattern(database);
}

TEST(multi_literal_search, several_search) {
    COMPILE_MULTI_LITERAL(PATTERN_ARRAY_GENERAL_5_5, 2, (5, 5));
    ASSERT_COMPILE_SUCCESS("test_multi_literal_search_several_search");
    SEARCH_MULTI_LITERAL(EXPR_NOISE_5, EXPR_NOISE_LEN, 1, (5), (0));
    EXPECT_SEARCH_SUCCESS("hs_multi_literal_search", pattern[0], buffer);
    SEARCH_MULTI_LITERAL(EXPR_NOISE_5_15, EXPR_NOISE_LEN, 2, (5, 15), (0, 0));
    EXPECT_SEARCH_SUCCESS("hs_multi_literal_search", pattern[0], buffer);
    hs_free_multi_literal_pattern(database);
}

TEST(multi_literal_search, first_pattern) {
    COMPILE_MULTI_LITERAL(PATTERN_ARRAY_GENERAL_5_5, 2, (5, 5));
    ASSERT_COMPILE_SUCCESS("test_multi_literal_search_first_pattern");
    SEARCH_MULTI_LITERAL(EXPR_NOISE_5, EXPR_NOISE_LEN, 1, (5), (0));
    EXPECT_SEARCH_SUCCESS("hs_multi_literal_search", pattern[0], buffer);
    hs_free_multi_literal_pattern(database);
}

TEST(multi_literal_search, last_pattern) {
    COMPILE_MULTI_LITERAL(PATTERN_ARRAY_GENERAL_5_5, 2, (5, 5));
    ASSERT_COMPILE_SUCCESS("test_multi_literal_search_last_pattern");
    SEARCH_MULTI_LITERAL(EXPR_NOISE_PAT2_5, EXPR_NOISE_LEN, 1, (5), (1));
    EXPECT_SEARCH_SUCCESS("hs_multi_literal_search", pattern[1], buffer);
    hs_free_multi_literal_pattern(database);
}

TEST(multi_literal_search, multi_pattern) {
    COMPILE_MULTI_LITERAL(PATTERN_ARRAY_GENERAL_5_5, 2, (5, 5));
    ASSERT_COMPILE_SUCCESS("test_multi_literal_search_multi_pattern");
    SEARCH_MULTI_LITERAL(EXPR_NOISE_DUO_5_15, EXPR_NOISE_LEN, 2, (5, 15),
                         (0, 1));
    EXPECT_SEARCH_SUCCESS("hs_multi_literal_search", pattern[0], buffer);
    hs_free_multi_literal_pattern(database);
}

TEST(multi_literal_search, overlap) {
    COMPILE_MULTI_LITERAL(PATTERN_ARRAY_OVERLAP_5_8, 2, (5, 8));
    ASSERT_COMPILE_SUCCESS("test_multi_literal_search_overlap");

    buffer = EXPR_NOISE_5;
    const size_t buffer_len = EXPR_NOISE_LEN;
    const size_t expected_match = 2;
    size_t expected_start_array[expected_match] = {5, 7};
    size_t expected_end_array[expected_match] = {5, 7};
    size_t expected_id_array[expected_match] = {0, 1};
    for (size_t i = 0; i < expected_match; i++) {
        // we need the length of the second pattern, hence not using the macro
        expected_end_array[i] += pattern_len[i];
    }
    context.expected_start_array = expected_start_array;
    context.expected_end_array = expected_end_array;
    context.expected_id_array = expected_id_array;
    context.array_size = expected_match;
    context.number_matched = 0;
    context.number_wrong = 0;

    ret = hs_multi_literal_search(database, buffer, buffer_len,
                                             callback, &context);

    EXPECT_SEARCH_SUCCESS("hs_multi_literal_search", pattern[0], buffer);
    hs_free_multi_literal_pattern(database);
}

TEST(multi_literal_search, pattern_mix) {
    COMPILE_MULTI_LITERAL(PATTERN_ARRAY_GENERAL_5_5, 2, (5, 5));
    ASSERT_COMPILE_SUCCESS("test_multi_literal_search_pattern_mix");
    // cppcheck-suppress unsignedLessThanZero
    SEARCH_MULTI_LITERAL(EXPR_NOISE_MIX, EXPR_NOISE_LEN, 0, (), ());
    EXPECT_SEARCH_SUCCESS("hs_multi_literal_search", pattern[0], buffer);
    hs_free_multi_literal_pattern(database);
}

TEST(multi_literal_search, duplicate) {
    COMPILE_MULTI_LITERAL(PATTERN_ARRAY_GENERAL_5_DUPLICATE, 2, (5, 5));
    ASSERT_COMPILE_SUCCESS("test_multi_literal_search_duplicate");
    SEARCH_MULTI_LITERAL(EXPR_NOISE_5, EXPR_NOISE_LEN, 1, (5), (0));
    EXPECT_SEARCH_SUCCESS("hs_multi_literal_search", pattern[0], buffer);
    hs_free_multi_literal_pattern(database);
}

TEST(multi_literal_search, single_char) {
    COMPILE_MULTI_LITERAL(PATTERN_ARRAY_SINGLE_CHAR_PAT_1, 1, (1));
    ASSERT_COMPILE_SUCCESS("test_multi_literal_search_single_char");
    SEARCH_MULTI_LITERAL(EXPR_NOISE_5_15, EXPR_NOISE_LEN, 2, (5, 15), (0, 0));
    EXPECT_SEARCH_SUCCESS("hs_multi_literal_search", pattern[0], buffer);
    hs_free_multi_literal_pattern(database);
}

TEST(multi_literal_search, single_char_end) {
    COMPILE_MULTI_LITERAL(PATTERN_ARRAY_SINGLE_CHAR_PAT_1, 1, (1));
    ASSERT_COMPILE_SUCCESS("test_multi_literal_search_single_char_end");
    SEARCH_MULTI_LITERAL(EXPR_NOISE_AB_END_30, EXPR_NOISE_LEN - 1, 1, (30),
                         (0));
    EXPECT_SEARCH_SUCCESS("hs_multi_literal_search", pattern[0], buffer);
    hs_free_multi_literal_pattern(database);
}

TEST(multi_literal_search, single_char_no_match) {
    COMPILE_MULTI_LITERAL(PATTERN_ARRAY_SINGLE_CHAR_PAT_1, 1, (1));
    ASSERT_COMPILE_SUCCESS("test_multi_literal_search_single_char_no_match");
    // cppcheck-suppress unsignedLessThanZero
    SEARCH_MULTI_LITERAL(EXPR_NOISE, EXPR_NOISE_LEN, 0, (), ());
    EXPECT_SEARCH_SUCCESS("hs_multi_literal_search", pattern[0], buffer);
    hs_free_multi_literal_pattern(database);
}

TEST(multi_literal_search, null_char_buff_and_pattern) {
    COMPILE_MULTI_LITERAL(PATTERN_ARRAY_WITH_NULL_5_5, 2, (5, 5));
    ASSERT_COMPILE_SUCCESS(
        "test_multi_literal_search_null_char_buff_and_pattern");
    SEARCH_MULTI_LITERAL(EXPR_NOISE_5_NULL, EXPR_NOISE_LEN, 1, (5), (0));
    EXPECT_SEARCH_SUCCESS("hs_multi_literal_search", pattern[0], buffer);
    hs_free_multi_literal_pattern(database);
}

TEST(multi_literal_search, null_char_buff) {
    COMPILE_MULTI_LITERAL(PATTERN_ARRAY_GENERAL_5_5, 2, (5, 5));
    ASSERT_COMPILE_SUCCESS("test_multi_literal_search_null_char_buff");
    // cppcheck-suppress unsignedLessThanZero
    SEARCH_MULTI_LITERAL(EXPR_NOISE_5_NULL, EXPR_NOISE_LEN, 0, (), ());
    EXPECT_SEARCH_SUCCESS("hs_multi_literal_search", pattern[0], buffer);
    hs_free_multi_literal_pattern(database);
}

TEST(multi_literal_search, empty_buff) {
    COMPILE_MULTI_LITERAL(PATTERN_ARRAY_GENERAL_5_5, 2, (5, 5));
    ASSERT_COMPILE_SUCCESS("test_multi_literal_search_empty_buff");
    // cppcheck-suppress unsignedLessThanZero
    SEARCH_MULTI_LITERAL("", 0, 0, (), ());
    EXPECT_SEARCH_SUCCESS("hs_multi_literal_search", pattern[0], buffer);
    hs_free_multi_literal_pattern(database);
}

#if !defined(RELEASE_BUILD)
// test asserts

TEST(multi_literal_search, nullptr_pattern) {
    const hs_multi_literal_compiled_pattern_t *database = nullptr;
    context_t context;
    EXPECT_DEATH(
        { 
            const char *buffer;
            hs_error_t ret;
            size_t pattern_len[2];
            pattern_len[0] = 5;
            pattern_len[1] = 5;
            // cppcheck-suppress unsignedLessThanZero
            // cppcheck-suppress unreadVariable
            SEARCH_MULTI_LITERAL(EXPR_NOISE_5, EXPR_NOISE_LEN, 0, (), ());
        },
        "called with nullptr database");
}

TEST(multi_literal_search, nullptr_buffer) {
    COMPILE_MULTI_LITERAL(PATTERN_ARRAY_GENERAL_5_5, 2, (5, 5));
    ASSERT_COMPILE_SUCCESS("test_multi_literal_search_nullptr_buffer");
    EXPECT_DEATH(
        { 
            // cppcheck-suppress unsignedLessThanZero
            // cppcheck-suppress unreadVariable
            SEARCH_MULTI_LITERAL(nullptr, EXPR_NOISE_LEN, 0, (), ());
        },
        "called with nullptr buffer");
}

TEST(multi_literal_search, nullptr_callback) {
    COMPILE_MULTI_LITERAL(PATTERN_ARRAY_GENERAL_5_5, 2, (5, 5));
    ASSERT_COMPILE_SUCCESS("test_multi_literal_search_nullptr_callback");

    buffer = EXPR_NOISE_5;
    const size_t buffer_len = EXPR_NOISE_LEN;
    const size_t expected_match = 1;
    size_t expected_start_array[expected_match] = {5};
    size_t expected_end_array[expected_match] = {5};
    for (size_t i = 0; i < expected_match; i++) {
        expected_end_array[i] += pattern_len[0];
    }
    context.expected_start_array = expected_start_array;
    context.expected_end_array = expected_end_array;
    context.array_size = expected_match;
    context.number_matched = 0;
    context.number_wrong = 0;

    EXPECT_DEATH(
        {
            hs_multi_literal_search(database, buffer, buffer_len, nullptr,
                                    &context);
        },
        "called with nullptr callback");
}

#endif
