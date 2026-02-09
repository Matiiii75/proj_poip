#include "TabuSearch.hpp"

TabuSearch::TabuSearch(const Data& _data, const Solution& _sol, int _solVal)
: LocalSearchMethods(_data, _sol, _solVal), tabuSize((int)_data.nbProd*(4.0/5.0)) 
{
    isInTabu.assign(_data.nbProd, false); 
}

void TabuSearch::addTabu(int prod) {

    while ((int)tabuQ.size() >= tabuSize) {
        isInTabu[tabuQ.front()] = false; 
        tabuQ.pop(); 
    }
    tabuQ.push(prod); isInTabu[prod] = true;

}

void TabuSearch::resetTabu() {

    std::queue<int> newTabu; 
    tabuQ = newTabu; 

    std::vector<bool> newIsInTabu(data.nbProd, false); 
    isInTabu = newIsInTabu; 

}

void TabuSearch::optimize() { 

    std::random_device rd; // create random generator
    std::mt19937 g(rd()); 
    std::vector<int> famIndex = genRandomOrderFam(data); // compute random order of family index
    
    bool improved = true; 
    int z = 0; 
    int totalCount = 1; 
    bool ok = true; 
    while(improved) {
        
        improved = false; 
        BestSwap bestSwap; 
        shuffleVector(famIndex, g); // shuffle fam index order to loop on

        for(const int& f : famIndex) { // for each fam 
            for(const int& prod : data.fam.famToProd[f]) { // for each prod of fam f 
                if(isInTabu[prod]) continue; // if prod is tabu -> skip

                int prodRack = bestSol.prodToRack[prod]; // get rack containing prod 
                int rackAisle = aInfos.rackToAisle[prodRack]; // get aisle containing prodRack 
                
                int fmin = fsi.defIntervals[f].first; // get defInterval of f 
                int fmax = fsi.defIntervals[f].second; 

                for(int newPos = fmin; newPos <= fmax; ++newPos) { // for each position in defIntervals 
                    if(newPos == prodRack) continue; // if same rack -> skip
                    // if(aInfos.rackToAisle[newPos] == rackAisle) continue; // ! TEST : ignorer si les deux rack sont dans la meme allée

                    bool enoughCap, enoughAer;
                    int newPosAisle = aInfos.rackToAisle[newPos]; // get newPos's aisle

                    if(newPosAisle == rackAisle) 
                        enoughAer = true; // have enough aer since we stay in same aisle & don't add anything
                    else enoughAer = aInfos.aisleAerEnough(newPosAisle); // check if enough aer in newPos's aisle to send prod
                    enoughCap = rackCapEnough(newPos); // check if enough Cap in newPos rack to send prod

                    if(enoughCap) { // if enough cap to send prod

                        if(enoughAer) { // if sending prod respect aeration 

                            int Delta = computeDeltaSend(prod, prodRack, newPos); // simulate send

                            if(Delta > bestSwap.bestDelta) {// if improvement -> applymove directly
                            
                                bestSwap.bestDelta = Delta; 
                                bestSwap.prod1ToProd2 = {prod, -1}; // memo swap
                                bestSwap.rack1ToRack2 = {prodRack, newPos};
                                goto next_iteration; // go applyMove & pass to next iteration
                            }

                            continue; // -> next iteration
                        }
                        
                        if(!enoughAer) { // if sending prod do not respect aeration 

                            int sameFamProd = findFamProdInRack(newPos, f); 
                            if(sameFamProd == -1) continue; // if no same fam prod found -> skip
                            // simulate trade 

                            if(isInTabu[sameFamProd]) continue; // ! TEST

                            int Delta = computeDeltaSwap(prod, sameFamProd, prodRack, newPos);
                            
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

                        int sameFamProd = findFamProdInRack(newPos,f); // search for same fam other prod 

                        if(sameFamProd == -1) continue; // if not found -> skip

                        if(isInTabu[sameFamProd]) continue; // ! TEST
                        // simulate trade
                        int Delta = computeDeltaSwap(prod, sameFamProd, prodRack, newPos); 
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
            applyMove(bestSwap); 
            addTabu(bestSwap.prod1ToProd2.first); // add prod1 to tabu
            if(bestSwap.prod1ToProd2.second != -1) addTabu(bestSwap.prod1ToProd2.second); 
            improved = true; 
        }

        std::cout << "solVal : " << bestVal << std::endl; 
        std::cout << "z : " << z << std::endl;
        std::cout << "ts: " << tabuSize << std::endl;
        
        totalCount++; 
        if(++z > data.nbProd) {
            resetTabu(); 
            z = 0; 
        }
        if(z == 7000) {
            tabuSize = tabuSize / 2; 
        }
        
    }

}