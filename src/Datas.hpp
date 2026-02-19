#pragma once 

#include <vector>
#include <iostream>
#include <unordered_set>
#include <unordered_map>
#include <map>
#include <fstream>
#include <cmath>
#include <set>
#include <queue>
#include <algorithm>
#include <random>
#include <chrono>


/**
* @brief Contains Familly related infos 
*/
struct Familly {

    /* ATTRIBUTES */
    std::vector<int> prodToFam; // wich fam associated to each prod
    std::vector<std::unordered_set<int>> famToProd; // wich prods associated to each fam

    /* METHODS */
    void displayData(); // display family's info
}; 

/**
* @brief Contains Orders related infos
*/
struct Orders {

    /* ATTRIBUTES */
    std::vector<std::unordered_set<int>> ordsToProd; // fastly check products associated to orders
    std::vector<std::unordered_set<int>> prodToOrds; // fastly check orders associated to product
    std::vector<std::vector<bool>> isProdInOrd; // for better performance 

    /* METHODS */
    void displayData(); // display order's info
};

/**
* @brief Contains all 6 files defining a complete instance 
*/
struct allFiles {

    /* ATTRIBUTES */
    std::string f1,f2,f3,f4,f5,f6;
    
    /* METHODS */
    void computeAllFiles(int numInstance); 
};




struct Solution{

    /* ATTRIBUTES */
    std::vector<int> prodToRack; //for each product store his rack
    std::vector<std::vector<int>> RackToProd; //for each rack store his products

    /* METHODS */
    /**
    * @brief write solution according to convention 
    * @param name name of the folder we write in 
    * @param numInstance number given in arg of main to id instance 
    */
   void displaySolution(int numInstance);


};

/**
* @brief Contains all the info of instance
*/

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
    std::vector<int> initCapRacksAer() const; // assign initial empty racks in order to respect aeration constr

    void throwErrorFile(const std::string& where); // throw error and display where it happened

        
    /**
    * @brief compute matrix of boleans where isProdInOrd[o][prod] = true if prod is in order o
    * @warning must be pre computed in main if data are const given in method (such as gradient) argument
    */ 
    void computeIsProdInOrd();
}; 

