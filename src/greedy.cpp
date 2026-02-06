#include "greedy.hpp"
#include <algorithm>

 

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
    sol.prodToRack.resize(data.nbProd); sol.RackToProd.resize(data.nbRacks); // initialize vectors of sol sizes
    std::vector<int> capRacksAer = data.initCapRacksAer();

    int rack = 1; // initialize current rack 
    for(const auto& [f, vf] : coi.coiFam ){ // loop over families sorted by increasing COI
        for(const auto& p : data.fam.famToProd[f]){ // loop over products of f sorted by increasing COI
            bool place = false; // bolean saying if p is place in a rack : initialize to false
            

            while(!place){ //iterate while p isn't place

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

Solution coiGreedyAmelioration(const Data& data){
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

Solution initSolRandom(const Data& data){

    std::random_device rd; std::mt19937 gen(rd());

    Solution sol;
    sol.RackToProd.resize(data.nbRacks); sol.prodToRack.resize(data.nbProd); //init sol sizes

    std::vector<int> newCapRack = data.initCapRacksAer(); 

    std::vector<int> orderFam = genRandomOrderFam(data);

    std::vector<std::vector<int>> OrderProd(data.nbFam);

    for(int f = 0 ; f < data.nbFam; ++f){ // for each family
        OrderProd[f].assign(data.fam.famToProd[f].begin(),data.fam.famToProd[f].end()); //convert unordered_set in vector
        shuffleVector(OrderProd[f], gen); // create a random order of th pruduct of the family
    }

    int rack = 1; // initialize current rack 

    for(int i = 0; i < data.nbFam; ++i){
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

    
    
