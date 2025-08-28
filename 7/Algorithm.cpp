#include <iostream>
#include "Graph.hpp"
#include <vector>

Graph findSCC (const Graph& g){
    
    std::vector<int> stack (g.getNumVertices());
    std::vector<bool> visited(g.getNumVertices(), false);  
    for (int i = 0; i < g.getNumVertices(); i++) {
        if (!visited[i]) {
            DFS(i, visited, g);
            stack.push_back(i);
        }
        //אני באמצע ליצור את הפונקציה. אל תמשיכי אם את לא חייבת
    }

}



