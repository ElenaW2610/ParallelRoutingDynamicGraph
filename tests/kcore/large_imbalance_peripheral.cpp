#include "dynamicgraph.h"

#include <chrono>
#include <iostream>
#include <string>
#include <random>
#include <unordered_set>
#include <vector>

#include <omp.h>

using std::cout;
using std::cerr;
using std::endl;

struct Config {
    int num_threads = -1;
    int k = 1000;
};

Config parse_args(int argc, char** argv) {
    Config cfg;
    for (int i=1; i < argc; i++) {
        std::string arg = argv[i];
        if ((arg == "-n" || arg == "--threads") && i + 1 < argc)
            cfg.num_threads = std::stoi(argv[++i]);
        else if ((arg == "-k" || arg == "--kcore") && i + 1 < argc)
            cfg.k = std::stoi(argv[++i]);
    }
    return cfg;
}

bool check_k_core_correctness(const DynamicGraph& g, int k, const std::vector<int>& core_vertices) {
    if (core_vertices.empty())
        return true;

    std::unordered_set<int> core_set;
    core_set.reserve(2*core_vertices.size());
    for (int v : core_vertices)
        core_set.insert(v);

    for (int v : core_vertices) {
        auto neigh = g.neighbors(v);
        int count = 0;
        for (int u : neigh) {
            if (core_set.count(u) != 0)
                count++;
        }
        if (count < k)
            return false;
    }
    return true;
}

int main(int argc, char** argv) {
    Config cfg = parse_args(argc, argv);

    if (cfg.num_threads > 0) {
        omp_set_num_threads(cfg.num_threads);
    }
    int used_threads = omp_get_max_threads();

    const int N = 8192;
    const int CORE1_START = 0;
    const int CORE1_END   = 2048;
    const int CORE2_START = 2048;
    const int CORE2_END   = 4096;
    const int PERIPH_START = 4096;

    cout << "k-core test on a graph with 8192 vertices with two 2048-vertices cores and large peripherals\n";
    cout << "Using " << used_threads << " thread(s), k = " << cfg.k << ".\n";

    // build graph
    DynamicGraph g(N);
    std::mt19937 rng(12345);

    std::size_t edge_count = 0;

    // Core 1
    for (int u = CORE1_START; u < CORE1_END; u++) {
        for (int v = u + 1; v < CORE1_END; v++) {
            g.add_edge(u, v);
            edge_count++;
        }
    }

    // Core 2
    for (int u = CORE2_START; u < CORE2_END; u++) {
        for (int v = u + 1; v < CORE2_END; v++) {
            g.add_edge(u, v);
            edge_count++;
        }
    }

    // connect CORE1 vertex i to CORE2 vertex i
    for (int i = 0; i < CORE1_END - CORE1_START; i++) {
        int u = CORE1_START + i;
        int v = CORE2_START + i;
        g.add_edge(u, v);
        edge_count++;
    }

    // remaining vertex connects to a single random vertex in core 1 or core 2
    std::uniform_int_distribution<int> dist_core(CORE1_START, CORE2_END - 1);

    for (int u = PERIPH_START; u < N; u++) {
        int v = dist_core(rng);  // random vertex in either dense core
        g.add_edge(u, v);
        edge_count++;
    }

    cout << "Graph generated: N = " << N << ", M ≈ " << edge_count << " edges.\n";

    g.snapshot();

    auto start = std::chrono::steady_clock::now();
    std::vector<int> kcore_vertices = g.k_core(cfg.k);
    auto end = std::chrono::steady_clock::now();

    double ms = std::chrono::duration<double, std::milli>(end - start).count();

    bool ok = check_k_core_correctness(g, cfg.k, kcore_vertices);

    cout << "k_core(" << cfg.k << "): time = " << ms << " ms, "
         << "size = " << kcore_vertices.size() << ", "
         << "correct = " << (ok ? "YES" : "NO") << "\n";

    return ok ? 0 : 1;
}
