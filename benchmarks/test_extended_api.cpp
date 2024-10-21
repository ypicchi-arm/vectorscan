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

#include <cstdlib>
#include <cstring>
#include <iostream>

#include "hs_compile.h"
#include "hs_runtime.h"

#include "hwlm/hwlm_literal.h"
#include "hwlm/noodle_build.h"
#include "hwlm/noodle_engine.h"
#include "hwlm/noodle_internal.h"

const char *buf1 = "azertyuioperty";
int buf1_len = 14;
const char *buf2 = "AZERTYUIOPERTY";
int buf2_len = 14;

typedef struct context {
    /* array of indices in the string where we expect match to be reported */
    size_t *expected_array;
    size_t array_size;
    /* counter of matches hapenning at a position in expected_array */
    size_t number_matched;
    /* counter of matches hapenning at a position NOT in expected_array */
    size_t number_wrong;
} context_t;

int callback(unsigned int id, unsigned long long start,
             unsigned long long end_offset, unsigned int flags,
             void *raw_context) {
    (void)id;
    (void)start;
    (void)flags;
    context_t *context = reinterpret_cast<context_t*>(raw_context);
    bool matched = false;
    // Check if the match is expected
    for (size_t i = 0; i < context->array_size; i++) {
        if (end_offset == context->expected_array[i]) {
            matched = true;
        }
    }
    // Tally the right counter wether the match was expected or not
    if (matched) {
        context->number_matched += 1;
    } else {
        context->number_wrong += 1;
        printf("unplanned match at index %llu\n", end_offset);
    }

    return CB_CONTINUE_MATCHING;
}

int test_noodle() {
    const char *pattern = "ert";
    hs_short_literal_compiled_pattern_t noodle_database;

    hs_error_t ret =
        hs_compile_short_literal_search(pattern, 3, &noodle_database);
    if (ret != HS_SUCCESS) {
        printf("Fail to build the pattern\n");
        return 1;
    }

    size_t expected_array[2] = {4, 12};
    context_t context = {&(expected_array[0]), 2, 0, 0};
    ret = hs_short_literal_search(&noodle_database, buf1, buf1_len, nullptr,
                                  callback, &context);
    if (ret != HS_SUCCESS) {
        printf("Fail to run noodle\n");
        return 1;
    }
    if (context.number_matched != context.array_size) {
        printf("1- missed some matches. Expected: %lu, got %lu\n",
               reinterpret_cast<unsigned long>(context.array_size),
               reinterpret_cast<unsigned long>(context.number_matched));
    }

    expected_array[0] = 8;
    context = {&(expected_array[0]), 1, 0, 0};
    ret = hs_short_literal_search(&noodle_database, buf1 + 4, buf1_len - 4,
                                  nullptr, callback, &context);
    if (ret != HS_SUCCESS) {
        printf("Fail to run noodle\n");
        return 1;
    }
    if (context.number_matched != context.array_size) {
        printf("2- missed some matches. Expected: %lu, got %lu\n",
               reinterpret_cast<unsigned long>(context.array_size),
               reinterpret_cast<unsigned long>(context.number_matched));
    }

    pattern = "ERT";
    ret = hs_compile_short_literal_search(pattern, 3, &noodle_database);
    if (ret != HS_SUCCESS) {
        printf("Fail to build the pattern\n");
        return 1;
    }

    expected_array[0] = 4;
    context = {&(expected_array[0]), 2, 0, 0};
    ret = hs_short_literal_search(&noodle_database, buf2, buf2_len, nullptr,
                                  callback, &context);
    if (ret != HS_SUCCESS) {
        printf("Fail to run noodle\n");
        return 1;
    }
    if (context.number_matched != context.array_size) {
        printf("3- missed some matches. Expected: %lu, got %lu\n",
               reinterpret_cast<unsigned long>(context.array_size),
               reinterpret_cast<unsigned long>(context.number_matched));
    }

    expected_array[0] = 8;
    context = {&(expected_array[0]), 1, 0, 0};
    ret = hs_short_literal_search(&noodle_database, buf2 + 4, buf2_len - 4,
                                  nullptr, callback, &context);
    if (ret != HS_SUCCESS) {
        printf("Fail to run noodle\n");
        return 1;
    }
    if (context.number_matched != context.array_size) {
        printf("4- missed some matches. Expected: %lu, got %lu\n",
               reinterpret_cast<unsigned long>(context.array_size),
               reinterpret_cast<unsigned long>(context.number_matched));
    }

    return 0;
}

int main() {
    // test_plain_noodle();
    if (!test_noodle()) {
        printf("all test passed\n");
    }

    return 0;
}