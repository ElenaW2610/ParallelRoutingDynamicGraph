// Baseline Implementation

#include "dynamicgraph.h"

#include <algorithm>
#include <cassert>
#include <queue>
#include <vector>
#include <limits>
#include <functional>

#include <omp.h>

DynamicGraph::DynamicGraph(std::size_t num_vertices)
    : adj_(num_vertices),
      snapshot_adj_(num_vertices),
      active_(num_vertices, true) {}

static inline void 
check_vertex(const DynamicGraph::Vertex v, std::size_t n) {
    assert(v >= 0 && static_cast<std::size_t>(v) < n && "Vertex ID out of range.");
}

// Dynamic Updates

void DynamicGraph::add_vertex(Vertex v) {
    if (v < 0) 
        return;
    std::size_t id = static_cast<std::size_t>(v);
    if (id >= adj_.size()) {
        adj_.resize(id+1);
        snapshot_adj_.resize(id+1);
        active_.resize(id + 1, false);
    }
    active_[id] = true;
}

void DynamicGraph::remove_vertex(Vertex v) {
    check_vertex(v, adj_.size());
    active_[v] = false;
}

void DynamicGraph::add_edge(Vertex u, Vertex v) {
    std::size_t n = adj_.size();
    check_vertex(u, n);
    check_vertex(v, n);
    if (!active_[u] || !active_[v]) 
        return;
    auto &nu = adj_[u];
    auto &nv = adj_[v];

    if (std::find(nu.begin(), nu.end(), v) == nu.end())
        nu.push_back(v);
    if (std::find(nv.begin(), nv.end(), u) == nv.end())
        nv.push_back(u);
}

void DynamicGraph::remove_edge(Vertex u, Vertex v) {
    std::size_t n = adj_.size();
    check_vertex(u, n);
    check_vertex(v, n);

    auto &nu = adj_[u];
    auto &nv = adj_[v];

    nu.erase(std::remove(nu.begin(), nu.end(), v), nu.end());
    nv.erase(std::remove(nv.begin(), nv.end(), u), nv.end());
}

const std::vector<DynamicGraph::Vertex>&
DynamicGraph::neighbors(Vertex v) const {
    std::size_t n = adj_.size();
    check_vertex(v, n);

    if (!snapshot_adj_.empty())
        return snapshot_adj_[v];
    return adj_[v];
}

// Snapshots
void DynamicGraph::snapshot() {
    snapshot_adj_ = adj_;

    for (std::size_t v=0; v < snapshot_adj_.size(); v++) {
        if (!active_[v])
            snapshot_adj_[v].clear();
    }
}

// Multiple Packages Routing Simulation
std::vector<std::vector<DynamicGraph::Vertex>>
DynamicGraph::min_cost_routing(const std::vector<std::pair<Vertex, Vertex>>& pairs, int max_depth) const
{
    const auto &G = snapshot_adj_.empty() ? adj_ : snapshot_adj_;
    std::size_t n = G.size();

    std::vector<std::vector<Vertex>> all_paths;
    all_paths.reserve(pairs.size());

    for (const auto &p : pairs) {
        Vertex s = p.first;
        Vertex t = p.second;

        if (s < 0 || t < 0 || static_cast<std::size_t>(s) >= n || static_cast<std::size_t>(t) >= n || !active_[s] || !active_[t]) {
            all_paths.emplace_back(); // empty path
            continue;
        }

        // bfs with depth limit
        std::vector<int> parent(n, -1);
        std::vector<int> depth(n, -1);
        std::queue<Vertex> q;

        depth[s] = 0;
        q.push(s);

        bool found = false;
        while (!q.empty()) {
            Vertex u = q.front();
            q.pop();

            if (u == t) {
                found = true;
                break;
            }
            if (depth[u] >= max_depth)
                continue;

            for (Vertex v : G[u]) {
                if (!active_[v]) continue;
                if (depth[v] == -1) {
                    depth[v] = depth[u] + 1;
                    parent[v] = u;
                    q.push(v);
                }
            }
        }

        if (!found && s != t) {
            all_paths.emplace_back();
            continue;
        }

        // reconstruct path from t back to s
        std::vector<Vertex> path;
        Vertex cur = t;
        path.push_back(cur);
        while (cur != s && parent[cur] != -1) {
            cur = parent[cur];
            path.push_back(cur);
        }
        if (cur != s) {
            // output as no path is cannot reconstruct
            all_paths.emplace_back();
            continue;
        }
        std::reverse(path.begin(), path.end());
        all_paths.push_back(std::move(path));
    }

    return all_paths;
}

// K-cores
std::vector<int> DynamicGraph::k_cores() const {
    const auto &G = snapshot_adj_.empty() ? adj_ : snapshot_adj_;
    std::size_t n = G.size();

    std::vector<int> degree(n, 0);
    std::vector<int> core(n, 0);
    std::vector<bool> removed(n, false);

    // initialize degrees
    for (std::size_t v=0; v < n; v++) {
        if (!active_[v]) {
            degree[v] = 0;
            continue;
        }
        int d = 0;
        for (Vertex u : G[v]) {
            if (active_[u]) 
                d++;
        }
        degree[v] = d;
    }

    // min heap of (degree, vertex)
    using Item = std::pair<int, Vertex>;
    auto cmp = std::greater<Item>();
    std::priority_queue<Item, std::vector<Item>, decltype(cmp)> pq(cmp);

    for (std::size_t v=0; v < n; v++) {
        if (active_[v])
            pq.emplace(degree[v], static_cast<Vertex>(v));
    }

    while (!pq.empty()) {
        auto [deg_v, v] = pq.top();
        pq.pop();

        if (removed[v] || deg_v != degree[v])
            continue; // stale

        removed[v] = true;
        core[v] = deg_v;

        for (Vertex u : G[v]) {
            if (!active_[u] || removed[u]) 
                continue;
            if (degree[u] > 0) {
                degree[u]--;
                pq.emplace(degree[u], u);
            }
        }
    }

    return core;
}
