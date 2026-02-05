#include "Datas.hpp"
#include "Gradient.hpp"

int computeOrderCost(const Solution& sol, const Data& data, int order) {

    int cost = 0; 
    std::set<int> pq; 
    int lastTreated = 0; // insert departure rack

    for(const int& prod : data.ords.ordsToProd[order]) {
        pq.insert(sol.prodToRack[prod]); 
    }

    while(!pq.empty()) {
        cost += data.dists[lastTreated][*pq.begin()]; 
        lastTreated = *pq.begin(); 
        pq.erase(*pq.begin()); 
    }

    cost += data.dists[lastTreated][data.nbRacks-1]; 

    return cost; 
}


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
    sol.displaySolution(numInstance);  // write sol in file 

    // display initial sol
    // for(int i = 0; i < (int)sol.RackToProd.size(); ++i) {
    //     std::cout << "rack " << i << " : "; 
    //     for(int j = 0; j < (int)sol.RackToProd[i].size(); ++j) { // this block display solution 
    //         std::cout << sol.RackToProd[i][j] << " "; 
    //     }
    //     std::cout << std::endl;
    // }

    int solVal = computeSolValue(sol, data); 
    std::cout << "sol value : " << solVal << std::endl; 

    std::vector<std::pair<int,int>> defIntervals; 
    // defIntervals = getDefIntervals(data,so); 

    for(int i = 0; i < (int)defIntervals.size(); ++i) {
        std::cout << "famille : " << i << " intervalle : " << defIntervals[i].first << "," << defIntervals[i].second << std::endl;
    }

    return 0; 
}