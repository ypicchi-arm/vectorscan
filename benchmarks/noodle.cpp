#include <iostream>
#include "ue2common.h"
#include "benchmarks.hpp"
#include "hwlm/noodle_build.h"
#include "hwlm/noodle_engine.h"
#include "hwlm/hwlm.h"
#include "hwlm/hwlm_literal.h"
#include "scratch.h"
#include <vector>
#include <chrono>


struct hlmMatchEntry {
    size_t to;
    u32 id;
    hlmMatchEntry(size_t end, u32 identifier) :
            to(end), id(identifier) {}
};

std::vector<hlmMatchEntry> ctxt;

static
hwlmcb_rv_t hlmSimpleCallback(size_t to, u32 id,
                              UNUSED struct hs_scratch *scratch) {
    DEBUG_PRINTF("match @%zu = %u\n", to, id);

    ctxt.push_back(hlmMatchEntry(to, id));

    return HWLM_CONTINUE_MATCHING;
}

void noodle_benchmarks(int size, int M, const char *lit_str, int lit_len, char nocase){
    ctxt.clear();
    u8 *data = new u8[size];
    memset(data, 'a', size);
    long double total_sec = 0;
    long double trans_size = 0;
    long double bw = 0;
    u32 id = 1000;
    ue2::hwlmLiteral lit(std::string(lit_str, lit_len), nocase, id);
    auto n = ue2::noodBuildTable(lit);
    assert(n != nullptr);
    struct hs_scratch scratch;
    auto start = std::chrono::steady_clock::now(); 
    for (int i = 0; i < M; i++){ 
        noodExec(n.get(), data, size, 0, hlmSimpleCallback, &scratch); 
    }
    auto end = std::chrono::steady_clock::now();
    total_sec += std::chrono::duration_cast<std::chrono::seconds>(end - start).count();
    /*calculate transferred size*/
    trans_size = size * M;
    /*convert to sec*/
    bw = trans_size / total_sec;
    /*convert to MB/s*/
    bw /=1048576;
    /*covert average time to μs*/
    long double ms = total_sec * 1000000;
    std::cout << "\x1B[35m Case with match in random pos and size: "<< size <<" lit_len: "<< lit_len <<" nocase: "<< (int)nocase
              << "\x1B[36m noodExec elapsetime: \x1B[0m" << (ms/M) << " (μs) \x1B[36m bandwidth: \x1B[0m" << bw <<" (MB/s)" << std::endl;    
    delete [] data;
}