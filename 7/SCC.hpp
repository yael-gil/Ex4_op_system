#ifndef SCC_HPP
#define SCC_HPP

#include "Algorithms.hpp"
#include "Graph.hpp"
#include <vector>
#include <stack>
#include <sstream>
#include <algorithm>

class SCC : public Algorithms {
private:
    void dfsFirst(int v, std::vector<bool>& visited, std::stack<int>& Stack, const Graph& g) {
        visited[v] = true;
        
        for (const auto& neighbor : g.getNeighbors(v)) {
            int u = neighbor.first;
            if (!visited[u]) {
                dfsFirst(u, visited, Stack, g);
            }
        }
        
        Stack.push(v);
    }
    
    void dfsSecond(int v, std::vector<bool>& visited, std::vector<int>& component, const Graph& g) {
        visited[v] = true;
        component.push_back(v);
        
        // For transpose graph, we need to find all vertices that have edge TO v
        for (int u = 0; u < g.getNumVertices(); u++) {
            if (!visited[u] && g.isEdgeConnected(u, v)) {
                dfsSecond(u, visited, component, g);
            }
        }
    }

public:
    std::string run(Graph& g) override {
        if (!g.isDirected()) {
            return "ERROR: SCC algorithm works only on directed graphs\n";
        }
        
        int V = g.getNumVertices();
        std::stack<int> Stack;
        std::vector<bool> visited(V, false);
        
        // Step 1: Fill vertices in stack according to their finishing times
        for (int i = 0; i < V; i++) {
            if (!visited[i]) {
                dfsFirst(i, visited, Stack, g);
            }
        }
        
        // Step 2: Process vertices in order defined by Stack
        std::fill(visited.begin(), visited.end(), false);
        std::vector<std::vector<int>> sccs;
        
        while (!Stack.empty()) {
            int v = Stack.top();
            Stack.pop();
            
            if (!visited[v]) {
                std::vector<int> component;
                dfsSecond(v, visited, component, g);
                std::sort(component.begin(), component.end());
                sccs.push_back(component);
            }
        }
        
        std::ostringstream result;
        result << "Number of Strongly Connected Components: " << sccs.size() << "\n";
        
        for (size_t i = 0; i < sccs.size(); i++) {
            result << "SCC " << (i + 1) << ": {";
            for (size_t j = 0; j < sccs[i].size(); j++) {
                if (j > 0) result << ", ";
                result << sccs[i][j];
            }
            result << "}\n";
        }
        
        return result.str();
    }
};

#endif