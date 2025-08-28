#include <iostream>
#include "Graph.hpp"
#include <random>
#include <unistd.h>

Graph generateRandomGraph(int V, int E, int seed) {

    srand(seed);

    Graph g(V);
   int count = 0;
    int edgesAdded = 0;
    while (edgesAdded < E) {
        int u = rand() % V;
        int v = rand() % V; //  0 to V-1
        
        if (u != v && !g.isEdgeConnected(u, v)) { // Avoid self-loops and duplicate edges
            g.addEdge(u, v);
            edgesAdded++;
            count++;
            std::cout <<"count:"<< count << std::endl;

        } else {
             std::cout << "Skipping edge (" << u << ", " << v << ") to avoid self-loop or duplicate." << std::endl;
        }


    }
    return g;
}


/*
int main() {
    
    int num_vertices =499; // Choose a number of vertices
    int num_edges = 124251;    // Choose a number of edges that is significantly larger than num_vertices
    int seed = 1;           // Start with a known seed

    // Loop to find an Eulerian graph
    while (true) {
        Graph g = generateRandomGraph(num_vertices, num_edges, seed);
        std::cout << "seed:" << seed << std::endl;

        if (g.isEulerian()) {
            std::cout << "Found an Eulerian graph!" << std::endl;
            std::cout << "To reproduce, use these parameters:" << std::endl;
            std::cout << "./graph_project -v " << num_vertices << " -e " << num_edges << " -s " << seed << std::endl;
            break; // Found a working seed, exit the loop
        }
        
        seed++; // Try the next seed
    }
    return 0;
} */


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