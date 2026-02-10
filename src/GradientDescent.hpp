#pragma once 

#include "LocalSearchMethods.hpp"

// GradientDesenct inherite from LocalSearchMethods
struct GradientDescent : LocalSearchMethods {

   GradientDescent(const Data& _data, Solution& _sol, int _solVal)
   : LocalSearchMethods(_data, _sol, _solVal) {}  

   void optimize(); 

}; 