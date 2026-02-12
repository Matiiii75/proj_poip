#include "Conductor.hpp"

Conductor::Conductor(const Data& _data): data(_data) {}

void Conductor::computeArgs() {

    displayCalling("oAr, aInfos & fsi building"); 

    oAr.computeOrderAndRacks(solution, data); 
    aInfos.computeRackToAisle(data); 
    aInfos.computeAisleCap(data); 
    aInfos.computeAisleAer(data); 
    aInfos.computeAisleRemainCap(data, solution); 
    fsi.computeFamInterFamOrd(data, solution); 
    fsi.computeDefIntervals(data, solution); 

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

/**
 * @brief copy current solution's "rackToProd" informations
 * only from racks "f1min" to "f2max"
 * @param f1min from rack
 * @param f2max to rack
 * @note access "solution" attribute of conductor class
 * @warning modify "solution" attribute of conductor class
 */
std::vector<std::vector<int>> Conductor::copyRackToProd(int f1min, int f2max) {}

/**
 * @brief compute the order of products encoded between two rack
 * @param f1min 1st rack containing the products we compute the order on
 * @param f2max last rack containing the products we compute the order on
 * @param prodOrder the vector the will contain the order computed
 * @return the ID of the 1st product of f1 
 * @note prodOrder is modified by reference passed in argument 
 * @warning prodOrder must be declared outside this function 
 */
int Conductor::computeProdOrder(int f1min, int f2max, std::vector<int>& prodOrder) {}

/**
 * @brief place products according order encoded in prodOrder vector
 * @param f1min, f2max define the interval bounds on wich we modify the order of product
 * @param prodOrder encoded order 
 * @note shall modify "solution" attribute of Conductor class, but isn't udpating
 * "oAr" attribute wich mean if this product placement is optimal, we will have to call
 * *INSERER NOM FONCTION POUR MAJ oAr* function
 * @warning modify "solution" attribute of Conductor class 
 */
void Conductor::placeProducts(int f1min, int f2max, const std::vector<int>& prodOrder) {}





