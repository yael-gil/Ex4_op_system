#ifndef ALGORITHMS_HPP
#define ALGORITHMS_HPP
#include "Graph.hpp"


class Algorithms{
    public:
    ~Algorithms()=default;
    /**
     * @brief Activate the algorithm on the graph.
     * @param g The graph
     * @return A string containing the result of the algorithm.
     */
    virtual std::string run(Graph& g)=0;
};

#endif