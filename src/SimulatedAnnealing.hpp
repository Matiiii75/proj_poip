#pragma once 

#include "LocalSearchMethods.hpp"

struct SAA : LocalSearchMethods {



    SAA(const Data& _data, Solution& _sol, int& _solVal, 
        orderAndRacks& _oAr, aisleInfos& _aInfos, famSolInfos& _fsi);  
    
    /**
     * @brief get random integer in [a,b]
     */
    int genRandomNumber(int a, int b, std::mt19937& g); 
    
    /**
     * @brief get random float in [0,1]
     */
    double uniformRandom(std::mt19937& g); 

    /**
     * @brief initiate structure to faslty check product associated to famillies 
     */
    std::vector<std::vector<int>> initFamToProdVector(const Data& data);

    /**
     * @brief randomly get a product of given familly
     */
    int getRandomProdInFam(int fam, const Data& data, std::mt19937& g, const std::vector<std::vector<int>>& famToProdVec); 

    /**
     * @brief randomly generate a new position (rack) to send or swap a product
     */
    int getRandomPos(int currPos, int fmin, int fmax, std::mt19937& g); 
    
    /**
     * @brief function that check metropolis criterion to decide 
     * if we keep a worsening solution based on temperature
     * @return boolean = "true" if we keep, "false" if we reject
     */
    bool metropolis(int Delta, double temp, std::mt19937& g); 
    
    /**
     * @brief can be call at each iteration of SAA to visualize
     * the evolution of solution quality 
     */
    void displayInfos(int iter, double temp); 
    
    /**
     * @brief simple display function used for debugging
     */
    void debug(std::string s); 

    /**
     * @brief initalize temperature based on algorithm described in mathematic litterature 
     */
    double initTemp(const std::vector<std::vector<int>>& famToProdVec, std::mt19937& g);  
    
    /**
     * @brief start optimizing a solution with simulated annealing meta heuristic 
     */
    void optimize();

}; 