#pragma once 

#include "Datas.hpp"
#include "GreedyMethods.hpp"
#include <map>

struct Coi{

    
    /* ATTRIBUTS */

    std::vector<std::vector<std::pair<int,double>>> coiProd;   // for each family, contains produts coi
    std::vector<std::pair<int,double>> coiFam; // contains families coi


    /* METHODS */

    /**
    * @brief struct constructor that initiate the different coi
    * @param data struct containing of instance
    */
    Coi(const Data& data); 
};

/**
* @brief greedy algorithm based on the COI principle
* @param data struct containing of instance
*/
Solution coiGreedy(const Data& data);
Solution coiGreedyAmelioration(const Data& data);

Solution initSol(const Data& data);

Solution initSolRandom(const Data& data);
