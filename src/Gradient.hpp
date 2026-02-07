#pragma once 

#include "GreedyMethods.hpp" 
#include "Datas.hpp"
#include "greedy.hpp"

struct orderAndRacks {

    std::vector<std::vector<int>> orderToRacks;
    std::vector<std::vector<int>> orderOccurRacks; 
    void computeOrderAndRacks(const Solution& sol, const Data& data); // construction orderToRacks & orderOccurRakcs
    
};

/**
 * @brief contains aisles informations such as : 
 * @param rackToAisle associating to each rack it's aisle
 * @param aisleCap associating to each aisle it's capacity
 * @param AisleAer to each aisle it's aeration
 * @param aisleRemainCap to each aisle, associate it's remaining capacity for a given solution
 * @note comes with methods to initiate thoses datas properly from given data and/or given solution
 */
struct aisleInfos {

    std::vector<int> rackToAisle; 
    std::vector<int> aisleCap; 
    std::vector<int> aisleAer; 
    std::vector<int> aisleRemainCap; 

    void computeRackToAisle(const Data& data); 
    void computeAisleCap(const Data& data); 
    void computeAisleAer(const Data& data); 
    void computeAisleRemainCap(const Data& data, const Solution& sol); 

    bool aisleAerEnough(int aisle); 

}; 

/**
 * @brief memorize movement 
 * @note can encode either a swap (if both prod1 and prod2 != -1)
 * @note or a send, consisting of simply sending prod1 to rack2 (wich mean prod2 = -1)
 * @note Default constructor initialize BestSwap with delta = 0, and racks/prods = -1
 */
struct BestSwap {

    int bestDelta; // register delta associated to swap
    std::pair<int,int> prod1ToProd2; // products to move (either swap or send).prod2 = -1 if it's a send 
    std::pair<int,int> rack1ToRack2; // origin rack and final rack 
    BestSwap() : bestDelta(0), prod1ToProd2({-1,-1}), rack1ToRack2({-1,-1}) {} // default constructor 

}; 

/**
 * @brief memorize family's intervals of defintion and order in solution
 * @param famOrder memorize family's order in solution (famOrder[i] = family at pos(i) in solution)
 * @param famIntervals memorize min rack and max rack occupied by each family
 * @param defIntervals memorize min rack and max rack a product of each family can be send 
 * @note comes with two function initiating the attributs of the structs
 * @note need to provide problem data and current solution treating 
 * @warning in @param computeFamInterFamOrd families are added by push_back in @param famOrder. 
 * @warning wich mean this function can be called only one time on a single declared object of famSolInfos
 */
struct famSolInfos {

    std::vector<int> famOrder; 
    std::vector<std::pair<int,int>> famIntervals; 
    std::vector<std::pair<int,int>> defIntervals; 

    void computeFamInterFamOrd(const Data& data, const Solution& sol); 
    void computeDefIntervals(const Data& data, const Solution& sol); 

}; 

/**
 * @brief check if rack still has capacity
 * @param data problem data 
 * @param sol solution we evalute on
 * @param rack rack we want to check if it still has at least 1 of capacity
 * @return true if it has enough capacity, else false 
 */
bool rackCapEnough(const Data& data, const Solution& sol, int rack); 

/**
 * @brief seek a product of given family in given rack of solution
 * @param rack rack we're seeking in
 * @param fam family of product we're seeking 
 * @param sol current solution we are treating 
 * @param data problem data
 * @return id of product found, -1 if no product of given family in rack 
 */
int findFamProdInRack(int rack, int fam, const Solution& sol, const Data& data); 

/**
 * @brief find both racks surrounding given one for given solution
 * @param rack rack we're searching surrounding racks 
 * @param order current order 
 * @param oAr structure containing order and racks relations 
 * @param data problem data
 * @return pair where .first = predecessor rack and .second = successor rack
 * @note if rack don't have predecessor, return begin depot
 * @note if rack don't have successor, return final depot
 * @note can find surroundings of "rack" even if "rack" isn't is the set 
 */
std::pair<int,int> findNeighsRack(int rack, int order, const orderAndRacks& oAr, const Data& data); 

/**
 * @brief compute the impact one position has on the solution cost 
 * @brief so, it computes the cost passing by currRack and the cost without passing. 
 * @param order order for wich we compute the cost 
 * @param currRack rack containing the produc
 * @param data problem data
 * @note if we enter the function, it means we're in the case where the product was alone in rack for order 
 */
int computeCurrRackImpact(int order, int currRack, const orderAndRacks& oAr, const Data& data); 

/**
 * @brief compute the gain we would have by occuping a new rack for an order 
 * @param order order we're computing on 
 * @param newRack rack we're about to occupy 
 * @param data problem data
 * @note if we enter this function, we're in the case where newRack wasn't occupied for order 
 */
int computeAddRackImpact(int order, int newRack, const orderAndRacks& oAr, const Data& data); 

/**
 * @brief compute the impact of a very specific movement : deleting a rack of the solution and adding a new one instead by moving a product
 * @param order order ID 
 * @param rack1 current position we are about to erase 
 * @param rack new position we are considering 
 * @param oAr orders and rack structure 
 * @param data problem data
 * @note take particulary attention to the case we're moving a product 
 * to a position nearer than it's neighbors in current solution
 */
int computeSpecialCase(int order, int rack1, int rack2, const orderAndRacks& oAr, const Data& data); 

/**
 * @brief compute delta induced by swapping two products 
 * @param prod1 product 1 
 * @param prod2 product 2
 * @param data problem data
 * @param oAr order and aisle relative structure
 * @param prod1Rack rack of product 1 
 * @param prod2Rack rack of product 2 
 * @return delta wich is value of gain/loss related to swapping prod1 & prod2
 * @note if delta > 0 : improvement
 * @note if delta < 0 : worsening 
 */
int computeDeltaSwap(int prod1, int prod2, const Data& data, const orderAndRacks& oAr, int prod1Rack, int prod2Rack); 

/**
 * @brief compute delta induced by sending product to a new rack
 * @param prod1 product we are sending 
 * @param prod1Rack rack of product 1
 * @param newRack rack we're sending prod1 in
 * @param data problem data
 * @param oAr order and aisle relative structure 
 * @return delta wich is value of gain/loss related to sending prod1 to newRack
 * @note if delta > 0 : improvement 
 * @note if delta < 0 : worsening 
 */
int computeDeltaSend(int prod1, int prod1Rack, int newRack, const Data& data, const orderAndRacks& oAr);  

/**
 * @brief apply move registered 
 * @param bestSwap structure encoding best swap/send registered 
 * @param bestSol solution we're working on 
 * @param bestVal solution value 
 * @param fsi families informations relative to current solution
 * @param aInfos aisle informations relative to current solution
 * @param oAr order and aisle relative structure 
 * @param data problem data 
 * @note bestSol, bestVal, fsi, aInfos, oAr are given by reference so, 
 * @note they will be updated in this function
 */
void applyMove(const BestSwap& bestSwap, Solution& bestSol, 
    int& bestVal, famSolInfos& fsi, aisleInfos& aInfos, 
    orderAndRacks& oAr, const Data& data); 

/**
 * @brief compute local search with best improvement policy 
 * @param data problem data
 * @param bestSol initial solution
 * @param bestVal initial solution value
 * @note bestSol & bestVal must be pre-computed 
 * @note results will be passed by reference on bestSol & bestVal 
 * @note no stopping criterion except "no improvement found over one iteration"
 */
void GradientBestImprov(const Data& data, Solution& bestSol, int& bestVal); 

/**
 * @brief compute local search with best improvement policy 
 * @param data problem data 
 * @param bestSol initial solution
 * @param bestVal initial solution value 
 * @note bestSol & bestVal must be precomputed 
 * @note results will be passed by reference on bestSol & bestVal 
 * @note no stopping criterion except "no improvement found over on iteration"
 */
void firstImprovLocalSearch(const Data& data, Solution& bestSol, int& bestVal); 

