#pragma once

#include "Datas.hpp"
#include "greedy.hpp"

Solution fillSolution(const Data& data, const std::vector<int>& famOrder); 
;  
/**
 * @brief display vector containing solution in terminal
 */
void displaySolution(const std::vector<int>& sol); 

/**
 * @brief add dummy familly to correlation matrix. 
 * @note dummy is a node that has 0 correlation with every other node 
 */
void addDummyFam(std::vector<std::vector<int>>& d); 

/**
 * @brief compute solution value, wich consist of suming correlation of adjacents famillies
 */
int computeSolVal(const std::vector<int>& sol, const std::vector<std::vector<int>>& d); 

/**
 * @brief compute delta before movement 
 * @param d distances between each vertices
 * @param sol current solution
 * @param i,j,k index of 3 nodes for whom we broke the entering edge 
 */
int computePreDelta(const std::vector<std::vector<int>>& d, 
    const std::vector<int>& sol, int i, int j, int k); 

/**
 * @brief compute delta after movement 
 * @param d distances between each vertices
 * @param sol current solution
 * @param i,j,k index of 3 nodes for whom we broke the entering edge 
 * @param bestMove returned by reference. ID of considered move
 * @note To compute delta, we compute 3-opt movements. Each of these movements 
 * are classified into 7 classes denoted by : I,...,VII. 
 */
int computePostDelta(const std::vector<std::vector<int>>& d, 
    const std::vector<int>& sol, int i, int j, int k, int& bestMove); 

/**
 * @brief apply bestmove registered for tsp
 * @param sol current solution we are modifying 
 * @param bestMove number ID in [1,7] corresponding of the choosen move
 * @param i,j,k index of 3 nodes for whom we broke the entering edge 
 * @note modify the solution by reference 
 */
void applyMove(std::vector<int>& sol, int bestMove, int i, int j, int k); 

/**
 * @brief take solution found by TSP as an input. erase dummy vertex from the solution
 */
void eraseDummy(std::vector<int>& sol); 

/**
 * @brief compute optimal order of famillies by applying Lin-Kernhigan heuristic to solve a TSP
 * @param distances matrix containing the correlation between each famillies
 * @return vector containing optimal ordering of famillies
 */
std::vector<int> computeTSP(const Data& data); 