#pragma once 

#include "LocalSearchMethods.hpp"

// GradientDesenct inherite from LocalSearchMethods
struct GradientDescent : LocalSearchMethods {

   GradientDescent(const Data& _data, Solution& _sol, int& _solVal, 
      orderAndRacks& _oAr, aisleInfos& _aInfos, famSolInfos& _fsi); 

   void optimize(); 

}; 