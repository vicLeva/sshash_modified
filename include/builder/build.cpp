#pragma once

#include "../util.hpp"
#include "../dictionary.hpp"
#include "../info.cpp"
#include "../../external/pthash/external/essentials/include/essentials.hpp"

#include "util.hpp"

#include <numeric>  // for std::accumulate

/** build steps **/
#include "parse_file.cpp"
#include "build_index.cpp"
#include "build_skew_index.cpp"
/*****************/

namespace sshash {

void dictionary::build(std::string const& filename, build_configuration const& build_config) {
    /* Validate the build configuration. */
    if (build_config.k == 0) throw std::runtime_error("k must be > 0");
    if (build_config.k > constants::max_k) {
        throw std::runtime_error("k must be less <= " + std::to_string(constants::max_k) +
                                 " but got k = " + std::to_string(build_config.k));
    }
    if (build_config.m == 0) throw std::runtime_error("m must be > 0");
    if (build_config.m > build_config.k) throw std::runtime_error("m must be < k");
    if (build_config.l > constants::max_l) {
        throw std::runtime_error("l must be <= " + std::to_string(constants::max_l));
    }

    m_k = build_config.k;
    m_m = build_config.m;
    m_seed = build_config.seed;
    m_canonical_parsing = build_config.canonical_parsing;
    m_skew_index.min_log2 = build_config.l;

    std::vector<double> timings;
    timings.reserve(5);
    essentials::timer_type timer;

    /* step 1: parse the input file and build compact string pool ***/
    timer.start();
    parse_data data = parse_file(filename, build_config);
    m_size = data.num_kmers;
    timer.stop();
    timings.push_back(timer.elapsed());
    print_time(timings.back(), data.num_kmers, "step 1: 'parse_file'");
    timer.reset();
    /******/

    if (build_config.weighted) {
        /* step 1.1: compress weights ***/
        timer.start();
        data.weights_builder.build(m_weights);
        timer.stop();
        timings.push_back(timer.elapsed());
        print_time(timings.back(), data.num_kmers, "step 1.1.: 'build_weights'");
        timer.reset();
        /******/
        if (build_config.verbose) {
            double entropy_weights = data.weights_builder.print_info(data.num_kmers);
            double avg_bits_per_weight = static_cast<double>(m_weights.num_bits()) / data.num_kmers;
            std::cout << "weights: " << avg_bits_per_weight << " [bits/kmer]" << std::endl;
            std::cout << "  (" << entropy_weights / avg_bits_per_weight
                      << "x smaller than the empirical entropy)" << std::endl;
        }
    }

    /* step 2: sort minimizers and build MPHF ***/
    timer.start();
    data.minimizers.sort();
    uint64_t num_buckets = 0;
    for (auto it = data.minimizers.begin(); it.has_next(); it.next()) ++num_buckets;
    m_minimizers.build(data.minimizers.begin(), num_buckets);
    timer.stop();
    timings.push_back(timer.elapsed());
    print_time(timings.back(), data.num_kmers, "step 2: 'build_minimizers'");
    timer.reset();
    /******/

    /* step 3: build index ***/
    timer.start();
    auto buckets_stats = build_index(data, m_minimizers, m_buckets);
    timer.stop();
    timings.push_back(timer.elapsed());
    print_time(timings.back(), data.num_kmers, "step 3: 'build_index'");
    timer.reset();
    /******/

    /* step 4: build skew index ***/
    timer.start();
    build_skew_index(m_skew_index, data, m_buckets, build_config, buckets_stats);
    timer.stop();
    timings.push_back(timer.elapsed());
    print_time(timings.back(), data.num_kmers, "step 4: 'build_skew_index'");
    timer.reset();
    /******/

    double total_time = std::accumulate(timings.begin(), timings.end(), 0.0);
    print_time(total_time, data.num_kmers, "total_time");

    print_space_breakdown();

    if (build_config.verbose) buckets_stats.print();
}

}  // namespace sshash
