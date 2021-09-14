#include "nfa/truffle.h"
#include "nfa/trufflecompile.h"
#include "benchmarks.hpp"
#include <iostream>
#include <chrono>
#include <cstring>
#include <ctime>

void truffle_benchmarks(int size, int loops, int M, bool has_match) {
    m128 lo, hi;
    ue2::CharReach chars;
    chars.set('a');
    truffleBuildMasks(chars, (u8 *)&lo, (u8 *)&hi);
    u8 *kt1 = new u8[size];
    memset(kt1,'b',size);
    double total_sec = 0.0;            
    u64a transferred_size = 0;
    double bandwitdh = 0.0;
    double max_bw = 0.0;
    double avg_time = 0.0;
    if (has_match) {
        int pos = 0;
        for(int j = 0; j < M; j++) {
            kt1[pos] = 'b';
            srand (time(NULL));
            pos = rand() % size + 0;
            kt1[pos] = 'a';
            unsigned long act_size = 0;
            auto start = std::chrono::steady_clock::now();
            for(int i = 0; i < loops; i++) {
                const u8 *res = truffleExec(lo, hi, kt1, kt1 + size);
                act_size += res - kt1;
            }
            auto end = std::chrono::steady_clock::now();
            double dt = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
            total_sec += dt;
            /*convert microseconds to seconds*/
            total_sec /= 1000000.0;
            /*calculate bandwidth*/
            bandwitdh  += act_size / total_sec;
            /*convert to MB/s*/
            bandwitdh  = bandwitdh  / 1048576.0;
            max_bw = std::max(bandwitdh ,max_bw);
            /*calculate average time*/
            avg_time += total_sec / loops;
        }
        avg_time /= M;
        bandwitdh /= M;
        /*convert average time to us*/
        avg_time *= 1000000.0;
        printf(KMAG "TruffleExec: case with %u matches, %u * %u iterations," KBLU " total elapsed time =" RST " %.3f s, " 
               KBLU "average time per call =" RST " %.3f μs," KBLU " bandwidth = " RST " %.3f MB/s," KBLU " average bandwidth =" RST " %.3f MB/s \n",
               M, size ,loops, total_sec, avg_time, max_bw, bandwitdh);
    } else {
        auto start = std::chrono::steady_clock::now();
        for (int i = 0; i < loops; i++) {
            truffleExec(lo, hi, kt1, kt1 + size);
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
        /*calculate average bandwidth*/
        bandwitdh = max_bw / loops;
        printf(KMAG "TruffleExec case without matches, %u * %u iterations," KBLU " total elapsed time =" RST " %.3f s, " 
               KBLU "average time per call =" RST " %.3f μs ," KBLU " bandwidth = " RST " %.3f MB/s," KBLU " average bandwidth =" RST " %.3f MB/s \n",
               size ,loops, total_sec, avg_time, max_bw, bandwitdh);
    }
    delete [] kt1;
}


void rtruffle_benchmarks(int size, int loops, int M, bool has_match) {  
     m128 lo, hi;
    ue2::CharReach chars;
    chars.set('a');
    truffleBuildMasks(chars, (u8 *)&lo, (u8 *)&hi);
    u8 *kt1 = new u8[size];
    memset(kt1,'b',size);
    double total_sec = 0.0;            
    u64a transferred_size = 0;
    double bandwitdh = 0.0;
    double max_bw = 0.0;
    double avg_time = 0.0;
    if (has_match) {
        int pos = 0;
        for(int j = 0; j < M; j++) {
            kt1[pos] = 'b';
            srand (time(NULL));
            pos = rand() % size + 0;
            kt1[pos] = 'a';
            unsigned long act_size = 0;
            auto start = std::chrono::steady_clock::now();
            for(int i = 0; i < loops; i++) {
                const u8 *res = rtruffleExec(lo, hi, kt1, kt1 + size);
                act_size += res - kt1;
            }
            auto end = std::chrono::steady_clock::now();
            double dt = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
            total_sec += dt;
            /*convert microseconds to seconds*/
            total_sec /= 1000000.0;
            /*calculate bandwidth*/
            bandwitdh  += act_size / total_sec;
            /*convert to MB/s*/
            bandwitdh  = bandwitdh  / 1048576.0;
            max_bw = std::max(bandwitdh ,max_bw);
            /*calculate average time*/
            avg_time += total_sec / loops;
        }
        avg_time /= M;
        bandwitdh /= M;
        /*convert average time to us*/
        avg_time *= 1000000.0;
        printf(KMAG "rTruffleExec: case with %u matches, %u * %u iterations," KBLU " total elapsed time =" RST " %.3f s, " 
               KBLU "average time per call =" RST " %.3f μs," KBLU " bandwidth = " RST " %.3f MB/s," KBLU " average bandwidth =" RST " %.3f MB/s \n",
               M, size ,loops, total_sec, avg_time, max_bw, bandwitdh);
    } else {
        auto start = std::chrono::steady_clock::now();
        for (int i = 0; i < loops; i++) {
            rtruffleExec(lo, hi, kt1, kt1 + size);
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
        /*calculate average bandwidth*/
        bandwitdh = max_bw / loops;
        printf(KMAG "rTruffleExec case without matches, %u * %u iterations," KBLU " total elapsed time =" RST " %.3f s, " 
               KBLU "average time per call =" RST " %.3f μs ," KBLU " bandwidth = " RST " %.3f MB/s," KBLU " average bandwidth =" RST " %.3f MB/s \n",
               size ,loops, total_sec, avg_time, max_bw, bandwitdh);
    }
    delete [] kt1;
}
