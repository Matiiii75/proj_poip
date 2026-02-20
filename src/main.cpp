#include "Datas.hpp"
#include "Conductor.hpp"
#include "FamilliesTSP.hpp"


int main(int argc, char* argv[]) {
    
    if(argc != 2) throw std::runtime_error("wrong number of args in main"); 
    int numInstance = std::atoi(argv[1]); 
    if(numInstance != 0 && numInstance != 1 
    && numInstance != 2 && numInstance != 3) { // check validity of arg given 
        throw std::runtime_error("this argument doesn't exists"); 
    } 
    allFiles allfiles; 
    allfiles.computeAllFiles(numInstance); 
    Data data(allfiles); // call and import datas 
    data.computeIsProdInOrd(); // compute prodInOrd 

    Conductor c(data); 
    c.executeUserChoice(); 
    
    return 0; 
}


