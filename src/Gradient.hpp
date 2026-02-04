#pragma once 

#include "GreedyMethods.hpp" 
#include "Datas.hpp"
#include "greedy.hpp"

struct orderAndRacks {

    std::vector<std::set<int>> orderToRacks;
    std::vector<std::vector<int>> orderOccurRacks; 

    // construction orderToRacks & orderOccurRakcs
    void computeOrderAndRacks(const Solution& sol, const Data& data) {

        orderToRacks.resize(data.nbOrd); 
        orderOccurRacks.resize(data.nbOrd, std::vector<int>(data.nbRacks, 0)); 

        for(int o = 0; o < data.nbOrd; ++o) { // pr chq order
            for(const int& prod : data.ords.ordsToProd[o]) { // pr chq prod ds order

                int prodRack = sol.prodToRack[prod]; // récup rack
                
                if(orderOccurRacks[o][prodRack] == 0) // si la clé prodRack existait pas -> ajout et valeur à 1 puis insertion dans set
                    orderToRacks[o].insert(prodRack); // ajt le rack 
                
                orderOccurRacks[o][prodRack]++; // incrémente 

            }
        }
    }

};

// retiens le mouvement à appliquer 
struct BestSwap {

    int bestDelta; 

    std::pair<int,int> prod1ToProd2; // produits à bouger 
    // si on fait un simple envoi de prod1, alors prod2 vaudra 0

    std::pair<int,int> rack1ToRack2; // rack d'origine et rack d'arrivée 

    BestSwap() : prod1ToProd2({-1,-1}), rack1ToRack2({-1,-1}), bestDelta(0) {} 

}; 

std::vector<std::pair<int,int>> computeFamIntervals(const Data& data, const Solution& sol, 
    std::vector<int>& famOrder); 

std::vector<std::pair<int,int>> getDefIntervals(const Data& data, const Solution& sol, 
    std::vector<int>& famOrder, std::vector<std::pair<int,int>>& famInter); 

bool posCapEnough(const Data& data, const Solution& sol, int pos); 

bool aisleAerEnough(const std::vector<int>& aisleAer, const std::vector<int>& aisleRemainCap, int posAisle); 

int findFamProdInRack(int rack, int fam, const Solution& sol, const Data& data); 

std::pair<int,int> findNeighsRack(int rack, int order, const orderAndRacks& oAr, const Data& data); 

int computeCurrRackImpact(int order, int currRack, const orderAndRacks& oAr, const Data& data); 

int computeAddRackImpact(int order, int newRack, const orderAndRacks& oAr, const Data& data); 

int computeDeltaSwap(int prod1, int prod2, const Data& data, const orderAndRacks& oAr, int prod1Rack, int prod2Rack); 

int computeDeltaSend(int prod1, int prod1Rack, int newRack, const Data& data, const orderAndRacks& oAr); 

void applyMove(const BestSwap& bestSwap, Solution& bestSol, 
    int& bestVal, std::vector<int>& aisleRemainCap, 
    orderAndRacks& oAr, std::vector<std::pair<int,int>>& defIntervals, 
    const std::vector<int>& rackToAisle, const Data& data, 
    std::vector<int>& famOrder, 
    std::vector<std::pair<int,int>>& famIntervals); 

void GradientBestImprov(const Data& data, Solution& bestSol, int& bestVal); 



