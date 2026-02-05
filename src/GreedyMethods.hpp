#pragma once 

#include "Datas.hpp" 

int computeSolValue(const Solution& sol, const Data& data); 

void shuffleVector(std::vector<int>& vec, std::mt19937& g); 

std::vector<int> genRandomOrderFam(const Data& data); 