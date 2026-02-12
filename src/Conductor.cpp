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

std::vector<std::vector<int>> Conductor::copyRackToProd(int f1min, int f2max) {}

int Conductor::computeProdOrder(int f1min, int f2max, std::vector<int>& prodOrder) {}

void Conductor::placeProducts(int f1min, int f2max, const std::vector<int>& prodOrder) {}





