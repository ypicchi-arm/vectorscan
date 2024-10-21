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

#include "hs_common.h"
#include "hs_compile.h"
#include "hs_runtime.h"
#include "scratch.h"

#include "hwlm/hwlm.h"
#include "hwlm/hwlm_literal.h"
#include "hwlm/noodle_build.h"
#include "hwlm/noodle_engine.h"
#include "hwlm/noodle_internal.h"

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

// Some algorithms don't use the scratch at all so we can save on memory
typedef struct scratchless_call_ctx {
    void *ctx_ptr;
    match_event_handler usr_cb;
} hs_scratchless_call_ctx_t;

// --- short_literal ---
static_assert(sizeof(hs_short_literal_compiled_pattern_t) >= sizeof(noodTable),
              "Short_literal_compiled_pattern_t is too small to fit the "
              "underlying type's data");

hwlmcb_rv_t HS_CDECL noodle_to_hs_callback(size_t end, u32 id,
                                  struct hs_scratch *scratch) {
    hs_scratchless_call_ctx_t *light_scratch =
        reinterpret_cast<hs_scratchless_call_ctx_t *>(scratch);
    return (hwlmcb_rv_t)(light_scratch->usr_cb(id, 0, end, 0,
                                               light_scratch->ctx_ptr));
}

HS_PUBLIC_API
hs_error_t HS_CDECL hs_compile_short_literal_search(
    const char *expression, size_t expression_length,
    hs_short_literal_compiled_pattern_t *output_database) {
    if (expression_length > HS_SHORT_PATTERN_THRESHOLD) {
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

    ue2::bytecode_ptr<noodTable> table = ue2::noodBuildTable(lit);

    if (table) {
        *reinterpret_cast<noodTable *>(output_database) = *(table.get());
        return HS_SUCCESS;
    } else {
        return HS_UNKNOWN_ERROR;
    }
}

HS_PUBLIC_API
hs_error_t HS_CDECL hs_short_literal_search(
                        const hs_short_literal_compiled_pattern_t *database,
                        const char *data, size_t length,
                        struct hs_scratch *scratch, match_event_handler onEvent,
                        void *context) {
    (void)scratch;

    hs_scratchless_call_ctx_t noodle_context;
    noodle_context.ctx_ptr = context;
    noodle_context.usr_cb = onEvent;
    /*
     * although noodle require a scratch, it never actually use it and just
     * pass it down to the callback, so we can pass our context instead
     */
    hwlm_error_t error = noodExec(reinterpret_cast<const noodTable *>(database),
                                  reinterpret_cast<const uint8_t *>(data),
                                  length, 0, noodle_to_hs_callback,
                                  reinterpret_cast<hs_scratch *>(&noodle_context));
    // TODO The above is a hack. We need a clean solution like changing noodle's
    //  prototype, or finding where in hs_scratch we can add user data. (I
    //  didn't find it, but I'm sure there's some place for it)
    return hwlm_to_hs_error(error);
}

/**
 * Some useful algorithm to implement the API:
 * noodle: find a pair of chars and then do a vect compare to find up to 16
 *      char long patterns
 * fdr: find a matching string of any size. The first stage check up to 8 char
 *      long patterns, then run a slower search on the result. fdr is optimized
 *      to match against many string at once
 * shufti: the simple version seems similar to truffle. find char of a charset.
 *      There's limitation on the charset (up to 8 different (char%16))
 *      The double version is looking for a set of two characters following. If
 *      we have "ab" and "cd", does it match for "ad"? No. It reject ad. So
 *      this could be used to make a generalized noodle that can match more
 *      than one pattern
 * truffle: find a char among charset
 */