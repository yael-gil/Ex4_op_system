#ifndef HAMILTON_HPP            // Include guard start: prevent multiple inclusion
#define HAMILTON_HPP            // Define the header guard macro

#include "Algorithms.hpp"               // Base interface for algorithms (declares virtual run)
#include "Graph.hpp"                    // Graph API: adjacency, neighbors, vertex count, etc.
#include <vector>                       // std::vector container
#include <sstream>                      // std::ostringstream for building string output

class Hamilton : public Algorithms { // Concrete algorithm class deriving from Algorithms
private:
    std::vector<int> path;              // Stores a found Hamiltonian circuit (ordering of vertices)
    bool found;                         // Flag indicating whether a Hamiltonian circuit was found
    
    bool isSafe(int v, const Graph& g, const std::vector<int>& currentPath, int pos) {
        // Checks if vertex 'v' can be placed at position 'pos' in 'currentPath'
        
        // Ensure 'v' is adjacent to the previously placed vertex (to maintain a path)
        if (pos > 0 && !g.isEdgeConnected(currentPath[pos - 1], v)) {
            return false;               // Not adjacent → cannot place 'v' here
        }
        
        // Ensure 'v' does not already appear earlier in the path (no repeated vertices)
        for (int i = 0; i < pos; i++) {
            if (currentPath[i] == v) {
                return false;           // Duplicate vertex → not safe
            }
        }
        
        return true;                    // Passed all checks → safe to place 'v'
    }
    
    bool hamiltonianUtil(const Graph& g, std::vector<int>& currentPath, int pos) {
        int V = g.getNumVertices();     // Number of vertices in the graph
        
        // Base case: all positions are filled with vertices (a Hamiltonian path candidate)
        if (pos == V) {
            // To be a circuit, last vertex must connect back to the first
            if (g.isEdgeConnected(currentPath[pos - 1], currentPath[0])) {
                path = currentPath;     // Save the completed Hamiltonian circuit
                return true;            // Signal success
            }
            return false;               // Not a circuit (no closing edge) → fail this branch
        }
        
        // Try every vertex as a candidate for the next position in the path
        for (int v = 0; v < V; v++) {
            if (isSafe(v, g, currentPath, pos)) { // Check placement constraints
                currentPath[pos] = v;  // Place vertex 'v' at position 'pos'
                
                // Recurse to place the next vertex
                if (hamiltonianUtil(g, currentPath, pos + 1)) {
                    return true;       // If any branch succeeds, bubble up success
                }
                
                // Backtrack: undo placement at 'pos' and try another vertex
                currentPath[pos] = -1;
            }
        }
        
        return false;                   // Exhausted all candidates for this 'pos' → no solution here
    }

public:
    std::string run(Graph& g) override { // Entry point defined by the Algorithms interface
        int V = g.getNumVertices();     // Get vertex count
        
        if (V == 0) {                   // Handle empty graph edge-case
            return "Hamilton Circuit: Empty graph\n";
        }
        
        path.clear();                   // Reset any previous result
        found = false;                  // Reset found flag
        std::vector<int> currentPath(V, -1); // Initialize path with sentinel values (-1)
        
        // Fix the start of the path at vertex 0 (symmetry breaking to reduce duplicates)
        currentPath[0] = 0;
        
        if (hamiltonianUtil(g, currentPath, 1)) { // Start search from position 1
            found = true;              // Mark success if a circuit was found
        }
        
        std::ostringstream result;     // Prepare textual result
        if (found) {
            result << "Hamilton Circuit found:\n"; // Header line
            result << "Path: ";        // Label for the path
            for (int i = 0; i < V; i++) {
                result << path[i];     // Output each vertex in order
                if (i < V - 1) result << " -> "; // Arrow between vertices
            }
            result << " -> " << path[0] << "\n"; // Close the cycle by returning to start
            
            // Optionally compute total weight (if the graph stores positive weights)
            int totalWeight = 0;       // Accumulates total edge weight along the circuit
            bool hasWeights = false;   // Tracks whether weights are present/positive
            for (int i = 0; i < V; i++) {
                int from = path[i];    // Current vertex
                int to = path[(i + 1) % V]; // Next vertex (wrap to start at the end)
                
                // Look for the edge (from -> to) among neighbors to read its weight
                for (const auto& neighbor : g.getNeighbors(from)) {
                    if (neighbor.first == to) {          // Found the target neighbor
                        totalWeight += neighbor.second;   // Accumulate edge weight
                        if (neighbor.second > 0) hasWeights = true; // Mark that weights exist
                        break;                            // Stop scanning neighbors of 'from'
                    }
                }
            }
            
            if (hasWeights) {          // Only print weight summary if weights are meaningful
                result << "Total circuit weight: " << totalWeight << "\n";
            }
        } else {
            result << "No Hamilton Circuit found\n"; // Report failure to find a circuit
        }
        
        return result.str();           // Return the assembled result string
    }
};

#endif                                   // End of include guard
