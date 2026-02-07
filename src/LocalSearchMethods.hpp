#pragma once 

#include "Datas.hpp"
#include "greedy.hpp"
#include "GreedyMethods.hpp"

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


struct LocalSearchMethods {

    const Data& data; 
    Solution bestSol; 
    int bestVal; 
    orderAndRacks oAr; 
    aisleInfos aInfos; 
    famSolInfos fsi; 

    LocalSearchMethods(const Data& _data, const Solution& _sol, int _solVal); 

    Solution getSolution() const; 

    int getSolValue() const; 

    bool rackCapEnough(int rack) const; 
    
    int findFamProdInRack(int rack, int fam) const; 

    std::pair<int,int> findNeighsRack(int rack, int order) const;

    int computeCurrRackImpact(int order, int currRack) const;

    int computeAddRackImpact(int order, int newRack) const;

    int computeSpecialCase(int order, int rack1, int rack2) const; 

    int computeDeltaSwap(int prod1, int prod2, int prod1Rack, int prod2Rack)  const; 

    int computeDeltaSend(int prod1, int prod1Rack, int newRack) const;  

    void applyMove(const BestSwap& bestSwap);

    virtual void optimize() = 0; // optimize will launch metaheuristics methods in herited structs
}; 