#include "nfa/shufti.h"
#include "nfa/shufticompile.h"
#include "benchmarks.hpp"
#include <iostream>
#include <chrono>
#include <cstring>
#include <ctime>
#include <cstdlib>

/*
#define RST  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"
*/


void shufti_benchmarks(int size, int loops, int M, bool has_match) { 
    m128 lo, hi;
    ue2::CharReach chars;
    chars.set('a');
    int ret = shuftiBuildMasks(chars, (u8 *)&lo, (u8 *)&hi);
    u8 *kt1 = new u8[size];
    memset(kt1,'b',size);
    double total_sec = 0;
    double bw = 0;
    if (has_match){
        int pos = 0;
        for(int j=0; j<M; j++){
            kt1[pos] = 'b';
            srand (time(NULL));
            pos = rand() % size + 0;
            kt1[pos] = 'a';
            unsigned long act_size = 0;
            auto start = std::chrono::steady_clock::now();
            for (int i = 0; i < loops; i++) {
                const u8 *res = shuftiExec(lo, hi, kt1, kt1 + size);
                act_size += res - kt1;
            }
            auto end = std::chrono::steady_clock::now();
            double dt = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
            /*average time*/
            dt /= loops;
            total_sec += dt;
            /*average size*/
            act_size /= loops;
            double mb_size = (double) act_size / 1048576;
            bw += mb_size / dt;
        }
        total_sec /= M;
        bw /= M;
        std::cout << "\x1B[35m Case with match in random pos and size: " << size << " for " << loops <<" loops ("
                  << M << " random possisions checked): \x1B[36m shuftiExec elapsetime: \x1B[0m" << (total_sec)
                  << " (μs) \x1B[36m bandwidth: \x1B[0m" << bw <<" (MB/μs)"<<std::endl;
    } else {
        auto start = std::chrono::steady_clock::now();
        for (int i = 0; i < loops; i++) {
            shuftiExec(lo, hi, kt1, kt1 + size);
        }
        auto end = std::chrono::steady_clock::now();
        total_sec += std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        /*average time*/
        total_sec /= loops;
        /*average size*/
        size /= loops;
        double mb_size = (double) size / 1048576;
        mb_size /= loops;
        bw = mb_size / total_sec;
        std::cout<<"\x1B[35m Case with no match in random pos and size: "<<size<<" for "<<loops<<" loops:"
                 <<"\x1B[36m shuftiExec elapsetime: \x1B[0m"<<(total_sec)<<" (μs) \x1B[36m bandwidth: \x1B[0m"<< bw <<" (MB/μs)"<<std::endl;
    }
    delete [] kt1;
}

void rshufti_benchmarks(int size, int loops, int M, bool has_match) {
    m128 lo, hi;
    ue2::CharReach chars;
    chars.set('a');
    int ret = shuftiBuildMasks(chars, (u8 *)&lo, (u8 *)&hi);
    u8 *kt1 = new u8[size];
    memset(kt1,'b',size);
    double total_sec = 0;
    long double bw = 0;
    if (has_match){
        int pos = 0;
        for(int j=0; j<M; j++){
            kt1[pos] = 'b';
            srand (time(NULL));
            pos = rand() % size + 0;
            kt1[pos] = 'a';
            unsigned long act_size = 0;
            auto start = std::chrono::steady_clock::now();
            for (int i = 0; i < loops; i++) {
                const u8 *res = rshuftiExec(lo, hi, kt1, kt1 + size);
                act_size += res - kt1; 
            }
            auto end = std::chrono::steady_clock::now();
            double dt = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
            /*average time*/
            dt /= loops;
            total_sec += dt;
            /*average size*/
            act_size /= loops;
            double mb_size = (double) act_size / 1048576;
            bw += mb_size / dt;
        }
        total_sec /= M;
        bw /= M;
        std::cout << "\x1B[35m Case with match in random pos and size: " << size << " for " << loops<<" loops ("
                  << M << " random possisions checked): \x1B[36m rshuftiExec elapsetime: \x1B[0m" << total_sec
                  <<" (μs) \x1B[36m bandwidth: \x1B[0m" << bw <<" (MB/μs)"<<std::endl;
    } else {
        auto start = std::chrono::steady_clock::now();
        for (int i = 0; i < loops; i++) {
            rshuftiExec(lo, hi, kt1, kt1 + size);
        }
        auto end = std::chrono::steady_clock::now();
        total_sec += std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        /*average time*/
        total_sec /=loops;
        /*average size*/
        size /=loops;
        double mb_size = (double) size / 1048576;
        bw = mb_size / total_sec;
        std::cout<<"\x1B[35m Case with no match in random pos and size: "<< size <<" for "<< loops <<" loops:"
                 <<"\x1B[36m rshuftiExec elapsetime: \x1B[0m"<< total_sec <<" (μs) \x1B[36m bandwidth: \x1B[0m"<< bw <<" (MB/μs)"<<std::endl;
    }
    delete [] kt1;
}
