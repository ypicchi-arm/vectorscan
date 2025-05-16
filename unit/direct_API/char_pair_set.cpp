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

#define COMPILE_CHAR_PAIR_SET(in_character_array, in_pair_count)               \
    const size_t pair_count = (in_pair_count);                                 \
    const char *character_array = (in_character_array);                        \
    hs_char_pair_set_compiled_pattern_t *database = nullptr;                   \
    hs_error_t compile_ret = hs_compile_char_pair_set_search(                  \
        character_array, pair_count, &database);                               \
    hs_error_t ret = 0;                                                        \
    (void)ret; /* suppress a cppcheck warning when SEARCH is not called */     \
    const char *buffer = nullptr;                                              \
    (void)buffer;                                                              \
    context_t context = {};                                                    \
    (void) context;

#define SEARCH_CHAR_PAIR_SET(in_buffer, in_buffer_len, in_expected_match,      \
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
            expected_end_array[i] += 2;                                        \
        }                                                                      \
        context.expected_start_array = expected_start_array;                   \
        context.expected_end_array = expected_end_array;                       \
        context.expected_id_array = expected_id_array;                         \
        context.array_size = expected_match;                                   \
        context.number_matched = 0;                                            \
        context.number_wrong = 0;                                              \
                                                                               \
        ret = hs_char_pair_set_search(database, buffer, buffer_len, callback,  \
                                      &context);                               \
    }

// ------------------------free tests-------------------------------------------

/*
hs_free_char_pair_set_pattern
    nullptr
    general
*/

TEST(char_pair_set_free, nullptr) {
    hs_char_pair_set_compiled_pattern_t *database = nullptr;
    hs_free_char_pair_set_pattern(database);
}

TEST(char_pair_set_free, general) {
    SETUP_MEM_LEAK_TEST();
    noodTable *clear_database =
        reinterpret_cast<noodTable *>(test_malloc(sizeof(noodTable)));

    hs_char_pair_set_compiled_pattern_t *database =
        reinterpret_cast<hs_char_pair_set_compiled_pattern_t*>(clear_database);

    hs_free_char_pair_set_pattern(database);
    EXPECT_MEMORY_CLEAN();
    UNSET_MEM_LEAK_TEST();
}

// ------------------------compile tests----------------------------------------

/*
hs_compile_char_pair_set_search
    single pair
    multiple pair
    pair duplicate
    valid pair including null char
    
    empty char array
    nullptr char array
    nullptr output
*/

TEST(char_pair_set_compile, single_pair) {
    COMPILE_CHAR_PAIR_SET(PAIR_SET_ABCD, 1);
    (void) ret;
    (void) buffer;   
    EXPECT_COMPILE_SUCCESS("test_compile_char_pair_set_single_pair");
    hs_free_char_pair_set_pattern(database);
}

TEST(char_pair_set_compile, two_pairs) {
    COMPILE_CHAR_PAIR_SET(PAIR_SET_ABCD, 2);
    (void) ret;
    (void) buffer;  
    EXPECT_COMPILE_SUCCESS("test_compile_char_pair_set_two_pairs");
    hs_free_char_pair_set_pattern(database);
}

TEST(char_pair_set_compile, duplicate) {
    COMPILE_CHAR_PAIR_SET(PAIR_SET_AB_DUPLICATE, 2);
    (void) ret;
    (void) buffer;  
    EXPECT_COMPILE_SUCCESS("test_compile_char_pair_set_duplicate");
    hs_free_char_pair_set_pattern(database);
}

TEST(char_pair_set_compile, null_char) {
    COMPILE_CHAR_PAIR_SET(PAIR_SET_A_NULL_BC, 1);
    (void) ret;
    (void) buffer;  
    EXPECT_COMPILE_SUCCESS("test_compile_char_pair_set_null_char");
    hs_free_char_pair_set_pattern(database);
}

#if !defined(RELEASE_BUILD)
// test asserts

TEST(char_pair_set_compile, no_expression) {
    const size_t pair_count = 0;
    const char *character_array = PAIR_SET_ABCD;
    hs_char_pair_set_compiled_pattern_t *database = nullptr;
    EXPECT_DEATH(hs_compile_char_pair_set_search(character_array, pair_count,
                                              &database),
                 "called with an empty set");
}

TEST(char_pair_set_compile, nullptr_char_array) {
        hs_char_pair_set_compiled_pattern_t *database = nullptr;
        EXPECT_DEATH(
            hs_compile_char_pair_set_search(nullptr, 1, &database),
            "called with nullptr");
}

TEST(char_pair_set_compile, nullptr_database) {
        const size_t pair_count = 2;
        const char *character_array = PAIR_SET_ABCD;
        EXPECT_DEATH(hs_compile_char_pair_set_search(character_array,
                                                  pair_count, nullptr),
                     "called with nullptr");
}

#endif

// ------------------------search tests-----------------------------------------

/*
hs_char_pair_set_search
    general pattern
        match at start
        match middle (general)
        match index 15 (cross over vector)
        match at end
        match past end
        match null char
        bad caseness
        search several times
        match a pair duplicate
        match several pattern in the same search
        match when there's more pairs than fit in a vector
    buffer containing null char
        pattern with null char
        general pattern

    buff size 0
    nullptr pattern
    nullptr buffer
    nullptr callback
*/

TEST(char_pair_set_search, start) {
    COMPILE_CHAR_PAIR_SET(PAIR_SET_ABCD, 1);
    ASSERT_COMPILE_SUCCESS("test_char_pair_set_search_start");
    SEARCH_CHAR_PAIR_SET(EXPR_NOISE_0, EXPR_NOISE_LEN, 1, (0), (0));
    EXPECT_SEARCH_SUCCESS("hs_char_pair_set_search", character_array, buffer);
    hs_free_char_pair_set_pattern(database);
}

TEST(char_pair_set_search, general) {
    SETUP_MEM_LEAK_TEST();
    COMPILE_CHAR_PAIR_SET(PAIR_SET_ABCD, 1);
    ASSERT_COMPILE_SUCCESS("test_char_pair_set_search_general");
    SEARCH_CHAR_PAIR_SET(EXPR_NOISE_5, EXPR_NOISE_LEN, 1, (5), (0));
    EXPECT_SEARCH_SUCCESS("hs_char_pair_set_search", character_array, buffer);
    hs_free_char_pair_set_pattern(database);
    EXPECT_MEMORY_CLEAN();
    UNSET_MEM_LEAK_TEST();
}

TEST(char_pair_set_search, cross_vector) {
    COMPILE_CHAR_PAIR_SET(PAIR_SET_ABCD, 1);
    ASSERT_COMPILE_SUCCESS("test_char_pair_set_search_cross_vector");
    SEARCH_CHAR_PAIR_SET(EXPR_NOISE_5_15, EXPR_NOISE_LEN, 2, (5, 15), (0, 0));
    EXPECT_SEARCH_SUCCESS("hs_char_pair_set_search", character_array, buffer);
    hs_free_char_pair_set_pattern(database);
}

TEST(char_pair_set_search, end) {
    COMPILE_CHAR_PAIR_SET(PAIR_SET_ABCD, 1);
    ASSERT_COMPILE_SUCCESS("test_char_pair_set_search_end");
    SEARCH_CHAR_PAIR_SET(EXPR_NOISE_AB_END_30, EXPR_NOISE_LEN, 1, (30), (0));
    EXPECT_SEARCH_SUCCESS("hs_char_pair_set_search", character_array, buffer);
    hs_free_char_pair_set_pattern(database);
}

TEST(char_pair_set_search, past_end) {
    COMPILE_CHAR_PAIR_SET(PAIR_SET_ABCD, 1);
    ASSERT_COMPILE_SUCCESS("test_char_pair_set_search_past_end");
    // cppcheck-suppress unsignedLessThanZero
    SEARCH_CHAR_PAIR_SET(EXPR_NOISE_AB_END_30, EXPR_NOISE_LEN - 1, 0, (), ());
    EXPECT_SEARCH_SUCCESS("hs_char_pair_set_search", character_array, buffer);
    hs_free_char_pair_set_pattern(database);
}

TEST(char_pair_set_search, null_char) {
    COMPILE_CHAR_PAIR_SET(PAIR_SET_A_NULL_BC, 1);
    ASSERT_COMPILE_SUCCESS("test_char_pair_set_search_null_char");
    SEARCH_CHAR_PAIR_SET(EXPR_NOISE_5_NULL, EXPR_NOISE_LEN, 1, (5), (0));
    EXPECT_SEARCH_SUCCESS("hs_char_pair_set_search", character_array, buffer);
    hs_free_char_pair_set_pattern(database);
}

TEST(char_pair_set_search, bad_case) {
    COMPILE_CHAR_PAIR_SET(PAIR_SET_ABCD, 1);
    ASSERT_COMPILE_SUCCESS("test_char_pair_set_search_bad_case");
    // cppcheck-suppress unsignedLessThanZero
    SEARCH_CHAR_PAIR_SET(EXPR_NOISE_5_15_BAD_CASE, EXPR_NOISE_LEN, 0, (), ());
    EXPECT_SEARCH_SUCCESS("hs_char_pair_set_search", character_array, buffer);
    hs_free_char_pair_set_pattern(database);
}

TEST(char_pair_set_search, several_search) {
    COMPILE_CHAR_PAIR_SET(PAIR_SET_ABCD, 1);
    ASSERT_COMPILE_SUCCESS("test_char_pair_set_search_several_search");
    SEARCH_CHAR_PAIR_SET(EXPR_NOISE_5, EXPR_NOISE_LEN, 1, (5), (0));
    EXPECT_SEARCH_SUCCESS("hs_char_pair_set_search", character_array, buffer);
    SEARCH_CHAR_PAIR_SET(EXPR_NOISE_5_15, EXPR_NOISE_LEN, 2, (5, 15), (0, 0));
    EXPECT_SEARCH_SUCCESS("hs_char_pair_set_search", character_array, buffer);
    hs_free_char_pair_set_pattern(database);
}

TEST(char_pair_set_search, duplicate) {
    COMPILE_CHAR_PAIR_SET(PAIR_SET_AB_DUPLICATE, 2);
    ASSERT_COMPILE_SUCCESS("char_pair_set_search_duplicate");
    SEARCH_CHAR_PAIR_SET(EXPR_NOISE_5, EXPR_NOISE_LEN, 1, (5), (0));
    EXPECT_SEARCH_SUCCESS("char_pair_set_search", character_array, buffer);
    hs_free_char_pair_set_pattern(database);
}

TEST(char_pair_set_search, match_multiple) {
    COMPILE_CHAR_PAIR_SET(PAIR_SET_ABCD, 2);
    ASSERT_COMPILE_SUCCESS("test_char_pair_set_search_match_multiple");
    SEARCH_CHAR_PAIR_SET(EXPR_NOISE_5, EXPR_NOISE_LEN, 2, (5, 7), (0, 1));
    EXPECT_SEARCH_SUCCESS("hs_char_pair_set_search", character_array, buffer);
    hs_free_char_pair_set_pattern(database);
}

TEST(char_pair_set_search, last_of_long_pattern) {
    COMPILE_CHAR_PAIR_SET(PAIR_SET_LONG_PATTERN_AB, 9);
    ASSERT_COMPILE_SUCCESS("test_char_pair_set_search_last_of_long_pattern");
    SEARCH_CHAR_PAIR_SET(EXPR_NOISE_5, EXPR_NOISE_LEN, 1, (5), (8));
    EXPECT_SEARCH_SUCCESS("hs_char_pair_set_search", character_array, buffer);
    hs_free_char_pair_set_pattern(database);
}

TEST(char_pair_set_search, null_char_buff_and_pattern) {
    COMPILE_CHAR_PAIR_SET(PAIR_SET_A_NULL_BC, 2);
    ASSERT_COMPILE_SUCCESS(
        "test_char_pair_set_search_null_char_buff_and_pattern");
    SEARCH_CHAR_PAIR_SET(EXPR_NOISE_5_NULL, EXPR_NOISE_LEN, 1, (5), (0));
    EXPECT_SEARCH_SUCCESS("hs_char_pair_set_search", character_array, buffer);
    hs_free_char_pair_set_pattern(database);
}

TEST(char_pair_set_search, null_char_buff) {
    COMPILE_CHAR_PAIR_SET(PAIR_SET_ABCD, 2);
    ASSERT_COMPILE_SUCCESS("test_char_pair_set_search_null_char_buff");
    // cppcheck-suppress unsignedLessThanZero
    SEARCH_CHAR_PAIR_SET(EXPR_NOISE_5_NULL, EXPR_NOISE_LEN, 0, (), ());
    EXPECT_SEARCH_SUCCESS("hs_char_pair_set_search", character_array, buffer);
    hs_free_char_pair_set_pattern(database);
}

TEST(char_pair_set_search, empty_buff) {
    COMPILE_CHAR_PAIR_SET(PAIR_SET_ABCD, 1);
    ASSERT_COMPILE_SUCCESS("test_char_pair_set_search_empty_buff");
    // cppcheck-suppress unsignedLessThanZero
    SEARCH_CHAR_PAIR_SET("", 0, 0, (), ());
    EXPECT_SEARCH_SUCCESS("hs_char_pair_set_search", character_array, buffer);
    hs_free_char_pair_set_pattern(database);
}

#if !defined(RELEASE_BUILD)
// test asserts

TEST(char_pair_set_search, nullptr_pattern) {
    const hs_char_pair_set_compiled_pattern_t *database = nullptr;
    context_t context;
    EXPECT_DEATH(
        {
            const char *buffer;
            hs_error_t ret;
            // cppcheck-suppress unsignedLessThanZero
            // cppcheck-suppress unreadVariable
            SEARCH_CHAR_PAIR_SET(EXPR_NOISE_5, EXPR_NOISE_LEN, 0, (), ());
        },
        "called with nullptr database");
}

TEST(char_pair_set_search, nullptr_buffer) {
    COMPILE_CHAR_PAIR_SET(PAIR_SET_ABCD, 1);
    ASSERT_COMPILE_SUCCESS("test_char_pair_set_search_nullptr_buffer");
    EXPECT_DEATH(
        { 
            // cppcheck-suppress unsignedLessThanZero
            // cppcheck-suppress unreadVariable
            SEARCH_CHAR_PAIR_SET(nullptr, EXPR_NOISE_LEN, 0, (), ());
        },
        "called with nullptr buffer");
}

TEST(char_pair_set_search, nullptr_callback) {
    COMPILE_CHAR_PAIR_SET(PAIR_SET_ABCD, 1);
    ASSERT_COMPILE_SUCCESS("test_char_pair_set_search_nullptr_callback");

    buffer = EXPR_NOISE_5;
    const size_t buffer_len = EXPR_NOISE_LEN;
    const size_t expected_match = 1;
    size_t expected_start_array[expected_match] = {5};
    size_t expected_end_array[expected_match] = {5};
    size_t expected_id_array[expected_match] = {0};
    for (size_t i = 0; i < expected_match; i++) {
        expected_end_array[i] += 2;
    }
    context.expected_start_array = expected_start_array;
    context.expected_end_array = expected_end_array;
    context.expected_id_array = expected_id_array;
    context.array_size = expected_match;
    context.number_matched = 0;
    context.number_wrong = 0;

    EXPECT_DEATH(
        {
            hs_char_pair_set_search(database, buffer, buffer_len, nullptr,
                                    &context);
        },
        "called with nullptr callback");
}

#endif
