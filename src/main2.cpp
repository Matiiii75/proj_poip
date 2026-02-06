#include "Datas.hpp"
#include "Gradient.hpp"

int main(int argc, char* argv[]) {

    if(argc != 2) throw std::runtime_error("Expected two args"); 

    int numInstance = atoi(argv[1]); // convert arg to int 

    allFiles allfiles; 
    allfiles.computeAllFiles(numInstance);  
    Data data(allfiles); 

    // data.displayData(); 

    std::vector<int> new_cap; 
    new_cap = data.initCapRacksAer();

    Solution sol = initSol(data);
    // sol.displaySolution(numInstance);  // write sol in file 

    int solVal = computeSolValue(sol, data); 
    std::cout << "sol value : " << solVal << std::endl; 

    data.computeIsProdInOrd(); // pre compute boolean matrix sayin faslty if prod is in order o

    firstImprovLocalSearch(data, sol, solVal);     

    sol.displaySolution(numInstance); // écrire solution
   
    return 0; 
}