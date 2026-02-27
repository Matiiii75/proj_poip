#pragma once 

#include "Datas.hpp" 


/**
* @brief Compute value for a given solution
*/
int computeSolValue(const Solution& sol, const Data& data); 



/**
* @brief shuffle elements contained in vector
* @param vec vector we want to shuffle 
* @param g 
*/
void shuffleVector(std::vector<int>& vec, std::mt19937& g); 



/**
* @brief Randomly generate an ordering of families 
*/
std::vector<int> genRandomOrderFam(const Data& data); 

/**
* @brief Add an element to a sorted vector in increasing order
*/
void addSortedVec(int x, std::vector<int>& v); 


/**
* @brief Erase an element from a sorted vector in increasing order
*/
void eraseSortedVec(int x, std::vector<int>& v); 

/**
* @brief generate random integer in [a,b]
* @param g random seed generator 
*/
int generateRandomInt(std::mt19937& g, int a, int b); 