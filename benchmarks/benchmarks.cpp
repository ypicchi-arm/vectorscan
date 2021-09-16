#include <iostream>
#include <chrono>
#include <cstring>
#include <ctime>
#include <cstdlib>
#include <memory>

#include "nfa/shufti.h"
#include "nfa/shufticompile.h"
#include "nfa/truffle.h"
#include "nfa/trufflecompile.h"
#include "benchmarks.hpp"

#define MAX_LOOPS    500000000
#define MAX_MATCHES  10

/*
void shuffle_init(){
    m128 lo, hi;
    ue2::CharReach chars;
    chars.set('a');
    shuftiBuildMasks(chars, (u8 *)&lo, (u8 *)&hi);
    std::unique_ptr<u8 []> kt1 ( new u8[size] );
    memset(kt1.get(),'b',size);  
}
*/

/*
void truffle_init(){
     m128 lo, hi;
    ue2::CharReach chars;
    chars.set('a');
    truffleBuildMasks(chars, (u8 *)&lo, (u8 *)&hi);
    std::unique_ptr<u8 []> kt1 ( new u8[size] );
    memset(kt1.get(),'b',size); 
}
*/

/*
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

void noodle_init(){
    ctxt.clear();
    std::unique_ptr<u8 []> data ( new u8[size] );
    memset(data.get(), 'a', size);
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
}
*/

void run_benchmarks(int size, int loops, int M, bool has_match, std::function <const u8 *(m128, m128, const u8 *, const u8 *)> function) {
    m128 lo, hi;
    ue2::CharReach chars;
    chars.set('a');
    shuftiBuildMasks(chars, (u8 *)&lo, (u8 *)&hi);
    std::unique_ptr<u8 []> kt1 ( new u8[size] );
    memset(kt1.get(),'b',size);
    double total_sec = 0.0;            
    u64a transferred_size = 0;
    double bandwidth = 0.0;
    double max_bw = 0.0;
    double avg_time = 0.0;
    if (has_match) {
        int pos = 0;
        for(int j = 0; j < M; j++) {
            kt1[pos] = 'b';
            pos = (j*size) / M ;
            kt1[pos] = 'a';
            unsigned long act_size = 0;
            auto start = std::chrono::steady_clock::now();
            for(int i = 0; i < loops; i++) { 
                const u8 *res = function(lo, hi, kt1.get(), kt1.get() + size);
                act_size += res - kt1.get();
            }
            auto end = std::chrono::steady_clock::now();
            double dt = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
            total_sec += dt;
            /*convert microseconds to seconds*/
            total_sec /= 1000000.0;
            /*calculate bandwidth*/
            bandwidth  += (act_size / dt) * 1000000.0;
            /*convert to MB/s*/
            bandwidth  = bandwidth  / 1048576.0;
            max_bw = std::max(bandwidth ,max_bw);
            /*calculate average time*/
            avg_time += total_sec / loops;
        }
        avg_time /= M;
        bandwidth /= M;
        /*convert average time to us*/
        avg_time *= 1000000.0;
        printf(KMAG "case with %u matches, %u * %u iterations," KBLU " total elapsed time =" RST " %.3f s, " 
               KBLU "average time per call =" RST " %.3f μs," KBLU " max bandwidth = " RST " %.3f MB/s," KBLU " average bandwidth =" RST " %.3f MB/s \n",
               M, size ,loops, total_sec, avg_time, max_bw, bandwidth);
    } else {
        auto start = std::chrono::steady_clock::now();
        for (int i = 0; i < loops; i++) {
            function(lo, hi, kt1.get(), kt1.get() + size);
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
        max_bw /= 1048576.0;
        printf(KMAG "case without matches, %u * %u iterations," KBLU " total elapsed time =" RST " %.3f s, " 
               KBLU "average time per call =" RST " %.3f μs ," KBLU " bandwidth = " RST " %.3f MB/s \n",
               size ,loops, total_sec, avg_time, max_bw);
    }
}


int main(){
    std::function <const u8 *(m128, m128, const u8 *, const u8 *)> functions[] = {shuftiExec, rshuftiExec, truffleExec, rtruffleExec};
    int sizes[] =  {  16000,  32000,  64000, 120000, 1600000, 2000000, 2500000, 3500000, 150000000, 250000000, 350000000, 500000000 };
    std::string labels[] = {"\x1B[33m shuftiExec Benchmarks \x1B[0m\n", "\x1B[33m rshuftiExec Benchmarks \x1B[0m\n",
                            "\x1B[33m triffleExec Benchmarks \x1B[0m\n", "\x1B[33m triffleExec Benchmarks \x1B[0m\n"};
    const char charset[] = "aAaAaAaAAAaaaaAAAAaaaaAAAAAAaaaAAaaa"; 
    
    for (size_t i = 0; i < std::size(sizes); i++) {
        for(size_t j = 0; j < std::size(functions); j++) {
            std::cout << labels[j];
            run_benchmarks(sizes[i], MAX_LOOPS / sizes[i], MAX_MATCHES, false, functions[j]);
            run_benchmarks(sizes[i], MAX_LOOPS / sizes[i], MAX_MATCHES, true, functions[j]);
        } 
    }
    
    for(size_t i=0; i < std::size(sizes); i++){
        //we imitate the noodle unit tests
        for (int char_len = 1; char_len < 9; char_len++) {
            std::unique_ptr<char []> str ( new char[char_len] );
            for (int j=0; j<char_len; j++) {
                srand (time(NULL));
                int key = rand() % + 36 ;
                str[char_len] = charset[key];
                str[char_len + 1] = '\0';
            }
            noodle_benchmarks(sizes[i],  MAX_LOOPS / sizes[i], str.get(), char_len, 0);  
        }
    }
    return 0;
}