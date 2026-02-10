#include "TabuSearch.hpp"

TabuSearch::TabuSearch(const Data& _data, Solution& _sol, int _solVal)
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

        if(totalCount == 40000) break; 
        
    }

}

// tire un entier entre a et b inclus 
int TabuSearch::genRandomNumber(int a, int b, std::mt19937& g) {
    if(a > b) 
        std::cerr << "a : " << a << ", b : " << b;  
    std::uniform_int_distribution<> dis(a,b); 
    return dis(g); 
}

// tire un réel entre 0 et 1 
double TabuSearch::uniformRandom(std::mt19937& g) {
    std::uniform_real_distribution<double> dis(0.0, 1.0); 
    return dis(g); 
}

std::vector<std::vector<int>> TabuSearch::initFamToProdVector(const Data& data) {
    std::vector<std::vector<int>> famToProdVec(data.nbFam); 
    for(int f = 0; f < data.nbFam; ++f) {
        for(const auto& prod : data.fam.famToProd[f])
            famToProdVec[f].push_back(prod); 
    }
    return famToProdVec; 
}

int TabuSearch::getRandomProdInFam(int fam, const Data& data, std::mt19937& g, const std::vector<std::vector<int>>& famToProdVec) {
    // get index of product wanted 
    if((int)data.fam.famToProd[fam].size() == 0) throw std::runtime_error("error with fam size"); 
    int index = genRandomNumber(0, (int)data.fam.famToProd[fam].size()-1, g); 
    return famToProdVec[fam][index]; 
}

// trouve aléatoirement la nouvelle position où envoyer un produit
int TabuSearch::getRandomPos(int currPos, int fmin, int fmax, std::mt19937& g) {
    int newPos = currPos; 
    if(currPos == fmin && currPos == fmax) return currPos; 
    else // search for another rack than currPos if possible 
        while(newPos == currPos) {newPos = genRandomNumber(fmin, fmax, g);}
    return newPos; 
}

// metropolis criterion 
bool TabuSearch::metropolis(int Delta, double temp, std::mt19937& g) {
    double p = uniformRandom(g); 
    if(p <= exp((double)Delta/temp)) return true; 
    return false; 
}

void TabuSearch::displayInfos(int iter, double temp) {
    if(iter % 1000 == 0)
        std::cout << "iter : " << iter << " | temp : " << temp << " | bestVal : " << bestVal << std::endl; 
}

void TabuSearch::debug(std::string s) {
    std::cout << s << std::endl;
}

double TabuSearch::initTemp(const std::vector<std::vector<int>>& famToProdVec, std::mt19937& g) {

    double temp = 1000; 
    double wantedAcceptRate = 0.8; // the acceptation rate we desire 
    std::vector<int> worseningDeltas;  

    // compute 1000 degrading movement's Delta and save them
    while((int)worseningDeltas.size() < 1000) {

        // tirages aléatoires
        int randFam = genRandomNumber(0, data.nbFam-1, g); 
        int randProd = getRandomProdInFam(randFam, data, g, famToProdVec); 
        int prodRack = bestSol.prodToRack[randProd]; 
        int fmin = fsi.defIntervals[randFam].first; // get def intervals 
        int fmax = fsi.defIntervals[randFam].second; 
        int randPos = getRandomPos(prodRack, fmin, fmax, g); 
        if(prodRack == randPos) continue; // skip if same rack

        // check if it can be send or swapped
        bool enoughCap, enoughAer; 
        int prodAisle = aInfos.rackToAisle[prodRack]; // get currProd's aisle
        int randPosAisle = aInfos.rackToAisle[randPos]; // get randPos's aisle

        if(randPosAisle == prodAisle) enoughAer = true; 
        else enoughAer = aInfos.aisleAerEnough(randPosAisle); 
        enoughCap = rackCapEnough(randPos); 

        int Delta;
        int sameFamProd = -1; // set -1 by default    

        if(enoughCap && enoughAer) // if enough rack cap & aisle aeration to send prod 
            Delta = computeDeltaSend(randProd, prodRack, randPos); 
        else { // case enoughCap && !enoughAer and !enoughCap
            sameFamProd = findFamProdInRack(randPos, randFam); // seek for same fam other prod 
            if(sameFamProd == -1) continue; // if not found -> skip
            Delta = computeDeltaSwap(randProd, sameFamProd, prodRack, randPos); 
        }
        // if delta is worsening the solution : add it 
        if(Delta < 0) worseningDeltas.push_back(Delta); 

    }

    // for each of these degrading delta, do : compute average acceptation rate 
    // if its near wantedAcceptRate : we have the good temperature -> STOP
    // else : arrange temperature with a formula and re compute average acceptation rate
    
    while(true) {
        
        double totalRate = 0.0; 
        for(int d : worseningDeltas) 
            totalRate += exp((double)d/temp); 
        double averageRate = totalRate/1000.0; 
        
        if(std::abs(averageRate - wantedAcceptRate) < 0.01)
            break;
        
        if(averageRate < 1e-10) averageRate = 1e-10; // security for log ]0,1[ definition
        if(averageRate > 0.999) averageRate = 0.999; 

        temp = temp * (log(averageRate)/log(wantedAcceptRate)); 

    } 

    return temp;
}

void TabuSearch::optiRecuit(double initialTemp) {

    std::random_device rd; 
    std::mt19937 g(rd()); 

    // ! dessous : on pourrait le passer en argument au recuit si on prévois de le faire plusieurs fois
    std::vector<std::vector<int>> famToProdVec = initFamToProdVector(data); 

    double temp = initTemp(famToProdVec, g); 
    int nbIterMax = data.nbProd * 10; 
    
    int iter = 0; 
    while(temp > 0.01) {
        for(int i = 0; i < nbIterMax; ++i) {

            // tirages aléatoires
            int randFam = genRandomNumber(0, data.nbFam-1, g); 
            int randProd = getRandomProdInFam(randFam, data, g, famToProdVec); 
            int prodRack = bestSol.prodToRack[randProd]; 
            int fmin = fsi.defIntervals[randFam].first; // get def intervals 
            int fmax = fsi.defIntervals[randFam].second; 
            int randPos = getRandomPos(prodRack, fmin, fmax, g); 
            if(prodRack == randPos) continue; // skip if same rack

            // check if it can be send or swapped
            bool enoughCap, enoughAer; 
            int prodAisle = aInfos.rackToAisle[prodRack]; // get currProd's aisle
            int randPosAisle = aInfos.rackToAisle[randPos]; // get randPos's aisle

            if(randPosAisle == prodAisle) enoughAer = true; 
            else enoughAer = aInfos.aisleAerEnough(randPosAisle); 
            enoughCap = rackCapEnough(randPos); 
 
            int Delta;
            int sameFamProd = -1; // set -1 by default   
            BestSwap bestSwap; 

            if(enoughCap && enoughAer) // if enough rack cap & aisle aeration to send prod 
                Delta = computeDeltaSend(randProd, prodRack, randPos); 
            else { // case enoughCap && !enoughAer and !enoughCap
                sameFamProd = findFamProdInRack(randPos, randFam); // seek for same fam other prod 
                if(sameFamProd == -1) continue; // if not found -> skip
                Delta = computeDeltaSwap(randProd, sameFamProd, prodRack, randPos); 
            }
            if(Delta > 0) { // if upgrading solution -> applymove 
                bestSwap.bestDelta = Delta; 
                bestSwap.prod1ToProd2 = {randProd, sameFamProd}; 
                bestSwap.rack1ToRack2 = {prodRack, randPos}; 
                applyMove(bestSwap); 
            }
            else { // -> check metropolis criterium
                bool accept = metropolis(Delta, temp, g);
                if(accept) { // if probability accept the move : 
                    bestSwap.bestDelta = Delta; 
                    bestSwap.prod1ToProd2 = {randProd, sameFamProd}; 
                    bestSwap.rack1ToRack2 = {prodRack, randPos}; 
                    applyMove(bestSwap);
                }
            }
            iter++;
            displayInfos(iter, temp); 
        }
        
        temp = temp*0.95; 
    }
}