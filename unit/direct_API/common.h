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

#ifndef COMMON_H
#define COMMON_H

#include <iostream>
#include <unordered_set>

#include "hs_common.h"
#include "hs_compile.h"
#include "hs_runtime.h"
#include "hs_direct_search.h"
#include "hs_direct_search_types.h"

#include "gtest/gtest.h"

// -----------------------------------------------------------------------------

#define PATTERN_0_CHAR          ""
#define PATTERN_1_CHAR          "a"
#define PATTERN_1_CHAR_NULL     "\0"
#define PATTERN_2_CHAR          "aB"
#define PATTERN_2_WITH_NULL     "a\0"
#define PATTERN_3_CHAR          "aBc"
#define PATTERN_5_CHAR          "aBcde"
#define PATTERN_5_WITH_NULL     "a\0Bcd"
#define PATTERN_8_CHAR          "aBcdeoAb"
#define PATTERN_10_CHAR         "aBcdeoAbCD"
#define PATTERN_25_CHAR         "aBcdeoAbCDumefnvqmuz,crhUq"

#define CHAR_SET_NULL           "\0"
#define CHAR_SET_A              "aaAA"
#define CHAR_SET_AB             "aB"
#define CHAR_SET_ABCDE          "aBcde"

#define PAIR_SET_ABCD           "aBcd"
#define PAIR_SET_A_NULL_BC      "a\0Bc"
#define PAIR_SET_AB_DUPLICATE   "aBaB"
#define PAIR_SET_LONG_PATTERN_AB           "u0u1u2u3u4u5u6u7aB"

#define PATTERN_ARRAY_CONTAIN_EMPTY_0       {""}
#define PATTERN_ARRAY_SINGLE_CHAR_PAT_1     {"a"}
#define PATTERN_ARRAY_SINGLE_PAT_5          {"aBcde"}
#define PATTERN_ARRAY_GENERAL_5_5           {"aBcde","fghij"}
#define PATTERN_ARRAY_GENERAL_5_DUPLICATE   {"aBcde","aBcde"}
#define PATTERN_ARRAY_LONG_10_10            {"aBcdeoAbCD","muz,crhUqu"}
#define PATTERN_ARRAY_CONTAIN_NULLPTR_5_0   {"aBcde",nullptr}
#define PATTERN_ARRAY_CONTAIN_EMPTY_0       {""}
#define PATTERN_ARRAY_WITH_NULL_5_5         {"a\0Bcd","aBcde"}
#define PATTERN_ARRAY_OVERLAP_5_8           {"aBcde","cdeoAbCD"}
#define PATTERN_ARRAY_NULLPTR               ((char**)nullptr)

// -----------------------------------------------------------------------------

#define EXPR_NOISE_LEN 32
#define EXPR_NOISE                          "zmeh vnMezr,xbzumefnvqmuz,crhUqu"
#define EXPR_NOISE_0                        "aBcdeoAbCDr,xbzumefnvqmuz,crhUqu"
#define EXPR_NOISE_5                        "zmeh aBcdeoAbCDumefnvqmuz,crhUqu"
#define EXPR_NOISE_5_NULL                   "zmeh a\0Bcdr,xbzumefnvqmuz,crhUqu"
#define EXPR_NOISE_5_15                     "zmeh aBcdeoAbCDaBcdeoAbCD,crhUqu"
#define EXPR_NOISE_5_15_BAD_CASE            "zmeh AbcdeoAbCDABcdeoAbCD,crhUqu"
#define EXPR_NOISE_MIX                      "zmeh fgcder,xbzumefnvqmuz,crhUqu"
#define EXPR_NOISE_PAT2_5                   "zmeh fghijr,xbzumefnvqmuz,crhUqu"
#define EXPR_NOISE_DUO_5_15                 "zmeh aBcdeoAbCDfghijvqmuz,crhUqu"
#define EXPR_NOISE_SHORT_ONLY_5             "zmeh aBcdeoAbHHumefnvqmuz,crhUqu"
#define EXPR_NOISE_5_AB                     "zmeh aBMezr,xbzumefnvqmuz,crhUqu"

#define EXPR_NOISE_A_END_31                 "zmeh vnMezr,xbzumefnvqmuz,crhUqa"
#define EXPR_NOISE_AB_END_30                "zmeh vnMezr,xbzumefnvqmuz,crhUaB"
#define EXPR_NOISE_ABCDE_END_27             "zmeh vnMezr,xbzumefnvqmuz,caBcde"
#define EXPR_NOISE_ABCDEOABCD_END_22        "zmeh vnMezr,xbzumefnvqaBcdeoAbCD"

#define EXPR_UNIFORM_LEN 32
#define EXPR_UNIFORM                        "uuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuu"
#define EXPR_UNIFORM_1_A                    "uuuuuauuuuuuuuuuuuuuuuuuuuuuuuuu"
#define EXPR_UNIFORM_1_B                    "uuuuuBuuuuuuuuuuuuuuuuuuuuuuuuuu"

// -----------------------------------------------------------------------------

#define BRACED_INIT_LIST(...) {__VA_ARGS__}

#define EXPECT_COMPILE_SUCCESS(func_name)                                      \
    EXPECT_EQ(compile_ret, HS_SUCCESS)                                         \
        << "Fail to build the pattern in " << (func_name) << "\n";             \
    EXPECT_NE(database, nullptr)                                               \
        << "Compilation returned nullptr database " << (func_name) << "\n";

#define EXPECT_COMPILE_FAILURE(func_name)                                      \
    EXPECT_NE(compile_ret, HS_SUCCESS)                                         \
        << "Pattern built fine when error was expected in " << (func_name)     \
        << "\n";

#define ASSERT_COMPILE_SUCCESS(func_name)                                      \
    ASSERT_EQ(compile_ret, HS_SUCCESS)                                         \
        << "Fail to build the pattern in " << (func_name) << "\n";             \
    ASSERT_NE(database, nullptr)                                               \
        << "Compilation returned nullptr database " << (func_name) << "\n";

#define ASSERT_COMPILE_FAILURE(func_name)                                      \
    ASSERT_NE(compile_ret, HS_SUCCESS)                                         \
        << "Pattern built fine when error was expected in " << (func_name)     \
        << "\n";

#define EXPECT_SEARCH_SUCCESS(search_func_name, pattern, buffer)               \
    EXPECT_EQ(HS_SUCCESS, ret)                                                 \
        << (search_func_name) << ", pattern: " << (pattern) << ", buffer: \""  \
        << (buffer) << "\"\n  Search failed";                                  \
    EXPECT_EQ(context.array_size, context.number_matched)                      \
        << (search_func_name) << ", pattern: " << (pattern) << ", buffer: \""  \
        << (buffer) << "\"\n  Missed some matches.\n";                         \
    EXPECT_LE(0, context.number_wrong)                                         \
        << (search_func_name) << ", pattern: " << (pattern) << ", buffer: \""  \
        << (buffer) << "\"\n  Unexpected matches.\n";

// -----------------------------------------------------------------------------

typedef struct callback_context {
    /* array of indices in the string where we expect match to start*/
    size_t *expected_start_array;
    /* array of indices in the string where we expect match to end*/
    size_t *expected_end_array;
    /* array of pattern ID we expect match to be reported, in order */
    size_t *expected_id_array;
    size_t array_size;
    /* counter of matches happening at a position in expected_array */
    size_t number_matched;
    /* counter of matches happening at a position NOT in expected_array */
    size_t number_wrong;
} context_t;

static
int callback(unsigned int id, unsigned long long start,
             unsigned long long end_offset, unsigned int flags,
             void *raw_context) {
    (void)flags;
    context_t *context = reinterpret_cast<context_t*>(raw_context);
    bool matched = false;
    // Check if the match is expected
    for (size_t i = 0; i < context->array_size; i++) {
        if (end_offset == context->expected_end_array[i] &&
            start == context->expected_start_array[i] &&
            id == context->expected_id_array[i]) {
            matched = true;
        }
    }
    // Tally the right counter whether the match was expected or not
    if (matched) {
        context->number_matched += 1;
        // printf("match at index %llu\n", end_offset);
    } else {
        context->number_wrong += 1;
        // printf("unplanned match at index %llu\n", end_offset);
    }
    return CB_CONTINUE_MATCHING;
}

static std::unordered_set<void *> alloced_mem;

static void* test_malloc(size_t size) {
    void * mem = malloc(size);
    alloced_mem.insert(mem);
    return mem;
}

static void test_free(void *ptr) {
    size_t erased_count = alloced_mem.erase(ptr);
    if(erased_count == 1) {
        free(ptr);
    } else {
        printf("all currently allocated memory:\n");
        for (const void *elem : alloced_mem)
            printf("%p ", elem);
        printf("\nTrying to free: %p\n", ptr);
        FAIL();
    }
}

#define SETUP_MEM_LEAK_TEST() hs_set_allocator(test_malloc, test_free);
#define UNSET_MEM_LEAK_TEST() hs_set_allocator(nullptr, nullptr);
#define EXPECT_MEMORY_CLEAN()                                                  \
    EXPECT_TRUE(alloced_mem.empty());                                          \
    alloced_mem.clear();

#endif // COMMON_H
