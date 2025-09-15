#ifndef SCC_HPP
#define SCC_HPP

#include "Algorithms.hpp"
#include "Graph.hpp"
#include <vector>
#include <string>

class SCC : public Algorithms {
public:
    ~SCC() = default;
    /**
     * @brief Activate the SCC algorithm on the given graph.
     * @param g The graph to process.
     * @return A string containing the result of the SCC algorithm.
     */
    std::string run(Graph& g) override;

    /**
     * @brief Find the strongly connected components (SCCs) in the given graph.
     * @param g The graph to process.
     * @return A string containing the SCCs found in the graph.
     */
    std::vector<std::vector<int>> findSCC(Graph& g);
};

#endif // SCC_HPP