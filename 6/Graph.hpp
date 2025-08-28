
#ifndef GRAPH_HPP
#define GRAPH_HPP

#include <vector>
#include <iostream>


class Graph
{
    int V; // Number of vertices
    std::vector<std::vector<int>> adj; // Adjacency list representation

    public:
        Graph(int V);

        void addEdge(int u, int v);

        const std::vector<int>& getNeighbors(int v) const;

        int getNumVertices() const;

        bool isEdgeConnected(int u, int v) const;

        void printGraph() const;

        int degree(int v) const;

        bool isEulerian() const;

        void findEulerCircuit(std::ostream& os) const;


        
};
#endif

/* bool Graph::isConnected() const {
    int V = Graph::getNumVertices();

    // Vector to track visited vertices
    std::vector<bool> visited(V, false);

    // Find a vertex with a non-zero degree to start DFS
    int startNode = 0;
    while (startNode < V && Graph::degree(startNode) == 0) {
        startNode++;
    }
    
    if (startNode == V) {
        return true; // The graph has no edges, it's connected trivially
    }

    // Perform DFS from the first non-isolated vertex
    DFS(startNode, visited, *this);

    for (int i = 0; i < V; ++i) {
        // If a vertex has a non-zero degree and wasn't visited, the graph is disconnected
        if (degree(i) > 0 && !visited[i]) {
            return false;
        }
    }
    
    return true; 
}
*/