#pragma once 

#include "LocalSearchMethods.hpp"

struct TabuSearch : LocalSearchMethods {

    int tabuSize; // max prod tabu list should contain
    std::queue<int> tabuQ; // tabu list
    std::vector<bool> isInTabu; // to check falsty if a prod is in tabuQ

    TabuSearch(const Data& _data, const Solution& _sol, int _solVal); 

    void addTabu(int prod); 

    void resetTabu(); 

    void optimize();

}; 