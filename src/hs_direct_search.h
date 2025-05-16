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

#ifndef DIRECT_SEARCH_H
#define DIRECT_SEARCH_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <string.h>
#include <stdint.h>

#include "allocator.h"

#include "fdr/fdr_internal.h"
#include "util/arch.h"

/*
 * FDR_pattern_storage memory layout:
 *
 * |-------------------------------------------------|
 * | size_t pattern_count                            |
 * |------------------------|------------------------|
 * | pattern_raw_storage    : char* pattern_ptrs[]   |
 * |                        :------------------------|
 * |                        : size_t pattern_sizes[] |
 * |                        :------------------------|
 * |                        : char actual_storage[]  |
 * |------------------------|------------------------|
 *
 * Use size_fdr_pattern() to get the size to allocate.
 */

struct FDR_pattern_storage {
    size_t pattern_count;
    char pattern_raw_storage[];
};

static inline char **get_pattern_ptrs(struct FDR_pattern_storage *pat) {
    // cppcheck-suppress cstyleCast
    return (char **)((char *)pat +
                     offsetof(struct FDR_pattern_storage, pattern_raw_storage));
}

static inline char *const *
get_const_pattern_ptrs(const struct FDR_pattern_storage *pat) {
    // cppcheck-suppress cstyleCast
    return (char *const *)((const char *)pat +
                           offsetof(struct FDR_pattern_storage,
                                    pattern_raw_storage));
}

static inline size_t *get_pattern_sizes(struct FDR_pattern_storage *pat) {
    // cppcheck-suppress cstyleCast
    return (size_t *)((char *)get_pattern_ptrs(pat) +
                      pat->pattern_count * sizeof(char *));
}

static inline const size_t *
get_const_pattern_sizes(const struct FDR_pattern_storage *pat) {
    // cppcheck-suppress cstyleCast
    return (const size_t *)((const char *)get_const_pattern_ptrs(pat) +
                            pat->pattern_count * sizeof(char *));
}

static inline char *
get_pattern_string_storage(struct FDR_pattern_storage *pat) {
    return (char *)get_pattern_sizes(pat) + pat->pattern_count * sizeof(size_t);
}

static inline const char *
get_const_pattern_string_storage(const struct FDR_pattern_storage *pat) {
    return (const char *)get_const_pattern_sizes(pat) +
           pat->pattern_count * sizeof(size_t);
}

static
void init_pattern_store(struct FDR_pattern_storage *storage,
                        const char **in_expression, size_t in_pattern_count,
                        const size_t *in_expression_length) {
    storage->pattern_count = in_pattern_count;
    memcpy(get_pattern_sizes(storage), in_expression_length,
            storage->pattern_count);
    char *next_string = get_pattern_string_storage(storage);
    for (size_t i = 0; i < storage->pattern_count; i++) {
        memcpy(next_string, in_expression[i], in_expression_length[i]);
        get_pattern_ptrs(storage)[i] = next_string;
        get_pattern_sizes(storage)[i] = in_expression_length[i];
        next_string += in_expression_length[i];
    }
}

static inline
void init_pattern_store_single(struct FDR_pattern_storage *storage,
                               const char *in_expression,
                               const size_t in_expression_length) {
    init_pattern_store(storage, &in_expression, 1, &in_expression_length);
}

static
size_t size_fdr_pattern(size_t in_pattern_count,
                    const size_t *in_expression_length) {
    size_t total_string_size = 0;
    for (size_t i = 0; i < in_pattern_count; i++) {
        total_string_size += in_expression_length[i];
    }
    size_t ptr_array_size = in_pattern_count * sizeof(char *);
    size_t pattern_sizes_array_size = in_pattern_count * sizeof(size_t);
    size_t required_mem = sizeof(struct FDR_pattern_storage) + ptr_array_size +
                            pattern_sizes_array_size + total_string_size;
    return required_mem;
}

/*
 * combined_fdr_database memory layout:
 *
 * |-------------------------------------------------|
 * | FDR *database                                   |
 * |-------------------------------------------------|
 * | FDR_pattern_storage *patterns                   |
 * |------------------------|------------------------|
 * | raw_storage            : FDR fdr_storage        |
 * |                        :------------------------|
 * |                        : FDR_pattern_storage    |
 * |------------------------|------------------------|
 *
 * Use size_fdr_database() to get the size to allocate.
 */
struct combined_fdr_database {
    struct FDR *database;
    struct FDR_pattern_storage *patterns;
    unsigned char raw_storage[];
};

void init_combined_fdr_database(struct combined_fdr_database *database,
                                size_t fdr_size, const char **in_expression,
                                size_t in_pattern_count,
                                const size_t *in_expression_length);

void init_combined_fdr_database_single(struct combined_fdr_database *database,
                                       size_t fdr_size,
                                       const char *in_expression,
                                       const size_t in_expression_length);
static inline
size_t size_fdr_database(size_t fdr_size, size_t in_pattern_count,
                    const size_t *in_expression_length) {
    return sizeof(struct combined_fdr_database) +
           size_fdr_pattern(in_pattern_count, in_expression_length) + fdr_size;
}

static inline
size_t size_fdr_database_single(size_t fdr_size,
                                const size_t in_expression_length) {
    return size_fdr_database(fdr_size, 1, &in_expression_length);
}

hwlmcb_rv_t HS_CDECL noodle_to_hs_callback(size_t end, u32 id,
                                           struct hs_scratch *scratch);

// Receive the FDR callback and perform the check for longer patterns (>8 char)
hwlmcb_rv_t HS_CDECL FDR_to_hs_callback(size_t end, u32 id,
                                        struct hs_scratch *scratch);

struct FDR_cb_context {
    void *usr_context;
    const struct FDR_pattern_storage *patterns;
    const char *buffer;
    size_t buffer_length;
};

struct noodle_context {
    void *usr_context;
    u8 pattern_length;
};

#ifdef __cplusplus
} // extern "C"
#endif

#endif // DIRECT_SEARCH_H
