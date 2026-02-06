#include "Gradient.hpp"


/* orderAndracks orderAndRacks methods */

void orderAndRacks::computeOrderAndRacks(const Solution& sol, const Data& data) {

    orderToRacks.assign(data.nbOrd, {}); 
    orderOccurRacks.assign(data.nbOrd, std::vector<int>(data.nbRacks, 0)); 

    for(int o = 0; o < data.nbOrd; ++o) { // for each order 
        for(const int& prod : data.ords.ordsToProd[o]) { // for each prod in o

            int prodRack = sol.prodToRack[prod]; // get rack
            
            if(orderOccurRacks[o][prodRack] == 0) // if prodrack didnt exist : 
                orderToRacks[o].push_back(prodRack); // add rack
            
            orderOccurRacks[o][prodRack]++; // incrémente 
        }

        std::sort(orderToRacks[o].begin(), orderToRacks[o].end()); 
    }
}

/* end orderAndRacks methods */

/* aisleInfos methods */

void aisleInfos::computeRackToAisle(const Data& data) {

    rackToAisle.resize(data.nbRacks); 
    for(int i = 0; i < data.nbAisle; ++i) {
        if(i!=0) { // if we're not on 1st aisle
            for(int j = data.aisle[i-1]+1; j <= data.aisle[i]; ++j) 
                rackToAisle[j] = i; 
        }
        else { // if we're on 1st aisle
            for(int j = 0; j <= data.aisle[i]; ++j) 
                rackToAisle[j] = i; 
        }
    }

}

void aisleInfos::computeAisleCap(const Data& data) {

    aisleCap.resize(data.nbAisle, 0);  
    for(int i = 0; i < data.nbAisle; ++i) { // for each aisle
        int firstRack, lastRack; 
        lastRack = data.aisle[i]; 
        if(i > 0) firstRack = data.aisle[i-1]-1; // if > 1st aisle
        else firstRack = 1; // if 1st aisle 
        for(int j = firstRack; j <= lastRack; ++j) // add caps
            aisleCap[i] += data.capRacks[j]; 
    }

}

void aisleInfos::computeAisleAer(const Data& data) {

    aisleAer.resize(data.nbAisle); 
    for(int i = 0; i < data.nbAisle; ++i) { // for each aisle 
        int cap = aisleCap[i]; 
        aisleAer[i] = std::ceil(aisleCap[i]*(data.aeration/100.0)); // compute aer
    } 

}

void aisleInfos::computeAisleRemainCap(const Data& data, const Solution& sol) {

    aisleRemainCap = aisleCap; // initiate with aisleCap
    for(int i = 0; i < data.nbAisle; ++i) { // for each aisle
        int firstRack, lastRack; 
        lastRack = data.aisle[i]; 
        if(i > 0) firstRack = data.aisle[i-1]-1; 
        else firstRack = 1; 
        for(int j = firstRack; j <= lastRack; ++j) // for each rack in aisle
            aisleRemainCap[i] -= sol.RackToProd[j].size(); // minus sol aisle occupation 
    }

}

/**
 * @brief calcule si c'est possible d'ajouter un sommet à l'allée posAisle (allée de pos) sans violer l'aération
 * @param aisle : numéro de l'allée où on va 
 */
bool aisleInfos::aisleAerEnough(int aisle) {
    if(aisleRemainCap[aisle] - aisleAer[aisle] > 0) return true; 
    else return false; 
}

/* end aisleInfos methods */

/* famSolInfos methods */

void famSolInfos::computeFamInterFamOrd(const Data& data, const Solution& sol) {

    famIntervals.resize(data.nbFam, {-1,-1}); 
    famOrder.reserve(data.nbFam); // reserve nbFam positions

    for(int i = 0; i < data.nbRacks; ++i) { // for each rack
        if(sol.RackToProd[i].size() == 0) continue; // if empty rack -> ignore it
        
        for(const int& prod : sol.RackToProd[i]) { // for each prod in rack
            int currFamProd = data.fam.prodToFam[prod]; // get prod family
            if(famIntervals[currFamProd].first == -1) { // if family had no begin
                famIntervals[currFamProd].first = i; // let rack be the begin
                famOrder.push_back(currFamProd); // add family in order
            }
            famIntervals[currFamProd].second = i; // update last rack seen for family 
        }
    }

}

void famSolInfos::computeDefIntervals(const Data& data, const Solution& sol) {

    computeFamInterFamOrd(data,sol); // compute famIntervals & famOrder 
    defIntervals.resize(data.nbFam); 
    
    for(int i = 0; i < data.nbFam; ++i) { // for each family in solution order 

        int currFam = famOrder[i]; // get the current family at this position in order 
        if(i == 0) // if 1st family in order 
            defIntervals[currFam].first = 1; // 1st family always start a rack 1 
        if(i == data.nbFam - 1) // if last family in order  
            defIntervals[currFam].second = data.nbRacks - 2; // last rack always end at rack n-2 (excluding final depot)

        if(i > 0) { // compute min bound of currFam if > 1st family in order 
            
            int prevFam = famOrder[i-1]; // get family preceding curr one in order 
            int prevFamLastRack = famIntervals[prevFam].second; // get family succeding curr one in order 
            if(sol.RackToProd[prevFamLastRack].size() < data.capRacks[prevFamLastRack]) // if enough place in last rack of predfamily
                defIntervals[currFam].first = prevFamLastRack; // define min bound of currfam as prevFamLastRack
            else // if no space -> place it on very next one 
                defIntervals[currFam].first = prevFamLastRack + 1;  // define min bound of currFam as prevFamLastRack + 1

        }
        if(i < data.nbFam - 1) { // compute max bound of currFam if not last family in order
            
            int nextFam = famOrder[i+1]; // next family in order
            int nextFam1stRack = famIntervals[nextFam].first; // 1st rack in nextFam
            if(sol.RackToProd[nextFam1stRack].size() < data.capRacks[nextFam1stRack]) // if enough space
                defIntervals[currFam].second = nextFam1stRack; // curr Fam max bound = next fam 1st rack
            else // else stop rack just before 
                defIntervals[currFam].second = nextFam1stRack - 1;

        }
        if (famIntervals[currFam].first != -1) { // security : if the family is not empty 
            defIntervals[currFam].first = std::min(defIntervals[currFam].first, famIntervals[currFam].first);
            defIntervals[currFam].second = std::max(defIntervals[currFam].second, famIntervals[currFam].second);
        }   // the idea here, if to ensure we are not defining fam min bound < def fam min bounds (and also for max bound)
    }
} 

/* end famSolInfos methods */

/* Gradient/Metaheuristiques functions */

bool rackCapEnough(const Data& data, const Solution& sol, int rack) {
    if(data.capRacks[rack] - sol.RackToProd[rack].size() > 0) return true; 
    else return false; 
}

int findFamProdInRack(int rack, int fam, const Solution& sol, const Data& data) {

    int prodFound = -1;     
    for(const int& prod : sol.RackToProd[rack]) { // for each prod in rack
        if(data.fam.prodToFam[prod] == fam) { // if prod's family is fam 
            prodFound = prod; // memo
            break; // we found one, break and return it 
        }
    } 

    return prodFound; 
}

std::pair<int,int> findNeighsRack(int rack, int order, const orderAndRacks& oAr, const Data& data) {

    const std::vector<int>& orderRacks = oAr.orderToRacks[order]; // alias for speed
    auto itSuccessor = std::lower_bound(orderRacks.begin(), orderRacks.end(), rack); 

    int prevRack = 0;  // begin depot by default
    int nextRack = data.nbRacks - 1; // final depot by default 

    if(itSuccessor != orderRacks.end()) { // if we found one 
        if(*itSuccessor == rack) { // if iterator point on rack
            auto itNext = std::next(itSuccessor); // get next rack on righ
            if(itNext != orderRacks.end()) // if we find one on the right 
                nextRack = *itNext; 
            // (else nextRack is final depot by default)
        } else nextRack = *itSuccessor; // point on element > rack
    }

    if(itSuccessor != orderRacks.begin()) // if not first rack in order 
        prevRack = *std::prev(itSuccessor); // get previous rack 

    return {prevRack, nextRack}; 
}

int computeCurrRackImpact(int order, int currRack, const orderAndRacks& oAr, const Data& data) {

    if(oAr.orderOccurRacks[order][currRack] > 1) throw std::runtime_error("error in computeCurrRackImpact"); 
    std::pair<int,int> encadrants = findNeighsRack(currRack, order, oAr, data); // find surrounding racks
    int currVal = data.dists[encadrants.first][currRack] + data.dists[currRack][encadrants.second]; 
    int newVal = data.dists[encadrants.first][encadrants.second]; 

    return currVal - newVal; //! old - new -> better if > 0
}

int computeAddRackImpact(int order, int newRack, const orderAndRacks& oAr, const Data& data) {
    
    if(oAr.orderOccurRacks[order][newRack] > 0) throw std::runtime_error("error in computeAddRackImpact"); 
    std::pair<int,int> encadrants = findNeighsRack(newRack, order, oAr, data); // find surroundings 
    int currVal = data.dists[encadrants.first][encadrants.second]; 
    int newVal = data.dists[encadrants.first][newRack] + data.dists[newRack][encadrants.second]; 

    return currVal - newVal; //! old - new -> better if > 0
}

int computeSpecialCase(int order, int rack1, int rack2, const orderAndRacks& oAr, const Data& data) {

    int Delta = 0; 
    std::pair<int,int> rack1Neighs = findNeighsRack(rack1, order, oAr, data); // compute R1 neighs

    if(rack2 > rack1Neighs.first && rack2 < rack1) { // if rack2 is in ]rack1neighs.min, rack1[
        
        Delta += data.dists[rack1Neighs.first][rack1] + data.dists[rack1][rack1Neighs.second]; 
        Delta -= data.dists[rack1Neighs.first][rack2] + data.dists[rack2][rack1Neighs.second]; 

    }

    else if(rack2 < rack1Neighs.second && rack2 > rack1) { // if rack2 is in ]rack1, rack1neighs.max[

        Delta += data.dists[rack1Neighs.first][rack1] + data.dists[rack1][rack1Neighs.second]; 
        Delta -= data.dists[rack1Neighs.first][rack2] + data.dists[rack2][rack1Neighs.second]; 

    }

    else { // if rack2 isn't in ]rack1neighs.min, rack1neighs.max[ (usual case)

        std::pair<int,int> rack2Neighs = findNeighsRack(rack2, order, oAr, data); // compute R2 neighs

        // compute rack1 go throught and erase impact
        Delta += data.dists[rack1Neighs.first][rack1] + data.dists[rack1][rack1Neighs.second]; 
        Delta -= data.dists[rack1Neighs.first][rack1Neighs.second]; 

        // compute rack2 erase and go throught impact 
        Delta += data.dists[rack2Neighs.first][rack2Neighs.second]; 
        Delta -= data.dists[rack2Neighs.first][rack2] + data.dists[rack2][rack2Neighs.second]; 

    }

    return Delta; 
}

int computeDeltaSwap(int prod1, int prod2, const Data& data, const orderAndRacks& oAr, int prod1Rack, int prod2Rack) {

    int totalDelta = 0;     
    
    for(const int& o : data.ords.prodToOrds[prod1]) { // for each order containing prod1  

        int countRack1 = oAr.orderOccurRacks[o][prod1Rack]; // memo 
        int countRack2 = oAr.orderOccurRacks[o][prod2Rack]; 

        if(data.ords.ordsToProd[o].count(prod2) > 0) 
            continue; // if prod1 & prod 2 in same order, swapping them don't impact
        
        if(countRack1 > 1 && countRack2 >= 1) 
            continue; // ignore cause we'll still visit both rack after trading  

        if(countRack1 > 1 && countRack2 == 0) { // only impact on end
            totalDelta += computeAddRackImpact(o, prod2Rack, oAr, data); 
            continue; 
        }

        if(countRack1 == 1 && countRack2 == 0) { // impact on begin and end
            totalDelta += computeSpecialCase(o, prod1Rack, prod2Rack, oAr, data); 
            continue; 
        } 

        if(countRack1 == 1 && countRack2 >= 1) { // only impact on begin
            totalDelta += computeCurrRackImpact(o, prod1Rack, oAr, data); 
            continue; 
        }
        
    }

    for(const int& o : data.ords.prodToOrds[prod2]) { // same reasonning for prod 2 bellow 

        int countRack1 = oAr.orderOccurRacks[o][prod1Rack]; 
        int countRack2 = oAr.orderOccurRacks[o][prod2Rack];

        if(data.ords.ordsToProd[o].count(prod1) > 0) 
            continue; 

        if(countRack2 > 1 && countRack1 >= 1) 
            continue; 

        if(countRack2 > 1 && countRack1 == 0) { 
            totalDelta += computeAddRackImpact(o, prod1Rack, oAr, data); 
            continue; 
        }

        if(countRack2 == 1 && countRack1 == 0) { 
            totalDelta += computeSpecialCase(o, prod2Rack, prod1Rack, oAr, data); 
            continue; 
        } 

        if(countRack2 == 1 && countRack1 >= 1) { 
            totalDelta += computeCurrRackImpact(o, prod2Rack, oAr, data); 
            continue; 
        }
    }

    return totalDelta; //! old - new -> better if > 0
}

int computeDeltaSend(int prod1, int prod1Rack, int newRack, const Data& data, const orderAndRacks& oAr) {

    int totalDelta = 0; 
    
    for(const int& o : data.ords.prodToOrds[prod1]) { // for each order containing prod1

        int countRack1 = oAr.orderOccurRacks[o][prod1Rack]; // memo 
        int countNewRack = oAr.orderOccurRacks[o][newRack]; 

        if(countRack1 > 1 && countNewRack >= 1)
            continue; // ignore cause we'll still visit both rack after trading

        if(countRack1 > 1 && countNewRack == 0) { // impact on end only 
            totalDelta += computeAddRackImpact(o, newRack, oAr, data); 
            continue; 
        }

        if(countRack1 == 1 && countNewRack == 0) { // impact on begin & end
            totalDelta += computeSpecialCase(o, prod1Rack, newRack, oAr, data); 
            continue; 
        }

        if(countRack1 == 1 && countNewRack >= 1) { // impact on begin only
            totalDelta += computeCurrRackImpact(o, prod1Rack, oAr, data); 
            continue; 
        }
    }

    return totalDelta; //! old - new -> better if > 0
}

void debug(const std::vector<int>& vec) {
    for(int i = 0; i < (int)vec.size()-1; ++i) {
        if(vec[i] >= vec[i+1]) {
            for(const int& i : vec) std::cout << i << " "; 
            std::cout << std::endl;
            throw std::runtime_error("ATTENTION"); 
        }
    }
}

void applyMove(const BestSwap& bestSwap, Solution& bestSol, 
    int& bestVal, famSolInfos& fsi, aisleInfos& aInfos, 
    orderAndRacks& oAr, const Data& data)   
{ 

    int delta = bestSwap.bestDelta; 
    int prod1 = bestSwap.prod1ToProd2.first; 
    int prod2 = bestSwap.prod1ToProd2.second;
    int prod1Rack = bestSwap.rack1ToRack2.first; 
    int prod2Rack = bestSwap.rack1ToRack2.second;   
    bool isSwap = true; 
    
    if(prod2 == -1) isSwap = false; // if prod2 == -1, bestSwap is a "send" 

    /* ACTUALIZE REMAINCAPS */

    if(!isSwap) { // if send, we impact aisleRemainCap

        int prod1Aisle = aInfos.rackToAisle[prod1Rack]; // get prod1Rack's aisle
        int newAisle = aInfos.rackToAisle[prod2Rack]; // get aisle receiving prod1

        aInfos.aisleRemainCap[prod1Aisle]++; 
        aInfos.aisleRemainCap[newAisle]--; 

    }

    /* ACTUALIZE OAR */

    for(const int& o : data.ords.prodToOrds[prod1]) { // for each order containing prod1

        if(--oAr.orderOccurRacks[o][prod1Rack] == 0) // decrement, and if reaching 0 :
            eraseSortedVec(prod1Rack, oAr.orderToRacks[o]); // erase from vec  
        
        if(++oAr.orderOccurRacks[o][prod2Rack] == 1) // increment, and if reaching 1 
            addSortedVec(prod2Rack, oAr.orderToRacks[o]); // means we just added rack to o

    }
        
    if(isSwap) { // if swap, every sol containing prod2 must be updated

        for(const int& o : data.ords.prodToOrds[prod2]) { // for each order containing prod2 

            if(--oAr.orderOccurRacks[o][prod2Rack] == 0) // decrement, and if reaching 0 : 
                eraseSortedVec(prod2Rack, oAr.orderToRacks[o]); // erase from set
            
            if(++oAr.orderOccurRacks[o][prod1Rack] == 1) // increment and if reaching 1 
                addSortedVec(prod1Rack, oAr.orderToRacks[o]); // means we just added rack to o 

        }

    }

    /* END ACTUALIZE OAR */

    /* UPDATE BESTSOL */

    bestSol.prodToRack[prod1] = prod2Rack; // memo prod1 is in prod2Rack 
    bestSol.RackToProd[prod2Rack].push_back(prod1); 

    auto& v = bestSol.RackToProd[prod1Rack]; 
    auto it = std::find(v.begin(), v.end(), prod1); // erase prod1 from prod1Rack
    if(it != v.end()) v.erase(it); 

    if(isSwap) { // if swap, do same with prod2

        bestSol.prodToRack[prod2] = prod1Rack; 
        bestSol.RackToProd[prod1Rack].push_back(prod2); 

        auto& v = bestSol.RackToProd[prod2Rack]; 
        auto it = std::find(v.begin(), v.end(), prod2); 
        if(it != v.end()) v.erase(it); 

    }

    /* END MAJ BESTSOL */

    /* ACTUALIZE FAMINTERVALS */

    int famProd1 = data.fam.prodToFam[prod1]; // get prod1 fam

    int newMax = -1; 
    int newMin = data.nbRacks; 
    for(int p : data.fam.famToProd[famProd1]) { // re-compute reals bounds of famprod1 
        int r = bestSol.prodToRack[p]; 
        if(r < newMin) newMin = r; 
        if(r > newMax) newMax = r; 
    }
    fsi.famIntervals[famProd1] = {newMin, newMax}; 

    /* END ACTUALIZE FAMINTERVALS */

    /* ACTUALIZE DEFINTERVALS */

    // seek surroundings families  
    
    int predFam = -1; 
    int succFam = -1; 

    for(int i = 0; i < data.nbFam; ++i) {
        if(fsi.famOrder[i] == famProd1) {
            if(i > 0) predFam = fsi.famOrder[i-1]; 
            if(i < data.nbFam-1) succFam = fsi.famOrder[i+1]; 
        }   
    }
    
    // update left family interval

    if(predFam != -1) {

        int contactPoint = fsi.famIntervals[famProd1].first; // new famProd1's rack far left 
        
        // if contactPoint rack between two fams isn't empty -> predFam can go there
        if(bestSol.RackToProd[contactPoint].size() < data.capRacks[contactPoint])
            fsi.defIntervals[predFam].second = contactPoint; 
        else // else, stop right before 
            fsi.defIntervals[predFam].second = contactPoint - 1; 

    }

    if(succFam != -1) {

        int contactPoint = fsi.famIntervals[famProd1].second; // new famProd1's rack far right

        // if contactPoint rack between two fams isn't empty -> predFam can go there
        if(bestSol.RackToProd[contactPoint].size() < data.capRacks[contactPoint])
            fsi.defIntervals[succFam].first = contactPoint; 
        else // else, stop right before 
            fsi.defIntervals[succFam].first = contactPoint + 1; 

    }

    /* END ACTUALIZE DEFINTERVALS */

    /* ACTUALIZE BESTVAL */
    
    bestVal -= bestSwap.bestDelta; 

}

void GradientBestImprov(const Data& data, Solution& bestSol, int& bestVal) {

    aisleInfos aInfos; 
    aInfos.computeRackToAisle(data);
    aInfos.computeAisleCap(data);       // computing aisle informations 
    aInfos.computeAisleAer(data);       // (capacities, aeration, remaining capacities, etc..)
    aInfos.computeAisleRemainCap(data,bestSol); 

    orderAndRacks orderAndRacks; // initiate order and racks relations
    orderAndRacks.computeOrderAndRacks(bestSol, data); 

    famSolInfos fsi; 
    fsi.computeFamInterFamOrd(data, bestSol); // compute famInterval & famOrder in bestSol
    fsi.computeDefIntervals(data, bestSol); // compute families defIntervals relative to bestSol


    bool improved = true; 
    
    while(improved) { // best improvement policy : retain best swap and while we find one -> keep going

        improved = false; 
        BestSwap bestSwap; // declare bestSwap object. (default constructor built it)

        for(int f = 0; f < data.nbFam; ++f) { // for each family 
            
            for(const int& prod : data.fam.famToProd[f]) { // for each prod of fam f 
                
                int prodRack = bestSol.prodToRack[prod]; // get rack containing prod 
                int rackAisle = aInfos.rackToAisle[prodRack]; // get aisle containing prodRack 
                
                int fmin = fsi.defIntervals[f].first; // get defInterval of f 
                int fmax = fsi.defIntervals[f].second; 

                for(int newPos = fmin; newPos <= fmax; ++newPos) { // for each position in defInterval
                    
                    if(newPos == prodRack) continue; // if same rack -> skip

                    bool enoughCap, enoughAer;
                    int newPosAisle = aInfos.rackToAisle[newPos]; // get newPos's aisle

                    if(newPosAisle == rackAisle) 
                        enoughAer = true; // have enough aer since we stay in same aisle & don't add anything
                    else enoughAer = aInfos.aisleAerEnough(newPosAisle); // check if enough aer in newPos's aisle to send prod
                    
                    enoughCap = rackCapEnough(data, bestSol, newPos); // check if enough Cap in newPos rack to send prod
                    
                    if(enoughCap) { // if enough cap to send prod 
                        
                        if(enoughAer) { // if sending prod respect aeration
                            
                            int Delta = computeDeltaSend(prod, prodRack, newPos, data, orderAndRacks); // simulate send 

                            if(Delta > bestSwap.bestDelta) { //? faire une fonction retiensSwap ? 
                                
                                bestSwap.bestDelta = Delta; 
                                bestSwap.prod1ToProd2 = {prod, -1}; // memo swap
                                bestSwap.rack1ToRack2 = {prodRack, newPos};

                            }

                            continue; // -> next iteration
                        }
                        
                        if(!enoughAer) { // if sending prod do not respect aeration
                             
                            int sameFamProd = findFamProdInRack(newPos, f, bestSol, data); // search sam fame other product
                            if(sameFamProd == -1) continue; // if no same fam prod found -> skip
                            // simulate trade 
                            int Delta = computeDeltaSwap(prod, sameFamProd, data, orderAndRacks, prodRack, newPos); 
                            
                            if(Delta > bestSwap.bestDelta) {
                    
                                bestSwap.bestDelta = Delta; 
                                bestSwap.prod1ToProd2 = {prod, sameFamProd}; // memo swap
                                bestSwap.rack1ToRack2 = {prodRack, newPos};
                            }

                            continue; // -> next iteration
                        }

                    }

                    if(!enoughCap) { // if not enough cap in rack to send prod 

                        int sameFamProd = findFamProdInRack(newPos, f, bestSol, data); // search for same fam other prod
                        if(sameFamProd == -1) continue; // if not found -> skip
                        // simulate trade  
                        int Delta = computeDeltaSwap(prod, sameFamProd, data, orderAndRacks, prodRack, newPos);
                        
                        if(Delta > bestSwap.bestDelta) {
                            
                            bestSwap.bestDelta = Delta; 
                            bestSwap.prod1ToProd2 = {prod, sameFamProd}; // memo swap
                            bestSwap.rack1ToRack2 = {prodRack, newPos}; 

                        }

                    }

                }  
            }
        }

        if(bestSwap.prod1ToProd2.first != -1) { // applying best swap found 
            applyMove(bestSwap, bestSol, bestVal, fsi, aInfos, orderAndRacks, data); 
            improved = true; 
        }

        std::cout << "solval : " << bestVal << std::endl;

    }
}

void firstImprovLocalSearch(const Data& data, Solution& bestSol, int& bestVal) {

    std::random_device rd; // create random generator
    std::mt19937 g(rd()); 

    std::vector<int> famIndex = genRandomOrderFam(data); // compute random order of family index

    aisleInfos aInfos; 
    aInfos.computeRackToAisle(data);
    aInfos.computeAisleCap(data);       // computing aisle informations 
    aInfos.computeAisleAer(data);       // (capacities, aeration, remaining capacities, etc..)
    aInfos.computeAisleRemainCap(data,bestSol); 

    orderAndRacks orderAndRacks; // initiate order and racks relations
    orderAndRacks.computeOrderAndRacks(bestSol, data); 

    famSolInfos fsi; 
    fsi.computeFamInterFamOrd(data, bestSol); // compute famInterval & famOrder in bestSol
    fsi.computeDefIntervals(data, bestSol); // compute families defIntervals relative to bestSol

    bool improved = true; 
    int z = 0; 
    while(improved) {
        
        improved = false; 
        BestSwap bestSwap; 
        shuffleVector(famIndex, g); // shuffle fam index order to loop on

        for(const int& f : famIndex) { // for each fam 
            for(const int& prod : data.fam.famToProd[f]) { // for each prod of fam f 

                int prodRack = bestSol.prodToRack[prod]; // get rack containing prod 
                int rackAisle = aInfos.rackToAisle[prodRack]; // get aisle containing prodRack 
                
                int fmin = fsi.defIntervals[f].first; // get defInterval of f 
                int fmax = fsi.defIntervals[f].second; 

                for(int newPos = fmin; newPos <= fmax; ++newPos) { // for each position in defIntervals 
                    if(newPos == prodRack) continue; // if same rack -> skip

                    bool enoughCap, enoughAer;
                    int newPosAisle = aInfos.rackToAisle[newPos]; // get newPos's aisle

                    if(newPosAisle == rackAisle) 
                        enoughAer = true; // have enough aer since we stay in same aisle & don't add anything
                    else enoughAer = aInfos.aisleAerEnough(newPosAisle); // check if enough aer in newPos's aisle to send prod
                    enoughCap = rackCapEnough(data, bestSol, newPos); // check if enough Cap in newPos rack to send prod

                    if(enoughCap) { // if enough cap to send prod

                        if(enoughAer) { // if sending prod respect aeration 

                            int Delta = computeDeltaSend(prod, prodRack, newPos, data, orderAndRacks); // simulate send

                            if(Delta > bestSwap.bestDelta) {// if improvement -> applymove directly
                            
                                bestSwap.bestDelta = Delta; 
                                bestSwap.prod1ToProd2 = {prod, -1}; // memo swap
                                bestSwap.rack1ToRack2 = {prodRack, newPos};
                                goto next_iteration; // go applyMove & pass to next iteration
                            }

                            continue; // -> next iteration
                        }
                        
                        if(!enoughAer) { // if sending prod do not respect aeration 

                            int sameFamProd = findFamProdInRack(newPos, f, bestSol, data); 
                            if(sameFamProd == -1) continue; // if no same fam prod found -> skip
                            // simulate trade 
                            int Delta = computeDeltaSwap(prod, sameFamProd, data, orderAndRacks, prodRack, newPos);
                            
                            if(Delta > bestSwap.bestDelta) {// if improvement -> applymove directly 

                                bestSwap.bestDelta = Delta; 
                                bestSwap.prod1ToProd2 = {prod, sameFamProd}; // memo swap
                                bestSwap.rack1ToRack2 = {prodRack, newPos};
                                goto next_iteration; // go applyMove & pass to next iteration
                            }

                            continue; // -> next iteration 
                        }

                    }

                    if(!enoughCap) { // if not enough cap in rack to send prod 

                        int sameFamProd = findFamProdInRack(newPos,f, bestSol, data); // search for same fam other prod 
                        if(sameFamProd == -1) continue; // if not found -> skip
                        // simulate trade
                        int Delta = computeDeltaSwap(prod, sameFamProd, data, orderAndRacks, prodRack, newPos); 
                        if(Delta > bestSwap.bestDelta) {

                            bestSwap.bestDelta = Delta; 
                            bestSwap.prod1ToProd2 = {prod, sameFamProd}; // memo swap
                            bestSwap.rack1ToRack2 = {prodRack, newPos};
                            goto next_iteration; // go applyMove & pass to next iteration
                        }  
                    }
                }
            }
        }

        next_iteration: 
        if(bestSwap.prod1ToProd2.first != -1) {
            applyMove(bestSwap, bestSol, bestVal, fsi, aInfos, orderAndRacks, data); 
            improved = true; 
        }

        std::cout << "solVal : " << bestVal << std::endl; 
        if(++z > 20000) break; 
    }

}