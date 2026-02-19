#include "FamilliesTSP.hpp"

Solution fillSolution(const Data& data, 
    const std::vector<int>& famOrder) 
{
    Solution sol; 
    sol.RackToProd.resize(data.nbRacks); 
    sol.prodToRack.resize(data.nbProd); 

    std::vector<int> newCapRack = data.initCapRacksAer(); 

    for(const int& i : famOrder) { // for each family in order based on TSP result
        for(const auto& prod : data.fam.famToProd[i]) { // for each product in this famiily
            
            int lastLocSeen = 0; 
            for(int j = lastLocSeen; j < data.nbRacks; ++j) { //  iterate over every rack from 0

            // from lastLocSeen, keep going until finding new free location for prod 
                if(newCapRack[j] == 0) continue; // ignore if rack is full

                // otherwise 
                sol.prodToRack[prod] = j; 
                sol.RackToProd[j].push_back(prod); 
                --newCapRack[j]; // update : we used 1 unit of capacity for prod
                lastLocSeen = j; // we memorize that we stopped at lastLocSeen
                break; 
            }
        }
    }

    return sol; 
}

void displaySolution(const std::vector<int>& sol) {
    for(int i : sol) std::cout << i << " "; 
    std::cout << std::endl;
}

void addDummyFam(std::vector<std::vector<int>>& d) {

    int nbFam = (int)d.size(); // get original nbFam
    std::vector<int> fictifVec(nbFam + 1, 0); 
    d.push_back(fictifVec); // add correlations of dummy with every fam
    for(auto& vec : d) // add 0 correlation for every fam with dummy
        vec.push_back(0);  

}

int computeSolVal(const std::vector<int>& sol, 
    const std::vector<std::vector<int>>& d) 
{
    int val = 0; 
    for(int i = 0; i < (int)sol.size()-1; ++i) {
        val += d[sol[i]][sol[i+1]]; 
    }
    
    return val;
}

// S1, S2 & S3 are continuous segments parts of the TSP solution
int computePreDelta(const std::vector<std::vector<int>>& d, 
    const std::vector<int>& sol, int i, int j, int k)
{   
    int delta = 0; 
    int startS1 = sol[i]; // get every begin points of 3 segments
    int startS2 = sol[j]; 
    int startS3 = sol[k]; 

    int lastS1 = sol[j-1]; // get every end points of 3 segments 
    int lastS2 = sol[k-1];
    int lastS3;  
    if(i == 0) lastS3 = sol[(int)sol.size()-1]; // case i = 0 -> k end at last pos of sol
    else lastS3 = sol[i-1]; 

    delta += d[lastS1][startS2]; // add values to delta 
    delta += d[lastS2][startS3]; 
    delta += d[lastS3][startS1];  

    return delta;
}

int computePostDelta(const std::vector<std::vector<int>>& d, 
    const std::vector<int>& sol, int i, int j, int k, int& bestMove)
{
    int delta = 0; 
    int currDelta; 
    
    int startS1 = sol[i]; int lastS1 = sol[j-1];
    int startS2 = sol[j]; int lastS2 = sol[k-1];
    int startS3 = sol[k]; int lastS3;  
    if(i == 0) lastS3 = sol[(int)sol.size()-1]; // case i = 0 -> k end at last pos of sol
    else lastS3 = sol[i-1]; 

    currDelta = 0; // I (reverse S1)
    currDelta = d[lastS3][lastS1] + d[startS1][startS2] + d[lastS2][startS3];
    if(currDelta > delta) {delta = currDelta; bestMove = 1;}
    currDelta = 0; // II (reverse S2)
    currDelta = d[lastS3][startS1] + d[lastS1][lastS2] + d[startS2][startS3];
    if(currDelta > delta) {delta = currDelta; bestMove = 2;}
    currDelta = 0; // III (reverse S1 & S2)
    currDelta = d[lastS3][lastS1] + d[startS1][lastS2] + d[startS2][startS3];
    if(currDelta > delta) {delta = currDelta; bestMove = 3;}
    currDelta = 0; // IV (swap S1 and S2)
    currDelta = d[lastS3][startS2] + d[lastS2][startS1] + d[lastS1][startS3];
    if(currDelta > delta) {delta = currDelta; bestMove = 4;}
    currDelta = 0; // V (reverse S2 and swap S2 & S1)
    currDelta = d[lastS3][lastS2] + d[startS2][startS1] + d[lastS1][startS3];
    if(currDelta > delta) {delta = currDelta; bestMove = 5;}
    currDelta = 0; // VI (reverse S1 and swap S2 & S1)
    currDelta = d[lastS3][startS2] + d[lastS2][lastS1] + d[startS1][startS3];
    if(currDelta > delta) {delta = currDelta; bestMove = 6;}
    currDelta = 0; // VII (reverse & swap both)
    currDelta = d[lastS3][lastS2] + d[startS2][lastS1] + d[startS1][startS3];
    if(currDelta > delta) {delta = currDelta; bestMove = 7;}

    return delta;
}

void applyMove(std::vector<int>& sol, 
    int bestMove, int i, int j, int k) 
{

    auto itI = sol.begin() + i; // iterators on sol
    auto itJ = sol.begin() + j; // to capture  
    auto itK = sol.begin() + k; // begins of all 3 segments 

    switch(bestMove) { // apply bestMove encoded choice

        case 1 : // reverse S1
            std::reverse(itI, itJ); break; 
        case 2 : // reverse S2
            std::reverse(itJ, itK); break; 
        case 3 : // reverse both S1 & S2 
            std::reverse(itI, itJ); std::reverse(itJ, itK); break; 
        case 4 : 
            std::reverse(itI, itJ); // reverse S1
            std::reverse(itJ, itK); // reverse S2
            std::reverse(itI, itK); // reverse (r(S1),r(S2)) -> obtain (S2,S1) in correct order
            break; 
        case 5 : 
            std::reverse(itI, itJ); // reverse S1
            std::reverse(itI, itK); // reverse (r(S1), S2) -> obtain (r(S2),S1)
            break; 
        case 6 : // reverse (S1,r(S2)) -> obtain (S2,r(S1))
            std::reverse(itJ, itK); std::reverse(itI, itK); 
            break; 
        case 7 : // reverse (S1,S2) -> obtain (r(S2),r(S1))
            std::reverse(itI, itK); break;
        default : 
            throw std::runtime_error("in applymove TSP : default case called");

    }

}

void eraseDummy(std::vector<int>& sol) {
    int size = sol.size(); // save size with dummy
    int indexDummyFam = -1; // search where is fictivFam 
    for(int i = 0; i < size; ++i) {
        if(sol[i] == size-1) {
            indexDummyFam = i; break;
        }
    }
    std::vector<int> realSolution; // will store solution without dummy
    if(indexDummyFam != size-1) { // if dummy isn't on last pos
        for(int i = indexDummyFam + 1; i < size; ++i) {
            realSolution.push_back(sol[i]); 
        }
    }
    if(indexDummyFam != 0) { // if dummy isn't on 1st pos
        for(int i = 0; i < indexDummyFam; ++i) {
            realSolution.push_back(sol[i]); 
        }
    } 
    sol = realSolution; // update sol by reference 
}

std::vector<int> computeTSP(const Data& data) {

    std::vector<std::vector<int>> distances = buildCorrFam(data); // compute distances 
    addDummyFam(distances); // add dummy vertex to graph
    int nbFam = (int)distances.size(); std::vector<int> sol; // save size & declare sol container 
    for(int i = 0; i < nbFam; ++i) sol.push_back(i); // fill by fam ascending order
    int val = computeSolVal(sol, distances); // compute value of solution
    // int initialSolValue = val; // memo for comparison

    bool improved = true; 
    while(improved) { // while improving movement found 
        improved = false; 
        for(int i = 0; i < nbFam - 4; ++i) { // iterate over all possibilities to break into 3 segments
            for(int j = i+2; j < nbFam -2; ++j) {
                for(int k = j+2; k < nbFam; ++k) {

                    int preDelta = computePreDelta(distances, sol, i, j, k); 
                    int bestMove = -1; 
                    int postDelta = computePostDelta(distances, sol, i , j, k, bestMove);

                    if(postDelta <= preDelta) continue; // ignore if not improving 

                    applyMove(sol, bestMove, i, j, k); // else apply bestMove registered
                    val += postDelta - preDelta; // update sol value 
                    improved = true; 
                    // std::cout << "val : " << val << std::endl;
                    break; 
                }
                if(improved) break; // goto next while iteration
            }   
            if(improved) break; // goto next while iteration
        }
    }
    eraseDummy(sol); // erase dummy from solution obtained
    return sol; 
}