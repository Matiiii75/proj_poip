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
* @brief randomly generate ordering of family  
*/
std::vector<int> genRandomOrderFam(const Data& data); 

/**
* @brief add element in increasing order sorted vector
*/
void addSortedVec(int x, std::vector<int>& v); 


/**
* @brief erase element of increasing order sorted vector
*/
void eraseSortedVec(int x, std::vector<int>& v); 

/**
* @brief generate random integer in [a,b]
* @param g random seed generator 
*/
int generateRandomInt(std::mt19937& g, int a, int b); 