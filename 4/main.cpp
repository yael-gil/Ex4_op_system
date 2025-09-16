#include <iostream>
#include "Graph.hpp"
#include <random>
#include <unistd.h>

Graph generateRandomGraph(int V, int E, int seed, bool directed = false) {

    srand(seed);

    Graph g(V, directed);
   
    int edgesAdded = 0;
    while (edgesAdded < E) {
        int u = rand() % V; // 0 to V-1
        int v = rand() % V;
        int w = rand() % 10 + 1; // Random weight between 1 and 10
        
        if (u != v && !g.isEdgeConnected(u, v)) { // Avoid self-loops and duplicate edges
            g.addEdge(u, v, w);
            edgesAdded++;
        }

    }
    return g;
}

int main(int argc, char *argv[]) {
    
    
    int V;
    int E;
    int seed;
    int opt;
    bool directed = false; // Default to not directed graph

    while ((opt = getopt(argc, argv, "v:e:s:d")) != -1) {
        switch (opt) {
            case 'v':
                V = std::stoi(optarg);
                break;
            case 'e':
                E = std::stoi(optarg);
                break;
            case 's':
                seed = std::stoi(optarg);
                break;
            case 'd':
                directed = true;
                break;
            case '?':
                if (optopt == 'v' || optopt == 'e' || optopt == 's') {
                    std::cerr << "Option -" << static_cast<char>(optopt) << " requires an argument." << std::endl;
                } else {
                    std::cerr << "Unknown option `-" << static_cast<char>(optopt) << "`." << std::endl;
                }
                return 1;
            default:
                abort(); // Stop the program on unexpected errors
        }
    }

    if (V <= 0) {
        std::cerr << "Error: Number of vertices must be positive." << std::endl;
        return 1;
    }
    if (E < 0) {
        std::cerr << "Error: Number of edges cannot be negative." << std::endl;
        return 1;
    }
    if (directed) {
        if (E > V * (V - 1)) {
            std::cerr << "Error: Too many edges for the number of vertices in a directed graph." << std::endl;
            return 1;
        }
    } else {
        if (E > V * (V - 1) / 2) {
            std::cerr << "Error: Too many edges for the number of vertices in an undirected graph." << std::endl;
            return 1;
        }
    }

    Graph g = generateRandomGraph(V, E, seed, directed);
    g.printGraph();

    if (g.isEulerian()) {
        std::cout << "The graph is Eulerian." << std::endl;
        g.findEulerCircuit();
    } else {
        std::cout << "The graph is not Eulerian." << std::endl;
    }

    return 0;
}