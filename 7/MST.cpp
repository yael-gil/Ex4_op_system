#include "MST.hpp"
#include <sstream>
#include <iostream>
#include <algorithm> // std::sort

// Helper: in an unweighted graph, every existing edge has weight 1
static inline int edgeWeight(const Graph& g, int u, int v) {
    return g.isEdgeConnected(u, v) ? 1 : 0;
}

std::vector<MST::Edge> MST::kruskalMST(Graph& g) {
    std::vector<Edge> edges;
    std::vector<Edge> mst;

    const int n = g.getNumVertices();
    if (n <= 1) return mst;

    // collect undirected edges once (u < v); weight = 1 for any existing edge
    for (int u = 0; u < n; ++u) {
        for (int v = u + 1; v < n; ++v) {
            if (g.isEdgeConnected(u, v)) {
                edges.emplace_back(u, v, /*weight=*/1);
            }
        }
    }

    // sort by weight (all 1 here, but keeps code future-proof)
    std::sort(edges.begin(), edges.end()); // uses Edge::operator<

    // Kruskal with Union-Find
    UnionFind uf(n);
    for (const Edge& e : edges) {
        if (uf.unite(e.src, e.dest)) {
            mst.push_back(e);
            if ((int)mst.size() == n - 1) break; // MST complete
        }
    }
    return mst;
}

std::string MST::run(Graph& g) {
    std::ostringstream out;
    const int n = g.getNumVertices();

    if (n == 0) return "Error: Empty graph.\n";
    if (n == 1) return "Minimum Spanning Tree (Kruskal):\nEdges in MST:\n\nTotal weight of MST: 0\nNumber of edges in MST: 0\n";

    std::vector<Edge> mst = kruskalMST(g);

    // In an undirected connected graph, MST must have exactly n-1 edges
    if ((int)mst.size() != n - 1) {
        return "Error: Graph is disconnected; no spanning tree covering all vertices.\n";
    }

    int totalWeight = 0;
    out << "Minimum Spanning Tree (Kruskal, unweighted):\n";
    out << "Edges in MST:\n";
    for (const Edge& e : mst) {
        out << "(" << e.src << " -- " << e.dest << ") weight: " << e.weight << "\n";
        totalWeight += e.weight; // sums to n-1
    }
    out << "\nTotal weight of MST: " << totalWeight << "\n";
    out << "Number of edges in MST: " << mst.size() << "\n";
    return out.str();
}
