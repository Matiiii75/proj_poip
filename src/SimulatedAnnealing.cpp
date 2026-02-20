#include "SimulatedAnnealing.hpp"
 
// ! faire constructeur 

SAA::SAA(const Data& _data, Solution& _sol, int& _solVal, orderAndRacks& _oAr, aisleInfos& _aInfos, famSolInfos& _fsi)
: LocalSearchMethods(_data, _sol, _solVal, _oAr, _aInfos, _fsi) {}

int SAA::genRandomNumber(int a, int b, std::mt19937& g) {
    if(a > b) 
        std::cerr << "a : " << a << ", b : " << b;  
    std::uniform_int_distribution<> dis(a,b); 
    return dis(g); 
}

double SAA::uniformRandom(std::mt19937& g) {
    std::uniform_real_distribution<double> dis(0.0, 1.0); 
    return dis(g); 
}

std::vector<std::vector<int>> SAA::initFamToProdVector(const Data& data) {
    std::vector<std::vector<int>> famToProdVec(data.nbFam); 
    for(int f = 0; f < data.nbFam; ++f) {
        for(const auto& prod : data.fam.famToProd[f])
            famToProdVec[f].push_back(prod); 
    }
    return famToProdVec; 
}

int SAA::getRandomProdInFam(int fam, const Data& data, std::mt19937& g, const std::vector<std::vector<int>>& famToProdVec) {
    // get index of product wanted 
    if((int)data.fam.famToProd[fam].size() == 0) throw std::runtime_error("error with fam size"); 
    int index = genRandomNumber(0, (int)data.fam.famToProd[fam].size()-1, g); 
    return famToProdVec[fam][index]; 
}

int SAA::getRandomPos(int currPos, int fmin, int fmax, std::mt19937& g) {
    int newPos = currPos; 
    if(currPos == fmin && currPos == fmax) return currPos; // ensure there's other pos in defInterval
    else // search for another rack than currPos if possible 
        while(newPos == currPos) {newPos = genRandomNumber(fmin, fmax, g);} // continue until != currPos
    return newPos; 
}

bool SAA::metropolis(int Delta, double temp, std::mt19937& g) {
    double p = uniformRandom(g); 
    if(p <= exp((double)Delta/temp)) return true; // accept Move 
    return false; // reject Move 
}

void SAA::displayInfos(int iter, double temp) {
    if(iter % 10000 == 0)
        std::cout << "iter : " << iter << " | temp : " << temp << " | bestVal : " << bestVal << std::endl; 
}

void SAA::debug(std::string s) {
    std::cout << s << std::endl;
}

double SAA::initTemp(const std::vector<std::vector<int>>& famToProdVec, std::mt19937& g) {

    double temp = 1000; 
    double wantedAcceptRate = 0.8; // the acceptation rate we desire 
    std::vector<int> worseningDeltas;  
    
    // compute 1000 degrading movement's Delta and save them
    while((int)worseningDeltas.size() < data.nbProd * 10) { 

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
        double averageRate = totalRate/(data.nbProd * 10); 
    
        if(std::abs(averageRate - wantedAcceptRate) < 0.01) break;
        if(averageRate < 1e-10) averageRate = 1e-10; // security for log ]0,1[ definition
        if(averageRate > 0.999) averageRate = 0.999;  

        temp = temp * (log(averageRate)/log(wantedAcceptRate)); 

    } 

    return temp;
}

void SAA::optimize() {

    std::random_device rd; 
    std::mt19937 g(rd()); 

    // ! dessous : on pourrait le passer en argument au recuit si on prévois de le faire plusieurs fois
    std::vector<std::vector<int>> famToProdVec = initFamToProdVector(data); 

    double temp = initTemp(famToProdVec, g);  
    int nbIterMax = data.nbProd * 10; 

    while(temp > 10) {
        for(int i = 0; i < nbIterMax; ++i) {

            // random initializations 
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
        }
        
        temp = temp*0.95; // update temperature 
    }
}
