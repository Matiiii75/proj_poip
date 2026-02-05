#include "GreedyMethods.hpp"

// Compute value for a given solution
int computeSolValue(const Solution& sol, const Data& data) {

    std::vector<int> orderVal(data.nbOrd, 0); // orderVal[k] = value (distance) to satisfy order k 
    std::vector<int> lastRackSeen(data.nbOrd, 0); // lastRackSeen[k] = last rack seen for this order (0) if none still

    for(int i = 1; i < data.nbRacks; ++i) { // for each rack in ascending order
        for(const int& currProd : sol.RackToProd[i]) { // for each prod in ths rack
            for(const int& currOrd : data.ords.prodToOrds[currProd]) { // for each order containing currProd

                int lrs = lastRackSeen[currOrd]; 
                orderVal[currOrd] += data.dists[lrs][i]; // add dist from last rack seen to curr one treating 
                lastRackSeen[currOrd] = i; // update last rack seen 

            }
        }
    }
    for(int i = 0; i < data.nbOrd; ++i) // for each order
        orderVal[i] += data.dists[lastRackSeen[i]][data.nbRacks-1]; // add disance from lastrack to depot 

    int solVal = 0; // compute total val 
    for(const int& val : orderVal) solVal += val;

    return solVal; 
}


/**
 * @brief shuffle elements contained in vector
 * @param vec vector we want to shuffle 
 * @param g 
 */
void shuffleVector(std::vector<int>& vec, std::mt19937& g) {
    std::shuffle(vec.begin(), vec.end(), g); // shuffle elements 
}


/**
 * @brief randomly generate ordering of family  
*/
std::vector<int> genRandomOrderFam(const Data& data) {
    std::vector<int> famIndex(data.nbFam); 
    std::iota(famIndex.begin(), famIndex.end(), 0); // fill with 0, 1, 2, ...
}
