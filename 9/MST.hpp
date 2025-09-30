#ifndef MST_HPP
#define MST_HPP

#include "Algorithms.hpp"
#include "Graph.hpp"
#include <vector>
#include <algorithm>
#include <sstream>

class MST : public Algorithms {
private:
    struct Edge {
        int u, v, weight;
        bool operator<(const Edge& other) const {
            return weight < other.weight;
        }
    };
    
    // Union-Find data structure for Kruskal's algorithm
    class UnionFind {
        std::vector<int> parent, rank;
    public:
        UnionFind(int n) : parent(n), rank(n, 0) {
            for (int i = 0; i < n; i++) {
                parent[i] = i;
            }
        }
        
        int find(int x) {
            if (parent[x] != x) {
                parent[x] = find(parent[x]);
            }
            return parent[x];
        }
        
        bool unite(int x, int y) {
            int px = find(x), py = find(y);
            if (px == py) return false;
            
            if (rank[px] < rank[py]) {
                parent[px] = py;
            } else if (rank[px] > rank[py]) {
                parent[py] = px;
            } else {
                parent[py] = px;
                rank[px]++;
            }
            return true;
        }
    };

public:
    std::string run(Graph& g) override {
        if (g.isDirected()) {
            return "ERROR: MST algorithm works only on undirected graphs\n";
        }
        
        std::vector<Edge> edges;
        int V = g.getNumVertices();
        
        // Collect all edges
        for (int u = 0; u < V; u++) {
            for (const auto& neighbor : g.getNeighbors(u)) {
                int v = neighbor.first;
                int weight = neighbor.second;
                // Add edge only once for undirected graph
                if (u < v) {
                    edges.push_back({u, v, weight});
                }
            }
        }
        
        if (edges.empty()) {
            return "MST: No edges in graph\n";
        }
        
        // Sort edges by weight
        std::sort(edges.begin(), edges.end());
        
        UnionFind uf(V);
        std::vector<Edge> mst;
        int totalWeight = 0;
        
        // Kruskal's algorithm
        for (const Edge& e : edges) {
            if (uf.unite(e.u, e.v)) {
                mst.push_back(e);
                totalWeight += e.weight;
                if (mst.size() == V - 1) break;
            }
        }
        
        std::ostringstream result;
        if (mst.size() < V - 1) {
            result << "MST: Graph is not connected\n";
        } else {
            result << "MST edges:\n";
            for (const Edge& e : mst) {
                result << e.u << " - " << e.v << " (weight: " << e.weight << ")\n";
            }
            result << "Total MST weight: " << totalWeight << "\n";
        }
        
        return result.str();
    }
};

#endif