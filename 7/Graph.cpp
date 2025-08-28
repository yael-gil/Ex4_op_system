#include <vector>
#include <iostream>
#include <algorithm> // For std::find
#include "Graph.hpp"

class Graph; 
void DFS(int start_node, std::vector<bool>& visited, const Graph& graph);


    
        Graph::Graph(int V) : V(V), adj(V) {}

        void Graph::addEdge(int u, int v)
            {
                if (u < 0 || u >= V || v < 0 || v >= V)
                {
                    std::cerr << "Error: Vertex out of bounds (addEdge)" << std::endl;
                    return;
                }
                if (u == v)
                {
                    std::cerr << "Error: No self-loops allowed (addEdge)" << std::endl;
                    return;
                }

                if(isEdgeConnected(u, v))
                {
                    std::cerr << "Error: Edge already exists (addEdge)" << std::endl;
                    return;
                }

                adj[u].push_back(v);
                adj[v].push_back(u); // For undirected graph
            }

        const std::vector<int>& Graph::getNeighbors(int v) const
            {
                if(v < 0 || v >= V)
                {
                    std::cerr << "Error: Vertex out of bounds (getNeighbros)" << std::endl;
                    static const std::vector<int> empty; // Return an empty vector on error
                    return empty;
                }
                return adj[v];
            }

        int Graph::getNumVertices() const
            {
                return V;
            }

        bool Graph::isEdgeConnected(int u, int v) const
            {
                if (u < 0 || u >= V || v < 0 || v >= V)
                {
                    std::cerr << "Error: Vertex out of bounds (isConnected)" << std::endl;
                    return false;
                }

                for (int neighbor : adj[u])
                {
                    if (neighbor == v)
                        return true;
                }
                return false;
            }
        
        void Graph::printGraph() const
            {
                for (int i = 0; i < V; ++i)
                {
                    std::cout << "Vertex " << i << ":";
                    for (int neighbor : adj[i])
                    {
                        std::cout << " " << neighbor;
                    }
                    std::cout << std::endl;
                }
            }
        
        int Graph::degree(int v) const
        {
            if (v < 0 || v >= V)
            {
                std::cerr << "Error: Vertex out of bounds (degree)" << std::endl;
                return -1;
            }
            return adj[v].size();
        }
        
        bool Graph::isEulerian() const
        {
            for (int i = 0; i < V; ++i)
            {
                if (degree(i) % 2 != 0){ // If any vertex has an odd degree
                    std::cout << "Vertex " << i << " has odd degree of " << degree(i) << std::endl;
                    return false;
                    }
            }

            std::vector<bool> visited(V, false);

            // Find a vertex with a non-zero degree to start DFS
            int start_node;
            for (start_node = 0; start_node < V; ++start_node) {
                if (degree(start_node) > 0) {
                    break;
                }
            }

            if (start_node == V) {
                // The graph has no edges, it's connected trivially
                std::cout << "The graph has no edges." << std::endl;
                return true; 
            }

            // Run DFS from the first non-isolated vertex
            DFS(start_node, visited, *this);

            // Check if all non-isolated vertices were visited
            for (int i = 0; i < V; ++i) {
                if (degree(i) > 0 && !visited[i]) {
                    std::cout << "Vertex " << i << " is not connected." << std::endl;
                    return false; // Found an unvisited non-isolated vertex
                }
            }

            return true;
        }

        void Graph::findEulerCircuit(){
            
            if (!Graph::isEulerian()) {
                std::cout << "The graph is not Eulerian." << std::endl;
                return;
            }

            std::vector<std::vector<int>> tempAdj = adj; // Create a copy of the adjacency list
            std::vector<int> circuit; // To store the Eulerian circuit
            std::vector<int> stack; // Stack to hold the current path

            // Start from the first vertex with edges
            int currVertex = 0;
            for (int i = 0; i < V; ++i) {
                if (degree(i) > 0) {
                    currVertex = i;
                    break;
                }
            }

            stack.push_back(currVertex);

            while (!stack.empty()) {
                if (tempAdj[currVertex].empty()) {
                    // If no more neighbors, add to circuit
                    circuit.push_back(currVertex);
                    currVertex = stack.back();
                    stack.pop_back();
                } else {
                    // Otherwise, continue traversing
                    stack.push_back(currVertex);
                    int nextVertex = tempAdj[currVertex].back();
                    // Remove the edge from the graph
                    tempAdj[currVertex].pop_back();
                    auto it = std::find(tempAdj[nextVertex].begin(), tempAdj[nextVertex].end(), currVertex);
                    if (it != tempAdj[nextVertex].end()) {
                        tempAdj[nextVertex].erase(it);
                    }
                    currVertex = nextVertex;
                }
            }

            // Print the Eulerian circuit
            std::cout << "Eulerian Circuit: ";
            for (auto it = circuit.rbegin(); it != circuit.rend(); ++it) {
                std::cout << *it << " ";
            }
            std::cout << std::endl;
        }


// Depth-First Search (DFS) helper function (not a member of Graph class)
void DFS(int start_node, std::vector<bool>& visited, const Graph& graph) {
    // Mark the current node as visited
    visited[start_node] = true;
    std::cout << start_node << " "; // Print the node

    // Iterate through all neighbors of the current node
    for (int neighbor : graph.getNeighbors(start_node)) {
        //  Recursively call DFS for unvisited neighbors
        if (!visited[neighbor]) {
            DFS(neighbor, visited, graph);
        }
    }
}
