#include <vector>
#include <iostream>
#include <algorithm> // For std::find

#include "Graph.hpp"


class Graph; 
void DFS(int start_node, std::vector<bool>& visited, const Graph& graph);



        Graph::Graph(int V, bool directed) : V(V), adj(V), directed(directed) {}

        void Graph::addEdge(int u, int v, int w) // Add edge from u to v
            {
                if (u < 0 || u >= V || v < 0 || v >= V)
                {
                    std::cerr << "Error: Vertex out of bounds (addEdge)" << std::endl;
                    return;
                }
				if(w < 0){
				 
                    std::cerr << "Error: Negative weight not allowed (addEdge)" << std::endl;
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

                adj[u].push_back({v, w}); // Add edge from u to v
                
                if(!directed){
                    adj[v].push_back({u, w});  // For undirected graph
                }
            
            }

		
        const std::vector<std::pair<int, int>>& Graph::getNeighbors(int v) const
            {
                if(v < 0 || v >= V)
                {
                    std::cerr << "Error: Vertex out of bounds (getNeighbros)" << std::endl;
                    static const std::vector<std::pair<int, int>> empty; // Return an empty vector on error
                    return empty;
                }
                return adj[v];
            }

        int Graph::getNumVertices() const
            {
                return V;
            }

		bool Graph::isDirected() const
            {
                return directed;
            }

        bool Graph::isEdgeConnected(int u, int v) const // Check if there's an edge from u to v
            {
                if (u < 0 || u >= V || v < 0 || v >= V)
                {
                    std::cerr << "Error: Vertex out of bounds (isConnected)" << std::endl;
                    return false;
                }

                for (std::pair neighbor : adj[u])
                {
                    if (neighbor.first == v)
                        return true;
                }
                return false;
            }

        
        void Graph::printGraph() const
            {
                for (int i = 0; i < V; ++i)
                {
                    std::cout << "Vertex " << i << ":";
                    for (std::pair<int,int> neighbor : adj[i])
                    {
                        std::cout << " " << neighbor.first << "(weight " << neighbor.second << ")";
                    }
                    std::cout << std::endl;
                }
            }
        int Graph::degree(int v) const // For undirected graphs
            {
                if (v < 0 || v >= V)
                {
                    std::cerr << "Error: Vertex out of bounds (degree)" << std::endl;
                    return -1;
                }
                if (directed)
                {
                    int in_deg = get_in_degree(v);
                    int out_deg = get_out_degree(v);
                    if (in_deg == -1 || out_deg == -1) // Error in in_degree or out_degree
                        return -1;
                    return in_deg + out_deg; // Total degree is sum of in-degree and out-degree
                }

                return adj[v].size(); // Degree in not directed graph is simply the size of the adjacency list
            }
        
        int Graph::get_in_degree(int v) const
        {
            if (v < 0 || v >= V)
            {
                std::cerr << "Error: Vertex out of bounds (in_degree)" << std::endl;
                return -1;
            }

            int in_deg = 0;
            for (int i = 0; i < V; ++i) // Iterate through all vertices
            {
                for (const auto& neighbor : adj[i]) // Check their adjacency lists
                {
                    if (neighbor.first == v) // If there's an edge to vertex v
                    {
                        ++in_deg;
                    }
                }
            }
            return in_deg;
        }

        int Graph::get_out_degree(int v) const
        {
            if (v < 0 || v >= V)
            {
                std::cerr << "Error: Vertex out of bounds (out_degree)" << std::endl;
                return -1;
            }
            return adj[v].size(); // Out-degree is simply the size of the adjacency list
        }
        
        bool Graph::isEulerian() const
        {
            // Find a vertex with a non-zero degree to start DFS
            int start_node = -1;
            for (int i = 0; i < V; ++i) {
                if (degree(i) > 0) {
                    start_node = i;
                    break;
                }
            
            }

            // The graph has no edges, it's connected trivially
            if (start_node == -1) {
                return true;
            }   

            if (directed){
                for(int j = 0; j < V; j++){
                    if(get_in_degree(j) != get_out_degree(j)){
                        return false;
                    }
                }
            } else{
                // For undirected graph, check if all vertices have even degree
                for (int i = 0; i < V; ++i)
            {
                if (degree(i) % 2 != 0) // If any vertex has an odd degree
                    return false;
            }

            }

            std::vector<bool> visited(V, false);

            // Run DFS from the first non-isolated vertex
            DFS(start_node, visited, *this);

            // Check if all non-isolated vertices were visited
            for (int i = 0; i < V; ++i) {
                if (directed){
                    if(get_out_degree(i) > 0 && !visited[i]){
                        return false;
                    }
                } else {
                    if (degree(i) > 0 && !visited[i]) {
                        return false; // Found an unvisited non-isolated vertex
                    }
            
                }
            }
            
            return true;
        }

        void Graph::findEulerCircuit(std::ostream& out){
            
            if (!isEulerian()) {
                out << "The graph is not Eulerian." << std::endl;
                return;
            }

            std::vector<std::vector<std::pair<int, int>>> tempAdj = adj; // Create a copy of the adjacency list
            std::vector<int> circuit; // To store the Eulerian circuit
            std::vector<int> stack; // Stack to hold the current path

            int currVertex = 0;

            // Start from the first vertex with edges
            for (int i = 0; i < V; ++i) {
                    if (directed) {
                        if (get_out_degree(i) > 0) {
                            currVertex = i;
                            break;
                        }
                    } else {
                        if (degree(i) > 0) {
                            currVertex = i;
                            break;
                        }
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
                    int nextVertex = tempAdj[currVertex].back().first;
                    // Remove the edge from the graph
                    tempAdj[currVertex].pop_back();
                    
                    // For undirected graph, remove the reverse edge as well
                    if (!directed) {
                        auto it = std::find_if(tempAdj[nextVertex].begin(), tempAdj[nextVertex].end(),
                                       [&](const auto& pair) { return pair.first == currVertex; });
                        if (it != tempAdj[nextVertex].end()) {
                            tempAdj[nextVertex].erase(it);
                        }

                    }
                    currVertex = nextVertex;
                }
            }

            // Print the Eulerian circuit
            out << "Eulerian Circuit: ";
            for (auto it = circuit.rbegin(); it != circuit.rend(); ++it) {
            out << *it << " ";
            }
            out << std::endl;
        }
    


void DFS(int start_node, std::vector<bool>& visited, const Graph& graph) {
    // Mark the current node as visited
    visited[start_node] = true;
    std::cout << start_node << " "; // Print the node

    // Iterate through all neighbors of the current node
    for (std::pair<int,int> neighbor : graph.getNeighbors(start_node)) {
        //  Recursively call DFS for unvisited neighbors
        if (!visited[neighbor.first]) {
            DFS(neighbor.first, visited, graph);
        }
    }
}


