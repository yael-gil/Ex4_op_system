
#ifndef FACTORY_HPP
#define FACTORY_HPP

#include "Algorithms.hpp"
#include "MST.hpp"
#include "SCC.hpp"

class AlgorithmFactory {
public:
    /**
     * @brief Creates an algorithm instance based on the given name.
     * @param name The name of the algorithm to create.
     * @return A pointer to the created algorithm instance, or nullptr if the name is invalid.
     * @throws std::invalid_argument if the algorithm name is not recognized.
     */
    static Algorithms* createAlgorithm(const std::string& name) {
        if (name == "MST")
            return new MST();
        else if (name == "SCC")
            return new SCC();
        else
            return nullptr;
    }
};

#endif 