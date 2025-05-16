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

#ifndef DIRECT_SEARCH_TYPES_H
#define DIRECT_SEARCH_TYPES_H

#include <stdalign.h>

#include "util/supervector/supervector.hpp"

#include "fdr/fdr_internal.h"
#include "hwlm/noodle_internal.h"


struct hs_short_literal_compiled_pattern {
    noodTable noodle_database;
    u8 pattern_length;
};

struct hs_long_literal_compiled_pattern {
    struct combined_fdr_database fdr_database;
};

struct hs_multi_literal_compiled_pattern {
    struct combined_fdr_database fdr_database;
};

struct hs_single_char_compiled_pattern {
    struct noodTable noodle_database;
};

struct hs_single_char_pair_compiled_pattern {
    struct noodTable noodle_database;
};

typedef struct hs_char_set_compiled_pattern {
    union
    {
        struct {
            uint8_t mask1[16] __attribute__((aligned));
            uint8_t mask2[16] __attribute__((aligned));
        };
        uint8_t wide_mask[32] __attribute__((aligned));
    };
    // allows us to get the id from the character
    u8 char_id_map[256];
} truffle_storage;

struct dshufti_storage {
    alignas(16) uint8_t mask1[16];
    alignas(16) uint8_t mask2[16];
    alignas(16) uint8_t mask3[16];
    alignas(16) uint8_t mask4[16];
    size_t pair_count;
    typename SuperVector<VECTORSIZE>::comparemask_type bit_filter_mask;
    alignas(VECTORSIZE) uint8_t all_pairs[];
};

struct hs_char_pair_set_compiled_pattern {
    struct dshufti_storage dshufti_database;
};
#endif // DIRECT_SEARCH_TYPES_H
