#include "SCC.hpp"
#include "Graph.hpp"
#include <vector>
#include <string>
#include <stack>
#include <sstream>
#include <algorithm>

void dfs(int u, const std::vector<std::vector<int>>& adj, std::vector<bool>& visited, std::vector<int>& order) {
    visited[u] = true;
    for (size_t v = 0; v < adj.size(); ++v) {
        if (adj[u][v] != 0 && !visited[v]) {
            dfs(v, adj, visited, order);
        }
    }
    order.push_back(u);  
}

void reverseDFS(int u, const std::vector<std::vector<int>>& revAdj, std::vector<bool>& visited, std::vector<int>& component) {
    visited[u] = true;
    component.push_back(u);
    for (size_t v = 0; v < revAdj.size(); ++v) {
        if (revAdj[u][v] != 0 && !visited[v]) {
            reverseDFS(v, revAdj, visited, component);
        }
    }
}

std::vector<std::vector<int>> SCC::findSCC(Graph& g) {
    int n = g.getNumVertices();
    const auto& adj = g.getAdjacency();
    std::vector<bool> visited(n, false);
    std::vector<int> order;

    for (int i = 0; i < n; ++i) {
        if (!visited[i]) {
            dfs(i, adj, visited, order);
        }
    }

    std::vector<std::vector<int>> revAdj(n, std::vector<int>(n, 0));
    for (int u = 0; u < n; ++u) {
        for (int v = 0; v < n; ++v) {
            if (adj[u][v] != 0) {
                revAdj[v][u] = adj[u][v];
            }
        }
    }
    std::fill(visited.begin(), visited.end(), false);
    std::reverse(order.begin(), order.end());

    std::vector<std::vector<int>> components;

    for (int u : order) {
        if (!visited[u]) {
            std::vector<int> component;
            reverseDFS(u, revAdj, visited, component);
            components.push_back(component);
        }
    }

    return components;
}

std::string SCC::run(Graph& g) {
    std::vector<std::vector<int>> components = findSCC(g);
    std::ostringstream oss;
    oss << "SCCs (" << components.size() << " components):\n";
    for (const auto& comp : components) {
        for (int v : comp) {
            oss << v << " ";
        }
        oss << "\n";
    }
    std::cout <<"The strongly connected components are:\n" <<oss.str()<<std::endl;
    return oss.str();
}