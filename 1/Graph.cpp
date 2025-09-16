#include <vector>
#include <iostream>
									 

			 
																		 


class Graph
{
    int V; // Number of vertices
    bool directed = false; // Directed or undirected graph
    std::vector<std::vector<std::pair<int, int>>> adj; // Adjacency list representation

    public:

        Graph(int V, bool directed) : V(V), adj(V), directed(directed) {}

        void addEdge(int u, int v, int w = 0) // Add edge from u to v
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

                adj[u].push_back({v, w}); 
                
                if(!directed){
                    adj[v].push_back({u, w}); // For undirected graph
                }

            }


   
        const std::vector<std::pair<int, int>>& getNeighbors(int v) const
            {
                if(v < 0 || v >= V)
                {
                    std::cerr << "Error: Vertex out of bounds (getNeighbros)" << std::endl;
                    static const std::vector<std::pair<int, int>> empty; // Return an empty vector on error
                    return empty;
                }
                return adj[v];
            }

        int getNumVertices() const
            {
                return V;
            }

        bool isDirected() const
            {
                return directed;
            }

        bool isEdgeConnected(int u, int v) const // Check if there's an edge from u to v
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

        
        void printGraph() const
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
        
       int degree(int v) const // For undirected graphs
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
        
        int get_in_degree(int v) const
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

        int get_out_degree(int v) const
        {
            if (v < 0 || v >= V)
            {
                std::cerr << "Error: Vertex out of bounds (out_degree)" << std::endl;
                return -1;
            }
            return adj[v].size(); // Out-degree is simply the size of the adjacency list
        }
        
};

																		  
									   
							   
													 

														
														 
														
								 
										  
		 
	 
 
