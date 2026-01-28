#pragma once 

#include <vector>
#include <iostream>
#include <unordered_set>
#include <unordered_map>
#include <fstream>
#include <cmath>

/**
 * @brief Contains Familly related infos 
 */
struct Familly {
    std::vector<int> prodToFam; // wich fam associated to each prod
    std::vector<std::unordered_set<int>> famToProd; // wich prods associated to each fam
    void displayData(); 
}; 

/**
 * @brief Contains Orders related infos
 */
struct Orders {
    std::vector<std::unordered_set<int>> ordsToProd; // fastly check products associated to orders
    std::vector<std::unordered_set<int>> prodToOrds; // fastly check orders associated to product
    void displayData(); 
};

/**
 * @brief Contains all 6 files defining a complete instance 
 */
struct allFiles {
    std::string f1,f2,f3,f4,f5,f6; 
};

struct Solution{
    std::vector<int> prodToRack; //for each product store his rack
    std::vector<std::vector<int>> RackToProd; //for each rack store his products
};

struct Data {

    /* ATTRIBUTES */

    int nbRacks, nbLoc, nbProd, nbFam, nbAisle, nbOrd;
    int aeration; // aeration defined as a 

    std::vector<int> capRacks; // for racks capacity
    std::vector<std::vector<int>> dists; // distances 
    std::vector<int> aisle; // Contains biggest rack id of the aisle 
    std::vector<int> capRacksAer; // Constrained aeration capacities

    Familly fam;  
    Orders ords; 

    /* METHODS */

    /**
     * @brief struct constructor that initiate problem values
     * @param allfiles struct containing of files of instance
     */
    Data(const allFiles& allfiles); 

    void readMetaData(const std::string& f1); // read metadatas.txt
    void readRackCapacity(const std::string& f2); // read rack_capacity.txt
    void readDistances(const std::string& f3); // read rack_adjacency_matrix.txt
    void readProdCircuits(const std::string& f4); // read product_circuit.txt
    void readAisleRacks(const std::string& f5); // read aisle_racks.txt
    void readOrders(const std::string& f6); // read orders.txt
        
    void displayData(); // display all data in the structure 

    std::vector<int> InitcapRacksAer(); // Initilisation of constrained aeration capacities


}; 