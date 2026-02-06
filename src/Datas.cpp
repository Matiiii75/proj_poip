#include "Datas.hpp"

/* display functions */

void Familly::displayData() {

    std::cout << "#### ENTERED FAMILLY DISPLAY FUNCTION ####" << std::endl;
    std::cout << std::endl;

    for(int i = 0; i < (int)prodToFam.size(); ++i) // prodToFam display
        std::cout << i << "->" << "familly " << prodToFam[i] << std::endl;
    std::cout << std::endl;
    for(int i = 0; i < (int)famToProd.size(); ++i) { // famToProd display
        std::cout << "fam : " << i << " : ";
        for(const auto& prods : famToProd[i]) std::cout << prods << " ";  
    }
    std::cout << std::endl << std::endl;
}

void Orders::displayData() {

    std::cout << "#### ENTERED ORDERS DISPLAY FUNCTION ####" << std::endl;
    std::cout << std::endl;

    for(int i = 0; i < (int)ordsToProd.size(); ++i) { // orders to product display
        std::cout << "order " << i << " : "; 
        for(const auto& prods : ordsToProd[i]) std::cout << prods << " ";
        std::cout << std::endl; 
    }   std::cout << std::endl;
    for(int i = 0; i < (int)prodToOrds.size(); ++i) { // product to order display 
        std::cout << "product " << i << " is in : "; 
        for(const auto& order : prodToOrds[i]) std::cout << order << " "; 
        std::cout << std::endl;
    }
    std::cout << std::endl << std::endl;
}

void allFiles::computeAllFiles(int numInstance) {
 
    std::string n1,n2,n3,n4,n5,n6;
    std::string path; 

    n1 = "metadata.txt"; n2 = "rack_capacity.txt";
    n3 = "rack_adjacency_matrix.txt"; n4 = "product_circuit.txt"; 
    n5 = "aisle_racks.txt"; n6 = "orders.txt"; 
    
    if(numInstance == 0) path = "warehouse_toy/"; 
    if(numInstance == 1) path = "warehouse_big_/warehouse_big_category/"; 
    if(numInstance == 2) path = "warehouse_big_/warehouse_big_family/"; 
    if(numInstance == 3) path = "warehouse_big_/warehouse_big_market/"; 

    f1 = "../data/" + path + n1; f2 = "../data/" + path + n2; 
    f3 = "../data/" + path + n3; f4 = "../data/" + path + n4; 
    f5 = "../data/" + path + n5; f6 = "../data/" + path + n6; 

}

void Solution::displaySolution(int numInstance) {

    // compute path from numInstance 
    std::string path; // path to file we write in 

    if(numInstance == 0) path = "../data/warehouse_toy/solutions/sol.txt"; 
    if(numInstance == 1) path = "../data/warehouse_big_/warehouse_big_category/solutions/sol.txt"; 
    if(numInstance == 2) path = "../data/warehouse_big_/warehouse_big_family/solutions/sol.txt"; 
    if(numInstance == 3) path = "../data/warehouse_big_/warehouse_big_market/solutions/sol.txt"; 

    std::ofstream write(path); // open output file
    int nbProd = prodToRack.size(); // compute  number of products

    write << nbProd << std::endl; // write number of froducts 
    for(int p = 0; p < nbProd; ++p) write << prodToRack[p] << std::endl; // for each product write his rack

    write.close(); // close output file

}

void Data::readMetaData(const std::string& f1) {

    std::ifstream f(f1); 
    if(!f.is_open()) throwErrorFile("readMetaData"); 

    f >> nbRacks >> nbLoc >> aeration >> nbProd >> nbFam >> nbAisle >> nbOrd; 

    capRacks.resize(nbRacks); // resizing structures
    dists.resize(nbRacks, std::vector<int>(nbRacks)); 
    fam.prodToFam.resize(nbProd); 
    fam.famToProd.resize(nbFam); 
    ords.ordsToProd.resize(nbOrd); 
    ords.prodToOrds.resize(nbProd); 
    aisle.resize(nbAisle); 

}   

void Data::readRackCapacity(const std::string& f2) {

    std::ifstream f(f2); 
    if(!f.is_open()) throwErrorFile("readRackCapacity"); 

    f >> nbRacks; 
    for(int i = 0; i < nbRacks; ++i) f >> capRacks[i]; 

}

void Data::readDistances(const std::string& f3) {

    std::ifstream f(f3); 
    if(!f.is_open()) throwErrorFile("readDistances"); 

    int nbNodes;
    f >> nbNodes; 

    for(int i = 0; i < nbNodes; ++i) // iterate over nb of racks both time
        for(int j = 0; j < nbNodes; ++j) f >> dists[i][j]; 

}

void Data::readProdCircuits(const std::string& f4) {

    std::ifstream f(f4); 
    if(!f.is_open()) throwErrorFile("readProdCircuits");  

    f >> nbProd; 
    for(int i = 0; i < nbProd; ++i) { 
        int currFam; f >> currFam;// store currFamilly 
        //--currFam; // re indexation of Fam
        fam.prodToFam[i] = currFam; // memorize product i is in familly currFam
        fam.famToProd[currFam].insert(i); // memorize currFam is assigned to product i
    }   

}

void Data::readAisleRacks(const std::string& f5) {

    std::ifstream f(f5); 
    if(!f.is_open()) throwErrorFile("readAisleRacks"); 
        
    f >> nbAisle; 
    for(int i = 0; i < nbAisle; ++i) {
        int currNbRacks; 
        f >> currNbRacks; 
        for(int j = 0; j < currNbRacks; ++j) {
            f >> aisle[i]; 
        }
    }

}

void Data::readOrders(const std::string& f6) {

    std::ifstream f(f6); 
    if(!f.is_open()) throwErrorFile("readOrders"); 

    f >> nbOrd; 
    for(int i = 0; i < nbOrd; ++i) { // iterate over number of order
        
        int currNbProd; f >> currNbProd; // get number of products in order i
        for(int j = 0; j < currNbProd; ++j) {
            int currProduct; f >> currProduct; 
            ords.ordsToProd[i].insert(currProduct); // memorize product is in order i 
            ords.prodToOrds[currProduct].insert(i); // memorize order i is in currProduct 
        }
    }

}

Data::Data(const allFiles& allfiles) {

    readMetaData(allfiles.f1);
    readRackCapacity(allfiles.f2);
    readDistances(allfiles.f3);     // call every readers
    readProdCircuits(allfiles.f4);
    readAisleRacks(allfiles.f5); 
    readOrders(allfiles.f6);

}

void Data::displayData() {

    std::cout << "#### ENTERED DISPLAY DATA FUNCTION #### " << std::endl << std::endl;
    std::cout << "nb racks : " << nbRacks << std::endl;
    std::cout << "nb locations : " << nbLoc << std::endl;
    std::cout << "nb products : " << nbProd << std::endl;
    std::cout << "nb familly (circuits) : " << nbFam << std::endl;
    std::cout << "nb of aisles : " << nbAisle << std::endl;
    std::cout << "nb of orders : " << nbOrd << std::endl;   
    std::cout << "aeration : " << aeration << std::endl << std::endl;

    // distance displays
    std::cout << "#### DISTANCE DISPLAYING ####" << std::endl << std::endl;
    for(int i = 0; i < (int)dists.size(); ++i) {
        for(int j = 0; j < (int)dists.size(); ++j) std::cout << dists[i][j] << " "; 
        std::cout << std::endl;
    } std::cout << std::endl << std::endl;

    //rack cap display 
    std::cout << "#### RACKS CAPACITY ####" << std::endl << std::endl;
    for(const auto& cap : capRacks)std::cout << cap << " "; 
    std::cout << std::endl << std::endl;

    // aisle display 
    std::cout << "#### DISPLAY AISLE RACK last id ####" << std::endl << std::endl;
    for(const auto& rack : aisle) std::cout << rack << " "; 
    std::cout << std::endl << std::endl;

    fam.displayData(); 
    ords.displayData(); 

}

std::vector<int> Data::initCapRacksAer() const {

    std::vector<int> racksCapCopy = capRacks; 
    for(int i = 0; i < (int)aisle.size(); ++i) { // for each aisle

        int capAisle = 0; 
        int lastRack = aisle[i]; // store last rack of current aisle
        int firstRack; 

        if (i > 0) { // case > first aisle 
            firstRack = aisle[i-1] + 1; // first rack of aisle is last rack of prev aisle + 1 
            for(int j = firstRack; j <= lastRack; ++j) capAisle += capRacks[j]; 
    
        } else { // case first aisle 
            firstRack = 1; // Then first rack of aisle is 1
            for(int j = 1; j <= lastRack; ++j) // for each rack in aisle 
                capAisle += capRacks[j]; // add cap of rack i
        }
        int aisleAeration = std::ceil(capAisle*(aeration/100.0));  // compute value of aisle i aeration 
        for(int count = 0; count < aisleAeration; ++count) { // take out 1 by 1 capacity in current aisle, until aeration >= 20%
            for(int j = firstRack; j <= lastRack; ++j) { // iterate from first rack to last rack of current aisle
                if(racksCapCopy[j] > 0) {
                    --racksCapCopy[j]; // decrease cap of rack by 1
                    break; // stop searching for rack to empty, go next aeration unit
                } 
            }
        }
    } 

    return racksCapCopy; // return new capacity of racks; wich determine implicitely wich racks are lock for aeration
}

void Data::throwErrorFile(const std::string& where) {
    throw std::runtime_error("Couldn't open file at " + where);
}

// ! A RE CONSIDERER
/**
 * @brief compute matrix of boleans where isProdInOrd[o][prod] = true if prod is in order o
 * @warning must be pre computed in main if data are const given in method (such as gradient) argument
 */
void Data::computeIsProdInOrd() {
    ords.isProdInOrd.assign(nbOrd, std::vector<bool>(nbProd, false)); 
    for(int o = 0; o < nbOrd; ++o) {
        for(const int& prod : ords.ordsToProd[o]) 
            ords.isProdInOrd[o][prod] = true; 
    }
}