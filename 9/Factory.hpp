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



#endif