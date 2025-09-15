#ifndef MST_HPP
#define MST_HPP

#include "Algorithms.hpp"
#include "Graph.hpp"
#include <vector>
#include <algorithm>

class MST : public Algorithms {
private:
    struct Edge {
        int src, dest, weight;
        
        Edge(int s, int d, int w) : src(s), dest(d), weight(w) {}
        
        bool operator<(const Edge& other) const {
            return weight < other.weight;
        }
    };
    
    // Union-Find data structure for Kruskal's algorithm
    class UnionFind {
    private:
        std::vector<int> parent, rank;
        
    public:
        UnionFind(int n) : parent(n), rank(n, 0) {
            for (int i = 0; i < n; i++) {
                parent[i] = i;
            }
        }
        
        int find(int x) {
            if (parent[x] != x) {
                parent[x] = find(parent[x]); // Path compression
            }
            return parent[x];
        }
        
        bool unite(int x, int y) {
            int rootX = find(x);
            int rootY = find(y);
            
            if (rootX == rootY) return false;
            
            // Union by rank
            if (rank[rootX] < rank[rootY]) {
                parent[rootX] = rootY;
            } else if (rank[rootX] > rank[rootY]) {
                parent[rootY] = rootX;
            } else {
                parent[rootY] = rootX;
                rank[rootX]++;
            }
            return true;
        }
    };
    
    // Kruskal's algorithm implementation
    std::vector<Edge> kruskalMST(Graph& g);
    
public:
    MST() = default;
    ~MST() = default;
    
    /**
     * @brief Calculate the Minimum Spanning Tree of the graph using Kruskal's algorithm
     * @param g The graph
     * @return A string containing the MST edges and total weight
     */
    std::string run(Graph& g) override;
};

#endif // MST_HPP