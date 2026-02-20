#pragma once 

#include "LocalSearchMethods.hpp"

// GradientDesenct inherite from LocalSearchMethods
struct GradientDescent : LocalSearchMethods {

   std::chrono::steady_clock::time_point start; 

   GradientDescent(const Data& _data, Solution& _sol, int& _solVal, 
      orderAndRacks& _oAr, aisleInfos& _aInfos, famSolInfos& _fsi); 
   
   /**
    * @brief start optimizing the problem with first improvement local search 
    */
   void optimize(); 

   void startTimer(); 

   double getTime(); 

}; 