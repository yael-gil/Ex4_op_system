#ifndef FACTORY_HPP
#define FACTORY_HPP

#include "Algorithms.hpp"
#include "Graph.hpp"
#include "MST.hpp"
#include "SCC.hpp"
#include "Hamilton.hpp"
#include "MaxClique.hpp"
#include <memory>
#include <string>
#include <sstream>
#include <iostream>
#include <vector>
#include <random>

struct ClientRequest {
    bool valid = false;
    std::string error;
    std::string algorithm;
    
    // For random graph generation
    int vertices = 0;
    int edges = 0;
    int seed = 0;
    
    // For matrix input
    bool hasMatrix = false;
    std::vector<std::vector<int>> matrix;
};

class AlgorithmFactory {
public:
    static std::unique_ptr<Algorithms> createAlgorithm(const std::string& algorithmName) {
        if (algorithmName == "MST") {
            return std::make_unique<MST>();
        } else if (algorithmName == "SCC") {
            return std::make_unique<SCC>();
        } else if(algorithmName=="HAMILTON"){
            return std::make_unique<Hamilton>();
        }
        else if(algorithmName=="MAXCLIQUE"){
            return std::make_unique<MaxClique>();
        }
        
        return nullptr;
    }
};

// Enhanced Graph class with additional methods needed for algorithms
class EnhancedGraph : public Graph {
public:
    EnhancedGraph(int V, bool directed) : Graph(V, directed) {}
    
    // Create graph from adjacency matrix
    static EnhancedGraph fromMatrix(const std::vector<std::vector<int>>& matrix) {
        int n = matrix.size();
        bool directed = false;
        
        // Check if matrix represents a directed graph
        for (int i = 0; i < n && !directed; i++) {
            for (int j = 0; j < n && !directed; j++) {
                if (matrix[i][j] != matrix[j][i]) {
                    directed = true;
                }
            }
        }
        
        EnhancedGraph g(n, directed);
        
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                if (matrix[i][j] > 0) {
                    // For undirected graphs, only add edge once (i < j)
                    if (!directed && i >= j) continue;
                    g.addEdge(i, j, matrix[i][j]);
                }
            }
        }
        
        return g;
    }
    
    // Generate random graph
    static EnhancedGraph generateRandom(int V, int E, int seed, bool directed = false) {
        EnhancedGraph g(V, directed);
        std::mt19937 gen(seed);
        std::uniform_int_distribution<> vertexDist(0, V - 1);
        std::uniform_int_distribution<> weightDist(1, 100);
        
        int maxEdges = directed ? V * (V - 1) : V * (V - 1) / 2;
        E = std::min(E, maxEdges);
        
        int addedEdges = 0;
        int attempts = 0;
        const int maxAttempts = E * 10;
        
        while (addedEdges < E && attempts < maxAttempts) {
            int u = vertexDist(gen);
            int v = vertexDist(gen);
            
            if (u != v && !g.isEdgeConnected(u, v)) {
                int weight = weightDist(gen);
                g.addEdge(u, v, weight);
                addedEdges++;
            }
            attempts++;
        }
        
        return g;
    }
};

// Function to parse client request
ClientRequest parseRequest(const std::string& request) {
    ClientRequest req;
    std::istringstream iss(request);
    std::string firstLine;
    
    if (!std::getline(iss, firstLine)) {
        req.error = "Empty request";
        return req;
    }
    
    // Parse first line
    if (firstLine.find("ALGORITHM:") == 0) {
        size_t colonPos = firstLine.find(':');
        if (colonPos == std::string::npos) {
            req.error = "Invalid algorithm format";
            return req;
        }
        
        std::string algPart = firstLine.substr(colonPos + 1);
        std::istringstream algStream(algPart);
        std::string token;
        
        while (algStream >> token) {
            if (req.algorithm.empty()) {
                req.algorithm = token;
            } else if (token.find("V:") == 0) {
                req.vertices = std::stoi(token.substr(2));
            } else if (token.find("E:") == 0) {
                req.edges = std::stoi(token.substr(2));
            } else if (token.find("S:") == 0) {
                req.seed = std::stoi(token.substr(2));
            }
        }
        
        // Check if this is a matrix request
        std::string line;
        std::vector<std::vector<int>> matrix;
        
        while (std::getline(iss, line)) {
            if (line.empty()) continue;
            
            std::istringstream lineStream(line);
            std::vector<int> row;
            int val;
            
            while (lineStream >> val) {
                row.push_back(val);
            }
            
            if (!row.empty()) {
                matrix.push_back(row);
            }
        }
        
        if (!matrix.empty()) {
            req.hasMatrix = true;
            req.matrix = matrix;
        }
        
        req.valid = true;
    } else {
        req.error = "Request must start with ALGORITHM:";
    }
    
    return req;
}

// Extended Algorithms base class to support different run methods
class ExtendedAlgorithms : public Algorithms {
public:
    // Original run method (pure virtual)
    virtual std::string run(Graph& g) override = 0;
    
    // Run with matrix
    virtual std::string run(const std::vector<std::vector<int>>& matrix) {
        EnhancedGraph g = EnhancedGraph::fromMatrix(matrix);
        return run(g);
    }
    
    // Run with random graph parameters
    virtual std::string run(int vertices, int edges, int seed) {
        if (vertices <= 0) {
            return "ERROR: Number of vertices must be positive\n";
        }
        if (edges < 0) {
            return "ERROR: Number of edges cannot be negative\n";
        }
        
        EnhancedGraph g = EnhancedGraph::generateRandom(vertices, edges, seed);
        return run(g);
    }
};

// Update all algorithm classes to inherit from ExtendedAlgorithms instead of Algorithms
// (Note: This would require modifying the individual algorithm files to change the inheritance)

#endif