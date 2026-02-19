#include "Conductor.hpp"

Conductor::Conductor(const Data& _data): data(_data) {}

void Conductor::computeArgs() {

    displayCalling("oAr, aInfos & fsi building"); 

    oAr = orderAndRacks(); 
    aInfos = aisleInfos(); // full reset of structure
    fsi = famSolInfos();  // useful for Stats (example)

    oAr.computeOrderAndRacks(solution, data); 
    aInfos.computeRackToAisle(data); 
    aInfos.computeAisleCap(data); 
    aInfos.computeAisleAer(data); 
    aInfos.computeAisleRemainCap(data, solution); 
    fsi.computeFamInterFamOrd(data, solution); 
    fsi.computeDefIntervals(data, solution); 

}

void Conductor::debug(const std::string& s) {
    std::cout << s << std::endl;
}

void Conductor::displayCalling(const std::string& call) {
    std::cout << "Conductor : [--- " << call << " ---]" << std::endl;
}

void Conductor::callInitRandomSolution() {
    solution = initSolRandom(data); 
    val = computeSolValue(solution, data); 
    computeArgs(); // once solution calculated -> computed relatives data
}

void Conductor::callInitSol() {
    solution = initSol(data); 
    val = computeSolValue(solution, data); 
    computeArgs(); 
}

void Conductor::callInitSolCoi() {
    solution = coiGreedy(data); 
    val = computeSolValue(solution, data); 
    computeArgs(); 
}

void Conductor::callInitSolTSP() {
    std::vector<int> famOrder = computeTSP(data); 
    solution = fillSolution(data, famOrder);
    val = computeSolValue(solution, data); 
    computeArgs(); 
}

void Conductor::calInitSolBestInsert() {
    solution = GreedyBestInsert(data); 
    val = computeSolValue(solution, data); 
    computeArgs(); 
}

void Conductor::SimulatedAnnealing() {
    SAA SaaInstance(data, solution, val, oAr, aInfos, fsi); 
    displayCalling("calling Simulated Annealing"); 
    SaaInstance.optimize(); 
}

void Conductor::FILS() {
    GradientDescent filsInstance(data, solution, val, oAr, aInfos, fsi); 
    displayCalling("calling FILS"); 
    filsInstance.optimize(); 
}

std::vector<std::vector<int>> Conductor::copyRackToProd(int f1min, int f2max) {

    std::vector<std::vector<int>> copyRackToProd(f2max - f1min +1); 
    int idVec = 0; // to re-index (for order of famillies instead of IDs)
    for(int r = f1min; r <= f2max; ++r) {  
        for(int prod : solution.RackToProd[r]) 
            copyRackToProd[idVec].push_back(prod); 
        ++idVec; 
    }

    return copyRackToProd;
}

int Conductor::computeProdOrder(int fam1, int fam2, std::vector<int>& prodOrder) {

    int f1min = fsi.famIntervals[fam1].first;
    int f1max = fsi.famIntervals[fam1].second; 
    int f2min = fsi.famIntervals[fam2].first; 
    int f2max = fsi.famIntervals[fam2].second;

    prodOrder.resize((int)data.fam.famToProd[fam1].size() + (int)data.fam.famToProd[fam2].size()); // reserve the plece who's the number of prod

    int idVec = 0; // indicate where we are in the vec prodOrder 
    for(int r = f2min; r <= f2max ; ++r) { // first, store order of fam2's products
        for(int p = 0; p < (int)solution.RackToProd[r].size(); ++p) { 
            int prod = solution.RackToProd[r][p]; // recover product in position p in rack r

            if(data.fam.prodToFam[prod] == fam2) { //! ajouter un if r = fmin/max
                prodOrder[idVec] = prod;
                idVec++;
            }
        }

    }
    bool firstProd = true; int FirstProdF1 = -1; // we want to store the number of the first product of f1
    for(int r = f1min; r <= f1max ; ++r) { //then, store order of fam2's products
        for(int p = 0; p < (int)solution.RackToProd[r].size(); ++p) {
            int prod = solution.RackToProd[r][p];
        
            if(data.fam.prodToFam[prod] == fam1) { //! ajouter un if r = fmin/max
                prodOrder[idVec]= prod; 
                if(firstProd) {FirstProdF1 = idVec; firstProd = false;} // recuperate the first product of F1
                idVec++;
            }
                
        }
    }

    return FirstProdF1;
}

void Conductor::placeProducts(int fam1, int fam2, int f1min, int f2max, const std::vector<int>& prodOrder) {

    int idVec = 0; // position of the product we want to place 
    int prod; // product in positon idVec

    for(int r = f1min; r <= f2max; ++r) {  // for all racks in the intervall we modify
        for(int p = 0; p < (int)solution.RackToProd[r].size(); ++p) {
            prod = prodOrder[idVec]; // product in positon idVec

            int prodBeforeSwap = solution.RackToProd[r][p]; // product before the swap

            if((data.fam.prodToFam[prodBeforeSwap] == fam1) 
            || (data.fam.prodToFam[prodBeforeSwap] == fam2)) { // if the product we want to change was in fam1 or fam2
                
                solution.RackToProd[r][p] = prod; // place prod in rack r
                solution.prodToRack[prod] = r; // associate prod to r
                idVec++; // next prod

            }
        }   
    }
}

void Conductor::updateFsi(int famId1, int famId2, int FirstProdF1, const std::vector<int>& prodOrder) {
     
    int fam1 = fsi.famOrder[famId1]; int fam2 = fsi.famOrder[famId2]; // recover fam1 and fam2
   
    int f1min = fsi.famIntervals[fam1].first; // recover bounds of fam1 and fam2
    int f2max = fsi.famIntervals[fam2].second;

    fsi.famOrder[famId2] = fam1; fsi.famOrder[famId1] = fam2; // swap the order of fam1 and fam2

    int tmpF1MinDef = fsi.defIntervals[fam1].first; // copy the infos of f1 to modify it

    int newMinF1 = solution.prodToRack[prodOrder[FirstProdF1]]; // memorization 
    int newMaxF2 = solution.prodToRack[prodOrder[FirstProdF1-1]];
 
    fsi.defIntervals[fam1].second = fsi.defIntervals[fam2].second; 
    fsi.famIntervals[fam1].second = f2max; // update fam1 properties 
    fsi.famIntervals[fam1].first = newMinF1; // the max for right family is still the same

    fsi.defIntervals[fam2].first = tmpF1MinDef;
    fsi.famIntervals[fam2].first = f1min; // update fam2 properties
    fsi.famIntervals[fam2].second = newMaxF2; // the min for left family is still the same 

    if(newMaxF2 == newMinF1) { // if f1 and f2 share a rack
        fsi.defIntervals[fam1].first = newMinF1; fsi.defIntervals[fam2].second = newMinF1;

    } else {

        int capRackMin = data.capRacks[newMinF1]; // get capacity of rack newMinF1
        int sizeRackMin = solution.RackToProd[newMinF1].size(); // get occupied capacity of newMinF1

        if(sizeRackMin < capRackMin) // if enough cap in newMin rack for family 1
            fsi.defIntervals[fam2].second = newMinF1; // fam2 can also go there 
        else // else, fam2 can go on rack juste before newMinF1
            fsi.defIntervals[fam2].second = newMinF1 - 1; 

        int capRackMax = data.capRacks[newMaxF2]; // get cap of new Max f2 
        int sizeRackMax = solution.RackToProd[newMaxF2].size(); // get occupied cap

        if(sizeRackMax < capRackMax) // if enough space in newMaxF2
            fsi.defIntervals[fam1].first = newMaxF2; // fam1 can also go there
        else // else, fam1 can go on the rack right next to newMaxF2
            fsi.defIntervals[fam1].first = newMaxF2 + 1; 
    }

}

void Conductor::updapteOar(int fam1, int fam2, int f1min, int f2max, const std::vector<std::vector<int>>& copyRackToProd) {

    int rackCopy = 0; // rack in copyRackToProd 0 <-> f1min
    int prod; 
    for(int r = f1min; r <= f2max; ++r) { 
        for(int p = 0; p < (int)solution.RackToProd[r].size(); ++p) {
            prod = solution.RackToProd[r][p]; // product in position p in rack r in sol after swap
            int prodBeforeSwap = copyRackToProd[rackCopy][p]; // product in position p in rack rackCopy(<->r) before the swap
            if((data.fam.prodToFam[prodBeforeSwap] == fam1) 
            || (data.fam.prodToFam[prodBeforeSwap] == fam2)) { // if the product before swap was in fam1 or fam2
                //update orderAndRacks for prodBefore
                for(const int& o : data.ords.prodToOrds[prodBeforeSwap]) { // for each order 
                    if(--oAr.orderOccurRacks[o][r] == 0) // if order o didn't use r
                        eraseSortedVec(r, oAr.orderToRacks[o]); // add r  
                }
                //update orderAndRacks for prod
                for(const int& o : data.ords.prodToOrds[prod]) { // for each order 
                    if(++oAr.orderOccurRacks[o][r] == 1) // if order o didn't use r
                        addSortedVec(r, oAr.orderToRacks[o]); // add r
                }
            }
        }  
        rackCopy++; // next rack 
    }
}

bool Conductor::applyMooveFamily(int fam1, int fam2, int f1min, int f2max, std::vector<int>& prodOrder) {

    placeProducts(fam1, fam2, f1min, f2max, prodOrder); // "make" the swap
    int newCost = computeSolValue(solution, data);
    if(newCost < val + (val * 5) / 100) { 
        val = newCost;
        return true;
    }else{ // we don't make the swap
        return false;
    }
}

void Conductor::repairSolution(int fam1, int fam2, int f1min, int f2max, std::vector<std::vector<int>> copyRackToProd) {

    int rackCopy = 0; // rack in copyRackToProd 0 <-> f1mon 

    for(int r = f1min; r <= f2max; ++r) { 
        for(int p = 0; p < (int)solution.RackToProd[r].size(); ++p) {
            
            int oldProd = solution.RackToProd[r][p]; // the product in rack r position p beforr repaire sol
            int newProd = copyRackToProd[rackCopy][p]; // the product we will place in rack r position p

            if((data.fam.prodToFam[oldProd] == fam1) 
            || (data.fam.prodToFam[oldProd] == fam2)){ // if oldProd's family were fam1 or fam2

                solution.RackToProd[r][p] = newProd;
                solution.prodToRack[newProd] = r;
            }

        }
        rackCopy++; //next rack
    }     
}

bool Conductor::SwapFamily(int famId1, int famId2) {
    
    if(famId1 +1 != famId2) throw std::runtime_error("non consecutive families");

    int fam1 = fsi.famOrder[famId1]; int fam2 = fsi.famOrder[famId2]; // recover fam1 and fam2   
    int f1min = fsi.famIntervals[fam1].first; /// recover bounds of fam1 and fam2
    int f2max = fsi.famIntervals[fam2].second;

    std::vector<std::vector<int>> CopyRackToProd = copyRackToProd(f1min, f2max); // copy the solution for products of fam1 and fam2
    std::vector<int> prodOrder;
    int FirstProdF1 = computeProdOrder(fam1, fam2, prodOrder);
    std::cout << "prodOrder size : " << prodOrder.size() << std::endl;
    //abort(); 
    bool isMooveApply = applyMooveFamily(fam1, fam2, f1min, f2max, prodOrder);
  
    if(isMooveApply){ // if we make the swap
        updapteOar(fam1, fam2, f1min, f2max, CopyRackToProd); // update the data
        updateFsi(famId1, famId2, FirstProdF1, prodOrder);
        return true;

    }
    // if we don't accept the move -> repair solution
    repairSolution(fam1, fam2, f1min, f2max, CopyRackToProd);
    return false;
}

void Conductor::swapFamSAA(int nbIterMax) {

    std::random_device rd; // create our random generator ID 
    std::mt19937 generator(rd()); 

    SimulatedAnnealing(); // apply SAA 1 first time
    
    Solution bestSol = solution; 
    int bestVal = val; // store best Solution and best value 

    int iter = 0; 
    bool applySwap; 

    while(iter < nbIterMax) {

        int f1Index, f2Index; 
        f1Index = generateRandomInt(generator, 0, data.nbFam-2);  // get random fam
        f2Index = f1Index + 1; 

        applySwap = false; // will be true if swapFam consider an upgrade 
        applySwap = SwapFamily(f1Index, f2Index); 

        if(applySwap) {
            SimulatedAnnealing(); 
            if(val < bestVal) {
                bestVal = val; bestSol = solution; 
            }
        }

        iter++; 
    }

    val = bestVal; solution = bestSol; 
}

void Conductor::SmartMultipleFamSwaps(int nbSwaps, int currentIter, int& globalBestVal) {

    // On répète l'opération 'nbSwaps' fois
    for (int k = 0; k < nbSwaps; ++k) {

        int bestLocalDelta = std::numeric_limits<int>::max(); // Meilleure amélioration pour CE pas
        int bestFam1Index = -1;
        int bestFam2Index = -1;
        
        // Structures pour mémoriser le meilleur mouvement de cette étape k
        std::vector<int> bestProdOrder;
        std::vector<std::vector<int>> bestCopy;
        int bestFirstProdF1 = -1;
        int bestStepVal = -1;

        // --- 1. EXPLORATION DU VOISINAGE (Trouver le meilleur coup pour l'étape k) ---
        for(int i = 0; i < data.nbFam - 1; ++i) { 

            int fam1 = fsi.famOrder[i]; 
            int fam2 = fsi.famOrder[i+1];
            
            int f1min = fsi.famIntervals[fam1].first;
            int f2max = fsi.famIntervals[fam2].second;

            // Simulation du Swap
            std::vector<std::vector<int>> copy = copyRackToProd(f1min, f2max);
            std::vector<int> prodOrder;
            int FirstProdF1 = computeProdOrder(fam1, fam2, prodOrder);
            placeProducts(fam1, fam2, f1min, f2max, prodOrder);
            
            // Évaluation
            int currentSwapCost = computeSolValue(solution, data);

            // --- LOGIQUE TABOU ---
            // Le mouvement est tabou si la date de fin d'interdiction est future
            bool isTabu = (tabuList[i] > currentIter); 
            
            // Aspiration : on s'en fiche du tabou si c'est un record absolu
            bool isAspiration = (currentSwapCost < globalBestVal); 

            // On cherche le meilleur coup DE CETTE ÉTAPE (k)
            if ((!isTabu && currentSwapCost < bestLocalDelta) || (isTabu && isAspiration)) {
                
                bestLocalDelta = currentSwapCost;
                bestStepVal = currentSwapCost;

                bestFam1Index = i;
                bestFam2Index = i+1;
                bestFirstProdF1 = FirstProdF1;
                bestProdOrder = prodOrder;
                bestCopy = copy;
            }

            // Annulation (Backtrack) pour tester le suivant
            repairSolution(fam1, fam2, f1min, f2max, copy);
        }

        // --- 2. APPLICATION DU MEILLEUR MOUVEMENT DE L'ÉTAPE k ---
        
        if (bestFam1Index != -1) {
            
            // Application physique définitive pour cette étape
            int bestFam1 = fsi.famOrder[bestFam1Index];
            int bestFam2 = fsi.famOrder[bestFam2Index];
            int f1min = fsi.famIntervals[bestFam1].first;
            int f2max = fsi.famIntervals[bestFam2].second;

            placeProducts(bestFam1, bestFam2, f1min, f2max, bestProdOrder);
            updapteOar(bestFam1, bestFam2, f1min, f2max, bestCopy);
            updateFsi(bestFam1Index, bestFam2Index, bestFirstProdF1, bestProdOrder);
            
            // Mise à jour de la valeur courante
            val = bestStepVal;

            // MISE À JOUR TABOU IMMÉDIATE
            // C'est crucial : on interdit ce mouvement pour les prochains tours de la boucle k
            // et pour les itérations futures du recuit.
            tabuList[bestFam1Index] = currentIter + tabuTenure + k; 

            // Mise à jour du Global Best si nécessaire
            if (val < globalBestVal) {
                globalBestVal = val;
                // Optionnel : Sauvegarder la meilleure solution globale ici si tu as une variable 'bestGlobalSol'
                 std::cout << ">>> New Global Best found inside Tabu Step " << k << ": " << globalBestVal << std::endl;
            }

            // Debug optionnel
            // std::cout << "Step " << k << ": Swapped " << bestFam1 << "-" << bestFam2 << " Val: " << val << std::endl;

        } else {
            // Aucun mouvement possible (tout est tabou et rien n'améliore). 
            // On arrête la séquence pour ne pas perdre de temps.
            break; 
        }
    }
}

void Conductor::FindBestFamSwap() {

    std::cout << "solval : " << val << std::endl;

    int bestValFound = std::numeric_limits<int>::max();
    int bestFam1Index = -1; 
    int bestFam2Index = -1; 
    std::vector<int> bestProdOrder; 
    std::vector<std::vector<int>> bestCopy;
    int bestFirstProdF1 = -1; 
    
    for(int i = 0; i < data.nbFam-2; ++i) { // for each index in FamOrder 

        int fam1 = fsi.famOrder[i]; int fam2 = fsi.famOrder[i+1]; 
        int f1min = fsi.famIntervals[fam1].first; 
        int f2max = fsi.famIntervals[fam2].second; 

        std::vector<std::vector<int>> copy = copyRackToProd(f1min, f2max); 
        std::vector<int> prodOrder; 

        int FirstProdF1 = computeProdOrder(fam1, fam2, prodOrder); 
        placeProducts(fam1, fam2, f1min, f2max, prodOrder); // try swapping
        int swapCost = computeSolValue(solution, data); // get value of new swap

        if(swapCost < bestValFound) { // memorize best swap informations 
            bestFam1Index = i; 
            bestFam2Index = i+1;
            bestFirstProdF1 = FirstProdF1;   
            bestValFound = swapCost; 
            bestProdOrder = prodOrder; 
            bestCopy = copy; 
        }

        repairSolution(fam1, fam2, f1min, f2max, copy); // reppair and continue testing
    }

    // apply best Swap found  
    int bestFam1 = fsi.famOrder[bestFam1Index]; 
    int bestFam2 = fsi.famOrder[bestFam2Index]; 
    int f1min = fsi.famIntervals[bestFam1].first; 
    int f2max = fsi.famIntervals[bestFam2].second;
    placeProducts(bestFam1, bestFam2, f1min, f2max, bestProdOrder); 

    std::cout << "best val found : " << bestValFound << std::endl;
    std::cout << "famillies swapped : " << bestFam1 << "," << bestFam2 << std::endl;

    updapteOar(bestFam1, bestFam2, f1min, f2max, bestCopy); // update structures 
    updateFsi(bestFam1Index, bestFam2Index, bestFirstProdF1, bestProdOrder); 
    val = bestValFound; 

}

void Conductor::swapFamSAA2(int nbIterMax) {
    
    std::random_device rd; 
    std::mt19937 generator(rd()); 

    // Initialisation Tabou
    if((int)tabuList.size() != data.nbFam) tabuList.assign(data.nbFam, 0);
    else std::fill(tabuList.begin(), tabuList.end(), 0);

    SimulatedAnnealing();
    
    Solution bestSol = solution;
    int bestVal = val; 
    
    int iter = 0;
    while(iter < nbIterMax) {

        // --- CHANGEMENT ICI ---
        // On demande 5 swaps intelligents à la suite
        // L'état de la solution change 5 fois à l'intérieur de cette fonction
        SmartMultipleFamSwaps(5, iter, bestVal);

        // Ensuite on optimise finement ce nouvel état
        SimulatedAnnealing();

        if(val < bestVal) {
            bestVal = val;
            bestSol = solution;
            std::cout << "ITER " << iter << " NEW BEST: " << bestVal << std::endl;
        } else {
            // Optionnel : Si le résultat est mauvais, on peut vouloir revenir à bestSol 
            // pour ne pas dériver trop loin, ou au contraire laisser dériver (Diversification).
            // Pour l'instant, on laisse dériver (c'est le but du Tabou).
        }

        iter++;
    }

    val = bestVal;
    solution = bestSol;
}

void Conductor::startTimer() {
    start = std::chrono::steady_clock::now(); 
}

double Conductor::stopTimer() {
    auto stop = std::chrono::steady_clock::now(); 
    std::chrono::duration<double> duration = stop - start; 
    return duration.count(); 
}

void Conductor::writeSolution(int numInstance, double temp, int nbIter, int val, double time, int solgen) {
    
    std::ofstream write("../Stats/genSolMethods.txt", std::ios::app); 
    if(!write.is_open()) // check if opened
        throw std::runtime_error("writeSolution : couldn't open file"); 
    write << numInstance << " " << temp << " " << nbIter << " " << val << " " << time << " " << solgen << std::endl; // write data
    write.close(); 

}

void Conductor::StatsSaa(int numInstance) { // give numInstance in argument so it can write in file 

    std::vector<double> allTemps = {100,-1.0}; 
    std::vector<int> allNbIter = {20}; 
    std::vector<int> allSolGen = {1,2,3,4}; // 1 : coi , 2 : initSol, 3 : TSP + pushed on left prods, 4 : bestInsert  


    for(double temp : allTemps) { // for each temp
        for(int iter : allNbIter) { // for each nbIter
            for(int gen : allSolGen) { // for each gen methods 

                if(gen == 1) callInitSolCoi(); 
                if(gen == 2) callInitSol(); 
                if(gen == 3) callInitSolTSP(); 
                if(gen == 4) calInitSolBestInsert(); 

                SAA SaaInstance(data, solution, val, oAr, aInfos, fsi); 
                std::cout << "temp : " << temp << " -- iter : " << iter 
                    << " -- solgen : "<< gen << std::flush; 
                startTimer(); 
                SaaInstance.optimize2(temp, iter); // apply SAA 
                double time = stopTimer(); // get time 
                std::cout << " -- value : " << val;
                std::cout << " -- time needed : " << time << std::endl;
                writeSolution(numInstance, temp, iter, val, time, gen);  // write data in file

            }
        }
    }
}

