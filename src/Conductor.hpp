#pragma once 

#include "GradientDescent.hpp" // call every other files in the profect except MILP
#include "SimulatedAnnealing.hpp"
#include "FamilliesTSP.hpp"

struct Conductor {

    Data data; 
    Solution solution; 
    int val;
    orderAndRacks oAr; 
    aisleInfos aInfos; 
    famSolInfos fsi;

    // time attributes
    std::chrono::steady_clock::time_point start; 
    double finalTime; 

    Conductor(const Data& _data); // constructor 

    // display debug prints in terminal
    void debug(const std::string& s); 

    // method to display informations for Conductor class methods 
    void displayCalling(const std::string& call); 

    // method to display results informations
    void displayResults(); 

    /**
     * @brief compute oar, aInfos, fsi 
     * @warning solution attribute must be initialized 
     */
    void computeArgs(); 

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
     * @brief call comptuteTSP & fillSolution from "FamilliesTSP.hpp" in order to compute 
     * an initial solution with optimal famillies ordering
     */
    void callInitSolTSP(); 

    /**
     * @brief call GreedyBestInsert function from "greedy.hpp" in order to compute 
     * an initial solution based on best insertion policy
     */
    void calInitSolBestInsert(); 

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

    /**
     * @brief initiate solution & call meta-heuristic choosen from standart input
     * by the user
     */
    void executeUserChoice(); 

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
     * @brief compute the order of products encoded between two rack and return the 1rst product of fam1
     * @param fam1, fam2 family of the products we compute the order on
     * @param prodOrder the vector the will contain the order computed
     * @return the ID of the 1st product of f1. Return -1 if f1 doesnt have first product (wich should not happen)
     * @note prodOrder is modified by reference passed in argument 
     * @warning prodOrder must be declared outside this function 
     */
    int computeProdOrder(int fam1, int fam2, std::vector<int>& prodOrder);

    /**
     * @brief place products according order encoded in prodOrder vector
     * @param fam1, fam2 family of the products we place
     * @param f1min, f2max define the interval bounds on wich we modify the order of product
     * @param prodOrder encoded order 
     * @note shall modify "solution" attribute of Conductor class, but isn't udpating
     * "oAr" attribute wich mean if this product placement is optimal, we will have to call
     * updateOar function
     * @warning modify "solution" attribute of Conductor class 
     */
    void placeProducts(int fam1, int fam2, int f1min, int f2max, const std::vector<int>& prodOrder); 

    /**
     * @brief update struct fsi after a swap
     * @param famId1, famId2 position of family in the solution before swap
     * @param FirstProdF1 1rst prod place of fam1
     * @param prodOrder order of the product of the solution after swap
     */ 
    void updateFsi(int famId1, int famId2, int FirstProdF1, const std::vector<int>& prodOrder);

    /**
     * @brief  update struct oAr after a swap
     * @param fam1, fam2 family we swap
     * @param f1min, fam2 bounds of the part of the solution we change
     * @param copyRackToProd copy of the solution before the swap
     */
    void updapteOar(int fam1, int fam2, int f1min, int f2max, const std::vector<std::vector<int>>& copyRackToProd);

    /**
     * @brief make the swap and return true if we actually supposed to do it or false if we need to repare the sol
     * @param fam1, fam2 family we swap
     * @param f1min, fam2 bounds of the part of solution we change
     * @param prodOrder order of the products of fam2 and fam1
     */
    bool applyMooveFamily(int fam1, int fam2, int f1min, int f2max, std::vector<int>& prodOrder);

    /**
     * @brief change solution after a swap as hox it was before swap
     * @param fam1, fam2 family we moove
     * @param f1min, f2max bounds of the part of solution we change
     *@param copyRackToProd copy of the solution before the swap
     */
    void repairSolution(int fam1, int fam2, int f1min, int f2max, std::vector<std::vector<int>> copyRackToProd);

    /**
     * @brief swap 2 consecutive family if it a goog idea -> return true and if isn't -> return false
     * @param famId1, famId2 position in solution of families we swap
     */
    bool SwapFamily(int famId1, int famId2); 

    /**
     * @brief call iteratively SAA & SwapFamilly to optimize the solution on product placement with SAA  
     * then on famillies placement with swapFamily
     * @param nbIterMax max number of SAA calls allowed 
     * @return best solution found 
     */ 
    void swapFamSAA(int nbIterMax); 

    /**
     * @brief start timer by attributing a value to "start" attribute of Conductor
     */
    void startTimer(); 

    /**
     * @brief get time passed since last call of startTimer method 
     */
    double stopTimer(); 

}; 




