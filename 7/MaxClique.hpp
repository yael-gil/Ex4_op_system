#ifndef MAX_CLIQUE_HPP
#define MAX_CLIQUE_HPP

#include "Algorithms.hpp"
#include "Graph.hpp"
#include <vector>
#include <sstream>
#include <algorithm>

class MaxClique : public Algorithms {
private:
    std::vector<int> maxClique;
    
    void bronKerbosch(std::vector<int>& R, std::vector<int>& P, std::vector<int>& X, const Graph& g) {
        if (P.empty() && X.empty()) {
            // Found a maximal clique
            if (R.size() > maxClique.size()) {
                maxClique = R;
            }
            return;
        }
        
        // Choose pivot vertex (heuristic: vertex with most connections to P)
        int pivot = -1;
        int maxConnections = -1;
        
        for (int v : P) {
            int connections = 0;
            for (int u : P) {
                if (v != u && g.isEdgeConnected(v, u)) {
                    connections++;
                }
            }
            if (connections > maxConnections) {
                maxConnections = connections;
                pivot = v;
            }
        }
        
        for (int v : X) {
            int connections = 0;
            for (int u : P) {
                if (g.isEdgeConnected(v, u)) {
                    connections++;
                }
            }
            if (connections > maxConnections) {
                maxConnections = connections;
                pivot = v;
            }
        }
        
        // For each vertex in P \ N(pivot)
        std::vector<int> candidates;
        for (int v : P) {
            bool connected_to_pivot = (pivot != -1) && g.isEdgeConnected(pivot, v);
            if (!connected_to_pivot) {
                candidates.push_back(v);
            }
        }
        
        for (int v : candidates) {
            // R' = R ∪ {v}
            std::vector<int> R_new = R;
            R_new.push_back(v);
            
            // P' = P ∩ N(v)
            std::vector<int> P_new;
            for (int u : P) {
                if (u != v && g.isEdgeConnected(v, u)) {
                    P_new.push_back(u);
                }
            }
            
            // X' = X ∩ N(v)
            std::vector<int> X_new;
            for (int u : X) {
                if (g.isEdgeConnected(v, u)) {
                    X_new.push_back(u);
                }
            }
            
            bronKerbosch(R_new, P_new, X_new, g);
            
            // Move v from P to X
            P.erase(std::find(P.begin(), P.end(), v));
            X.push_back(v);
        }
    }

public:
    std::string run(Graph& g) override {
        if (g.isDirected()) {
            return "ERROR: Max Clique algorithm works only on undirected graphs\n";
        }
        
        maxClique.clear();
        int V = g.getNumVertices();
        
        std::vector<int> R; // Current clique
        std::vector<int> P; // Candidate vertices
        std::vector<int> X; // Excluded vertices
        
        // Initialize P with all vertices
        for (int i = 0; i < V; i++) {
            P.push_back(i);
        }
        
        bronKerbosch(R, P, X, g);
        
        std::ostringstream result;
        if (maxClique.empty()) {
            result << "Maximum clique: empty (no vertices)\n";
        } else {
            std::sort(maxClique.begin(), maxClique.end());
            result << "Maximum clique size: " << maxClique.size() << "\n";
            result << "Maximum clique vertices: {";
            for (size_t i = 0; i < maxClique.size(); i++) {
                if (i > 0) result << ", ";
                result << maxClique[i];
            }
            result << "}\n";
        }
        
        return result.str();
    }
};

#endif