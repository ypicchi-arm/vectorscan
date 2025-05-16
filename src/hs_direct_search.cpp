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
#include "hs_runtime.h"
#include "hs_direct_search.h"
#include "hs_direct_search_types.h"

#include "scratch.h"
#include "util/arch.h" // CAN_USE_WIDE_TRUFFLE
#include "util/bitutils.h" // ctz64()
#include "util/simd_utils.h" // load128()
#include "util/supervector/supervector.hpp"

#include "fdr/fdr.h"
#include "hwlm/noodle_engine.h"
#include "nfa/shufti.h"
#include "nfa/truffle.h"

typedef typename SuperVector<VECTORSIZE>::comparemask_type vector_mask_type;

static_assert((uint64_t)CB_CONTINUE_MATCHING == HWLM_CONTINUE_MATCHING,
              "CB_CONTINUE_MATCHING doesn't match HWLM_CONTINUE_MATCHING");
static_assert((uint64_t)CB_TERMINATE_MATCHING == HWLM_TERMINATE_MATCHING,
              "CB_TERMINATE_MATCHING doesn't match HWLM_TERMINATE_MATCHING");

static inline hs_error_t hwlm_to_hs_error(const hwlm_error_t error) {
    switch (error) {
    case HWLM_SUCCESS:
        return HS_SUCCESS;
    case HWLM_TERMINATED:
        return HS_SCAN_TERMINATED;
    case HWLM_ERROR_UNKNOWN:
        return HS_UNKNOWN_ERROR;
    case HWLM_LITERAL_MAX_LEN:
        return HS_COMPILER_ERROR;
    default:
        return HS_UNKNOWN_ERROR;
    }
}

// convert the callback type of Noodle
hwlmcb_rv_t HS_CDECL noodle_to_hs_callback(size_t end, u32 id,
                                  struct hs_scratch *scratch) {
    struct noodle_context *storage = reinterpret_cast<struct noodle_context *>(
        scratch->core_info.userContext);
    // hwlm's end is the last char of the pattern, but hs's end is the first
    // char after the pattern
    size_t match_start = end + 1 - storage->pattern_length;
    return (hwlmcb_rv_t)(scratch->core_info.userCallback(
        id, match_start, end + 1, 0, storage->usr_context));
}

// Receive the FDR callback and perform the check for longer patterns (>8 char)
hwlmcb_rv_t HS_CDECL FDR_to_hs_callback(size_t end, u32 id,
                                        struct hs_scratch *scratch) {
    const struct FDR_cb_context *combined_ctx =
        reinterpret_cast<struct FDR_cb_context *>(
            scratch->core_info.userContext);
    const FDR_pattern_storage *ps = combined_ctx->patterns;
    size_t pattern_length = get_const_pattern_sizes(ps)[id];
    size_t start_offset =
            end + 1 - std::min(pattern_length, (size_t)HWLM_LITERAL_MAX_LEN);
    if (pattern_length > HWLM_LITERAL_MAX_LEN) {
        // long pattern for FDR, we need to confirm it.
        const char *pattern = get_const_pattern_ptrs(ps)[id];
        const char *buffer = combined_ctx->buffer;
        size_t buffer_length = combined_ctx->buffer_length;

        if (start_offset + pattern_length > buffer_length) {
            // pattern too long for the remaining buffer, no match
            return HWLM_CONTINUE_MATCHING;
        }

        const char *confirm_buffer_start =
            buffer + start_offset + HWLM_LITERAL_MAX_LEN;
        const char *confirm_pattern_start = pattern + HWLM_LITERAL_MAX_LEN;
        size_t confirm_len = pattern_length - HWLM_LITERAL_MAX_LEN;

        if (confirm_len >= VECTORSIZE) {
            while (confirm_len > VECTORSIZE) {
                SuperVector<VECTORSIZE> buffer_vector =
                    SuperVector<VECTORSIZE>::loadu(confirm_buffer_start);
                SuperVector<VECTORSIZE> pattern_vector =
                    SuperVector<VECTORSIZE>::loadu(confirm_pattern_start);
                vector_mask_type mask = buffer_vector.eqmask(pattern_vector);
                if(~mask)
                    // don't match the pattern, continue searching
                    return HWLM_CONTINUE_MATCHING;
                confirm_buffer_start += VECTORSIZE;
                confirm_pattern_start += VECTORSIZE;
                confirm_len -= VECTORSIZE;
            }

            // unaligned load: we cannot risk loading any extra byte, so we run
            // the vector one last time with an offset to overlap the previous
            // check, but avoid overflowing.
            size_t overlap = VECTORSIZE - confirm_len;
            SuperVector<VECTORSIZE> buffer_vector =
                SuperVector<VECTORSIZE>::loadu(confirm_buffer_start - overlap);
            SuperVector<VECTORSIZE> pattern_vector =
                SuperVector<VECTORSIZE>::loadu(confirm_pattern_start - overlap);
            vector_mask_type mask = buffer_vector.eqmask(pattern_vector);
            if(~mask)
                // don't match the pattern, continue searching
                return HWLM_CONTINUE_MATCHING;
        } else {
            size_t confirm_64 = confirm_len / 8;
            for (size_t i = 0; i < confirm_64; i++) {
                if ((reinterpret_cast<const uint64_t *>(confirm_buffer_start))[i] !=
                    (reinterpret_cast<const uint64_t *>(confirm_pattern_start))[i])
                    // don't match the pattern, continue searching
                    return HWLM_CONTINUE_MATCHING;
            }
            confirm_len = confirm_len % 8;

            for (size_t i = 0; i < confirm_len; i++) {
                if (confirm_buffer_start[i] != confirm_pattern_start[i])
                    // don't match the pattern, continue searching
                    return HWLM_CONTINUE_MATCHING;
            }
        }

        // we have a valid match. Call the user callback
        return (hwlmcb_rv_t)(scratch->core_info.userCallback(
            id, start_offset, start_offset + pattern_length, 0,
            combined_ctx->usr_context));
    } else {
        // short pattern, no confirmation needed
        return (hwlmcb_rv_t)(scratch->core_info.userCallback(
            id, start_offset, end + 1, 0, combined_ctx->usr_context));
    }
}




// --- short_literal (Noodle) ---

HS_PUBLIC_API
hs_error_t HS_CDECL hs_short_literal_search(
    const hs_short_literal_compiled_pattern *database, const char *data,
    size_t length, match_event_handler onEvent, void *context) {
    assert(onEvent != nullptr &&
           "hs_short_literal_search called with nullptr callback");
    assert(data != nullptr &&
           "hs_short_literal_search called with nullptr buffer");
    assert(database != nullptr &&
           "hs_short_literal_search called with nullptr database");
    struct noodle_context storage;
    storage.usr_context = context;
    storage.pattern_length = database->pattern_length;
    struct hs_scratch scratch;
    scratch.core_info.userContext = &storage;
    scratch.core_info.userCallback = onEvent;

    hwlm_error_t error = noodExec(&(database->noodle_database),
                                  reinterpret_cast<const uint8_t *>(data),
                                  length, 0, noodle_to_hs_callback, &scratch);
    return hwlm_to_hs_error(error);
}





// --- long_literal (FDR) ---

HS_PUBLIC_API
hs_error_t HS_CDECL hs_long_literal_search(
    const hs_long_literal_compiled_pattern_t *database, const char *data,
    size_t length, match_event_handler onEvent,
    void *context) {
    assert(onEvent != nullptr &&
           "hs_long_literal_search called with nullptr callback");
    assert(data != nullptr &&
           "hs_long_literal_search called with nullptr buffer");
    assert(database != nullptr &&
           "hs_long_literal_search called with nullptr database");

    struct hs_scratch scratch;
    struct FDR_cb_context combined_ctx = {
        context, database->fdr_database.patterns, data, length};
    scratch.core_info.userContext = &combined_ctx;
    scratch.core_info.userCallback = onEvent;
    scratch.fdr_conf = nullptr;
    hwlm_error_t error =
        fdrExec(database->fdr_database.database,
                reinterpret_cast<const uint8_t *>(data), length, 0,
                FDR_to_hs_callback, &scratch, HWLM_ALL_GROUPS);
    return hwlm_to_hs_error(error);
}






// --- multi_literal (FDR) ---

HS_PUBLIC_API
hs_error_t HS_CDECL hs_multi_literal_search(
    const hs_multi_literal_compiled_pattern_t *database, const char *data,
    size_t length, match_event_handler onEvent, void *context) {
    assert(onEvent != nullptr &&
           "hs_multi_literal_search called with nullptr callback");
    assert(data != nullptr &&
           "hs_multi_literal_search called with nullptr buffer");
    assert(database != nullptr &&
           "hs_multi_literal_search called with nullptr database");

    struct hs_scratch scratch;
    struct FDR_cb_context combined_ctx = {
        context, database->fdr_database.patterns, data, length};
    scratch.core_info.userContext = &combined_ctx;
    scratch.core_info.userCallback = onEvent;
    scratch.fdr_conf = nullptr;
    hwlm_error_t error =
        fdrExec(database->fdr_database.database,
                reinterpret_cast<const uint8_t *>(data), length, 0,
                FDR_to_hs_callback, &scratch, HWLM_ALL_GROUPS);
    return hwlm_to_hs_error(error);
}





// --- single_char (Noodle) ---

HS_PUBLIC_API
hs_error_t HS_CDECL hs_single_char_search(
    const hs_single_char_compiled_pattern *database, const char *data,
    size_t length, match_event_handler onEvent, void *context) {
    assert(onEvent != nullptr &&
           "hs_single_char_search called with nullptr callback");
    assert(data != nullptr &&
           "hs_single_char_search called with nullptr buffer");
    assert(database != nullptr &&
           "hs_single_char_search called with nullptr database");
    struct noodle_context storage;
    storage.usr_context = context;
    storage.pattern_length = 1;
    struct hs_scratch scratch;
    scratch.core_info.userContext = &storage;
    scratch.core_info.userCallback = onEvent;

    hwlm_error_t error = noodExec(&(database->noodle_database),
                                  reinterpret_cast<const uint8_t *>(data),
                                  length, 0, noodle_to_hs_callback, &scratch);
    return hwlm_to_hs_error(error);
}





// --- char_set (Truffle) ---

HS_PUBLIC_API
hs_error_t HS_CDECL hs_char_set_search(
    const hs_char_set_compiled_pattern *database, const char *data,
    size_t length, match_event_handler onEvent, void *context) {
    assert(onEvent != nullptr &&
           "hs_char_set_search called with nullptr callback");
    assert(data != nullptr &&
           "hs_char_set_search called with nullptr buffer");
    assert(database != nullptr &&
           "hs_char_set_search called with nullptr database");

    const u8 *current_buf = reinterpret_cast<const u8*>(data);
    // buf_end must be the first char past the buffer, so current_buf==buf_end
    // means current_buf is empty.
    const u8 *buf_end = reinterpret_cast<const u8*>(data) + length;
    while(current_buf < buf_end) {
        const u8 *current_match;
#ifdef CAN_USE_WIDE_TRUFFLE
        current_match = truffleExecWide(
            loadu256(database->wide_mask), current_buf, buf_end);
#else
        current_match = truffleExec(load128(database->mask1),
                                    load128(database->mask2),
                                    current_buf, buf_end);
#endif
        // current_match is the pointer to the matching char, NOT past the
        // matching char. or buf_end if no match.
        if(current_match < buf_end) {
            size_t id = database->char_id_map[*current_match];
            size_t match_start =
                current_match - reinterpret_cast<const u8 *>(data);
            if( ! onEvent(id, match_start, match_start + 1, 0, context)) {
                // user requested to stop matching
                break;
            }
        }
        current_buf = current_match + 1;
    }

    return HS_SUCCESS;
}





// --- single_char_pair (Noodle) ---

HS_PUBLIC_API
hs_error_t HS_CDECL hs_single_char_pair_search(
    const hs_single_char_pair_compiled_pattern *database,
    const char *data, size_t length, match_event_handler onEvent,
    void *context) {
    assert(onEvent != nullptr &&
           "hs_single_char_pair_search called with nullptr callback");
    assert(data != nullptr &&
           "hs_single_char_pair_search called with nullptr buffer");
    assert(database != nullptr &&
           "hs_single_char_pair_search called with nullptr database");
    struct noodle_context storage;
    storage.usr_context = context;
    storage.pattern_length = 2;
    struct hs_scratch scratch;
    scratch.core_info.userContext = &storage;
    scratch.core_info.userCallback = onEvent;

    hwlm_error_t error = noodExec(&(database->noodle_database),
                                  reinterpret_cast<const uint8_t *>(data),
                                  length, 0, noodle_to_hs_callback, &scratch);
    return hwlm_to_hs_error(error);
}





// --- char_pair_set (Double shufti) ---

HS_PUBLIC_API
hs_error_t HS_CDECL hs_char_pair_set_search(
    const hs_char_pair_set_compiled_pattern *database, const char *data,
    size_t length, match_event_handler onEvent, void *context) {
    assert(onEvent != nullptr &&
           "hs_char_pair_set_search called with nullptr callback");
    assert(data != nullptr &&
           "hs_char_pair_set_search called with nullptr buffer");
    assert(database != nullptr &&
           "hs_char_pair_set_search called with nullptr database");

    const u8 *current_buf = reinterpret_cast<const u8*>(data);
    // buf_end must be the first char past the buffer, so current_buf==buf_end
    // means current_buf is empty.
    const u8 *buf_end = reinterpret_cast<const u8*>(data) + length;
    while(current_buf < buf_end) {
        const u8 *current_match;
        current_match = shuftiDoubleExec(
            load128(database->dshufti_database.mask1),
            load128(database->dshufti_database.mask2),
            load128(database->dshufti_database.mask3),
            load128(database->dshufti_database.mask4), current_buf, buf_end);
        // current_match is the pointer to the matching char, NOT past the
        // matching char. or buf_end if no match.
        if (current_match < buf_end) {
            // Shufti doesn't return which pair matched so we have to find out.
            // Use a 16 bits vector search on the original pattern string,
            // then return the <first match>/2 as ID.
            SuperVector<VECTORSIZE> found_pair = SuperVector<VECTORSIZE>(
                *reinterpret_cast<const u16 *>(current_match));
            size_t width = SuperVector<VECTORSIZE>::mask_width();
            SuperVector<VECTORSIZE> all_pair;
            vector_mask_type mask;
            vector_mask_type merged_mask;
            size_t loop = 0;
            size_t vector_match_iterations_needed =
                ((database->dshufti_database.pair_count - 1) /
                 (VECTORSIZE / 2));
            for (; loop <= vector_match_iterations_needed; loop++) {
                all_pair = SuperVector<VECTORSIZE>::load(
                    database->dshufti_database.all_pairs + (VECTORSIZE * loop));
                // It is fine if the vector isn't filled as we are guaranteed to
                // have a match before reaching the garbage data
                mask = all_pair.eqmask(found_pair);
                // now we have <width> bit set to 1 when a char match.
                // first we merge the lane result to keep only consecutive
                // matches
                merged_mask = mask & (mask >> width);
                // Then we filter to keep only a single bit per lane, and only
                // every other lane
                merged_mask =
                    merged_mask & database->dshufti_database.bit_filter_mask;
                if (merged_mask)
                    break;
            }
            // And finaly we can ctz to get the first pair that match
            unsigned int id =
                (ctz64(merged_mask) / width / 2) + (loop * (VECTORSIZE / 2));
            size_t match_start = current_match - reinterpret_cast<const u8*>(data);
            if (!onEvent(id, match_start, match_start + 2, 0, context)) {
                // user requested to stop matching
                break;
            }
        }
        current_buf = current_match + 1;
    }

    return HS_SUCCESS;
}
