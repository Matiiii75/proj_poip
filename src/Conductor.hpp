#pragma once 

#include "GradientDescent.hpp" // call every other files in the profect except MILP
#include "SimulatedAnnealing.hpp"

struct Conductor {

    Data data; 
    Solution solution; 
    int val;
    orderAndRacks oAr; 
    aisleInfos aInfos; 
    famSolInfos fsi; 

    Conductor(const Data& _data); // constructor 

    // method to display informations for Conductor class methods 
    void displayCalling(const std::string& call); 

    /**
     * @brief compute oar, aInfos, fsi 
     * @warning solution attribute must be initialized 
     */
    void computeArgs(); 

    /**
     * @brief copy current solution's "rackToProd" informations
     * only from racks "f1min" to "f2max"
     * @param f1min from rack
     * @param f2max to rack
     * @note access "solution" attribute of conductor class
     * @warning modify "solution" attribute of conductor class
     */
    std::vector<std::vector<int>> copyRackToProd(int f1min, int f2max); 

    /**
     * @brief compute the order of products encoded between two rack
     * @param f1min 1st rack containing the products we compute the order on
     * @param f2max last rack containing the products we compute the order on
     * @param prodOrder the vector the will contain the order computed
     * @return the ID of the 1st product of f1 
     * @note prodOrder is modified by reference passed in argument 
     * @warning prodOrder must be declared outside this function 
     */
    int computeProdOrder(int f1min, int f2max, std::vector<int>& prodOrder);

    /**
     * @brief place products according order encoded in prodOrder vector
     * @param f1min, f2max define the interval bounds on wich we modify the order of product
     * @param prodOrder encoded order 
     * @note shall modify "solution" attribute of Conductor class, but isn't udpating
     * "oAr" attribute wich mean if this product placement is optimal, we will have to call
     * *INSERER NOM FONCTION POUR MAJ oAr* function
     * @warning modify "solution" attribute of Conductor class 
     */
    void placeProducts(int f1min, int f2max, const std::vector<int>& prodOrder); 

    /**
     * @brief call initSolRandom function from "greedy.hpp" in order to compute
     * a random initial solution
     */
    void callInitRandomSolution(); 

    /**
     * @brief call initSol function from "greedy.hpp" in order to compute 
     * an initial solution 
     * @note the initial solution obtained will follow ascending ordering of familly
     * and left-aligned products 
     */
    void callInitSol();   

    /**
     * @brief call coiGreedy function from "greedy.hpp" in order to compute 
     * an initial solution
     * @note the initial solution obtained will follow "cube order index" heuristic 
     */
    void callInitSolCoi(); 

    /**
     * @brief create and execute Simulated Annealing algorithm in order 
     * to optimize products placing on current "solution" attribute of Conductor Class
     * @note do not optimize on Familly ordering
     */
    void SimulatedAnnealing(); 

    /**
     * @brief create and execute First Improvement Local Search algorithm in order 
     * to optimize products placing on current "solution" attribute of Conductor Class
     * @note do not optimize on Familly ordering
     */
    void FILS(); 

}; 




