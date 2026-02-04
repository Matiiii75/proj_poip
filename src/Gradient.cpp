#include "Gradient.hpp"


std::vector<std::pair<int,int>> computeFamIntervals(const Data& data, const Solution& sol, 
    std::vector<int>& famOrder) 
{

    /* famIntervals réels calculs */
    
    // va stocker le rack de début et de fin de chaque famille dans la solution
    std::vector<std::pair<int,int>> famIntervals(data.nbFam, {-1,-1}); 
    famOrder.reserve(data.nbFam); // on lui réserve nbFam positions
    // famOrder[i] = la famille en i eme position dans la solution
    
    for(int i = 0; i < data.nbRacks; ++i) { // pr chaque rack 
        if(sol.RackToProd[i].size() == 0) continue; // si le rack est vide -> on le traite pas
        
        for(const int& prod : sol.RackToProd[i]) { // pr chaque produit dans le rack
            int currFamProd = data.fam.prodToFam[prod]; // récup la famille du produit
            if(famIntervals[currFamProd].first == -1) { // si la famille avait pas de départ
                famIntervals[currFamProd].first = i; // assigner le rack comme départ   
                famOrder.push_back(currFamProd); // ajouter la famille dans l'ordre 
            }
            famIntervals[currFamProd].second = i; // maj dernier rack vu pour la famille
        }
    }

    return famIntervals; 
}

/**
 * @brief calcule les intervalles de définition des familles
 * @param famOrder passé par référence, permet de connaitre l'ordre des familles dans la solution
 */
std::vector<std::pair<int,int>> getDefIntervals(const Data& data, const Solution& sol, 
    std::vector<int>& famOrder, std::vector<std::pair<int,int>>& famInter) 
{

    std::vector<std::pair<int,int>> famIntervals = computeFamIntervals(data,sol,famOrder); 
    famInter = famIntervals; 
    /* defIntervals calculs */

    // will contain theoricals intervals of definition of each fam
    std::vector<std::pair<int,int>> defIntervals(data.nbFam); 

    for(int i = 0; i < data.nbFam; ++i) { // pour chaque famille dans l'ordre de la solution 

        int currFam = famOrder[i]; 

        if(i == 0) // si premiere famille dans l'ordre 
            defIntervals[currFam].first = 1; // la premiere famille commence toujours à 1

        if(i == data.nbFam - 1) // si derniere famille dans l'ordre 
            defIntervals[currFam].second = data.nbRacks - 2; 

        if(i > 0) { // calcule .first si > 1ere famille dans l'ordre 
            
            int prevFam = famOrder[i-1]; 
            int prevFamLastRack = famIntervals[prevFam].second; 

            if(sol.RackToProd[prevFamLastRack].size() < data.capRacks[prevFamLastRack]) { // si y'a de la place au dernier rack de la famille d'avant 
                defIntervals[currFam].first = prevFamLastRack; 
            }
            else { // si y'a pas de place -> placer juste après
                defIntervals[currFam].first = prevFamLastRack + 1; 
            }

        }

        if(i < data.nbFam - 1) { // calcule .second si < derniere famille dans l'ordre 

            int nextFam = famOrder[i+1]; // famille suivante ds l'ordre
            int nextFam1stRack = famIntervals[nextFam].first; // 1er rack nextFam

            if(sol.RackToProd[nextFam1stRack].size() < data.capRacks[nextFam1stRack]) { // si rest de la place, on peut aller jusqu'à 1er rack de nextFam
                defIntervals[currFam].second = nextFam1stRack;  
            }
            else { // sinon on s'arrete au rack juste avant
                defIntervals[currFam].second = nextFam1stRack - 1;
            }

        }

        if (famIntervals[currFam].first != -1) { // Si la famille n'est pas vide
            defIntervals[currFam].first = std::min(defIntervals[currFam].first, famIntervals[currFam].first);
            defIntervals[currFam].second = std::max(defIntervals[currFam].second, famIntervals[currFam].second);
        }

    }
    
    return defIntervals; 
}


/**
 * @brief renvoie true la capacité du rack pos permet d'accueillir un nouvel élément
 * @note si caprack[pos] > (strict) au nombre de produit qui occupent le rack dans la solution -> on peut ajouter un prod
 */
bool posCapEnough(const Data& data, const Solution& sol, int pos) {
    if(data.capRacks[pos] - sol.RackToProd[pos].size() > 0) return true; 
    else return false; 
}

/**
 * @brief calcule si c'est possible d'ajouter un sommet à l'allée posAisle (allée de pos) sans violer l'aération
 * @param posAisle : numéro de l'allée où on va 
 * @param aisleAer : vecteur tq aisleAer[i] = aération de l'allée i 
 * @param aisleRemainCap: vecteur qui dit la capacité restante de l'allée (sans compter l'aération)
 */
bool aisleAerEnough(const std::vector<int>& aisleAer, const std::vector<int>& aisleRemainCap, int posAisle) {
    if(aisleRemainCap[posAisle] - aisleAer[posAisle] > 0) return true; 
    else return false; 
}

/**
 * @brief cherche un produit de la famille fam dans le rack rack pour la solution
 * @return produit si a pu trouver un produit, sinon -1
 */
int findFamProdInRack(int rack, int fam, const Solution& sol, const Data& data) {

    int prodFound = -1;     
    
    for(const int& prod : sol.RackToProd[rack]) { // pr chq produit dans le rack 
        if(data.fam.prodToFam[prod] == fam) { // si la famille de prod est fam 
            prodFound = prod; // enregistre
            break; // sortir de la boucle et renvoyer prodFound 
        }
    } 

    return prodFound; 
}

/**
 * @brief trouve les deux racks encadrant rack dans la solution pour la commande order
 * @param rack le rack dont on veut trouver les encadrants 
 * @param order la commande considérée
 * @param oAr structure qui permet de connaitre les racks contenant les produits de la commande order (ds ordre croissant)
 * @param data données du pb
 * @return une paire où .first = prédecesseur et .second = succésseur de rack
 * @note si rack n'avait pas de prédecesseur, renvoie le dépot de départ
 * @note si rack n'avait pas de successeur, renvoie le dépot de fin
 * @note trouve les encadrants de "rack" meme si "rack" n'est pas dans le set 
 */
std::pair<int,int> findNeighsRack(int rack, int order, const orderAndRacks& oAr, const Data& data) {

    const std::set<int>& orderRacks = oAr.orderToRacks[order]; // alias pour rapidité

    // lower bound trouve le premier élément qui est >= rack
    auto itSuccessor = orderRacks.lower_bound(rack); 

    int prevRack = 0;  // dépot de départ par défaut
    int nextRack = data.nbRacks - 1; // dépôt de fin par défaut 

    // trouver le successeur (1st rack apres rack)
    if(itSuccessor != orderRacks.end()) 
        nextRack = *itSuccessor; 

    // trouver le prédécesseur (l'élément juste avant)
    if(itSuccessor != orderRacks.begin())
        prevRack = *std::prev(itSuccessor); 

    return {prevRack, nextRack}; 
}

/**
 * @brief calcule l'impact qu'à la position d'un produit sur le cout de la solution 
 * @brief calcule dont le cout en passant par currRack et celui en ne passant pas par currRack
 * @param order commande pour laquelle on calcule le coût 
 * @param currRack rack contenant le produit 
 * @param data données du problème 
 * @note si on rentre dans cette fonction, c'est qu'on est dans le cas où le produit était seul dans ce rack pour l'order 
 */
int computeCurrRackImpact(int order, int currRack, const orderAndRacks& oAr, const Data& data) {

    // trouver les encadrants 
    std::pair<int,int> encadrants = findNeighsRack(currRack, order, oAr, data); 

    int currVal = data.dists[encadrants.first][currRack] + data.dists[currRack][encadrants.second]; 
    int newVal = data.dists[encadrants.first][encadrants.second]; 

    return currVal - newVal; //! ancien - nouveau -> meilleur si > 0
}

/**
 * @brief calcule le gain qu'on aurait a aller occuper newRack pour une commande
 * @param order commande pour laquelle on calcule
 * @param newRack rack qu'on va occuper 
 * @param data données du problème 
 * @note si on rentre dans cette fonction, c'est qu'on est dans le cas où newRack n'était pas encore occupé pour la commande order
 */
int computeAddRackImpact(int order, int newRack, const orderAndRacks& oAr, const Data& data) {

    // trouver les encadrants de newRack 
    std::pair<int,int> encadrants = findNeighsRack(newRack, order, oAr, data); 

    int currVal = data.dists[encadrants.first][encadrants.second]; 
    int newVal = data.dists[encadrants.first][newRack] + data.dists[newRack][encadrants.second]; 

    return currVal - newVal; //! ancien - nouveau -> meilleur si > 0
}

/**
 * @brief calcule le delta lié à un échange entre produit 1 et produit 2 
 * @param prod1 le produit 1 qu'on échange
 * @param prod2 le produit 2 qu'on échange
 * @param data les données 
 * @param oAr structure relatives aux infos sur order et allées
 * @param prod1Rack rack du produit 1 
 * @param prod2Rack rack du produit 2
 */
int computeDeltaSwap(int prod1, int prod2, const Data& data, const orderAndRacks& oAr, int prod1Rack, int prod2Rack) {

    int totalDelta = 0;     
    
    for(const int& o : data.ords.prodToOrds[prod1]) { // pr chq commande où apparaît prod1 

        int countRack1 = oAr.orderOccurRacks[o][prod1Rack]; // mémorise 
        int countRack2 = oAr.orderOccurRacks[o][prod2Rack]; 

        if(data.ords.ordsToProd[o].count(prod2) > 0) 
            continue; // si prod1 & prod2 ds meme commande, leur échange ne change rien au calcul de la valeur de o  

        
        if(countRack1 > 1 && countRack2 >= 1) 
            continue; // on ignore car de toute façon, on continuera à emprunter les deux racks apres échange. 

        if(countRack1 > 1 && countRack2 == 0) { // ne calculer que les changements à l'arrivée 
            totalDelta += computeAddRackImpact(o, prod2Rack, oAr, data); 
            continue; 
        }

        if(countRack1 == 1 && countRack2 == 0) { // déplacer impactera sur le début et l'arrivée
            totalDelta += computeCurrRackImpact(o, prod1Rack, oAr, data);
            totalDelta += computeAddRackImpact(o, prod2Rack, oAr, data); 
            continue; 
        } 

        if(countRack1 == 1 && countRack2 >= 1) { // déplacer impactera le début seulement
            totalDelta += computeCurrRackImpact(o, prod1Rack, oAr, data); 
            continue; 
        }
        
    }

    for(const int& o : data.ords.prodToOrds[prod2]) { // pr chq commande où apparaît prod2

        int countRack1 = oAr.orderOccurRacks[o][prod1Rack]; // mémorise 
        int countRack2 = oAr.orderOccurRacks[o][prod2Rack];

        if(data.ords.ordsToProd[o].count(prod1) > 0) 
            continue; // si prod1 & prod2 ds meme commande, leur échange ne change rien au calcul de la valeur de o 

        if(countRack2 > 1 && countRack1 >= 1) 
            continue; // on ignore car de toute façon, on continuera à emprunter les deux racks apres échange. 

        if(countRack2 > 1 && countRack1 == 0) { // ne calculer que les changements à l'arrivée 
            totalDelta += computeAddRackImpact(o, prod1Rack, oAr, data); 
            continue; 
        }

        if(countRack2 == 1 && countRack1 == 0) { // déplacer impactera sur le début et l'arrivée
            totalDelta += computeCurrRackImpact(o, prod2Rack, oAr, data);
            totalDelta += computeAddRackImpact(o, prod1Rack, oAr, data); 
            continue; 
        } 

        if(countRack2 == 1 && countRack1 >= 1) { // déplacer impactera le début seulement
            totalDelta += computeCurrRackImpact(o, prod2Rack, oAr, data); 
            continue; 
        }
    }

    return totalDelta; //! ancien - nouveau -> meilleur si > 0
}

int computeDeltaSend(int prod1, int prod1Rack, int newRack, const Data& data, const orderAndRacks& oAr) {

    int totalDelta = 0; 
    
    for(const int& o : data.ords.prodToOrds[prod1]) { // pr chq commande où est prod1 

        int countRack1 = oAr.orderOccurRacks[o][prod1Rack]; 
        int countNewRack = oAr.orderOccurRacks[o][newRack]; 

        if(countRack1 > 1 && countNewRack >= 1)
            continue; // ignorer car on continuera de passer par les même racks pour le calcul

        if(countRack1 > 1 && countNewRack == 0) { // changements qu'à l'arrivée
            totalDelta += computeAddRackImpact(o, newRack, oAr, data); 
            continue; 
        }

        if(countRack1 == 1 && countNewRack == 0) { // déplacer impactera sur le début et l'arrivée
            totalDelta += computeCurrRackImpact(o, prod1Rack, oAr, data); 
            totalDelta += computeAddRackImpact(o, newRack, oAr, data); 
            continue; 
        }

        if(countRack1 == 1 && countNewRack >= 1) { // déplacer impactera que le début 
            totalDelta += computeCurrRackImpact(o, prod1Rack, oAr, data); 
            continue; 
        }
    }

    return totalDelta; //! ancien - nouveau -> meilleur si > 0
}

/**
 * @brief applique le meilleur mouvement enregistré sur une itération de recherche locale
 * @param bestSwap meilleur swap enregistré 
 */
void applyMove(const BestSwap& bestSwap, Solution& bestSol, 
    int& bestVal, std::vector<int>& aisleRemainCap, 
    orderAndRacks& oAr, std::vector<std::pair<int,int>>& defIntervals, 
    const std::vector<int>& rackToAisle, const Data& data, 
    std::vector<int>& famOrder, 
    std::vector<std::pair<int,int>>& famIntervals)   
{

    int delta = bestSwap.bestDelta; 
    int prod1 = bestSwap.prod1ToProd2.first; 
    int prod2 = bestSwap.prod1ToProd2.second;
    int prod1Rack = bestSwap.rack1ToRack2.first; 
    int prod2Rack = bestSwap.rack1ToRack2.second;   
    bool isSwap = true; 
    
    if(prod2 == -1) isSwap = false; // si prod2 == -1, bestSwap est un envoie 

    /* //* ACTUALISE REMAINCAPS */

    if(!isSwap) { // si c'est un envoi, on a modifié aisleRemainCap

        int prod1Aisle = rackToAisle[prod1Rack]; // get prod1Rack's aisle
        int newAisle = rackToAisle[prod2Rack]; // get aisle receiving prod1

        aisleRemainCap[prod1Aisle]++; 
        aisleRemainCap[newAisle]--; 

    }

    /* //* ACTUALISE OAR */

    for(const int& o : data.ords.prodToOrds[prod1]) { // pour chaque order contenant prod 1

        if(--oAr.orderOccurRacks[o][prod1Rack] == 0) // on décrémente et si on arrive à 0
            oAr.orderToRacks[o].erase(prod1Rack); // effacer du set
        
        if(++oAr.orderOccurRacks[o][prod2Rack] == 1) // on incrémente et si on arrive a 1 
            oAr.orderToRacks[o].insert(prod2Rack); // c'est que on viens d'jaouter le rack à o

    }

    if(isSwap) { // si c'est un swap, toute solution contenant prod2 doit aussi etre MAJ

        for(const int& o : data.ords.prodToOrds[prod2]) { // pour chaque order contenant prod 2

            if(--oAr.orderOccurRacks[o][prod2Rack] == 0) // on décrémente et si on arrive à 0
                oAr.orderToRacks[o].erase(prod2Rack); // effacer du set
            
            if(++oAr.orderOccurRacks[o][prod1Rack] == 1) // on incrémente et si on arrive a 1 
                oAr.orderToRacks[o].insert(prod1Rack); // c'est que on viens d'jaouter le rack à o

        }

    }

    /* //* FIN ACTUALISE OAR */

    /* //* MAJ BESTSOL */

    bestSol.prodToRack[prod1] = prod2Rack; // retiens prod1 dans prod2rack
    bestSol.RackToProd[prod2Rack].push_back(prod1); 

    auto& v = bestSol.RackToProd[prod1Rack]; 
    auto it = std::find(v.begin(), v.end(), prod1); // suppression prod1 de prod1Rack
    if(it != v.end()) v.erase(it); 

    if(isSwap) { // si c'est un swap, faire pareil avec prod2 

        bestSol.prodToRack[prod2] = prod1Rack; 
        bestSol.RackToProd[prod1Rack].push_back(prod2); 

        auto& v = bestSol.RackToProd[prod2Rack]; 
        auto it = std::find(v.begin(), v.end(), prod2); 
        if(it != v.end()) v.erase(it); 

    }

    /* //* FIN MAJ BESTSOL */


    /* //* ACTUALISER FAMINTERVALS */

    int famProd1 = data.fam.prodToFam[prod1]; // récup famille de prod1

    int newMax = -1; 
    int newMin = data.nbRacks; 
    for(int p : data.fam.famToProd[famProd1]) { // on recalcule les vrais bornes de la famille famprod1
        int r = bestSol.prodToRack[p]; 
        if(r < newMin) newMin = r; 
        if(r > newMax) newMax = r; 
    }
    famIntervals[famProd1] = {newMin, newMax}; 

    /* //* FIN ACTUALISER FAMINTERVALS */

    /* //* ACTUALISER DEFINTERVALS */

    // recherche des deux familles encadrantes : 
    
    int predFam = -1; 
    int succFam = -1; 

    for(int i = 0; i < data.nbFam; ++i) {
        if(famOrder[i] == famProd1) {
            if(i > 0) predFam = famOrder[i-1]; 
            if(i < data.nbFam-1) succFam = famOrder[i+1]; 
        }   
    }
    
    // actualiser intervalle de la famille de gauche 

    if(predFam != -1) {

        int pointDeContact = famIntervals[famProd1].first; // nouveau rack le plus à gauche de famprod1

        // si le rack au point de contact entre les deux familles a de la place, predFam peut y aller
        if(bestSol.RackToProd[pointDeContact].size() < data.capRacks[pointDeContact])
            defIntervals[predFam].second = pointDeContact; 
        else // sinon elle doit s'arreter juste avant 
            defIntervals[predFam].second = pointDeContact - 1; 

    }

    if(succFam != -1) {

        int pointDeContact = famIntervals[famProd1].second; // nouveau rack le plus a droite de famprod1

        // si le rack au point de contact des deux familles a de la place, succFam peut y aller 
        if(bestSol.RackToProd[pointDeContact].size() < data.capRacks[pointDeContact])
            defIntervals[succFam].first = pointDeContact; 
        else // sinon, elle s'arrete juste avant 
            defIntervals[succFam].first = pointDeContact + 1; 

    }

    /* //* FIN ACTUALISATION DEFINTERVALS */

    /* //* ACTUALISE BESTVAL */

    bestVal -= bestSwap.bestDelta; 

}

// il faudra calculer bestVal avant 
void GradientBestImprov(const Data& data, Solution& bestSol, int& bestVal) {

    //? initialiser les données 

    /* ASSOCIER ALLEES */
    
    std::vector<int> rackToAisle(data.nbRacks); 
    for(int i = 0; i < data.nbAisle; ++i) {
        if(i!=0) { // si on est pas sur la premiere allée
            for(int j = data.aisle[i-1]+1; j <= data.aisle[i]; ++j) 
                rackToAisle[j] = i; 
        }
        else { // si on est sur la premiere allée 
            for(int j = 0; j <= data.aisle[i]; ++j) 
                rackToAisle[j] = i; 
        }
    }
    
    /* FIN ASSOCIER ALLEES */


    /* CALCUL AISLECAP, AISLEAER & AISLEREMAINCAP */

    // aisleCap
    std::vector<int> aisleCap(data.nbAisle, 0); 
    for(int i = 0; i < data.nbAisle; ++i) { // pour chaque allée 
        int firstRack, lastRack; 
        lastRack = data.aisle[i]; 
        if(i > 0) firstRack = data.aisle[i-1]-1; // si > 1ere allée
        else firstRack = 1; // si 1ere allée 
        for(int j = firstRack; j < lastRack; ++j) // ajouter les caps
            aisleCap[i] += data.capRacks[j]; 
    }
    
    // aisleAer : aisleAer[i] = aération nécéssaire dans allée i
    std::vector<int> aisleAer(data.nbAisle);
    for(int i = 0; i < data.nbAisle; ++i) { // pour chaque allée
        int cap = aisleCap[i]; 
        aisleAer[i] = std::ceil(aisleCap[i]*(data.aeration/100.0)); // compute aer
    } 

    // aisleRemainCap 
    //! je pourrais boucler sur les racks pour calculer ? (comme compute sol value ?)
    std::vector<int> aisleRemainCap = aisleCap;
    for(int i = 0; i < data.nbAisle; ++i) { // pr chaque allée
        int firstRack, lastRack; 
        lastRack = data.aisle[i]; 
        if(i > 0) firstRack = data.aisle[i-1]-1; 
        else firstRack = 1; 
        for(int j = firstRack; j < lastRack; ++j) // pr chq rack dans l'allée 
            aisleRemainCap[i] -= bestSol.RackToProd[j].size(); //
    }
    
    /* FIN CALCUL AISLECAP, AISLEAER & AISLEREMAINCAP */

    orderAndRacks orderAndRacks; 
    orderAndRacks.computeOrderAndRacks(bestSol, data); 

    // création des intervalles de définition de chaque famille en fonction de solution initiale
    std::vector<std::pair<int,int>> defIntervals;  
    std::vector<std::pair<int,int>> famIntervals; 
    std::vector<int> famOrder;
    defIntervals = getDefIntervals(data, bestSol, famOrder, famIntervals); // récupère intervalles initiaux

    bool improved = true; 
    
    while(improved) {

        improved = false; 
        BestSwap bestSwap;  

        for(int f = 0; f < data.nbFam; ++f) { // pour chaque famille  
            
            for(const int& prod : data.fam.famToProd[f]) { // pr chq prod de fam i 
                
                int prodRack = bestSol.prodToRack[prod]; // récup rack contenant prod
                int rackAisle = rackToAisle[prodRack]; // récup allée contenant iRack
                
                int fmin = defIntervals[f].first; // récup intervalle de définition de f
                int fmax = defIntervals[f].second; 

                for(int newPos = fmin; newPos <= fmax; ++newPos) { // pr chq position (rack) dans l'intervalle de définition 
                    
                    if(newPos == prodRack) continue; // si mm rack -> inutile

                    bool enoughCap, enoughAer;
                    int newPosAisle = rackToAisle[newPos]; // récup l'allée de NP

                    if(newPosAisle == rackAisle) 
                        enoughAer = true; // on a forcément assez d'aération puisqu'on reste dans la meme allée et qu'on ajt rien
                    else enoughAer = aisleAerEnough(aisleAer, aisleRemainCap, newPosAisle); // check if enough aer in newPos's aisle to send prod
                    
                    enoughCap = posCapEnough(data, bestSol, newPos); // check if enough Cap in newPos rack to send prod
                    
                    if(enoughCap) { // si assez de cap pour envoyer prod
                            
                        if(enoughAer) { // si envoi prod respect l'aération
                            // simuler envoie
                            int Delta = computeDeltaSend(prod, prodRack, newPos, data, orderAndRacks); 

                            if(Delta > bestSwap.bestDelta) { //? faire une fonction retiensSwap ? 
                                // retenir le swap
                                bestSwap.bestDelta = Delta; 
                                bestSwap.prod1ToProd2 = {prod, -1};
                                bestSwap.rack1ToRack2 = {prodRack, newPos};
                            }

                            continue; // -> next iteration
                        }
                        
                        if(!enoughAer) { // si l'envoi ne respecte pas l'aération
                             
                            int sameFamProd = findFamProdInRack(newPos, f, bestSol, data); // si on trouve un produit de mm famille à newPos
                            if(sameFamProd == -1) continue; // continuer à la prochaine itération
                            // simuler l'échange de prod avec sameFamProd
                            int Delta = computeDeltaSwap(prod, sameFamProd, data, orderAndRacks, prodRack, newPos); 
                            
                            if(Delta > bestSwap.bestDelta) {
                                // retenir le swap
                                bestSwap.bestDelta = Delta; 
                                bestSwap.prod1ToProd2 = {prod, sameFamProd};
                                bestSwap.rack1ToRack2 = {prodRack, newPos};
                            }

                            continue; // -> next iteration
                        }

                    }

                    if(!enoughCap) { // si pas assez de cap pour envoyer prod 

                        int sameFamProd = findFamProdInRack(newPos, f, bestSol, data); // si on trouve prod de mm famille à newPos
                        if(sameFamProd == -1) continue; // continuer à la prochaine itération 
                        // simuler l'échange 
                        int Delta = computeDeltaSwap(prod, sameFamProd, data, orderAndRacks, prodRack, newPos);
                        
                        if(Delta > bestSwap.bestDelta) {
                            // retenir le swap
                            bestSwap.bestDelta = Delta; 
                            bestSwap.prod1ToProd2 = {prod, sameFamProd};
                            bestSwap.rack1ToRack2 = {prodRack, newPos}; 

                        }

                    }

                }  
            }
        }

        if(bestSwap.prod1ToProd2.first != -1) { // appliquer le bestSwap
            applyMove(bestSwap, bestSol, bestVal, aisleRemainCap, orderAndRacks, defIntervals, rackToAisle, data, famOrder, famIntervals); 
            improved = true; 
        }

        std::cout << "solval : " << bestVal << std::endl;

    }
}