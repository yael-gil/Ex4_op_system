
#ifndef GRAPH_HPP
#define GRAPH_HPP

#include <vector>
#include <iostream>
#include <utility> // For std::pair


class Graph
{
    int V; // Number of vertices
    bool directed = false; // Directed or undirected graph												  
    std::vector<std::vector<std::pair<int,int>>> adj; // Adjacency list representation

    public:
        Graph(int V, bool directed);

        void addEdge(int u, int v, int w = 0);

        const std::vector<std::pair<int, int>>& getNeighbors(int v) const;

        bool isDirected() const;

        int getNumVertices() const;

        bool isEdgeConnected(int u, int v) const;

        void printGraph() const;

        int degree(int v) const;
        
        int get_in_degree(int v) const;
    
        int get_out_degree(int v) const;

        bool isEulerian() const;

        void findEulerCircuit();

        
};
#endif


