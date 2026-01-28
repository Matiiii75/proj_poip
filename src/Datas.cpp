#include "Datas.hpp"

/* display functions */

void Familly::displayDatas() {

    std::cout << "#### ENTERED FAMILLY DISPLAY FUNCTION ####" << std::endl;
    std::cout << std::endl;

    for(int i = 0; i < (int)prodToFam.size(); ++i) // prodToFam display
        std::cout << i << "->" << "familly " << prodToFam[i] << std::endl;
    std::cout << std::endl;

    for(int i = 0; i < (int)famToProd.size(); ++i) { // famToProd display
        std::cout << "fam : " << i << " : ";
        for(const auto& prods : famToProd[i]) 
            std::cout << prods << " ";  
    }

    std::cout << std::endl << std::endl;
}

void Orders::displayDatas() {

    std::cout << "#### ENTERED ORDERS DISPLAY FUNCTION ####" << std::endl;
    std::cout << std::endl;

    for(int i = 0; i < (int)ordsToProd.size(); ++i) { // orders to product display
        std::cout << "order " << i << " : "; 
        for(const auto& prods : ordsToProd[i])
            std::cout << prods << " ";
        std::cout << std::endl; 
    }

    std::cout << std::endl;
    
    for(int i = 0; i < (int)prodToOrds.size(); ++i) { // product to order display 
        std::cout << "product " << i << " is in : "; 
        for(const auto& order : prodToOrds[i]) 
            std::cout << order << " "; 
        std::cout << std::endl;
    }

    std::cout << std::endl << std::endl;

}


void Datas::readMetaDatas(const std::string& f1) {

    std::ifstream f(f1); 
    if(!f.is_open()) 
        throw std::runtime_error("couldn't open file");

    f >> nbRacks >> nbLoc >> aeration >> nbProd >> nbFam >> nbAisle >> nbOrd; 

    // resizing structures

    capRacks.resize(nbRacks); 
    dists.resize(nbRacks, std::vector<int>(nbRacks)); 
    fam.prodToFam.resize(nbProd); 
    fam.famToProd.resize(nbFam); 
    ords.ordsToProd.resize(nbOrd); 
    ords.prodToOrds.resize(nbProd); 
    aisle.resize(nbAisle); 

}   

void Datas::readRackCapacity(const std::string& f2) {

    std::ifstream f(f2); 
    if(!f.is_open()) 
        throw std::runtime_error("couldn't open file");

    f >> nbRacks; 
    for(int i = 0; i < nbRacks; ++i) 
        f >> capRacks[i]; 

}

void Datas::readDistances(const std::string& f3) {

    std::ifstream f(f3); 
    if(!f.is_open())
        throw std::runtime_error("couldn't open file"); 

    int nbNodes;
    f >> nbNodes; 

    for(int i = 0; i < nbNodes; ++i) { // iterate over nb of racks both time
        for(int j = 0; j < nbNodes; ++j) {
            f >> dists[i][j]; 
        }
    }

}

void Datas::readProdCircuits(const std::string& f4) {

    std::ifstream f(f4); 
    if(!f.is_open())
        throw std::runtime_error("couldn't open file"); 

    f >> nbProd; 
    
    for(int i = 0; i < nbProd; ++i) { 
        int currFam; // store currFamilly
        f >> currFam; 
        --currFam; // re indexation of Fam
        fam.prodToFam[i] = currFam; // memorize product i is in familly currFam
        fam.famToProd[currFam].insert(i); // memorize currFam is assigned to product i
    }   

}

void Datas::readAisleRacks(const std::string& f5) {

    std::ifstream f(f5); 
    if(!f.is_open())
        throw std::runtime_error("couldn't open file");
        
    f >> nbAisle; 
    for(int i = 0; i < nbAisle; ++i) {
        
        int currNbRacks; 
        int temp; 

        f >> currNbRacks; 
        aisle[i] = currNbRacks+currNbRacks*i; 

        for(int j = 0; j < currNbRacks; ++j) 
            f >> temp;

    }

}

void Datas::readOrders(const std::string& f6) {

    std::ifstream f(f6); 
    if(!f.is_open())
        throw std::runtime_error("couldn't open file");

    f >> nbOrd; 
    for(int i = 0; i < nbOrd; ++i) { // iterate over number of order
        
        int currNbProd; 
        f >> currNbProd; // get number of products in order i
        
        for(int j = 0; j < currNbProd; ++j) {
            
            int currProduct; 
            f >> currProduct; 
            
            ords.ordsToProd[i].insert(currProduct); // memorize product is in order i 
            ords.prodToOrds[currProduct].insert(i); // memorize order i is in currProduct 
        }
    }

}

Datas::Datas(const allFiles& allfiles) {

    readMetaDatas(allfiles.f1);
    readRackCapacity(allfiles.f2); 
    readDistances(allfiles.f3);     // call every readers
    readProdCircuits(allfiles.f4); 
    readAisleRacks(allfiles.f5); 
    readOrders(allfiles.f6);

}

void Datas::displayDatas() {

    std::cout << "#### ENTERED DISPLAY DATA FUNCTION #### " << std::endl << std::endl;

    std::cout << "nb racks : " << nbRacks << std::endl;
    std::cout << "nb locations : " << nbLoc << std::endl;
    std::cout << "nb products : " << nbProd << std::endl;
    std::cout << "nb familly (circuits) : " << nbFam << std::endl;
    std::cout << "nb of aisles : " << nbAisle << std::endl;
    std::cout << "nb of orders : " << nbOrd << std::endl;   
    std::cout << "aeration : " << aeration << std::endl;
    std::cout << std::endl;

    // distance displays

    std::cout << "#### DISTANCE DISPLAYING ####" << std::endl << std::endl;
    for(int i = 0; i < (int)dists.size(); ++i) {
        for(int j = 0; j < (int)dists.size(); ++j) {
            std::cout << dists[i][j] << " "; 
        }
        std::cout << std::endl;
    }
    std::cout << std::endl << std::endl;

    //rack cap display 

    std::cout << "#### RACKS CAPACITY ####" << std::endl << std::endl;
    for(const auto& cap : capRacks)
        std::cout << cap << " "; 
    std::cout << std::endl << std::endl;

    // aisle display 

    std::cout << "#### DISPLAY AISLE RACK last id ####" << std::endl << std::endl;
    for(const auto& rack : aisle) 
        std::cout << rack << " "; 
    std::cout << std::endl << std::endl;

    fam.displayDatas(); 
    ords.displayDatas(); 

}


