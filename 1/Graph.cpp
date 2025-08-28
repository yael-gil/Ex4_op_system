#include <vector>
#include <iostream>



class Graph
{
    int V; // Number of vertices
    std::vector<std::vector<int>> adj; // Adjacency list representation

    public:

        Graph(int V) : V(V), adj(V) {}

        void addEdge(int u, int v)
            {
                if (u < 0 || u >= V || v < 0 || v >= V)
                {
                    std::cerr << "Error: Vertex out of bounds (addEdge)" << std::endl;
                    return;
                }

                adj[u].push_back(v);
                adj[v].push_back(u); // For undirected graph
            }

        const std::vector<int>& getNeighbors(int v) const
            {
                if(v < 0 || v >= V)
                {
                    std::cerr << "Error: Vertex out of bounds (getNeighbros)" << std::endl;
                    static const std::vector<int> empty; // Return an empty vector on error
                    return empty;
                }
                return adj[v];
            }

        int getNumVertices() const
            {
                return V;
            }

        bool isEdgeConnected(int u, int v) const
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
        
        void printGraph() const
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
        
        int degree(int v) const
        {
            if (v < 0 || v >= V)
            {
                std::cerr << "Error: Vertex out of bounds (degree)" << std::endl;
                return -1;
            }
            return adj[v].size();
        }
        
            

};
