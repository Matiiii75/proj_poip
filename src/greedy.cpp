#include "greedy.hpp"

Coi::Coi(const Data& data){
    coiProd.resize(data.nbFam); coiFam.resize(data.nbFam); // resize maps with data's informations

    for(int f = 0; f < (int) data.nbFam; ++f)   coiFam[f].first = f; // initialise map
      
    for(int p = 0; p < data.nbProd; ++p){ 
        double temp = data.ords.prodToOrds[p].size(); // compute coi of product p
        int pFam = data.fam.prodToFam[p]; // save family of p
        coiProd[pFam].push_back({p, 1.0 / temp}); coiFam[pFam].second += temp; // update values of maps
    } 
    for(int f = 0; f < data.nbFam; ++f) {
        coiFam[f].second = 1 /coiFam[f].second; coiFam[f].second *= data.fam.famToProd[f].size(); // compute the coi of f
    }

    std::sort(coiFam.begin(), coiFam.end(), [](const auto& a, const auto& b) { // sort families by descending coi
        return a.second > b.second;
    });

    for (auto& row : coiProd) // sort products by descending coi
        std::sort(row.begin(), row.end(), [](auto& a, auto& b){ return a.second > b.second; });


}

Solution coiGreedy(const Data& data){
    Coi coi(data); // create coi struct
    Solution sol; // create sol struct
    sol.prodToRack.resize(data.nbProd); sol.RackToProd.resize(data.nbRacks); // initialize vectors of sol's sizes
    std::vector<int> capRacksAer = data.initCapRacksAer();  //compute aeration



    int rack = 1; // initialize current rack 
    for(const auto& [f, vf] : coi.coiFam ){ // loop over families sorted by increasing COI
        for(const auto& [p, vp] : coi.coiProd[f]){ // loop over products of f sorted by increasing COI
            bool place = false; // bolean saying if p is place in a rack : initialize to false


            while(!place){ // iterate while p isn't place

                if (rack >= data.nbRacks) {
                    throw std::runtime_error("Product placement failed: no racks available");
                }

                if((int) sol.RackToProd[rack].size() + 1 <= capRacksAer[rack]){ // if there is the space in rack for a new product
                    sol.prodToRack[p]= rack; sol.RackToProd[rack].push_back(p); place = true; //update sol and bolean value : p is place  
                }else{++rack;} // if there isn't enough of space : pass to next rack                     
            }
        }
    }

    return sol;
}

Solution initSol(const Data& data) {

    Solution sol; 
    sol.RackToProd.resize(data.nbRacks); 
    sol.prodToRack.resize(data.nbProd); 

    std::vector<int> newCapRack = data.initCapRacksAer(); 

    for(int i = 0; i < data.nbFam; ++i) { // for each familiy
        for(const auto& prod : data.fam.famToProd[i]) { // for each product in this famiily
            
            int lastLocSeen = 0; 
            for(int j = lastLocSeen; j < data.nbRacks; ++j) { //  iterate over every rack from 0

            // from lastLocSeen, keep going until finding new free location for prod 
                if(newCapRack[j] == 0) continue; // ignore if rack is full

                // otherwise 
                sol.prodToRack[prod] = j; 
                sol.RackToProd[j].push_back(prod); 
                --newCapRack[j]; // update : we used 1 unit of capacity for prod
                lastLocSeen = j; // we memorize that we stopped at lastLocSeen
                break; 
            }
        }
    }

    return sol; 
}

Solution initSolRandom(const Data& data) {

    std::random_device rd; std::mt19937 gen(rd());

    Solution sol;
    sol.RackToProd.resize(data.nbRacks); sol.prodToRack.resize(data.nbProd); //init sol sizes

    std::vector<int> newCapRack = data.initCapRacksAer(); 

    std::vector<int> orderFam = genRandomOrderFam(data);

    std::vector<std::vector<int>> OrderProd(data.nbFam);

    for(int f = 0 ; f < data.nbFam; ++f) { // for each family
        OrderProd[f].assign(data.fam.famToProd[f].begin(),data.fam.famToProd[f].end()); //convert unordered_set in vector
        shuffleVector(OrderProd[f], gen); // create a random order of th pruduct of the family
    }

    int rack = 1; // initialize current rack 

    for(int i = 0; i < data.nbFam; ++i) {
        for(int j = 0; j < (int) OrderProd[i].size(); ++j){
            int prod = OrderProd[i][j]; //get product at index j for family in index i
            
            bool place = false; // bolean saying if prod is place in a rack : initialize to false
            while(!place){
                if (rack >= data.nbRacks) 
                    throw std::runtime_error("Product placement failed: no racks available");

                if((int) sol.RackToProd[rack].size() + 1 <= newCapRack[rack]){ // if there is the space in rack for a new product
                    sol.prodToRack[prod]= rack; sol.RackToProd[rack].push_back(prod); place = true; //update sol and bolean value : p is place//update bolean value : p is place    
                }else{++rack;} // if there isn't enough of space : pass to next rack                     
            }
        }
    }

    return sol;
}

void updateCorrMatrix(std::vector<std::vector<int>>& famCorr, 
    const std::vector<int>& famInOrder)
{
    for(int i = 0; i < (int)famInOrder.size()-1; ++i) { // for each fam 
        for(int j = i+1; j < (int)famInOrder.size(); ++j) {
            famCorr[famInOrder[i]][famInOrder[j]]++; // increment seen famillies
            famCorr[famInOrder[j]][famInOrder[i]]++; 
        }
    }
}

std::vector<std::vector<int>> buildCorrFam(const Data& data) {

    std::vector<std::vector<int>> famCorr(data.nbFam, std::vector<int>(data.nbFam, 0)); 

    for(int o = 0; o < data.nbOrd; ++o) { // for each order

        std::vector<bool> isFamInOrd(data.nbFam, false); // declarations 
        std::vector<int> famInOrder; 

        for(int prod : data.ords.ordsToProd[o]) { // for each prod in order 
            int prodFam = data.fam.prodToFam[prod]; // get familly of prod 
            if(isFamInOrd[prodFam]) continue; // if already seen -> skip
            famInOrder.push_back(prodFam); // add in order 
            isFamInOrd[prodFam] = true; // memo already seen
        }
        updateCorrMatrix(famCorr, famInOrder); // update infos 
    }

    return famCorr; 
}

void CorrelationSameFam(Correlation& out, const Data& data) {
    for (int o = 0; o < data.nbOrd; ++o) {
        const auto& prods = data.ords.ordsToProd[o]; // products contained in order o

        for (auto it = prods.begin(); it != prods.end(); ++it) {
            auto jt = it; ++jt;

            for (; jt != prods.end(); ++jt) {
                int p = *it;int q = *jt;// product pair appearing together

                int fp = data.fam.prodToFam[p]; int fq = data.fam.prodToFam[q];

                if (fp != fq) continue;// keep only same-family pairs

                int a = std::min(p, q); int b = std::max(p, q);
                out.cor[a][b]++; // increment correlation count
            }
        }
    }
}

Correlation BuildCorSameFam(const Data& data, bool display) {
    Correlation out;
    out.cor.assign(data.nbProd, std::vector<int>(data.nbProd, 0));// resize matrix with data's informations

    CorrelationSameFam(out,data);
    
    if (display) {
        DisplaySame(out);
    }
    
    return out;
}

void CorrelationDiffFam(Correlation& out, const Data& data){

    for (int o = 0; o < data.nbOrd; ++o) {
        const auto& prods = data.ords.ordsToProd[o]; // products contained in order o
        
        for (auto it = prods.begin(); it != prods.end(); ++it) {
            auto jt = it; ++jt;
            
            for (; jt != prods.end(); ++jt) {
                int p = *it; int q = *jt; // product pair appearing together
                
                int fp = data.fam.prodToFam[p];
                int fq = data.fam.prodToFam[q];
                
                if (fp == fq) continue; // keep only different-family pairs
                
                int a = std::min(p, q); int b = std::max(p, q);// product-product correlation
                out.cor[a][b]++;
                
                int f1 = std::min(fp, fq); int f2 = std::max(fp, fq);// family-family correlation
                out.corFam[f1][f2]++;
            }
        }
    }
}

Correlation BuildCorDiffFam(const Data& data, bool display) {

    Correlation out;
    out.cor.assign(data.nbProd, std::vector<int>(data.nbProd, 0));// product-product matrix
    out.corFam.assign(data.nbFam, std::vector<int>(data.nbFam, 0));// family-family matrix
    
    CorrelationDiffFam(out,data);// compute correlations
    
    if (display) {
        DisplayDiff(out);// display matrices if required
    }

    return out;
}

void BuildOrderFam(std::vector<int>& FamOrd, std::vector<bool>& FamPlaced,
                   const Correlation& CorBetFam, const Data& data){

    int bestweight_init = -1;
    int fimax_init = -1;
    int fjmax_init = -1;

    for (int i = 0; i < data.nbFam; i++) {// find best initial pair
        for (int j = i + 1; j < data.nbFam; j++) {
            if (CorBetFam.corFam[i][j] > bestweight_init) {
                bestweight_init = CorBetFam.corFam[i][j];
                fimax_init = i;
                fjmax_init = j;
            }
        }
    }

    FamOrd.push_back(fimax_init);FamOrd.push_back(fjmax_init);// initialize order
    FamPlaced[fimax_init] = true;FamPlaced[fjmax_init] = true;

    while ((int)FamOrd.size() < data.nbFam) {

        int L = FamOrd.front();// left end
        int R = FamOrd.back();// right end
        int bestFam  = -1;
        int bestGain = -1;
        int bestSide = 0;

        for (int f = 0; f < data.nbFam; ++f) {
            if (FamPlaced[f] == false) {

                int gLeft  = CorBetFam.corFam[std::min(f, L)][std::max(f, L)];// gain if placed left
                int gRight = CorBetFam.corFam[std::min(f, R)][std::max(f, R)];// gain if placed right

                if (gLeft > bestGain) {
                    bestGain = gLeft;
                    bestFam  = f;
                    bestSide = 0;
                }

                if (gRight > bestGain || (gRight == bestGain && bestSide == 0)) {
                    bestGain = gRight;
                    bestFam  = f;
                    bestSide = 1;
                }
            }
        }

        if (bestFam == -1) {// fallback: pick any remaining family
            for (int i = 0; i < data.nbFam; ++i) {
                if (FamPlaced[i] == false) {
                    bestFam = i;
                    break;
                }
            }
        }

        if (bestSide == 0) FamOrd.insert(FamOrd.begin(), bestFam);// insert at best side
        else               FamOrd.push_back(bestFam);

        FamPlaced[bestFam] = true;// mark as placed
    }
}

void BuildOrderFamBestInsert(std::vector<int>& FamOrd, std::vector<bool>& FamPlaced,
                   const Correlation& CorBetFam, const Data& data){

    int bestweight_init = -1;
    int fimax_init = -1;
    int fjmax_init = -1;

    for (int i = 0; i < data.nbFam; i++) {// find best initial pair
        for (int j = i + 1; j < data.nbFam; j++) {
            if (CorBetFam.corFam[i][j] > bestweight_init) {
                bestweight_init = CorBetFam.corFam[i][j];
                fimax_init = i;
                fjmax_init = j;
            }
        }
    }

    FamOrd.push_back(fimax_init);FamOrd.push_back(fjmax_init);// initialize order
    FamPlaced[fimax_init] = true;FamPlaced[fjmax_init] = true;

    while ((int)FamOrd.size() < data.nbFam) {

        int L = FamOrd.front();// left end
        int R = FamOrd.back();// right end
        int bestFam  = -1;
        int bestGain = -1;
        int bestSide = -1;
        int bestPos = -1;

        for (int f = 0; f < data.nbFam; ++f) {
            if (FamPlaced[f] == false) {

                int gLeft  = CorBetFam.corFam[std::min(f, L)][std::max(f, L)];// gain if placed left
                int gRight = CorBetFam.corFam[std::min(f, R)][std::max(f, R)];// gain if placed right

                if (gLeft > bestGain) {
                    bestGain = gLeft;
                    bestFam  = f;
                    bestSide = 0;
                }

                if (gRight > bestGain || (gRight == bestGain && bestSide == 0)) {
                    bestGain = gRight;
                    bestFam  = f;
                    bestSide = 1;
                }
                //insertion entre deux Ã©lÃ©ments consÃ©cutifs
                for(int i = 0; i < (int)FamOrd.size()-2; i++){
                    int u = FamOrd[i];
                    int v = FamOrd[i+1];

                    // delta = nouveau - ancien
                    int gain = CorBetFam.corFam[std::min(u, f)][std::max(u, f)] + 
                     CorBetFam.corFam[std::min(f, v)][std::max(f, v)] - 
                      CorBetFam.corFam[std::min(u, v)][std::max(u, v)];
                    
                    if(gain > bestGain) {
                        bestGain = gain;
                        bestFam = f;
                        bestSide = 2;
                        bestPos = i;
                    }
                }
            }
        }

        if (bestFam == -1) {// fallback: pick any remaining family
            for (int i = 0; i < data.nbFam; ++i) {
                if (FamPlaced[i] == false) {
                    bestFam = i;
                    break;
                }
            }
        }

        if (bestSide == 0) {
            FamOrd.insert(FamOrd.begin(), bestFam);
        } else if (bestSide == 1) {
            FamOrd.push_back(bestFam);
        } else { 
            FamOrd.insert(FamOrd.begin() + bestPos + 1, bestFam);
        }

        FamPlaced[bestFam] = true;// mark as placed
    }
}

void BuildProdOrder(const Correlation& CorInFam, std::vector<int>& ProdOrd,
                    const Data& data, const std::vector<int>& FamOrd){

    for (int fidx = 0; fidx < (int)FamOrd.size(); ++fidx) {
        int fam = FamOrd[fidx];// current family

        std::vector<int>  ProdF;// product order for this family
        std::vector<bool> ProdPlaced(data.nbProd, false);// placed products

        if (data.fam.famToProd[fam].size() == 0) {// empty family
            continue;
        }

        if (data.fam.famToProd[fam].size() == 1) {// single product
            int p = *(data.fam.famToProd[fam].begin());
            ProdF.push_back(p);// insert the only product
        } else {
            int bestw = -1;
            int p0 = -1;
            int p1 = -1;

            for (auto it = data.fam.famToProd[fam].begin(); it != data.fam.famToProd[fam].end(); ++it) {
                auto jt = it; ++jt;
                for (; jt != data.fam.famToProd[fam].end(); ++jt) {
                    int p = *it;
                    int q = *jt;
                    int w = CorInFam.cor[std::min(p, q)][std::max(p, q)];// correlation weight
                    if (w > bestw) {
                        bestw = w;
                        p0 = p;
                        p1 = q;
                    }
                }
            }

            ProdF.push_back(p0);// initialize with best pair
            ProdF.push_back(p1);
            ProdPlaced[p0] = true;// mark placed
            ProdPlaced[p1] = true;

            if (data.fam.famToProd[fam].size() > 2) {
                while ((int)ProdF.size() < (int)data.fam.famToProd[fam].size()) {
                    int L = ProdF.front();// left end
                    int R = ProdF.back();// right end
                    int bestProd = -1;
                    int bestGain = -1;
                    int bestSide = 0;

                    for (auto it = data.fam.famToProd[fam].begin(); it != data.fam.famToProd[fam].end(); ++it) {
                        int p = *it;
                        if (ProdPlaced[p] == false) {
                            int gLeft  = CorInFam.cor[std::min(p, L)][std::max(p, L)];// gain left
                            int gRight = CorInFam.cor[std::min(p, R)][std::max(p, R)];// gain right

                            if (gLeft > bestGain) {
                                bestGain = gLeft;
                                bestProd = p;
                                bestSide = 0;
                            }
                            if (gRight > bestGain || (gRight == bestGain && bestSide == 0)) {
                                bestGain = gRight;
                                bestProd = p;
                                bestSide = 1;
                            }
                        }
                    }

                    if (bestProd == -1) {// fallback: pick any remaining product
                        for (auto it = data.fam.famToProd[fam].begin(); it != data.fam.famToProd[fam].end(); ++it) {
                            int p = *it;
                            if (ProdPlaced[p] == false) {
                                bestProd = p;
                                break;
                            }
                        }
                    }

                    if (bestSide == 0) ProdF.insert(ProdF.begin(), bestProd);// insert left
                    else               ProdF.push_back(bestProd);// insert right

                    ProdPlaced[bestProd] = true;// mark placed
                }
            }
        }

        ProdOrd.insert(ProdOrd.end(), ProdF.begin(), ProdF.end());// append to global order
    }
}

void BuildProdOrderBestInsert(const Correlation& CorInFam, std::vector<int>& ProdOrd,
                    const Data& data, const std::vector<int>& FamOrd){

    for (int fidx = 0; fidx < (int)FamOrd.size(); ++fidx) {
        int fam = FamOrd[fidx];// current family

        std::vector<int>  ProdF;// product order for this family
        std::vector<bool> ProdPlaced(data.nbProd, false);// placed products

        if (data.fam.famToProd[fam].size() == 0) {// empty family
            continue;
        }

        if (data.fam.famToProd[fam].size() == 1) {// single product
            int p = *(data.fam.famToProd[fam].begin());
            ProdF.push_back(p);// insert the only product
        } else {
            int bestw = -1;
            int p0 = -1;
            int p1 = -1;

            for (auto it = data.fam.famToProd[fam].begin(); it != data.fam.famToProd[fam].end(); ++it) {
                auto jt = it; ++jt;
                for (; jt != data.fam.famToProd[fam].end(); ++jt) {
                    int p = *it;
                    int q = *jt;
                    int w = CorInFam.cor[std::min(p, q)][std::max(p, q)];// correlation weight
                    if (w > bestw) {
                        bestw = w;
                        p0 = p;
                        p1 = q;
                    }
                }
            }

            ProdF.push_back(p0);// initialize with best pair
            ProdF.push_back(p1);
            ProdPlaced[p0] = true;// mark placed
            ProdPlaced[p1] = true;

            if (data.fam.famToProd[fam].size() > 2) {
                while ((int)ProdF.size() < (int)data.fam.famToProd[fam].size()) {
                    int L = ProdF.front();// left end
                    int R = ProdF.back();// right end
                    int bestProd = -1; int bestGain = -1;
                    int bestSide = -1; int bestPos = -1;

                    for (auto it = data.fam.famToProd[fam].begin(); it != data.fam.famToProd[fam].end(); ++it) {
                        int p = *it;
                        if (ProdPlaced[p] == false) {
                            int gLeft  = CorInFam.cor[std::min(p, L)][std::max(p, L)];// gain left
                            int gRight = CorInFam.cor[std::min(p, R)][std::max(p, R)];// gain right

                            if (gLeft > bestGain) { bestGain = gLeft; bestProd = p; bestSide = 0;}
                            if (gRight > bestGain || (gRight == bestGain && bestSide == 0)) {
                                bestGain = gRight; bestProd = p; bestSide = 1;
                            }
                            for(int i = 0; i < (int)ProdF.size()-1;++i) {
                                int u = ProdF[i];int v = ProdF[i+1];
                                int gain = CorInFam.cor[std::min(u, p)][std::max(u, p)] + 
                                CorInFam.cor[std::min(p, v)][std::max(p, v)] - 
                                CorInFam.cor[std::min(u, v)][std::max(u, v)];

                                if(gain > bestGain) {bestGain = gain; bestProd = p;bestSide = 2;bestPos = i;}
                            }

                        }
                    }

                    if (bestProd == -1) {// fallback: pick any remaining product
                        for (auto it = data.fam.famToProd[fam].begin(); it != data.fam.famToProd[fam].end(); ++it) {
                            int p = *it;
                            if (ProdPlaced[p] == false) {
                                bestProd = p;
                                break;
                            }
                        }
                    }

                    if (bestSide == 0) {
                        ProdF.insert(ProdF.begin(), bestProd);// insert left
                    } else if(bestSide == 1){
                        ProdF.push_back(bestProd);// insert right
                    } else {
                        ProdF.insert(ProdF.begin() + bestPos + 1, bestProd);
                    }

                    ProdPlaced[bestProd] = true;// mark placed
                }
            }
        }

        ProdOrd.insert(ProdOrd.end(), ProdF.begin(), ProdF.end());// append to global order
    }
}

void FillRacks(Solution& sol, const Data& data, const std::vector<int>& ProdOrd){
    std::vector<int> Capaer = data.initCapRacksAer();// capacities
    int r = 0;// current rack

    for (int idx = 0; idx < (int)ProdOrd.size(); ++idx) {
        int p = ProdOrd[idx];// current product

        while (r < data.nbRacks && Capaer[r] == 0) ++r;// skip full racks

        if (r >= data.nbRacks) {// safety check
            std::cout << "Error: not enough rack capacity to place all products.\n";
            break;
        }

        sol.prodToRack[p] = r;// assign rack
        sol.RackToProd[r].push_back(p);// update rack content
        --Capaer[r];// decrease capacity
    }
}

Solution GreedyBestInsert(const Data& data) {
    Solution sol;

    Correlation CorInFam  = BuildCorSameFam(data, false);// within-family correlations
    Correlation CorBetFam = BuildCorDiffFam(data, false);// between-family correlations

    CorBetFam.corFam = buildCorrFam(data);

    std::vector<int>  FamOrd;// family placement order
    std::vector<bool> FamPlaced(data.nbFam, false);// placed families

    //BuildOrderFam(FamOrd, FamPlaced, CorBetFam, data);// build family order
    BuildOrderFamBestInsert(FamOrd, FamPlaced, CorBetFam, data);// build family order
    
    std::vector<int> ProdOrd;// product placement order

    BuildProdOrder(CorInFam, ProdOrd, data, FamOrd);// build product order
    // BuildProdOrderBestInsert(CorInFam, ProdOrd, data, FamOrd);// build product order

    sol.prodToRack.assign(data.nbProd, -1);// initialize assignments
    sol.RackToProd.assign(data.nbRacks, {});// initialize rack contents

    FillRacks(sol, data, ProdOrd);// fill racks sequentially

    return sol;// return greedy solution
}

void DisplayDiff(Correlation out){
    // std::cout << "Product correlation matrix (DIFF_FAM)\n";
    // for (auto &row : out.cor) {
    //     for (auto v : row) std::cout << v << " ";
    //     std::cout << "\n";
    // }
    std::cout << "\nFamily correlation matrix (DIFF_FAM)\n";
    for (auto &row : out.corFam) {
        for (auto v : row) std::cout << v << " ";
        std::cout << "\n";
    }
}

void DisplaySame(Correlation out){
    std::cout << "Product correlation matrix (SAME_FAM)\n";
    for (auto &row : out.cor) {
        for (auto v : row) std::cout << v << " ";
        std::cout << "\n";
    }
}   
