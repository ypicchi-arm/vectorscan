/*
 * Copyright (c) 2020, 2021, VectorCamp PC
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

#include <iostream>
#include <chrono>
#include <cstring>
#include <ctime>
#include <cstdlib>
#include <memory>
#include <functional>

#include "benchmarks.hpp"

#define MAX_LOOPS    1000000000
#define MAX_MATCHES  5
#define N            8

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

template<typename InitFunc, typename BenchFunc>
static void run_benchmarks(int size, int loops, int max_matches, bool is_reverse, MicroBenchmark &bench, InitFunc &&init, BenchFunc &&func) {
    init(bench);
    double total_sec = 0.0;            
    u64a total_size = 0;
    double bw = 0.0;
    double avg_bw = 0.0;
    double max_bw = 0.0;
    double avg_time = 0.0;
    if (max_matches) {
        int pos = 0;
        for(int j = 0; j < max_matches - 1; j++) {
            bench.buf[pos] = 'b';
            pos = (j+1) *size / max_matches ;
            bench.buf[pos] = 'a';
            u64a actual_size = 0;
            auto start = std::chrono::steady_clock::now();
            for(int i = 0; i < loops; i++) { 
                const u8 *res = func(bench);
		if (is_reverse)
		   actual_size += bench.buf.data() + size - res;
		else
                   actual_size += res - bench.buf.data();
            }
            auto end = std::chrono::steady_clock::now();
            double dt = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
            total_sec += dt;
            /*convert microseconds to seconds*/
            /*calculate bandwidth*/
            bw  = (actual_size / dt) * 1000000.0 / 1048576.0;
	    /*std::cout << "act_size = " << act_size << std::endl;
	    std::cout << "dt = " << dt << std::endl;
	    std::cout << "bw = " << bw << std::endl;*/
	    avg_bw += bw;
            /*convert to MB/s*/
            max_bw = std::max(bw, max_bw);
            /*calculate average time*/
            avg_time += total_sec / loops;
        }
        avg_time /= max_matches;
        avg_bw /= max_matches;
	total_sec /= 1000000.0;
        /*convert average time to us*/
        printf(KMAG "%s: %u matches, %u * %u iterations," KBLU " total elapsed time =" RST " %.3f s, " 
               KBLU "average time per call =" RST " %.3f μs," KBLU " max bandwidth = " RST " %.3f MB/s," KBLU " average bandwidth =" RST " %.3f MB/s \n",
               bench.label, max_matches, size ,loops, total_sec, avg_time, max_bw, avg_bw);
    } else {
        auto start = std::chrono::steady_clock::now();
        for (int i = 0; i < loops; i++) {
            const u8 *res = func(bench);
        }
        auto end = std::chrono::steady_clock::now();
        total_sec += std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        /*calculate transferred size*/
        total_size = size * loops;
        /*calculate average time*/
        avg_time = total_sec / loops;
        /*convert microseconds to seconds*/
        total_sec /= 1000000.0;
        /*calculate maximum bandwidth*/
        max_bw = total_size / total_sec;
        /*convert to MB/s*/
        max_bw /= 1048576.0;
        printf(KMAG "%s: no matches, %u * %u iterations," KBLU " total elapsed time =" RST " %.3f s, " 
               KBLU "average time per call =" RST " %.3f μs ," KBLU " bandwidth = " RST " %.3f MB/s \n",
               bench.label, size ,loops, total_sec, avg_time, max_bw );
    }
}



/*
(gdb) p cc
$15 = (const ue2::CompileContext &) @0xfffff18577d8: {streaming = false, vectored = false, target_info = {tune = 0, cpu_features = 0}, 
  grey = {optimiseComponentTree = true, calcComponents = true, performGraphSimplification = true, prefilterReductions = true, 
    removeEdgeRedundancy = true, allowGough = true, allowHaigLit = true, allowLitHaig = true, allowLbr = true, allowMcClellan = true, 
    allowSheng = true, allowMcSheng = true, allowPuff = true, allowLiteral = true, allowViolet = true, allowExtendedNFA = true, 
    allowLimExNFA = true, allowAnchoredAcyclic = true, allowSmallLiteralSet = true, allowCastle = true, allowDecoratedLiteral = true, 
    allowApproximateMatching = true, allowNoodle = true, fdrAllowTeddy = true, fdrAllowFlood = true, violetAvoidSuffixes = 1, 
    violetAvoidWeakInfixes = true, violetDoubleCut = true, violetExtractStrongLiterals = true, violetLiteralChains = true, 
    violetDoubleCutLiteralLen = 3, violetEarlyCleanLiteralLen = 6, puffImproveHead = true, castleExclusive = true, mergeSEP = true, 
    mergeRose = true, mergeSuffixes = true, mergeOutfixes = true, onlyOneOutfix = false, allowShermanStates = true, 
    allowMcClellan8 = true, allowWideStates = true, highlanderPruneDFA = true, minimizeDFA = true, accelerateDFA = true, 
    accelerateNFA = true, reverseAccelerate = true, squashNFA = true, compressNFAState = true, numberNFAStatesWrong = false, 
    highlanderSquash = true, allowZombies = true, floodAsPuffette = false, nfaForceSize = 0, maxHistoryAvailable = 110, 
    minHistoryAvailable = 0, maxAnchoredRegion = 63, minRoseLiteralLength = 3, minRoseNetflowLiteralLength = 2, 
    maxRoseNetflowEdges = 50000, maxEditDistance = 16, minExtBoundedRepeatSize = 32, goughCopyPropagate = true, 
    goughRegisterAllocate = true, shortcutLiterals = true, roseGraphReduction = true, roseRoleAliasing = true, roseMasks = true, 
    roseConvertFloodProneSuffixes = true, roseMergeRosesDuringAliasing = true, roseMultiTopRoses = true, roseHamsterMasks = true, 
    roseLookaroundMasks = true, roseMcClellanPrefix = 1, roseMcClellanSuffix = 1, roseMcClellanOutfix = 2, roseTransformDelay = true, 
    earlyMcClellanPrefix = true, earlyMcClellanInfix = true, earlyMcClellanSuffix = true, allowCountingMiracles = true, 
    allowSomChain = true, somMaxRevNfaLength = 126, hamsterAccelForward = true, hamsterAccelReverse = false, miracleHistoryBonus = 16, 
    equivalenceEnable = true, allowSmallWrite = true, allowSmallWriteSheng = false, smallWriteLargestBuffer = 70, 
    smallWriteLargestBufferBad = 35, limitSmallWriteOutfixSize = 1048576, smallWriteMaxPatterns = 10000, smallWriteMaxLiterals = 10000, 
    smallWriteMergeBatchSize = 20, allowTamarama = true, tamaChunkSize = 100, dumpFlags = 0, dumpPath = "", limitPatternCount = 8000000, 
    limitPatternLength = 16000, limitGraphVertices = 500000, limitGraphEdges = 1000000, limitReportCount = 32000000, 
    limitLiteralCount = 8000000, limitLiteralLength = 16000, limitLiteralMatcherChars = 1073741824, limitLiteralMatcherSize = 1073741824, 
    limitRoseRoleCount = 32000000, limitRoseEngineCount = 8000000, limitRoseAnchoredSize = 1073741824, limitEngineSize = 1073741824, 
    limitDFASize = 1073741824, limitNFASize = 1048576, limitLBRSize = 1048576, limitApproxMatchingVertices = 5000}}

(gdb) p raw
$17 = (ue2::raw_dfa &) @0xaaaab54a6480: {_vptr.raw_dfa = 0xfffff7947b80 <vtable for ue2::raw_dfa+16>, kind = ue2::NFA_OUTFIX, 
  states = std::vector of length 3, capacity 3 = {{next = std::vector of length 3, capacity 3 = {0, 0, 0}, daddy = 0, impl_id = 0, 
      reports = {<ue2::flat_detail::flat_base<unsigned int, std::less<unsigned int>, std::allocator<unsigned int> >> = {
          storage = std::tuple containing = {
            [0] = {<boost::container::small_vector_base<unsigned int, std::allocator<unsigned int>, void>> = {<boost::container::vector<unsigned int, boost::container::small_vector_allocator<unsigned int, std::allocator<void>, void>, void>> = {
                  m_holder = {<boost::container::small_vector_allocator<unsigned int, std::allocator<void>, void>> = {<std::allocator<unsigned int>> = {<__gnu_cxx::new_allocator<unsigned int>> = {<No data fields>}, <No data fields>}, <No data fields>}, m_start = 0xaaaab528f118, 
                    m_size = 0, m_capacity = 2}}, static final_alignment = 4, m_storage_start = {aligner = {data = "\360\026)\265"}, 
                  data = "\360\026)\265"}}, <boost::container::small_vector_storage<boost::move_detail::aligned_struct_wrapper<4, 4>, 0>> = {<No data fields>}, static needed_extra_storages = <optimized out>, static needed_bytes = <optimized out>, 
              static header_bytes = <optimized out>, static s_start = <optimized out>, static static_capacity = <optimized out>}, 
            [1] = {<std::less<unsigned int>> = {<std::binary_function<unsigned int, unsigned int, bool>> = {<No data fields>}, <No data fields>}, <No data fields>}}}, <ue2::totally_ordered<ue2::flat_set<unsigned int, std::less<unsigned int>, std::allocator<unsigned int> > >> = {<No data fields>}, <No data fields>}, 
      reports_eod = {<ue2::flat_detail::flat_base<unsigned int, std::less<unsigned int>, std::allocator<unsigned int> >> = {
          storage = std::tuple containing = {
            [0] = {<boost::container::small_vector_base<unsigned int, std::allocator<unsigned int>, void>> = {<boost::container::vector<unsigned int, boost::container::small_vector_allocator<unsigned int, std::allocator<void>, void>, void>> = {
                  m_holder = {<boost::container::small_vector_allocator<unsigned int, std::allocator<void>, void>> = {<std::allocator<unsigned int>> = {<__gnu_cxx::new_allocator<unsigned int>> = {<No data fields>}, <No data fields>}, <No data fields>}, m_start = 0xaaaab528f138, 
                    m_size = 0, m_capacity = 2}}, static final_alignment = 4, m_storage_start = {aligner = {data = "\000\030)\265"}, 
                  data = "\000\030)\265"}}, <boost::container::small_vector_storage<boost::move_detail::aligned_struct_wrapper<4, 4>, 0>> = {<No data fields>}, static needed_extra_storages = <optimized out>, static needed_bytes = <optimized out>, 
              static header_bytes = <optimized out>, static s_start = <optimized out>, static static_capacity = <optimized out>}, 
            [1] = {<std::less<unsigned int>> = {<std::binary_function<unsigned int, unsigned int, bool>> = {<No data fields>}, <No data fields>}, <No data fields>}}}, <ue2::totally_ordered<ue2::flat_set<unsigned int, std::less<unsigned int>, std::allocator<unsigned int> > >> = {<No data fields>}, <No data fields>}},
	  {next = std::vector of length 3, capacity 3 = {2, 1, 1}, daddy = 0, impl_id = 1, 
      reports = {<ue2::flat_detail::flat_base<unsigned int, std::less<unsigned int>, std::allocator<unsigned int> >> = {
          storage = std::tuple containing = {
            [0] = {<boost::container::small_vector_base<unsigned int, std::allocator<unsigned int>, void>> = {<boost::container::vector<unsigned int, boost::container::small_vector_allocator<unsigned int, std::allocator<void>, void>, void>> = {
                  m_holder = {<boost::container::small_vector_allocator<unsigned int, std::allocator<void>, void>> = {<std::allocator<unsigned int>> = {<__gnu_cxx::new_allocator<unsigned int>> = {<No data fields>}, <No data fields>}, <No data fields>}, m_start = 0xaaaab528f178, 
                    m_size = 0, m_capacity = 2}}, static final_alignment = 4, m_storage_start = {aligner = {data = "\a\000\000"}, 
                  data = "\a\000\000"}}, <boost::container::small_vector_storage<boost::move_detail::aligned_struct_wrapper<4, 4>, 0>> = {<No data fields>}, static needed_extra_storages = <optimized out>, static needed_bytes = <optimized out>, 
              static header_bytes = <optimized out>, static s_start = <optimized out>, static static_capacity = <optimized out>}, 
            [1] = {<std::less<unsigned int>> = {<std::binary_function<unsigned int, unsigned int, bool>> = {<No data fields>}, <No data fields>}, <No data fields>}}}, <ue2::totally_ordered<ue2::flat_set<unsigned int, std::less<unsigned int>, std::allocator<unsigned int> > >> = {<No data fields>}, <No data fields>}, 
      reports_eod = {<ue2::flat_detail::flat_base<unsigned int, std::less<unsigned int>, std::allocator<unsigned int> >> = {
          storage = std::tuple containing = {
            [0] = {<boost::container::small_vector_base<unsigned int, std::allocator<unsigned int>, void>> = {<boost::container::vector<unsigned int, boost::container::small_vector_allocator<unsigned int, std::allocator<void>, void>, void>> = {
                  m_holder = {<boost::container::small_vector_allocator<unsigned int, std::allocator<void>, void>> = {<std::allocator<unsigned int>> = {<__gnu_cxx::new_allocator<unsigned int>> = {<No data fields>}, <No data fields>}, <No data fields>}, m_start = 0xaaaab528f198, 
                    m_size = 0, m_capacity = 2}}, static final_alignment = 4, m_storage_start = {aligner = {data = "\220\034)\265"}, 
                  data = "\220\034)\265"}}, <boost::container::small_vector_storage<boost::move_detail::aligned_struct_wrapper<4, 4>, 0>> = {<No data fields>}, static needed_extra_storages = <optimized out>, static needed_bytes = <optimized out>, 
              static header_bytes = <optimized out>, static s_start = <optimized out>, static static_capacity = <optimized out>}, 
            [1] = {<std::less<unsigned int>> = {<std::binary_function<unsigned int, unsigned int, bool>> = {<No data fields>}, <No data fields>}, <No data fields>}}}, <ue2::totally_ordered<ue2::flat_set<unsigned int, std::less<unsigned int>, std::allocator<unsigned int> > >> = {<No data fields>}, <No data fields>}},
	  {next = std::vector of length 3, capacity 3 = {0, 0, 2}, daddy = 1, impl_id = 2, 
      reports = {<ue2::flat_detail::flat_base<unsigned int, std::less<unsigned int>, std::allocator<unsigned int> >> = {
          storage = std::tuple containing = {
            [0] = {<boost::container::small_vector_base<unsigned int, std::allocator<unsigned int>, void>> = {<boost::container::vector<unsigned int, boost::container::small_vector_allocator<unsigned int, std::allocator<void>, void>, void>> = {
                  m_holder = {<boost::container::small_vector_allocator<unsigned int, std::allocator<void>, void>> = {<std::allocator<unsigned int>> = {<__gnu_cxx::new_allocator<unsigned int>> = {<No data fields>}, <No data fields>}, <No data fields>}, m_start = 0xaaaaaf5214f0, 
                    m_size = 3, m_capacity = 3}}, static final_alignment = 4, m_storage_start = {aligner = {data = "\a\000\000"}, 
                  data = "\a\000\000"}}, <boost::container::small_vector_storage<boost::move_detail::aligned_struct_wrapper<4, 4>, 0>> = {<No data fields>}, static needed_extra_storages = <optimized out>, static needed_bytes = <optimized out>, 
              static header_bytes = <optimized out>, static s_start = <optimized out>, static static_capacity = <optimized out>}, 
            [1] = {<std::less<unsigned int>> = {<std::binary_function<unsigned int, unsigned int, bool>> = {<No data fields>}, <No data fields>}, <No data fields>}}}, <ue2::totally_ordered<ue2::flat_set<unsigned int, std::less<unsigned int>, std::allocator<unsigned int> > >> = {<No data fields>}, <No data fields>}, 
      reports_eod = {<ue2::flat_detail::flat_base<unsigned int, std::less<unsigned int>, std::allocator<unsigned int> >> = {
          storage = std::tuple containing = {
            [0] = {<boost::container::small_vector_base<unsigned int, std::allocator<unsigned int>, void>> = {<boost::container::vector<unsigned int, boost::container::small_vector_allocator<unsigned int, std::allocator<void>, void>, void>> = {
                  m_holder = {<boost::container::small_vector_allocator<unsigned int, std::allocator<void>, void>> = {<std::allocator<unsigned int>> = {<__gnu_cxx::new_allocator<unsigned int>> = {<No data fields>}, <No data fields>}, <No data fields>}, m_start = 0xaaaab528f1f8, 
                    m_size = 0, m_capacity = 2}}, static final_alignment = 4, m_storage_start = {aligner = {data = "\004\000\000"}, 
                  data = "\004\000\000"}}, <boost::container::small_vector_storage<boost::move_detail::aligned_struct_wrapper<4, 4>, 0>> = {<No data fields>}, static needed_extra_storages = <optimized out>, static needed_bytes = <optimized out>, 
              static header_bytes = <optimized out>, static s_start = <optimized out>, static static_capacity = <optimized out>}, 
            [1] = {<std::less<unsigned int>> = {<std::binary_function<unsigned int, unsigned int, bool>> = {<No data fields>}, <No data fields>}, <No data fields>}}}, <ue2::totally_ordered<ue2::flat_set<unsigned int, std::less<unsigned int>, std::allocator<unsigned int> > >> = {<No data fields>}, <No data fields>}}},
  start_anchored = 1, start_floating = 1, alpha_size = 3, alpha_remap = {_M_elems = {1, 1, 1, 0, 1 <repeats 252 times>, 2}}}




$31 = (ue2::raw_dfa &) @0xaaaab57b3200: {_vptr.raw_dfa = 0xfffff7947b80 <vtable for ue2::raw_dfa+16>, kind = ue2::NFA_OUTFIX, 
  states = std::vector of length 8, capacity 8 = {
      {next = std::vector of length 4, capacity 4 = {0, 0, 0, 0}, daddy = 0, impl_id = 0, 
      reports = {<ue2::flat_detail::flat_base<unsigned int, std::less<unsigned int>, std::allocator<unsigned int> >> = {
          storage = std::tuple containing = {
            [0] = {<boost::container::small_vector_base<unsigned int, std::allocator<unsigned int>, void>> = {<boost::container::vector<unsigned int, boost::container::small_vector_allocator<unsigned int, std::allocator<void>, void>, void>> = {
                  m_holder = {<boost::container::small_vector_allocator<unsigned int, std::allocator<void>, void>> = {<std::allocator<unsigned int>> = {<__gnu_cxx::new_allocator<unsigned int>> = {<No data fields>}, <No data fields>}, <No data fields>}, m_start = 0xaaaab52a8c38, 
                    m_size = 0, m_capacity = 2}}, static final_alignment = 4, m_storage_start = {aligner = {data = "\004\000\000"}, 
                  data = "\004\000\000"}}, <boost::container::small_vector_storage<boost::move_detail::aligned_struct_wrapper<4, 4>, 0>> = {<No data fields>}, static needed_extra_storages = <optimized out>, static needed_bytes = <optimized out>, 
              static header_bytes = <optimized out>, static s_start = <optimized out>, static static_capacity = <optimized out>}, 
            [1] = {<std::less<unsigned int>> = {<std::binary_function<unsigned int, unsigned int, bool>> = {<No data fields>}, <No data fields>}, <No data fields>}}}, <ue2::totally_ordered<ue2::flat_set<unsigned int, std::less<unsigned int>, std::allocator<unsigned int> > >> = {<No data fields>}, <No data fields>}, 
      reports_eod = {<ue2::flat_detail::flat_base<unsigned int, std::less<unsigned int>, std::allocator<unsigned int> >> = {
          storage = std::tuple containing = {
            [0] = {<boost::container::small_vector_base<unsigned int, std::allocator<unsigned int>, void>> = {<boost::container::vector<unsigned int, boost::container::small_vector_allocator<unsigned int, std::allocator<void>, void>, void>> = {
                  m_holder = {<boost::container::small_vector_allocator<unsigned int, std::allocator<void>, void>> = {<std::allocator<unsigned int>> = {<__gnu_cxx::new_allocator<unsigned int>> = {<No data fields>}, <No data fields>}, <No data fields>}, m_start = 0xaaaab52a8c58, 
                    m_size = 0, m_capacity = 2}}, static final_alignment = 4, m_storage_start = {aligner = {data = "\001\000\000"}, 
                  data = "\001\000\000"}}, <boost::container::small_vector_storage<boost::move_detail::aligned_struct_wrapper<4, 4>, 0>> = {<No data fields>}, static needed_extra_storages = <optimized out>, static needed_bytes = <optimized out>, 
              static header_bytes = <optimized out>, static s_start = <optimized out>, static static_capacity = <optimized out>}, 
            [1] = {<std::less<unsigned int>> = {<std::binary_function<unsigned int, unsigned int, bool>> = {<No data fields>}, <No data fields>}, <No data fields>}}}, <ue2::totally_ordered<ue2::flat_set<unsigned int, std::less<unsigned int>, std::allocator<unsigned int> > >> = {<No data fields>}, <No data fields>}},
      {next = std::vector of length 4, capacity 4 = {2, 3, 1, 1}, daddy = 0, impl_id = 1, 
      reports = {<ue2::flat_detail::flat_base<unsigned int, std::less<unsigned int>, std::allocator<unsigned int> >> = {
          storage = std::tuple containing = {
            [0] = {<boost::container::small_vector_base<unsigned int, std::allocator<unsigned int>, void>> = {<boost::container::vector<unsigned int, boost::container::small_vector_allocator<unsigned int, std::allocator<void>, void>, void>> = {
                  m_holder = {<boost::container::small_vector_allocator<unsigned int, std::allocator<void>, void>> = {<std::allocator<unsigned int>> = {<__gnu_cxx::new_allocator<unsigned int>> = {<No data fields>}, <No data fields>}, <No data fields>}, m_start = 0xaaaab52a8c98, 
                    m_size = 0, m_capacity = 2}}, static final_alignment = 4, m_storage_start = {aligner = {data = "\006\000\000"}, 
                  data = "\006\000\000"}}, <boost::container::small_vector_storage<boost::move_detail::aligned_struct_wrapper<4, 4>, 0>> = {<No data fields>}, static needed_extra_storages = <optimized out>, static needed_bytes = <optimized out>, 
              static header_bytes = <optimized out>, static s_start = <optimized out>, static static_capacity = <optimized out>}, 
            [1] = {<std::less<unsigned int>> = {<std::binary_function<unsigned int, unsigned int, bool>> = {<No data fields>}, <No data fields>}, <No data fields>}}}, <ue2::totally_ordered<ue2::flat_set<unsigned int, std::less<unsigned int>, std::allocator<unsigned int> > >> = {<No data fields>}, <No data fields>}, 
      reports_eod = {<ue2::flat_detail::flat_base<unsigned int, std::less<unsigned int>, std::allocator<unsigned int> >> = {
          storage = std::tuple containing = {
            [0] = {<boost::container::small_vector_base<unsigned int, std::allocator<unsigned int>, void>> = {<boost::container::vector<unsigned int, boost::container::small_vector_allocator<unsigned int, std::allocator<void>, void>, void>> = {
                  m_holder = {<boost::container::small_vector_allocator<unsigned int, std::allocator<void>, void>> = {<std::allocator<unsigned int>> = {<__gnu_cxx::new_allocator<unsigned int>> = {<No data fields>}, <No data fields>}, <No data fields>}, m_start = 0xaaaab52a8cb8, 
                    m_size = 0, m_capacity = 2}}, static final_alignment = 4, m_storage_start = {aligner = {data = "\001\000\000"}, 
                  data = "\001\000\000"}}, <boost::container::small_vector_storage<boost::move_detail::aligned_struct_wrapper<4, 4>, 0>> = {<No data fields>}, static needed_extra_storages = <optimized out>, static needed_bytes = <optimized out>, 
              static header_bytes = <optimized out>, static s_start = <optimized out>, static static_capacity = <optimized out>}, 
            [1] = {<std::less<unsigned int>> = {<std::binary_function<unsigned int, unsigned int, bool>> = {<No data fields>}, <No data fields>}, <No data fields>}}}, <ue2::totally_ordered<ue2::flat_set<unsigned int, std::less<unsigned int>, std::allocator<unsigned int> > >> = {<No data fields>}, <No data fields>}},
      {next = std::vector of length 4, capacity 4 = {4, 5, 4, 2}, daddy = 1, impl_id = 2, 
      reports = {<ue2::flat_detail::flat_base<unsigned int, std::less<unsigned int>, std::allocator<unsigned int> >> = {
          storage = std::tuple containing = {
            [0] = {<boost::container::small_vector_base<unsigned int, std::allocator<unsigned int>, void>> = {<boost::container::vector<unsigned int, boost::container::small_vector_allocator<unsigned int, std::allocator<void>, void>, void>> = {
                  m_holder = {<boost::container::small_vector_allocator<unsigned int, std::allocator<void>, void>> = {<std::allocator<unsigned int>> = {<__gnu_cxx::new_allocator<unsigned int>> = {<No data fields>}, <No data fields>}, <No data fields>}, m_start = 0xaaaab52a8cf8, 
                    m_size = 2, m_capacity = 2}}, static final_alignment = 4, m_storage_start = {aligner = {data = "\205\000\000"}, 
                  data = "\205\000\000"}}, <boost::container::small_vector_storage<boost::move_detail::aligned_struct_wrapper<4, 4>, 0>> = {<No data fields>}, static needed_extra_storages = <optimized out>, static needed_bytes = <optimized out>, 
              static header_bytes = <optimized out>, static s_start = <optimized out>, static static_capacity = <optimized out>}, 
            [1] = {<std::less<unsigned int>> = {<std::binary_function<unsigned int, unsigned int, bool>> = {<No data fields>}, <No data fields>}, <No data fields>}}}, <ue2::totally_ordered<ue2::flat_set<unsigned int, std::less<unsigned int>, std::allocator<unsigned int> > >> = {<No data fields>}, <No data fields>}, 
      reports_eod = {<ue2::flat_detail::flat_base<unsigned int, std::less<unsigned int>, std::allocator<unsigned int> >> = {
          storage = std::tuple containing = {
            [0] = {<boost::container::small_vector_base<unsigned int, std::allocator<unsigned int>, void>> = {<boost::container::vector<unsigned int, boost::container::small_vector_allocator<unsigned int, std::allocator<void>, void>, void>> = {
                  m_holder = {<boost::container::small_vector_allocator<unsigned int, std::allocator<void>, void>> = {<std::allocator<unsigned int>> = {<__gnu_cxx::new_allocator<unsigned int>> = {<No data fields>}, <No data fields>}, <No data fields>}, m_start = 0xaaaab52a8d18, 
                    m_size = 0, m_capacity = 2}}, static final_alignment = 4, m_storage_start = {aligner = {data = "\001\000\000"}, 
                  data = "\001\000\000"}}, <boost::container::small_vector_storage<boost::move_detail::aligned_struct_wrapper<4, 4>, 0>> = {<No data fields>}, static needed_extra_storages = <optimized out>, static needed_bytes = <optimized out>, 
              static header_bytes = <optimized out>, static s_start = <optimized out>, static static_capacity = <optimized out>}, 
            [1] = {<std::less<unsigned int>> = {<std::binary_function<unsigned int, unsigned int, bool>> = {<No data fields>}, <No data fields>}, <No data fields>}}}, <ue2::totally_ordered<ue2::flat_set<unsigned int, std::less<unsigned int>, std::allocator<unsigned int> > >> = {<No data fields>}, <No data fields>}},
      {next = std::vector of length 4, capacity 4 = {6, 7, 7, 3}, daddy = 1, impl_id = 3, 
      reports = {<ue2::flat_detail::flat_base<unsigned int, std::less<unsigned int>, std::allocator<unsigned int> >> = {
          storage = std::tuple containing = {
            [0] = {<boost::container::small_vector_base<unsigned int, std::allocator<unsigned int>, void>> = {<boost::container::vector<unsigned int, boost::container::small_vector_allocator<unsigned int, std::allocator<void>, void>, void>> = {
                  m_holder = {<boost::container::small_vector_allocator<unsigned int, std::allocator<void>, void>> = {<std::allocator<unsigned int>> = {<__gnu_cxx::new_allocator<unsigned int>> = {<No data fields>}, <No data fields>}, <No data fields>}, m_start = 0xaaaab52a8d58, 
                    m_size = 1, m_capacity = 2}}, static final_alignment = 4, m_storage_start = {aligner = {data = "\003\000\000"}, 
                  data = "\003\000\000"}}, <boost::container::small_vector_storage<boost::move_detail::aligned_struct_wrapper<4, 4>, 0>> = {<No data fields>}, static needed_extra_storages = <optimized out>, static needed_bytes = <optimized out>, 
              static header_bytes = <optimized out>, static s_start = <optimized out>, static static_capacity = <optimized out>}, 
            [1] = {<std::less<unsigned int>> = {<std::binary_function<unsigned int, unsigned int, bool>> = {<No data fields>}, <No data fields>}, <No data fields>}}}, <ue2::totally_ordered<ue2::flat_set<unsigned int, std::less<unsigned int>, std::allocator<unsigned int> > >> = {<No data fields>}, <No data fields>}, 
      reports_eod = {<ue2::flat_detail::flat_base<unsigned int, std::less<unsigned int>, std::allocator<unsigned int> >> = {
          storage = std::tuple containing = {
            [0] = {<boost::container::small_vector_base<unsigned int, std::allocator<unsigned int>, void>> = {<boost::container::vector<unsigned int, boost::container::small_vector_allocator<unsigned int, std::allocator<void>, void>, void>> = {
                  m_holder = {<boost::container::small_vector_allocator<unsigned int, std::allocator<void>, void>> = {<std::allocator<unsigned int>> = {<__gnu_cxx::new_allocator<unsigned int>> = {<No data fields>}, <No data fields>}, <No data fields>}, m_start = 0xaaaab52a8d78, 
                    m_size = 0, m_capacity = 2}}, static final_alignment = 4, m_storage_start = {aligner = {data = "\001\000\000"}, 
                  data = "\001\000\000"}}, <boost::container::small_vector_storage<boost::move_detail::aligned_struct_wrapper<4, 4>, 0>> = {<No data fields>}, static needed_extra_storages = <optimized out>, static needed_bytes = <optimized out>, 
              static header_bytes = <optimized out>, static s_start = <optimized out>, static static_capacity = <optimized out>}, 
            [1] = {<std::less<unsigned int>> = {<std::binary_function<unsigned int, unsigned int, bool>> = {<No data fields>}, <No data fields>}, <No data fields>}}}, <ue2::totally_ordered<ue2::flat_set<unsigned int, std::less<unsigned int>, std::allocator<unsigned int> > >> = {<No data fields>}, <No data fields>}},
      {next = std::vector of length 4, capacity 4 = {4, 5, 4, 4}, daddy = 2, impl_id = 4, 
      reports = {<ue2::flat_detail::flat_base<unsigned int, std::less<unsigned int>, std::allocator<unsigned int> >> = {
          storage = std::tuple containing = {
            [0] = {<boost::container::small_vector_base<unsigned int, std::allocator<unsigned int>, void>> = {<boost::container::vector<unsigned int, boost::container::small_vector_allocator<unsigned int, std::allocator<void>, void>, void>> = {
                  m_holder = {<boost::container::small_vector_allocator<unsigned int, std::allocator<void>, void>> = {<std::allocator<unsigned int>> = {<__gnu_cxx::new_allocator<unsigned int>> = {<No data fields>}, <No data fields>}, <No data fields>}, m_start = 0xaaaab52a8db8, 
                    m_size = 0, m_capacity = 2}}, static final_alignment = 4, m_storage_start = {aligner = {data = "\f\000\000"}, 
                  data = "\f\000\000"}}, <boost::container::small_vector_storage<boost::move_detail::aligned_struct_wrapper<4, 4>, 0>> = {<No data fields>}, static needed_extra_storages = <optimized out>, static needed_bytes = <optimized out>, 
              static header_bytes = <optimized out>, static s_start = <optimized out>, static static_capacity = <optimized out>}, 
            [1] = {<std::less<unsigned int>> = {<std::binary_function<unsigned int, unsigned int, bool>> = {<No data fields>}, <No data fields>}, <No data fields>}}}, <ue2::totally_ordered<ue2::flat_set<unsigned int, std::less<unsigned int>, std::allocator<unsigned int> > >> = {<No data fields>}, <No data fields>}, 
      reports_eod = {<ue2::flat_detail::flat_base<unsigned int, std::less<unsigned int>, std::allocator<unsigned int> >> = {
          storage = std::tuple containing = {
            [0] = {<boost::container::small_vector_base<unsigned int, std::allocator<unsigned int>, void>> = {<boost::container::vector<unsigned int, boost::container::small_vector_allocator<unsigned int, std::allocator<void>, void>, void>> = {
                  m_holder = {<boost::container::small_vector_allocator<unsigned int, std::allocator<void>, void>> = {<std::allocator<unsigned int>> = {<__gnu_cxx::new_allocator<unsigned int>> = {<No data fields>}, <No data fields>}, <No data fields>}, m_start = 0xaaaab52a8dd8, 
                    m_size = 0, m_capacity = 2}}, static final_alignment = 4, m_storage_start = {aligner = {data = "\001\000\000"}, 
                  data = "\001\000\000"}}, <boost::container::small_vector_storage<boost::move_detail::aligned_struct_wrapper<4, 4>, 0>> = {<No data fields>}, static needed_extra_storages = <optimized out>, static needed_bytes = <optimized out>, 
              static header_bytes = <optimized out>, static s_start = <optimized out>, static static_capacity = <optimized out>}, 
            [1] = {<std::less<unsigned int>> = {<std::binary_function<unsigned int, unsigned int, bool>> = {<No data fields>}, <No data fields>}, <No data fields>}}}, <ue2::totally_ordered<ue2::flat_set<unsigned int, std::less<unsigned int>, std::allocator<unsigned int> > >> = {<No data fields>}, <No data fields>}},
      {next = std::vector of length 4, capacity 4 = {0, 0, 0, 5}, daddy = 2, impl_id = 5, 
      reports = {<ue2::flat_detail::flat_base<unsigned int, std::less<unsigned int>, std::allocator<unsigned int> >> = {
          storage = std::tuple containing = {
            [0] = {<boost::container::small_vector_base<unsigned int, std::allocator<unsigned int>, void>> = {<boost::container::vector<unsigned int, boost::container::small_vector_allocator<unsigned int, std::allocator<void>, void>, void>> = {
                  m_holder = {<boost::container::small_vector_allocator<unsigned int, std::allocator<void>, void>> = {<std::allocator<unsigned int>> = {<__gnu_cxx::new_allocator<unsigned int>> = {<No data fields>}, <No data fields>}, <No data fields>}, m_start = 0xaaaab52a8e18, 
                    m_size = 1, m_capacity = 2}}, static final_alignment = 4, m_storage_start = {aligner = {data = "\003\000\000"}, 
                  data = "\003\000\000"}}, <boost::container::small_vector_storage<boost::move_detail::aligned_struct_wrapper<4, 4>, 0>> = {<No data fields>}, static needed_extra_storages = <optimized out>, static needed_bytes = <optimized out>, 
              static header_bytes = <optimized out>, static s_start = <optimized out>, static static_capacity = <optimized out>}, 
            [1] = {<std::less<unsigned int>> = {<std::binary_function<unsigned int, unsigned int, bool>> = {<No data fields>}, <No data fields>}, <No data fields>}}}, <ue2::totally_ordered<ue2::flat_set<unsigned int, std::less<unsigned int>, std::allocator<unsigned int> > >> = {<No data fields>}, <No data fields>}, 
      reports_eod = {<ue2::flat_detail::flat_base<unsigned int, std::less<unsigned int>, std::allocator<unsigned int> >> = {
          storage = std::tuple containing = {
            [0] = {<boost::container::small_vector_base<unsigned int, std::allocator<unsigned int>, void>> = {<boost::container::vector<unsigned int, boost::container::small_vector_allocator<unsigned int, std::allocator<void>, void>, void>> = {
                  m_holder = {<boost::container::small_vector_allocator<unsigned int, std::allocator<void>, void>> = {<std::allocator<unsigned int>> = {<__gnu_cxx::new_allocator<unsigned int>> = {<No data fields>}, <No data fields>}, <No data fields>}, m_start = 0xaaaab52a8e38, 
                    m_size = 0, m_capacity = 2}}, static final_alignment = 4, m_storage_start = {aligner = {data = "\001\000\000"}, 
                  data = "\001\000\000"}}, <boost::container::small_vector_storage<boost::move_detail::aligned_struct_wrapper<4, 4>, 0>> = {<No data fields>}, static needed_extra_storages = <optimized out>, static needed_bytes = <optimized out>, 
              static header_bytes = <optimized out>, static s_start = <optimized out>, static static_capacity = <optimized out>}, 
            [1] = {<std::less<unsigned int>> = {<std::binary_function<unsigned int, unsigned int, bool>> = {<No data fields>}, <No data fields>}, <No data fields>}}}, <ue2::totally_ordered<ue2::flat_set<unsigned int, std::less<unsigned int>, std::allocator<unsigned int> > >> = {<No data fields>}, <No data fields>}},
      {next = std::vector of length 4, capacity 4 = {0, 0, 0, 6}, daddy = 3, impl_id = 6, 
      reports = {<ue2::flat_detail::flat_base<unsigned int, std::less<unsigned int>, std::allocator<unsigned int> >> = {
          storage = std::tuple containing = {
            [0] = {<boost::container::small_vector_base<unsigned int, std::allocator<unsigned int>, void>> = {<boost::container::vector<unsigned int, boost::container::small_vector_allocator<unsigned int, std::allocator<void>, void>, void>> = {
                  m_holder = {<boost::container::small_vector_allocator<unsigned int, std::allocator<void>, void>> = {<std::allocator<unsigned int>> = {<__gnu_cxx::new_allocator<unsigned int>> = {<No data fields>}, <No data fields>}, <No data fields>}, m_start = 0xaaaab52a8e78, 
                    m_size = 2, m_capacity = 2}}, static final_alignment = 4, m_storage_start = {aligner = {data = "\205\000\000"}, 
                  data = "\205\000\000"}}, <boost::container::small_vector_storage<boost::move_detail::aligned_struct_wrapper<4, 4>, 0>> = {<No data fields>}, static needed_extra_storages = <optimized out>, static needed_bytes = <optimized out>, 
              static header_bytes = <optimized out>, static s_start = <optimized out>, static static_capacity = <optimized out>}, 
            [1] = {<std::less<unsigned int>> = {<std::binary_function<unsigned int, unsigned int, bool>> = {<No data fields>}, <No data fields>}, <No data fields>}}}, <ue2::totally_ordered<ue2::flat_set<unsigned int, std::less<unsigned int>, std::allocator<unsigned int> > >> = {<No data fields>}, <No data fields>}, 
      reports_eod = {<ue2::flat_detail::flat_base<unsigned int, std::less<unsigned int>, std::allocator<unsigned int> >> = {
          storage = std::tuple containing = {
            [0] = {<boost::container::small_vector_base<unsigned int, std::allocator<unsigned int>, void>> = {<boost::container::vector<unsigned int, boost::container::small_vector_allocator<unsigned int, std::allocator<void>, void>, void>> = {
                  m_holder = {<boost::container::small_vector_allocator<unsigned int, std::allocator<void>, void>> = {<std::allocator<unsigned int>> = {<__gnu_cxx::new_allocator<unsigned int>> = {<No data fields>}, <No data fields>}, <No data fields>}, m_start = 0xaaaab52a8e98, 
                    m_size = 0, m_capacity = 2}}, static final_alignment = 4, m_storage_start = {aligner = {data = "\001\000\000"}, 
                  data = "\001\000\000"}}, <boost::container::small_vector_storage<boost::move_detail::aligned_struct_wrapper<4, 4>, 0>> = {<No data fields>}, static needed_extra_storages = <optimized out>, static needed_bytes = <optimized out>, 
              static header_bytes = <optimized out>, static s_start = <optimized out>, static static_capacity = <optimized out>}, 
            [1] = {<std::less<unsigned int>> = {<std::binary_function<unsigned int, unsigned int, bool>> = {<No data fields>}, <No data fields>}, <No data fields>}}}, <ue2::totally_ordered<ue2::flat_set<unsigned int, std::less<unsigned int>, std::allocator<unsigned int> > >> = {<No data fields>}, <No data fields>}},
      {next = std::vector of length 4, capacity 4 = {6, 7, 7, 7}, daddy = 3, impl_id = 7, 
      reports = {<ue2::flat_detail::flat_base<unsigned int, std::less<unsigned int>, std::allocator<unsigned int> >> = {
          storage = std::tuple containing = {
            [0] = {<boost::container::small_vector_base<unsigned int, std::allocator<unsigned int>, void>> = {<boost::container::vector<unsigned int, boost::container::small_vector_allocator<unsigned int, std::allocator<void>, void>, void>> = {
                  m_holder = {<boost::container::small_vector_allocator<unsigned int, std::allocator<void>, void>> = {<std::allocator<unsigned int>> = {<__gnu_cxx::new_allocator<unsigned int>> = {<No data fields>}, <No data fields>}, <No data fields>}, m_start = 0xaaaab52a8ed8, 
                    m_size = 0, m_capacity = 2}}, static final_alignment = 4, m_storage_start = {aligner = {data = "\022\000\000"}, 
                  data = "\022\000\000"}}, <boost::container::small_vector_storage<boost::move_detail::aligned_struct_wrapper<4, 4>, 0>> = {<No data fields>}, static needed_extra_storages = <optimized out>, static needed_bytes = <optimized out>, 
              static header_bytes = <optimized out>, static s_start = <optimized out>, static static_capacity = <optimized out>}, 
            [1] = {<std::less<unsigned int>> = {<std::binary_function<unsigned int, unsigned int, bool>> = {<No data fields>}, <No data fields>}, <No data fields>}}}, <ue2::totally_ordered<ue2::flat_set<unsigned int, std::less<unsigned int>, std::allocator<unsigned int> > >> = {<No data fields>}, <No data fields>}, 
      reports_eod = {<ue2::flat_detail::flat_base<unsigned int, std::less<unsigned int>, std::allocator<unsigned int> >> = {
          storage = std::tuple containing = {
            [0] = {<boost::container::small_vector_base<unsigned int, std::allocator<unsigned int>, void>> = {<boost::container::vector<unsigned int, boost::container::small_vector_allocator<unsigned int, std::allocator<void>, void>, void>> = {
                  m_holder = {<boost::container::small_vector_allocator<unsigned int, std::allocator<void>, void>> = {<std::allocator<unsigned int>> = {<__gnu_cxx::new_allocator<unsigned int>> = {<No data fields>}, <No data fields>}, <No data fields>}, m_start = 0xaaaab52a8ef8, 
                    m_size = 0, m_capacity = 2}}, static final_alignment = 4, m_storage_start = {aligner = {data = "\001\000\000"}, 
                  data = "\001\000\000"}}, <boost::container::small_vector_storage<boost::move_detail::aligned_struct_wrapper<4, 4>, 0>> = {<No data fields>}, static needed_extra_storages = <optimized out>, static needed_bytes = <optimized out>, 
              static header_bytes = <optimized out>, static s_start = <optimized out>, static static_capacity = <optimized out>}, 
            [1] = {<std::less<unsigned int>> = {<std::binary_function<unsigned int, unsigned int, bool>> = {<No data fields>}, <No data fields>}, <No data fields>}}}, <ue2::totally_ordered<ue2::flat_set<unsigned int, std::less<unsigned int>, std::allocator<unsigned int> > >> = {<No data fields>}, <No data fields>}}},
  start_anchored = 1, start_floating = 1, alpha_size = 4, alpha_remap = {_M_elems = {2 <repeats 22 times>, 0, 2 <repeats 14 times>, 1, 2 <repeats 218 times>, 3}}}

$33 = {shuffle_masks = {{1195655456, 1193287748, 0, 0} <repeats 22 times>, {373559840, 371204164, 0, 0}, {1195655456, 1193287748, 0, 
      0} <repeats 14 times>, {1192563488, 1193287701, 0, 0}, {1195655456, 1193287748, 0, 0} <repeats 218 times>}, length = 4544, 
  aux_offset = 4192, report_offset = 4320, accel_offset = 4352, n_states = 8 '\b', anchored = 65 'A', floating = 65 'A', flags = 6 '\006', 
  report = 0}

*/

static int dummy_callback(u64a start, u64a end, ReportID id, void *context)
{
	(void) context;
    printf("callback %llu %llu %u\n", start, end, id);
	return 0;
}

static void init_raw_dfa(struct ue2::raw_dfa *dfa, const ReportID rID)
{
    dfa->start_anchored = 1;
    dfa->start_floating = 1;
    dfa->alpha_size = 8;

    int nb_state = 8;
    for(int i = 0; i < nb_state; i++) {
        struct ue2::dstate state(dfa->alpha_size);
        state.next = std::vector<ue2::dstate_id_t>(nb_state);
        state.daddy = 0; // ?
        state.impl_id = i; //seems to be the id of the state
        state.reports = ue2::flat_set<ReportID>();
        state.reports_eod = ue2::flat_set<ReportID>();
        dfa->states.push_back(state);
    }

    // add a report to every accept state
    dfa->states[7].reports.insert(rID);

    // [a,b][c-e]{3}of
    // (1) -a,b-> (2) -c,d,e-> (3) -c,d,e-> (4) -c,d,e-> (5) -o-> (6) -f-> ((7))
    // (0) = dead

    for(int i = 0; i < ue2::ALPHABET_SIZE; i++) {
        dfa->alpha_remap[i] = 0;
    }

    dfa->alpha_remap['a'] = 0;
    dfa->alpha_remap['b'] = 1;
    dfa->alpha_remap['c'] = 2;
    dfa->alpha_remap['d'] = 3;
    dfa->alpha_remap['e'] = 4;
    dfa->alpha_remap['o'] = 5;
    dfa->alpha_remap['f'] = 6; //for some reason there's a check that run on dfa->alpha_size-1
    dfa->alpha_remap[257] = 7;


    dfa->states[0].next = {0,0,0,0,0,0,0};
    dfa->states[1].next = {2,2,1,1,1,1,1}; // A nothing
    dfa->states[2].next = {2,2,3,3,3,1,1}; // B [a,b]
    dfa->states[3].next = {2,2,4,4,4,1,1}; // C [a,b][c-e]{1}
    dfa->states[4].next = {2,2,5,5,5,1,1}; // D [a,b][c-e]{2}
    dfa->states[5].next = {2,2,1,1,1,6,1}; // E [a,b][c-e]{3}
    dfa->states[6].next = {2,2,1,1,1,1,7}; // F [a,b][c-e]{3}o
    dfa->states[7].next = {2,2,1,1,1,1,1}; // G [a,b][c-e]{3}of
}

static void init_grey(struct ue2::Grey *g)
{
    // Recorded from some run with snort
    g->optimiseComponentTree = true;
    g->calcComponents = true;
    g->performGraphSimplification = true;
    g->prefilterReductions = true;
    g->removeEdgeRedundancy = true;
    g->allowGough = true;
    g->allowHaigLit = true;
    g->allowLitHaig = true;
    g->allowLbr = true;
    g->allowMcClellan = true;
    g->allowSheng = true;
    g->allowMcSheng = true;
    g->allowPuff = true;
    g->allowLiteral = true;
    g->allowViolet = true;
    g->allowExtendedNFA = true;
    g->allowLimExNFA = true;
    g->allowAnchoredAcyclic = true;
    g->allowSmallLiteralSet = true;
    g->allowCastle = true;
    g->allowDecoratedLiteral = true;
    g->allowApproximateMatching = true;
    g->allowNoodle = true;
    g->fdrAllowTeddy = true;
    g->fdrAllowFlood = true;
    g->violetAvoidSuffixes = 1;
    g->violetAvoidWeakInfixes = true;
    g->violetDoubleCut = true;
    g->violetExtractStrongLiterals = true;
    g->violetLiteralChains = true;
    g->violetDoubleCutLiteralLen = 3;
    g->violetEarlyCleanLiteralLen = 6;
    g->puffImproveHead = true;
    g->castleExclusive = true;
    g->mergeSEP = true;
    g->mergeRose = true;
    g->mergeSuffixes = true;
    g->mergeOutfixes = true;
    g->onlyOneOutfix = false;
    g->allowShermanStates = true;
    g->allowMcClellan8 = true;
    g->allowWideStates = true;
    g->highlanderPruneDFA = true;
    g->minimizeDFA = true;
    g->accelerateDFA = true;
    g->accelerateNFA = true;
    g->reverseAccelerate = true;
    g->squashNFA = true;
    g->compressNFAState = true;
    g->numberNFAStatesWrong = false;
    g->highlanderSquash = true;
    g->allowZombies = true;
    g->floodAsPuffette = false;
    g->nfaForceSize = 0;
    g->maxHistoryAvailable = 110;
    g->minHistoryAvailable = 0;
    g->maxAnchoredRegion = 63;
    g->minRoseLiteralLength = 3;
    g->minRoseNetflowLiteralLength = 2;
    g->maxRoseNetflowEdges = 50000;
    g->maxEditDistance = 16;
    g->minExtBoundedRepeatSize = 32;
    g->goughCopyPropagate = true;
    g->goughRegisterAllocate = true;
    g->shortcutLiterals = true;
    g->roseGraphReduction = true;
    g->roseRoleAliasing = true;
    g->roseMasks = true;
    g->roseConvertFloodProneSuffixes = true;
    g->roseMergeRosesDuringAliasing = true;
    g->roseMultiTopRoses = true;
    g->roseHamsterMasks = true;
    g->roseLookaroundMasks = true;
    g->roseMcClellanPrefix = 1;
    g->roseMcClellanSuffix = 1;
    g->roseMcClellanOutfix = 2;
    g->roseTransformDelay = true;
    g->earlyMcClellanPrefix = true;
    g->earlyMcClellanInfix = true;
    g->earlyMcClellanSuffix = true;
    g->allowCountingMiracles = true;
    g->allowSomChain = true;
    g->somMaxRevNfaLength = 126;
    g->hamsterAccelForward = true;
    g->hamsterAccelReverse = false;
    g->miracleHistoryBonus = 16;
    g->equivalenceEnable = true;
    g->allowSmallWrite = true;
    g->allowSmallWriteSheng = false;
    g->smallWriteLargestBuffer = 70;
    g->smallWriteLargestBufferBad = 35;
    g->limitSmallWriteOutfixSize = 1048576;
    g->smallWriteMaxPatterns = 10000;
    g->smallWriteMaxLiterals = 10000;
    g->smallWriteMergeBatchSize = 20;
    g->allowTamarama = true;
    g->tamaChunkSize = 100;
    g->dumpFlags = 0;
    g->dumpPath = "";
    g->limitPatternCount = 8000000;
    g->limitPatternLength = 16000;
    g->limitGraphVertices = 500000;
    g->limitGraphEdges = 1000000;
    g->limitReportCount = 32000000;
    g->limitLiteralCount = 8000000;
    g->limitLiteralLength = 16000;
    g->limitLiteralMatcherChars = 1073741824;
    g->limitLiteralMatcherSize = 1073741824;
    g->limitRoseRoleCount = 32000000;
    g->limitRoseEngineCount = 8000000;
    g->limitRoseAnchoredSize = 1073741824;
    g->limitEngineSize = 1073741824;
    g->limitDFASize = 1073741824;
    g->limitNFASize = 1048576;
    g->limitLBRSize = 1048576;
    g->limitApproxMatchingVertices = 5000;
}

static really_inline
void init_queue(struct mq *q, const struct NFA *nfa) {
    q->nfa = nfa;
    q->end = 0;
    q->cur = 0;
    // q->state = (char*) calloc(64, 1);
    q->state = (char*) malloc(1);
    // *(q->state) = 1;
    q->streamState = nullptr;
    q->offset = 0;
    q->buffer = (u8*) malloc(64);
    q->length = 64;
    q->history = (u8*) malloc(64);
    q->hlength = 64;
    q->cb = dummy_callback;
    q->context = nullptr;
    q->report_current = 0;

    // const struct NFA *nfa; /**< nfa corresponding to the queue */
    // u32 cur; /**< index of the first valid item in the queue */
    // u32 end; /**< index one past the last valid item in the queue */
    // char *state; /**< uncompressed stream state; lives in scratch */
    // char *streamState; /**<
    //                     * real stream state; used to access structures which
    //                     * not duplicated the scratch state (bounded repeats,
    //                     * etc) */
    // u64a offset; /**< base offset of the buffer */
    // const u8 *buffer; /**< buffer to scan */
    // size_t length; /**< length of buffer */
    // const u8 *history; /**<
    //                     * history buffer; (logically) immediately before the
    //                     * main buffer */
    // size_t hlength; /**< length of the history buffer */
    // struct hs_scratch *scratch; /**< global scratch space */
    // char report_current; /**<
    //                       * report_current matches at starting offset through
    //                       * callback. If true, the queue must be located at a
    //                       * point where MO_MATCHES_PENDING was returned */
    // NfaCallback cb; /**< callback to trigger on matches */
    // void *context; /**< context to pass along with a callback */
    // struct mq_item items[MAX_MQE_LEN]; /**< queue items */

}

static void init_sheng_queue(struct mq **out_q, size_t max_size)
{
    ue2::Grey *g = new ue2::Grey();
    init_grey(g);
    hs_platform_info plat_info = {0, 0, 0, 0};
    ue2::CompileContext *cc = new ue2::CompileContext(false, false, ue2::target_t(plat_info), *g);
    ue2::ReportManager *rm = new ue2::ReportManager(*g);
    ue2::Report *report = new ue2::Report(ue2::EXTERNAL_CALLBACK, 0);
    ReportID rID = rm->getInternalId(*report);
    rm->setProgramOffset(0, 0);

    struct ue2::raw_dfa *dfa = new ue2::raw_dfa(ue2::NFA_OUTFIX);
    init_raw_dfa(dfa, rID);

    struct NFA *nfa = (shengCompile(*dfa, *cc, *rm, false)).release();
    assert(nfa);

    struct mq *q = new mq();

    // init_queue(q, nfa, scratch);
    init_queue(q, nfa);
    q->cb = dummy_callback;
    q->length = max_size; // setting this as the max length scanable

    if (nfa != q->nfa) {
        printf("Something went wrong while initializing sheng.\n");
    }
    nfaQueueInitState(nfa, q);
    pushQueueAt(q, 0, MQE_START, 0);
    //pushQueueAt(q, 1, MQE_TOP, 0);
    pushQueueAt(q, 1, MQE_END, q->length );

    *out_q = q;
}

static
void fill_pattern(MicroBenchmark &b, unsigned int start_offset, unsigned int period, const char *pattern, unsigned int pattern_length) {
    memset(b.buf.data(), '_', b.size);

    for (unsigned int i = 0; i < b.size - 8; i+= 8) {
        // filling with some junk, including some character used for a valid state, to prevent the use of shufti
        memcpy(b.buf.data() + i, "jgohcxbf", 8); 
    }

    for (unsigned int i = start_offset; i < b.size - pattern_length; i += period) {
        memcpy(b.buf.data() + i, pattern, pattern_length);
    }
}

int main(){
    int matches[] = {0, MAX_MATCHES};
    std::vector<size_t> sizes;
    for (size_t i = 0; i < N; i++) sizes.push_back(16000 << i*2);
    const char charset[] = "aAaAaAaAAAaaaaAAAAaaaaAAAAAAaaaAAaaa"; 
  
    for (int m = 0; m < 2; m++) {

        for (size_t i = 0; i < std::size(sizes); i++) {
            MicroBenchmark bench("Sheng", sizes[i]);
            unsigned int pattern_length = 6;
            unsigned int period = 128;
            unsigned int expected_matches = sizes[i]/period;
            run_benchmarks(sizes[i], MAX_LOOPS / sizes[i], m?:0, false, bench,
                [&](MicroBenchmark &b) {
                    init_sheng_queue(&(b.q), b.size);
                    fill_pattern(b, 0, period, "acecof", pattern_length);
					b.q->buffer = (const u8*) b.buf.data();
                },
                [&](MicroBenchmark &b) {
                    b.q->cur = 0;
                    b.q->items[b.q->cur].location = 0;
                    char ret_val;
                    unsigned int location;
                    unsigned int loop_count = 0;
                    do {
                        ret_val = nfaExecSheng_Q2(b.q->nfa, b.q, (s64a) b.size);
                        location = (unsigned int)(b.q->items[b.q->cur].location);
                        loop_count++;
                    } while(likely((ret_val == MO_MATCHES_PENDING) && (location < b.size) && ((location % period) == pattern_length)));
                    
                    if (unlikely((ret_val == MO_MATCHES_PENDING) && ((location % period) != pattern_length))) {
                        printf("unexpected location: %d\n", location);
                    }

                    if (unlikely((loop_count-1) != expected_matches)) {
                        printf("Sheng exited early. Found %u/%lu matches\n", loop_count-1, expected_matches);
                    }

					return b.q->buffer;
                }
            );
        }

        for (size_t i = 0; i < std::size(sizes); i++) {
            MicroBenchmark bench("Shufti", sizes[i]);
            run_benchmarks(sizes[i], MAX_LOOPS / sizes[i], matches[m], false, bench,
                [&](MicroBenchmark &b) {
                    b.chars.set('a');
                    ue2::shuftiBuildMasks(b.chars, (u8 *)&b.lo, (u8 *)&b.hi);
                    memset(b.buf.data(), 'b', b.size);
                },
                [&](MicroBenchmark &b) {
                    return shuftiExec(b.lo, b.hi, b.buf.data(), b.buf.data() + b.size);
                }
            );
        }

        for (size_t i = 0; i < std::size(sizes); i++) {
            MicroBenchmark bench("Reverse Shufti", sizes[i]);
            run_benchmarks(sizes[i], MAX_LOOPS / sizes[i], matches[m], true, bench,
                [&](MicroBenchmark &b) {
                    b.chars.set('a');
                    ue2::shuftiBuildMasks(b.chars, (u8 *)&b.lo, (u8 *)&b.hi);
                    memset(b.buf.data(), 'b', b.size);
                },
                [&](MicroBenchmark &b) {
                    return rshuftiExec(b.lo, b.hi, b.buf.data(), b.buf.data() + b.size);
                }
            );
        }

        for (size_t i = 0; i < std::size(sizes); i++) {
            MicroBenchmark bench("Truffle", sizes[i]);
            run_benchmarks(sizes[i], MAX_LOOPS / sizes[i], matches[m], false, bench,
                [&](MicroBenchmark &b) {
                    b.chars.set('a');
                    ue2::truffleBuildMasks(b.chars, (u8 *)&b.lo, (u8 *)&b.hi);
                    memset(b.buf.data(), 'b', b.size);
                },
                [&](MicroBenchmark &b) {
                    return truffleExec(b.lo, b.hi, b.buf.data(), b.buf.data() + b.size);
                }
            );
        }

        for (size_t i = 0; i < std::size(sizes); i++) {
            MicroBenchmark bench("Reverse Truffle", sizes[i]);
            run_benchmarks(sizes[i], MAX_LOOPS / sizes[i], matches[m], true, bench,
                [&](MicroBenchmark &b) {
                    b.chars.set('a');
                    ue2::truffleBuildMasks(b.chars, (u8 *)&b.lo, (u8 *)&b.hi);
                    memset(b.buf.data(), 'b', b.size);
                },
                [&](MicroBenchmark &b) {
                    return rtruffleExec(b.lo, b.hi, b.buf.data(), b.buf.data() + b.size);
                }
            );
        }

        for (size_t i = 0; i < std::size(sizes); i++) {
            MicroBenchmark bench("Vermicelli", sizes[i]);
            run_benchmarks(sizes[i], MAX_LOOPS / sizes[i], matches[m], false, bench,
                [&](MicroBenchmark &b) {
                    b.chars.set('a');
                    ue2::truffleBuildMasks(b.chars, (u8 *)&b.lo, (u8 *)&b.hi);
                    memset(b.buf.data(), 'b', b.size);
                },
                [&](MicroBenchmark &b) {
                    return vermicelliExec('a', 'b', b.buf.data(), b.buf.data() + b.size);
                }
            );
        }

        for (size_t i = 0; i < std::size(sizes); i++) {
            MicroBenchmark bench("Reverse Vermicelli", sizes[i]);
            run_benchmarks(sizes[i], MAX_LOOPS / sizes[i], matches[m], true, bench,
                [&](MicroBenchmark &b) {
                    b.chars.set('a');
                    ue2::truffleBuildMasks(b.chars, (u8 *)&b.lo, (u8 *)&b.hi);
                    memset(b.buf.data(), 'b', b.size);
                },
                [&](MicroBenchmark &b) {
                    return rvermicelliExec('a', 'b', b.buf.data(), b.buf.data() + b.size);
                }
            );
        }

        for (size_t i = 0; i < std::size(sizes); i++) {
            //we imitate the noodle unit tests
            std::string str;
            const size_t char_len = 5;
            str.resize(char_len + 1);
            for (size_t j=0; j < char_len; j++) {
                srand (time(NULL));
                int key = rand() % + 36 ;
                str[char_len] = charset[key];
                str[char_len + 1] = '\0';
            }

            MicroBenchmark bench("Noodle", sizes[i]);
            run_benchmarks(sizes[i], MAX_LOOPS / sizes[i], matches[m], false, bench,
                [&](MicroBenchmark &b) {
                    ctxt.clear();
                    memset(b.buf.data(), 'a', b.size);
                    u32 id = 1000;
                    ue2::hwlmLiteral lit(str, true, id);
                    b.nt = ue2::noodBuildTable(lit);
                    assert(b.nt != nullptr);
                },
                [&](MicroBenchmark &b) {
                    noodExec(b.nt.get(), b.buf.data(), b.size, 0, hlmSimpleCallback, &b.scratch);
                    return b.buf.data() + b.size;
                }
           );
        }
    }

    return 0;
}
