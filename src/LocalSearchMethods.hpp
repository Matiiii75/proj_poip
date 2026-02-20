#pragma once 

#include "Datas.hpp"
#include "greedy.hpp"
#include "Common.hpp"

/**
 * @brief contains orders and racks relatives informations 
 * @param orderToRacks associating racks used to satisfy each order
 * @param orderOccurRacks tells how much time we use a certain rack for each order
 * @param computeOrderAndRacks method that compute the two attributs described above from 
 * a given solution and the problem data
 */
struct orderAndRacks {

    std::vector<std::vector<int>> orderToRacks;
    std::vector<std::vector<int>> orderOccurRacks; 
    void computeOrderAndRacks(const Solution& sol, const Data& data); 
    
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


struct LocalSearchMethods {

    const Data& data; 
    Solution& bestSol; 
    int& bestVal; 
    orderAndRacks& oAr; 
    aisleInfos& aInfos; 
    famSolInfos &fsi; 

    LocalSearchMethods(const Data& _data, Solution& _sol, int& _solVal, orderAndRacks& _oAr, aisleInfos& _aInfos, famSolInfos& _fsi); 

    Solution getSolution() const; 

    /**
     * @brief return bestVal
     */
    int getSolValue() const; 

    /**
     * @brief check if enough capacity in rack 
     * @param rack rack we check the capacity for 
     * @param return true if enough capacity, else false
     */
    bool rackCapEnough(int rack) const; 
    
    /**
     * @brief search if there exists another product of familly fam in rack rack
     * @param rack rack we are looking in
     * @param fam fam we are seeking 
     * @return product ID, or -1 if not found 
     */
    int findFamProdInRack(int rack, int fam) const; 

    /**
     * @brief return the two racks surrounding a given rack in a given order
     * @param rack rack we search the surrounds 
     * @param order order we consider 
     * @return a pair where .first is the left rack and .second is the right rack. 
     * @note .first is set on 0 (depot rack) and .second is set on nbRack -1 (final rack) by default
     */
    std::pair<int,int> findNeighsRack(int rack, int order) const;

    /**
     * @brief compute the impact one position has on the solution cost 
     * @brief so, it computes the cost passing by currRack and the cost without passing. 
     * @param order order for wich we compute the cost 
     * @param currRack rack containing the product
     * @note if we enter the function, it means we're in the case where the product was alone in rack for order 
     */
    int computeCurrRackImpact(int order, int currRack) const;

   /**
     * @brief compute the gain we would have by occuping a new rack for an order 
     * @param order order we're computing on 
     * @param newRack rack we're about to occupy 
     * @note if we enter this function, we're in the case where newRack wasn't occupied for order 
     */
    int computeAddRackImpact(int order, int newRack) const;

    /**
     * @brief compute the impact of a very specific movement : deleting a rack of the solution and adding a new one instead by moving a product
     * @param order order ID 
     * @param rack1 current position we are about to erase 
     * @param rack new position we are considering 
     * @note take particulary attention to the case we're moving a product 
     * to a position nearer than it's neighbors in current solution
     */
    int computeSpecialCase(int order, int rack1, int rack2) const; 

    /**
     * @brief compute delta induced by swapping two products 
     * @param prod1 product 1 
     * @param prod2 product 2
     * @param prod1Rack rack of product 1 
     * @param prod2Rack rack of product 2 
     * @return delta wich is value of gain/loss related to swapping prod1 & prod2
     * @note if delta > 0 : improvement
     * @note if delta < 0 : worsening 
     */
    int computeDeltaSwap(int prod1, int prod2, int prod1Rack, int prod2Rack)  const; 

    /**
     * @brief compute delta induced by sending product to a new rack
     * @param prod1 product we are sending 
     * @param prod1Rack rack of product 1
     * @param newRack rack we're sending prod1 in 
     * @return delta wich is value of gain/loss related to sending prod1 to newRack
     * @note if delta > 0 : improvement 
     * @note if delta < 0 : worsening 
     */
    int computeDeltaSend(int prod1, int prod1Rack, int newRack) const;  

    /**
     * @brief apply move registered 
     * @param bestSwap structure encoding best swap/send registered 
     */
    void applyMove(const BestSwap& bestSwap);

    virtual void optimize() = 0; // optimize will launch metaheuristics methods in herited structs
}; 