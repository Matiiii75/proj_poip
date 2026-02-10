#pragma once 

#include "LocalSearchMethods.hpp"

struct TabuSearch : LocalSearchMethods {

    int tabuSize; // max prod tabu list should contain
    std::queue<int> tabuQ; // tabu list
    std::vector<bool> isInTabu; // to check falsty if a prod is in tabuQ

    TabuSearch(const Data& _data, Solution& _sol, int _solVal); 

    void addTabu(int prod); 

    void resetTabu(); 

    void optimize();

    // TESTS RECUIT

    int genRandomNumber(int a, int b, std::mt19937& g); 

    double uniformRandom(std::mt19937& g); 

    std::vector<std::vector<int>> initFamToProdVector(const Data& data);

    int getRandomProdInFam(int fam, const Data& data, std::mt19937& g, const std::vector<std::vector<int>>& famToProdVec); 

    int getRandomPos(int currPos, int fmin, int fmax, std::mt19937& g); 

    bool metropolis(int Delta, double temp, std::mt19937& g); 

    void displayInfos(int iter, double temp); 

    void debug(std::string s); 

    double initTemp(const std::vector<std::vector<int>>& famToProdVec, std::mt19937& g); 

    void optiRecuit(double initialTemp); 

}; 