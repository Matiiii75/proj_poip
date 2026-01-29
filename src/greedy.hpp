#pragma once 

#include "Datas.hpp"
#include <map>

struct Coi{

    
    /* ATTRIBUTS */

    std::map<int,double> coiFam;
    std::map<int,double> coiProd;


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

Solution initSol(const Data& data);