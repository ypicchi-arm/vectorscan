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

void noodle_benchmarks(int size, int loops, const char *lit_str, int lit_len, char nocase){
    ctxt.clear();
    u8 *data = new u8[size];
    memset(data, 'a', size);
    double total_sec = 0.0;
    u64a transferred_size = 0;
    double avg_time = 0.0;
    double max_bw = 0.0;
    double bandwitdh = 0.0;
    u32 id = 1000;
    ue2::hwlmLiteral lit(std::string(lit_str, lit_len), nocase, id);
    auto n = ue2::noodBuildTable(lit);
    assert(n != nullptr);
    struct hs_scratch scratch;
    auto start = std::chrono::steady_clock::now(); 
    for (int i = 0; i < loops; i++){ 
        noodExec(n.get(), data, size, 0, hlmSimpleCallback, &scratch); 
    }
    auto end = std::chrono::steady_clock::now();
    total_sec += std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    /*calculate transferred size*/
    transferred_size = size * loops;
    /*calculate average time*/
    avg_time = total_sec / loops;
    /*convert microseconds to seconds*/
    total_sec /= 1000000.0;
    /*calculate maximum bandwidth*/
    max_bw = transferred_size / total_sec;
    /*convert to MB/s*/
    max_bw /=1048576.0;
    /*calculate average bandwidth*/
    bandwitdh = max_bw / loops;
    printf(KMAG "Case with %u matches in random pos with %u * %u iterations," KBLU " total elapsed time =" RST " %.3f s, " 
               KBLU "average time per call =" RST " %.3f μs," KBLU " bandwidth = " RST " %.3f MB/s," KBLU " average bandwidth =" RST " %.3f MB/s \n",
               lit_len, size ,loops, total_sec, avg_time, max_bw, bandwitdh);    
    delete [] data;
}