#pragma once 

#include "Datas.hpp"
#include "Common.hpp"
#include "FamilliesTSP.hpp"

struct Coi{

    
    /* ATTRIBUTS */

    std::vector<std::vector<std::pair<int,double>>> coiProd;   // for each family, contains produts coi
    std::vector<std::pair<int,double>> coiFam; // contains families coi


    /* METHODS */

    /**
    * @brief struct constructor that initiate the different coi
    * @param data struct containing of instance
    */
    Coi(const Data& data); 
};

/**
* @brief greedy algorithm based on the COI principle
* @param data struct containing of instance
*/
Solution coiGreedy(const Data& data);

/**
 * @brief Builds a deterministic initial feasible solution.
 * @param data problem data
 * @return A solution where products are assigned sequentially to racks
 *         following the family order.
 */
Solution initSol(const Data& data);


/**
 * @brief Generates a random initial feasible solution.
 * @param data problem data
 * @return A solution where products are randomly ordered within each family
 *         and sequentially assigned to racks respecting capacity constraints.
 */
Solution initSolRandom(const Data& data);

/**
 * @brief Contains Correlation related infos
 * @note cor and corFam are stored as upper triangular matrices.
 */
struct Correlation {
    std::vector<std::vector<int>> cor;     // product-product (upper triangular filled)
    std::vector<std::vector<int>> corFam;  // family-family (upper triangular filled)
};

/**
 * @brief Simple function that update CorrMatrix 
 * @note shall only be called by buildCorrFam
 */
void updateCorrMatrix(std::vector<std::vector<int>>& famCorr, 
    const std::vector<int>& famInOrder); 

/**
 * @brief compute the famillies correlation matrix 
 * @param data problem data 
 * @return correlation matrix 
 */
std::vector<std::vector<int>> buildCorrFam(const Data& data);

/**
 * @brief Builds the correlation matrix between different products within the same family.
 * @param out correlation matrix to construct
 * @param data problem data
 * @note Only same-family product pairs are counted.
 */
void CorrelationSameFam(Correlation& out, const Data& data);

/**
 * @brief Builds and returns the correlation matrix between different products within the same family.
 * @param data problem data
 * @param display whether to display the matrix
 * @return Correlation structure containing the matrix
 */
Correlation BuildCorSameFam(const Data& data, bool display);

/**
 * @brief Builds the correlation matrices between products belonging to different families.
 * @param out correlation matrices to construct
 * @param data problem data
 * @note Counts both product-product and family-family correlations.
 */
void CorrelationDiffFam(Correlation& out, const Data& data);

/**
 * @brief Builds and returns the correlation matrices for different-family correlations.
 * @param data problem data
 * @param display whether to display the matrices
 * @return Correlation structure containing product-product and family-family matrices
 */
Correlation BuildCorDiffFam(const Data& data, bool display);

/**
 * @brief Builds a family placement order based on the correlation matrix.
 * @param FamOrd output vector storing the family order
 * @param FamPlaced working array indicating whether each family has been placed
 * @param CorBetFam correlation data used to rank families
 * @param data problem data
 * @note The order is built incrementally by inserting families on the left or right.
 */
void BuildOrderFam(std::vector<int>& FamOrd, std::vector<bool>& FamPlaced,
                   const Correlation& CorBetFam, const Data& data);
/**
 * @brief Builds a family placement order based on the correlation matrix.
 * @param FamOrd output vector storing the family order
 * @param FamPlaced working array indicating whether each family has been placed
 * @param CorBetFam correlation data used to rank families
 * @param data problem data
 * @note The order is built incrementally by inserting families on the left,
 *       on the right, or between two consecutive families (best insertion rule).
 */
void BuildOrderFamBestInsert(std::vector<int>& FamOrd, std::vector<bool>& FamPlaced,
                   const Correlation& CorBetFam, const Data& data);

/**
 * @brief Builds a product placement order for each family following the family order.
 * @param CorInFam correlation data used to rank products within families
 * @param ProdOrd output vector storing the global product order
 * @param data problem data
 * @param FamOrd family order previously computed
 * @note Products of each family are inserted contiguously in the global order.
 */
void BuildProdOrder(const Correlation& CorInFam, std::vector<int>& ProdOrd,
                    const Data& data, const std::vector<int>& FamOrd);

/**
 * @brief Builds a product placement order using a best insertion strategy within each family.
 * @param CorInFam correlation data used to evaluate insertion gains
 * @param ProdOrd output vector storing the global product order
 * @param data problem data
 * @param FamOrd family order previously computed
 * @note For each family in FamOrd, products are ordered using a best
 *       insertion rule based on correlation gains, then inserted
 *       contiguously into the global order.
 */
void BuildProdOrderBestInsert(const Correlation& CorInFam, std::vector<int>& ProdOrd,
                    const Data& data, const std::vector<int>& FamOrd);

/**
 * @brief Fills racks sequentially using the given product placement order.
 * @param sol solution to fill (product-to-rack and rack-to-product assignments)
 * @param data problem data
 * @param ProdOrd product placement order
 * @warning If total rack capacity is insufficient, some products may remain unassigned.
 */
void FillRacks(Solution& sol, const Data& data, const std::vector<int>& ProdOrd);

/**
 * @brief Build initial solution with best insertion policy
 * @param data problem data
 * @return computed solution
 * @note The heuristic builds family order, then product order, then fills racks sequentially.
 */
Solution GreedyBestInsert(const Data& data);

/**
 * @brief Displays the correlation matrices for different-family correlations.
 * @param out correlation data to display
 */
void DisplayDiff(Correlation out);

/**
 * @brief Displays the correlation matrix for within-family correlations.
 * @param out correlation data to display
 */
void DisplaySame(Correlation out);