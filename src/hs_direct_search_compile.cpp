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


#include <string>
#include <cstring>

#include "hs_common.h"
#include "hs_compile.h"
#include "hs_direct_search.h"
#include "hs_direct_search_types.h"

#include "allocator.h" // hs_database_alloc()
#include "grey.h"
#include "hwlm/hwlm.h" // HWLM_LITERAL_MAX_LEN
#include "hwlm/hwlm_internal.h" // HWLM_ENGINE_FDR
#include "hwlm/hwlm_literal.h" // ue2::hwlmLiteral
#include "hwlm/noodle_internal.h" // noodTable
#include "ue2common.h" // likely() - unlikely()
#include "util/arch.h" // CAN_USE_WIDE_TRUFFLE
#include "util/bytecode_ptr.h"
#include "util/charreach.h"
#include "util/flat_containers.h" // flat_set
#include "util/supervector/supervector.hpp"
#include "util/target_info.h" // target_t

#include "fdr/fdr_compile.h"
#include "hwlm/noodle_build.h"
#include "nfa/shufticompile.h"
#include "nfa/trufflecompile.h"

typedef typename SuperVector<VECTORSIZE>::comparemask_type vector_mask_type;

void init_combined_fdr_database(struct combined_fdr_database *database,
                                size_t fdr_size, const char **in_expression,
                                size_t in_pattern_count,
                                const size_t *in_expression_length) {
    database->database = reinterpret_cast<FDR *>(database->raw_storage);
    database->patterns = reinterpret_cast<FDR_pattern_storage *>(
        database->raw_storage + fdr_size);
    init_pattern_store(database->patterns, in_expression, in_pattern_count,
                       in_expression_length);
};

void init_combined_fdr_database_single(struct combined_fdr_database *database,
                                       size_t fdr_size,
                                       const char *in_expression,
                                       const size_t in_expression_length) {
    database->database = reinterpret_cast<FDR *>(database->raw_storage);
    database->patterns = reinterpret_cast<FDR_pattern_storage *>(
        database->raw_storage + fdr_size);
    init_pattern_store_single(database->patterns, in_expression,
                              in_expression_length);
};

inline void generic_free(void *database) {
    if (likely(database)) {
        hs_database_free(database);
    }
}




// --- short_literal (Noodle) ---

HS_PUBLIC_API
hs_error_t HS_CDECL hs_compile_short_literal_search(
    const char *expression, size_t expression_length,
    hs_short_literal_compiled_pattern **output_database) {
    assert(expression_length > 0 &&
           "hs_compile_short_literal_search called with an empty pattern");
    assert(expression != nullptr &&
           "hs_compile_short_literal_search called with nullptr");
    assert(output_database != nullptr &&
           "hs_compile_short_literal_search called with nullptr");
    if (unlikely(expression_length > HS_SHORT_PATTERN_THRESHOLD)) {
        return HS_INVALID;
    }
    /*
     * Exposing caseness at the api level may restrict our ability to change
     * the backing algorithm, so we decided to make all algo case sensitive
     */
    bool is_case_insensitive = false;
    bool only_need_first_match = false;
    ue2::hwlmLiteral lit(std::string(expression, expression_length),
                         is_case_insensitive, only_need_first_match, 0,
                         HWLM_ALL_GROUPS, {}, {});

    hs_short_literal_compiled_pattern *database =
        reinterpret_cast<hs_short_literal_compiled_pattern *>(hs_database_alloc(
            sizeof(hs_short_literal_compiled_pattern)));
    if (unlikely(database == nullptr)) {
        return HS_NOMEM;
    }
    ue2::bytecode_ptr<noodTable> bytecode_database = ue2::noodBuildTable(lit);
    if (unlikely(bytecode_database.get() == nullptr)) {
        return HS_UNKNOWN_ERROR;
    }
    database->pattern_length = expression_length;
    memcpy(&(database->noodle_database), bytecode_database.get(),
           sizeof(noodTable));
    *output_database = database;

    return HS_SUCCESS;
}

HS_PUBLIC_API
void hs_free_short_literal_pattern(
    hs_short_literal_compiled_pattern *database) {
    generic_free(database);
}





// --- long_literal (FDR) ---

HS_PUBLIC_API
hs_error_t HS_CDECL hs_compile_long_literal_search(
    const char *expression, size_t expression_length,
    hs_long_literal_compiled_pattern_t **output_database) {
    assert(expression_length > 0 &&
           "hs_compile_long_literal_search called with an empty pattern");
    assert(expression != nullptr &&
           "hs_compile_long_literal_search called with nullptr");
    assert(output_database != nullptr &&
           "hs_compile_long_literal_search called with nullptr");
    /*
     * Exposing caseness at the api level may restrict our ability to change
     * the backing algorithm, so we decided to make all algo case sensitive
     */
    bool is_case_insensitive = false;
    bool only_need_first_match = false;
    std::vector<ue2::hwlmLiteral> lits;
    // longer strings are checked in the callback
    ue2::hwlmLiteral lit(
        std::string(expression,
                    std::min(expression_length, (size_t)HWLM_LITERAL_MAX_LEN)),
        is_case_insensitive, only_need_first_match, 0, HWLM_ALL_GROUPS, {}, {});
    lits.push_back(lit);

    ue2::Grey g = ue2::Grey();
    u8 engType = HWLM_ENGINE_FDR;
    bool make_small = false;

    hs_platform_info platform_info;
    hs_populate_platform(&platform_info);

    ue2::target_t target = ue2::target_t(platform_info);

    std::unique_ptr<ue2::HWLMProto> proto =
        ue2::fdrBuildProto(engType, lits, make_small, target, g);

    ue2::bytecode_ptr<FDR> bytecode_database = ue2::fdrBuildTable(*proto, g);
    if (unlikely(bytecode_database.get() == nullptr)) {
        return HS_UNKNOWN_ERROR;
    }
    size_t fdr_size = bytecode_database.get()->size;

    size_t mem_required = size_fdr_database_single(fdr_size, expression_length);
    struct combined_fdr_database *combined_database =
        reinterpret_cast<struct combined_fdr_database *>(
            hs_database_alloc(mem_required));
    if (unlikely(combined_database == nullptr)) {
        return HS_NOMEM;
    }
    init_combined_fdr_database_single(combined_database, fdr_size, expression,
                                      expression_length);
    memcpy(combined_database->database, bytecode_database.get(), fdr_size);
    *output_database = reinterpret_cast<hs_long_literal_compiled_pattern_t *>(
        combined_database);

    return HS_SUCCESS;
}

HS_PUBLIC_API
void hs_free_long_literal_pattern(
    hs_long_literal_compiled_pattern_t *database) {
    generic_free(database);
}






// --- multi_literal (FDR) ---

HS_PUBLIC_API
hs_error_t HS_CDECL hs_compile_multi_literal_search(
    const char **expression, size_t pattern_count,
    const size_t *expression_length,
    hs_multi_literal_compiled_pattern_t **output_database) {
    assert(pattern_count > 0 &&
           "hs_compile_multi_literal_search called with no pattern");
    assert(expression != nullptr &&
           "hs_compile_multi_literal_search called with nullptr");
    assert(expression_length != nullptr &&
           "hs_compile_multi_literal_search called with nullptr");
    assert(output_database != nullptr &&
           "hs_compile_multi_literal_search called with nullptr");
    /*
     * Exposing caseness at the api level may restrict our ability to change
     * the backing algorithm, so we decided to make all algo case sensitive
     */
    bool is_case_insensitive = false;
    bool only_need_first_match = false;
    std::vector<ue2::hwlmLiteral> lits;
    for (size_t i = 0; i < pattern_count; i++) {
        assert(expression_length[i] > 0 && expression[i] &&
               "hs_compile_multi_literal_search called with an empty pattern");
        // longer strings are checked in the callback
        ue2::hwlmLiteral lit(
            std::string(expression[i], std::min(expression_length[i],
                                                (size_t)HWLM_LITERAL_MAX_LEN)),
            is_case_insensitive, only_need_first_match, i, HWLM_ALL_GROUPS, {},
            {});
        lits.push_back(lit);
    }

    ue2::Grey g = ue2::Grey();
    u8 engType = HWLM_ENGINE_FDR;
    bool make_small = false;

    hs_platform_info platform_info;
    hs_populate_platform(&platform_info);

    ue2::target_t target = ue2::target_t(platform_info);

    std::unique_ptr<ue2::HWLMProto> proto =
        ue2::fdrBuildProto(engType, lits, make_small, target, g);

    ue2::bytecode_ptr<FDR> bytecode_database = ue2::fdrBuildTable(*proto, g);
    if (unlikely(bytecode_database.get() == nullptr)) {
        return HS_UNKNOWN_ERROR;
    }
    size_t fdr_size = bytecode_database.get()->size;

    size_t mem_required =
        size_fdr_database(fdr_size, pattern_count, expression_length);
    struct combined_fdr_database *combined_database =
        reinterpret_cast<struct combined_fdr_database *>(
            hs_database_alloc(mem_required));
    if (unlikely(combined_database == nullptr)) {
        return HS_NOMEM;
    }
    init_combined_fdr_database(combined_database, fdr_size, expression,
                               pattern_count, expression_length);
    memcpy(combined_database->database, bytecode_database.get(), fdr_size);
    *output_database = reinterpret_cast<hs_multi_literal_compiled_pattern_t *>(
        combined_database);

    return HS_SUCCESS;
}

HS_PUBLIC_API
void hs_free_multi_literal_pattern(
    hs_multi_literal_compiled_pattern_t *database) {
    generic_free(database);
}





// --- single_char (Noodle) ---

HS_PUBLIC_API
hs_error_t HS_CDECL hs_compile_single_char_search(
    const char character, hs_single_char_compiled_pattern **output_database) {
    assert(output_database != nullptr &&
           "hs_compile_single_char_search called with nullptr");

    /*
     * Exposing caseness at the api level may restrict our ability to change
     * the backing algorithm, so we decided to make all algo case sensitive
     */
    bool is_case_insensitive = false;
    bool only_need_first_match = false;
    ue2::hwlmLiteral lit(std::string(&character, 1), is_case_insensitive,
                         only_need_first_match, 0, HWLM_ALL_GROUPS, {}, {});

    hs_single_char_compiled_pattern *database =
        reinterpret_cast<hs_single_char_compiled_pattern *>(hs_database_alloc(
            sizeof(hs_single_char_compiled_pattern)));
    if (unlikely(database == nullptr)) {
        return HS_NOMEM;
    }
    ue2::bytecode_ptr<noodTable> bytecode_database = ue2::noodBuildTable(lit);
    if (unlikely(bytecode_database.get() == nullptr)) {
        return HS_UNKNOWN_ERROR;
    }
    memcpy(&(database->noodle_database), bytecode_database.get(),
           sizeof(noodTable));
    *output_database = database;

    return HS_SUCCESS;
}

HS_PUBLIC_API
void hs_free_single_char_pattern(
    hs_single_char_compiled_pattern *database) {
    generic_free(database);
}





// --- char_set (Truffle) ---

HS_PUBLIC_API
hs_error_t HS_CDECL
hs_compile_char_set_search(const char *character_array, size_t character_count,
                           hs_char_set_compiled_pattern **output_database) {
    assert(character_count > 0 &&
           "hs_compile_char_set_search called with an empty set");
    assert(character_array != nullptr &&
           "hs_compile_char_set_search called with nullptr");
    assert(output_database != nullptr &&
           "hs_compile_char_set_search called with nullptr");

    const ue2::CharReach cr =
        ue2::CharReach(std::string(character_array, character_count));
    truffle_storage *database = reinterpret_cast<truffle_storage *>(
        hs_database_alloc(sizeof(truffle_storage)));
    // hs_database_alloc is meant to align to a machine word (likely 64b), which
    // is actually required here
    assert((((intptr_t)(database) & 3) == 0) &&
           "user-provided alloc didn't meet alignment requirement in "
           "hs_compile_char_set_search");
    for (u8 i = 0; i < character_count; i++) {
        database->char_id_map[(u8)character_array[i]] = i;
    }

#ifdef CAN_USE_WIDE_TRUFFLE
        ue2::truffleBuildMasksWide(cr, database->wide_mask);
#else
        ue2::truffleBuildMasks(cr, database->mask1,
                          database->mask2);
#endif

    *output_database = database;

    return HS_SUCCESS;
}

HS_PUBLIC_API
void hs_free_char_set_pattern(hs_char_set_compiled_pattern *database) {
    generic_free(database);
}





// --- single_char_pair (Noodle) ---

HS_PUBLIC_API
hs_error_t HS_CDECL hs_compile_single_char_pair_search(
    const char *pair, hs_single_char_pair_compiled_pattern **output_database) {
    assert(pair != nullptr &&
           "hs_compile_single_char_pair_search called with nullptr");
    assert(output_database != nullptr &&
           "hs_compile_single_char_pair_search called with nullptr");

    /*
     * Exposing caseness at the api level may restrict our ability to change
     * the backing algorithm, so we decided to make all algo case sensitive
     */
    bool is_case_insensitive = false;
    bool only_need_first_match = false;
    ue2::hwlmLiteral lit(std::string(pair, 2), is_case_insensitive,
                         only_need_first_match, 0, HWLM_ALL_GROUPS, {}, {});

    hs_single_char_pair_compiled_pattern *database =
        reinterpret_cast<hs_single_char_pair_compiled_pattern *>(
            hs_database_alloc(sizeof(hs_single_char_pair_compiled_pattern)));
    if (unlikely(database == nullptr)) {
        return HS_NOMEM;
    }
    ue2::bytecode_ptr<noodTable> bytecode_database = ue2::noodBuildTable(lit);
    if (unlikely(bytecode_database.get() == nullptr)) {
        return HS_UNKNOWN_ERROR;
    }
    memcpy(&(database->noodle_database), bytecode_database.get(),
           sizeof(noodTable));
    *output_database = database;

    return HS_SUCCESS;
}

HS_PUBLIC_API
void hs_free_single_char_pair_pattern(
    hs_single_char_pair_compiled_pattern *database) {
    generic_free(database);
}





// --- char_pair_set (Double shufti) ---

HS_PUBLIC_API
hs_error_t HS_CDECL hs_compile_char_pair_set_search(
    const char *expression, size_t pair_count,
    hs_char_pair_set_compiled_pattern **output_database) {
    assert(pair_count > 0 &&
           "hs_compile_char_pair_set_search called with an empty set");
    assert(expression != nullptr &&
           "hs_compile_char_pair_set_search called with nullptr");
    assert(output_database != nullptr &&
           "hs_compile_char_pair_set_search called with nullptr");

    ue2::flat_set<std::pair<u8, u8>> pairs;
    for (u8 i = 0; i < pair_count; i++) {
        pairs.insert(
            std::make_pair((u8)expression[2 * i], (u8)expression[2 * i + 1]));
    }

    hs_char_pair_set_compiled_pattern *database =
        reinterpret_cast<hs_char_pair_set_compiled_pattern *>(hs_database_alloc(
            sizeof(hs_char_pair_set_compiled_pattern) +
            sizeof(char) * 2 * pair_count));
    // hs_database_alloc is meant to align to a machine word (likely 64b), which
    // is actually required here
    assert((((intptr_t)(database) & 3) == 0) &&
           "user-provided alloc didn't meet alignment requirement in "
           "hs_compile_char_pair_set_search");

    bool success = ue2::shuftiBuildDoubleMasks(
        ue2::CharReach(), pairs, database->dshufti_database.mask1,
        database->dshufti_database.mask2, database->dshufti_database.mask3,
        database->dshufti_database.mask4);

    if (!success) {
        return HS_COMPILER_ERROR;
    }

    database->dshufti_database.pair_count = pair_count;

    size_t width = SuperVector<VECTORSIZE>::mask_width();
    assert(width <= 4 &&
           "Code needs rework if supervector's mask are bigger than 4");
    assert(width != 3 &&
           "Code needs rework if supervector's mask aren't a power of 2");
    // we need a mask such that every 2*width bits, only the lsb is set to 1
    // so for a width of 4, we repeat 0X01
    unsigned char bit_filter_mask = 0;
    for (size_t i = 8; i > 0; i -= 2 * width) {
        bit_filter_mask = bit_filter_mask << (2 * width) | 0x1;
    }
    memset(&(database->dshufti_database.bit_filter_mask), bit_filter_mask,
           sizeof(vector_mask_type));
    memcpy(database->dshufti_database.all_pairs, expression, 2 * pair_count);

    *output_database = database;

    return HS_SUCCESS;
}

HS_PUBLIC_API
void hs_free_char_pair_set_pattern(
    hs_char_pair_set_compiled_pattern *database) {
    generic_free(database);
}

