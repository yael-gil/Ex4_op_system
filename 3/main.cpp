#include <iostream>
#include "Graph.hpp"
#include <random>
#include <unistd.h>

Graph generateRandomGraph(int V, int E, int seed) {

    srand(seed);

    Graph g(V);
   
    int edgesAdded = 0;
    while (edgesAdded < E) {
        int u = rand() % V; // 0 to V-1
        int v = rand() % V;
        
        if (u != v && !g.isEdgeConnected(u, v)) { // Avoid self-loops and duplicate edges
            g.addEdge(u, v);
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

    while ((opt = getopt(argc, argv, "v:e:s:")) != -1) {
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

    if (E > V * (V - 1) / 2) {
        std::cerr << "Error: Too many edges for the number of vertices." << std::endl;
        return 1;
    }

    Graph g = generateRandomGraph(V, E, seed);
    g.printGraph();

    if (g.isEulerian()) {
        std::cout << "The graph is Eulerian." << std::endl;
        g.findEulerCircuit();
    } else {
        std::cout << "The graph is not Eulerian." << std::endl;
    }

    return 0;
}