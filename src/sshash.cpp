#include <iostream>

#include "common.hpp"
#include "bench_utils.hpp"
#include "check_utils.hpp"
#include "build.cpp"
#include "query.cpp"
#include "permute.cpp"

using namespace sshash;

int main(int argc, char** argv) {
    std::cout << "Start expe" << std::endl;

    std::cout << "Start 6_1" << std::endl;

    dictionary dict;
    build_configuration build_config;
    build_config.k = 31;
    build_config.m = 17;
    build_config.verbose = false;
    build_config.weighted = true;

    auto ttot = std::chrono::high_resolution_clock::now();
    dict.build("~/data/AHX_ACXIOSF_6_1_C2FGHACXX.IND4_clean.fastq.unitigs.fa.ust.fa.gz", build_config);
    std::cout << std::to_string( std::chrono::duration<double, std::milli>( std::chrono::high_resolution_clock::now() - ttot ).count()) << " ms (build+inserts)\n";

    ttot = std::chrono::high_resolution_clock::now();
    dict.streaming_query_from_file("~/data/6_1_seqs.fasta", true);
    std::cout << std::to_string( std::chrono::duration<double, std::milli>( std::chrono::high_resolution_clock::now() - ttot ).count()) << " ms (pos queries)\n";

    ttot = std::chrono::high_resolution_clock::now();
    dict.streaming_query_from_file("~/data/neg_seqs.fasta", true);
    std::cout << std::to_string( std::chrono::duration<double, std::milli>( std::chrono::high_resolution_clock::now() - ttot ).count()) << " ms (neg queries)\n";
}