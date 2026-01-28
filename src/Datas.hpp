#pragma once 

#include <vector>
#include <iostream>
#include <unordered_set>
#include <unordered_map>
#include <fstream>

struct Familly { // will contain infos relatives to fam and products 
    std::vector<int> prodToFam; 
    std::vector<std::unordered_set<int>> famToProd; 
    void displayDatas(); 
}; 

struct Orders {
    std::vector<std::unordered_set<int>> ordsToProd; // fastly check products associated to orders
    std::vector<std::unordered_set<int>> prodToOrds; // fastly check orders associated to product
    void displayDatas(); 
};

struct allFiles {
    std::string f1,f2,f3,f4,f5,f6; 
};

struct Datas {

    /* ATTRIBUTS */

    int nbRacks, nbLoc, nbProd, nbFam, nbAisle, nbOrd;
    int aeration; 

    std::vector<int> capRacks; // for racks capacity
    std::vector<std::vector<int>> dists; // distances 
    std::vector<int> aisle; 

    Familly fam;  
    Orders ords; 

    /* METHODS */

    Datas(const allFiles& allfiles); 

    void readMetaDatas(const std::string& f1); // read meta datas
    void readRackCapacity(const std::string& f2); 
    void readDistances(const std::string& f3); 
    void readProdCircuits(const std::string& f4); 
    void readAisleRacks(const std::string& f5); 
    void readOrders(const std::string& f6); 

    void displayDatas(); 

}; 