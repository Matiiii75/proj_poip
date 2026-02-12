#include "Datas.hpp"

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



/**
 * @brief swap deux familles adjacentes 
 * @param data donnees du pb
 * @param bestSol solution courante qu'on modifie 
 * @param bestVal valeur courante
 * @param oAr doit etre modifiÃ© car Ã©changer deux familles, c'est changeant l'attribution des racks aux produits de ces familles
 * @param aInfos informations des allÃ©es -> ne change pas
 * @param fsi doit changer. (tout change). 
 * @param famId1, famId2 les indexs des familles qu'on Ã©change dans famOrder 
 */
void swapFamily(   const Data& data, Solution &bestSol,
    int& bestVal, orderAndRacks& oAr,
    aisleInfos aInfos, famSolInfos& fsi,
    int famId1,    int famId2) // sa position dans famOrder
{

    if(famId1 +1 != famId2) throw std::runtime_error("non consecutive families");


    int fam1 = fsi.famOrder[famId1];  int fam2 = fsi.famOrder[famId2]; // recover fam1 and fam2
   

    int f1min = fsi.famIntervals[fam1].first; 
    int f1max = fsi.famIntervals[fam1].second; 
    int f2min = fsi.famIntervals[fam2].first; 
    int f2max = fsi.famIntervals[fam2].second;

    std::vector<std::vector<int>> copyRackToProd(f2max - f1min +1); //!faire fonction copy

        for(int r = f1min; r <= f2max; ++r) {
            for(int prod : bestSol.RackToProd[r])
                copyRackToProd[r].push_back(prod); 
        }

    std::vector<int> prodOrder(data.fam.famToProd[fam1].size() + data.fam.famToProd[fam2].size()); // vector who contain the orde of the products
    

    int idVec = 0; // idicate where we are in the vec prodOrder //!faire fonction order
    for(int r = f2min; r <= f2max ; ++r) { // first, store order of fam2's products
        for(int p = 0; p < (int)bestSol.RackToProd[r].size(); ++p) { 
            int prod = bestSol.RackToProd[r][p]; // recover product in position p in rack r

            if(data.fam.prodToFam[prod] == fam2) {  //! ajouter un if r = fmin/max
                prodOrder[idVec] = prod;
                idVec++;
            }
        }

    }
    bool firstProd = true; int FirstProdF1; // we want to store the number of the first product of f1
    for(int r = f1min; r <= f1max ; ++r) { //then,  store order of fam2's products
        for(int p = 0; p < (int)bestSol.RackToProd[r].size(); ++p) {
            int prod = bestSol.RackToProd[r][p];
        
            if(data.fam.prodToFam[prod] == fam1) { //! ajouter un if r = fmin/max
                prodOrder[idVec]= prod; idVec++;
                if(firstProd)  {FirstProdF1 = idVec; firstProd = false;} // recuperate the first product of F1
            }
                
        }
    }

    std::cout << "prodOrder : "; 
    for(int i : prodOrder) std::cout << i << " "; 
    std::cout << std::endl;


    //We place the products accordind to the order define in prodOrder
    idVec = 0; // position of the product we want to place //!faire foncyion place
    int prod; // product in positon idVec

    for(int r = f1min; r <= f2max; ++r) { 
        for(int p = 0; p < (int)bestSol.RackToProd[r].size(); ++p) {
            prod = prodOrder[idVec]; // product in positon idVec

            int prodBeforeSwap = bestSol.RackToProd[r][p]; // product before the swap

            if((data.fam.prodToFam[prodBeforeSwap] == fam1) || (data.fam.prodToFam[prodBeforeSwap] == fam2)) { // if the product we want to change was in fam1 or fam2

                //update orderAndRacks for prodBefore
                for(const int& o : data.ords.prodToOrds[prodBeforeSwap]) { // for each order 
                    
                   // oAr.orderOccurRacks[o][r] --;
                    if(--oAr.orderOccurRacks[o][r] == 0)  // if order o didn't use r
                        eraseSortedVec(r, oAr.orderToRacks[o]); // add r  
                }

                bestSol.RackToProd[r][p] = prod; // place prod in rack r
                bestSol.prodToRack[prod] = r; // associate prod to r

                //update orderAndRacks for prod
                for(const int& o : data.ords.prodToOrds[prod]) { // for each order 
                 //   oAr.orderOccurRacks[o][r]++;
                    if(++oAr.orderOccurRacks[o][r] == 1)  // if order o didn't use r
                        addSortedVec(r, oAr.orderToRacks[o]); // add r
                }
                idVec++; 

            }
            
        }   
    }

    //update famSolInfos
    //!faire une fonction updateFsi : ne pas donner f1min et f2max en argument de celle ci, 
    // ! mais utiliser fsi pour les calculer dans updateFsi
    fsi.famOrder[famId2] = fam1;  fsi.famOrder[famId1] = fam2;

    int tmpF1MinDef = fsi.defIntervals[fam1].first; // copy the infos of f1 to modify it

    int newMinF1 = bestSol.prodToRack[prodOrder[FirstProdF1]]; // memorization 
    int newMaxF2 = bestSol.prodToRack[prodOrder[FirstProdF1-1]];
 
    fsi.defIntervals[fam1].second = fsi.defIntervals[fam2].second; 
    fsi.famIntervals[fam1].second = f2max;      // update fam1 properties 
    fsi.famIntervals[fam1].first = newMinF1;    // the max for right family is still the same

    fsi.defIntervals[fam2].first = tmpF1MinDef;
    fsi.famIntervals[fam2].first = f1min;       // update fam2 properties
    fsi.famIntervals[fam2].second = newMaxF2;   // the min for left family is still the same 

    if(newMaxF2 == newMinF1) { // if f1 and f2 share a rack
        fsi.defIntervals[fam1].first = newMinF1; fsi.defIntervals[fam2].second = newMinF1;

    } else { // if f1 and f2 are completly separate
        if(aInfos.aisleRemainCap[newMinF1] - aInfos.aisleAer[newMinF1] > 0)// ! faire cahngement struct la fonction n'existe pas
            fsi.defIntervals[fam2].second = newMinF1;
        else
            fsi.defIntervals[fam2].second = newMinF1 - 1;

        if(aInfos.aisleRemainCap[newMaxF2] - aInfos.aisleAer[newMaxF2] > 0) // if newLaxF2 isn't full
            fsi.defIntervals[fam1].first = newMaxF2;
        else
            fsi.defIntervals[fam1].first = newMaxF2 + 1; 
    }
   

    // 

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
    //sol.displaySolution(numInstance);  // write sol in file 

    

    aisleInfos aInfos;
    orderAndRacks oAr;
    famSolInfos fsi;
 
    aInfos.computeRackToAisle(data); 
    aInfos.computeAisleCap(data); 
    aInfos.computeAisleAer(data); 
    aInfos.computeAisleRemainCap(data, sol); 
    oAr.computeOrderAndRacks(sol, data); 
    fsi.computeFamInterFamOrd(data, sol); 
    fsi.computeDefIntervals(data, sol); 

    std::cout <<" Avant le Swap !!!" << std::endl;
    for(int i = 0; i < (int)sol.RackToProd.size(); ++i) {
        std::cout << "rack " << i << " : "; 
        for(int j = 0; j < (int)sol.RackToProd[i].size(); ++j) { // this block display solution 
            std::cout << sol.RackToProd[i][j] << " "; 
        }
        std::cout << std::endl;
    }

    int solVal = computeSolValue(sol, data); 
    std::cout << "sol value : " << solVal << std::endl; 

    
    
    for(int i = 0; i < (int)fsi.famIntervals.size(); ++i) {
        std::cout << "famille : " << i << " fam intervalle : " << fsi.famIntervals[i].first << "," << fsi.famIntervals[i].second << std::endl;
    }
     

    for(int i = 0; i < (int)fsi.defIntervals.size(); ++i) {
        std::cout << "famille : " << i << " def intervalle : " << fsi.defIntervals[i].first << "," << fsi.defIntervals[i].second << std::endl;
    }

    swapFamily(data, sol, solVal, oAr, aInfos, fsi, 0, 1);


    
    std::cout <<" Apres le Swap !!!" << std::endl;
    for(int i = 0; i < (int)sol.RackToProd.size(); ++i) {
        std::cout << "rack " << i << " : "; 
        for(int j = 0; j < (int)sol.RackToProd[i].size(); ++j) { // this block display solution 
            std::cout << sol.RackToProd[i][j] << " "; 
        }
        std::cout << std::endl;
    }

    solVal = computeSolValue(sol, data); 
    std::cout << "sol value : " << solVal << std::endl; 

    
    for(int i = 0; i < (int)fsi.famIntervals.size(); ++i) {
        std::cout << "famille : " << i << " fam intervalle : " << fsi.famIntervals[i].first << "," << fsi.famIntervals[i].second << std::endl;
    }

    for(int i = 0; i < (int)fsi.defIntervals.size(); ++i) {
        std::cout << "famille : " << i << " def intervalle : " << fsi.defIntervals[i].first << "," << fsi.defIntervals[i].second << std::endl;
    }

    sol.displaySolution(1); 

    return 0; 
}


