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

#include "hwlm/noodle_internal.h"

#define COMPILE_SINGLE_CHAR(in_pattern)                                        \
    const char pattern = *(in_pattern);                                        \
    hs_single_char_compiled_pattern_t *database = nullptr;                     \
    hs_error_t compile_ret = hs_compile_single_char_search(pattern, &database);\
    hs_error_t ret = 0;                                                        \
    (void)ret; /* suppress a cppcheck warning when SEARCH is not called */     \
    const char *buffer = nullptr;                                              \
    (void)buffer;                                                              \
    context_t context = {};                                                    \
    (void) context;

// expected match array here is the index of the start of match.
#define SEARCH_SINGLE_CHAR(in_buffer, in_buffer_len, in_expected_match,        \
                           in_expected_start_array)                            \
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
            expected_end_array[i] += 1;                                        \
            expected_id_array[i] = 0;                                          \
        }                                                                      \
        context.expected_start_array = expected_start_array;                   \
        context.expected_end_array = expected_end_array;                       \
        context.expected_id_array = expected_id_array;                         \
        context.array_size = expected_match;                                   \
        context.number_matched = 0;                                            \
        context.number_wrong = 0;                                              \
                                                                               \
        ret = hs_single_char_search(database, buffer, buffer_len, callback,    \
                                    &context);                                 \
    }

// ------------------------free tests-------------------------------------------

/*
hs_free_single_char_pattern
    nullptr
    general
*/

TEST(single_char_free, nullptr) {
    hs_single_char_compiled_pattern_t *database = nullptr;
    hs_free_single_char_pattern(database);
}

TEST(single_char_free, general) {
    SETUP_MEM_LEAK_TEST();
    truffle_storage *clear_database = reinterpret_cast<truffle_storage *>(
        test_malloc(sizeof(truffle_storage)));
    hs_single_char_compiled_pattern_t *database =
        reinterpret_cast<hs_single_char_compiled_pattern_t*>(clear_database);

    hs_free_single_char_pattern(database);
    EXPECT_MEMORY_CLEAN();
    UNSET_MEM_LEAK_TEST();
}

// ------------------------compile tests----------------------------------------

/*
hs_compile_single_char_search
    general (1 char)
    null char pattern

    nullptr output
*/

TEST(single_char_compile, general) {
    COMPILE_SINGLE_CHAR(PATTERN_1_CHAR)
    EXPECT_COMPILE_SUCCESS("test_compile_single_char_general")
    hs_free_single_char_pattern(database);
}

TEST(single_char_compile, null_char) {
    COMPILE_SINGLE_CHAR(PATTERN_1_CHAR)
    EXPECT_COMPILE_SUCCESS("test_compile_single_char_null_char")
    hs_free_single_char_pattern(database);
}

#if !defined(RELEASE_BUILD)
// test asserts

TEST(single_char_compile, nullptr_database) {
    EXPECT_DEATH(hs_compile_single_char_search(*PATTERN_1_CHAR, nullptr),
                 "called with nullptr");
}

#endif

// ------------------------search tests-----------------------------------------

/*
hs_single_char_search
    general pattern
        match at start
        match middle (general)
        match vector end
        match at buffer end
        match past end
        bad caseness
        search several times
    buffer containing null char
        null char pattern
        general pattern
    buff size 0
    nullptr pattern
    nullptr buffer
    nullptr callback
*/

TEST(single_char_search, start) {
    COMPILE_SINGLE_CHAR(PATTERN_1_CHAR);
    ASSERT_COMPILE_SUCCESS("test_single_char_search_start");
    SEARCH_SINGLE_CHAR(EXPR_NOISE_0, EXPR_NOISE_LEN, 1, (0));
    EXPECT_SEARCH_SUCCESS("hs_single_char_search", pattern, buffer);
    hs_free_single_char_pattern(database);
}

TEST(single_char_search, general) {
    SETUP_MEM_LEAK_TEST();
    COMPILE_SINGLE_CHAR(PATTERN_1_CHAR);
    ASSERT_COMPILE_SUCCESS("test_single_char_search_general");
    SEARCH_SINGLE_CHAR(EXPR_NOISE_5, EXPR_NOISE_LEN, 1, (5));
    EXPECT_SEARCH_SUCCESS("hs_single_char_search", pattern, buffer);
    hs_free_single_char_pattern(database);
    EXPECT_MEMORY_CLEAN();
    UNSET_MEM_LEAK_TEST();
}

TEST(single_char_search, end_vector) {
    COMPILE_SINGLE_CHAR(PATTERN_1_CHAR);
    ASSERT_COMPILE_SUCCESS("test_single_char_search_end_vector");
    SEARCH_SINGLE_CHAR(EXPR_NOISE_5_15, EXPR_NOISE_LEN, 2, (5, 15));
    EXPECT_SEARCH_SUCCESS("hs_single_char_search", pattern, buffer);
    hs_free_single_char_pattern(database);
}

TEST(single_char_search, end) {
    COMPILE_SINGLE_CHAR(PATTERN_1_CHAR);
    ASSERT_COMPILE_SUCCESS("test_single_char_search_end");
    SEARCH_SINGLE_CHAR(EXPR_NOISE_A_END_31, EXPR_NOISE_LEN, 1, (31));
    EXPECT_SEARCH_SUCCESS("hs_single_char_search", pattern, buffer);
    hs_free_single_char_pattern(database);
}

TEST(single_char_search, past_end) {
    COMPILE_SINGLE_CHAR(PATTERN_1_CHAR);
    ASSERT_COMPILE_SUCCESS("test_single_char_search_past_end");
    // cppcheck-suppress unsignedLessThanZero
    SEARCH_SINGLE_CHAR(EXPR_NOISE_A_END_31, EXPR_NOISE_LEN - 1, 0, ());
    EXPECT_SEARCH_SUCCESS("hs_single_char_search", pattern, buffer);
    hs_free_single_char_pattern(database);
}

TEST(single_char_search, bad_case) {
    COMPILE_SINGLE_CHAR(PATTERN_1_CHAR);
    ASSERT_COMPILE_SUCCESS("test_single_char_search_bad_case");
    // cppcheck-suppress unsignedLessThanZero
    SEARCH_SINGLE_CHAR(EXPR_NOISE_5_15_BAD_CASE, EXPR_NOISE_LEN, 0, ());
    EXPECT_SEARCH_SUCCESS("hs_single_char_search", pattern, buffer);
    hs_free_single_char_pattern(database);
}

TEST(single_char_search, several_search) {
    COMPILE_SINGLE_CHAR(PATTERN_1_CHAR);
    ASSERT_COMPILE_SUCCESS("test_single_char_search_several_search");
    SEARCH_SINGLE_CHAR(EXPR_NOISE_5, EXPR_NOISE_LEN, 1, (5));
    EXPECT_SEARCH_SUCCESS("hs_single_char_search", pattern, buffer);
    SEARCH_SINGLE_CHAR(EXPR_NOISE_5_15, EXPR_NOISE_LEN, 2, (5, 15));
    EXPECT_SEARCH_SUCCESS("hs_single_char_search", pattern, buffer);
    hs_free_single_char_pattern(database);
}

TEST(single_char_search, null_char_buff_and_pattern) {
    COMPILE_SINGLE_CHAR(PATTERN_1_CHAR_NULL);
    ASSERT_COMPILE_SUCCESS(
        "test_single_char_search_null_char_buff_and_pattern");
    SEARCH_SINGLE_CHAR(EXPR_NOISE_5_NULL, EXPR_NOISE_LEN, 1, (6));
    EXPECT_SEARCH_SUCCESS("hs_single_char_search", pattern, buffer);
    hs_free_single_char_pattern(database);
}

TEST(single_char_search, null_char_buff) {
    COMPILE_SINGLE_CHAR(PATTERN_1_CHAR);
    ASSERT_COMPILE_SUCCESS("test_single_char_search_null_char_buff");
    SEARCH_SINGLE_CHAR(EXPR_NOISE_5_NULL, EXPR_NOISE_LEN, 1, (5));
    EXPECT_SEARCH_SUCCESS("hs_single_char_search", pattern, buffer);
    hs_free_single_char_pattern(database);
}

TEST(single_char_search, empty_buff) {
    COMPILE_SINGLE_CHAR(PATTERN_1_CHAR);
    ASSERT_COMPILE_SUCCESS("test_single_char_search_empty_buff");
    // cppcheck-suppress unsignedLessThanZero
    SEARCH_SINGLE_CHAR("", 0, 0, ());
    EXPECT_SEARCH_SUCCESS("hs_single_char_search", pattern, buffer);
    hs_free_single_char_pattern(database);
}

#if !defined(RELEASE_BUILD)
// test asserts

TEST(single_char_search, nullptr_pattern) {
    const hs_single_char_compiled_pattern_t *database = nullptr;
    context_t context;
    EXPECT_DEATH(
        { 
            const char *buffer;
            hs_error_t ret;
            // cppcheck-suppress unsignedLessThanZero
            // cppcheck-suppress unreadVariable
            SEARCH_SINGLE_CHAR(EXPR_NOISE_5, EXPR_NOISE_LEN, 0, ());
        },
        "called with nullptr database");
}

TEST(single_char_search, nullptr_buffer) {
    COMPILE_SINGLE_CHAR(PATTERN_1_CHAR);
    ASSERT_COMPILE_SUCCESS("test_single_char_search_nullptr_buffer");
    EXPECT_DEATH(
        { 
            // cppcheck-suppress unsignedLessThanZero
            // cppcheck-suppress unreadVariable
            SEARCH_SINGLE_CHAR(nullptr, EXPR_NOISE_LEN, 0, ());
        },
        "called with nullptr buffer");
}

TEST(single_char_search, nullptr_callback) {
    COMPILE_SINGLE_CHAR(PATTERN_1_CHAR);
    ASSERT_COMPILE_SUCCESS("test_single_char_search_nullptr_callback");

    buffer = EXPR_NOISE_5;
    const size_t buffer_len = EXPR_NOISE_LEN;
    const size_t expected_match = 1;
    size_t expected_start_array[expected_match] = {5};
    size_t expected_end_array[expected_match] = {5};
    for (size_t i = 0; i < expected_match; i++) {
        expected_end_array[i] += 1;
    }
    context.expected_start_array = expected_start_array;
    context.expected_end_array = expected_end_array;
    context.array_size = expected_match;
    context.number_matched = 0;
    context.number_wrong = 0;

    EXPECT_DEATH(
        {
            hs_single_char_search(database, buffer, buffer_len, nullptr,
                                  &context);
        },
        "called with nullptr callback");
}

#endif
