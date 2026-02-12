#pragma once 

#include "GradientDescent.hpp" // call every other files in the profect except MILP
#include "SimulatedAnnealing.hpp"

struct Conductor {

    Data data; 
    Solution solution; 
    int val;
    orderAndRacks oAr; 
    aisleInfos aInfos; 
    famSolInfos fsi; 

    Conductor(const Data& _data); 

    void displayCalling(const std::string& call); 

    /**
     * @brief compute oar, aInfos, fsi 
     * @warning solution attribute must be initialized 
     */
    void computeArgs(); 

    // todo : coder 3 méthodes relative a swap FAM + swap fam

    std::vector<std::vector<int>> copyRackToProd(int f1min, int f2max); 
    int computeProdOrder(int f1min, int f2max, std::vector<int>& prodOrder);
    void placeProducts(int f1min, int f2max, const std::vector<int>& prodOrder); 


    void callInitRandomSolution(); 
    void callInitSol();     // initSol methods 
    void callInitSolCoi(); 

    void SimulatedAnnealing(); 
    void FILS(); 

}; 




