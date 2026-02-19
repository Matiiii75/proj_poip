#pragma once

#include "gurobi_c++.h"
#include "Datas.hpp"
#include <string>

/**
 * @brief MILP for the warehouse placement + routing model (0-based indexing).
 *
 * Indices:
 * - products: 0..P-1
 * - racks   : 0..R-1 (start rack = 0, end rack = R-1)
 * - families: 0..F-1
 * - orders  : 0..O-1
 *
 * Variable definition:
 * - y[o][r][l] is created only for r < l (otherwise it is fixed to 0).
 */
struct WarehouseMILP {

    GRBEnv*   env;
    GRBModel* model;

    // Decision variables
    GRBVar**  x;   // x[r][p] in {0,1}
    GRBVar**  z;   // z[f][r] in {0,1}
    GRBVar*** y;   // y[o][r][l] in {0,1} if r<l, else fixed to 0

    Data instance;

    WarehouseMILP(const Data& data);
    ~WarehouseMILP();

    /**
     * @brief Solves the MILP and returns a placement solution (prodToRack + RackToProd).
     * @param writeLP if true, writes the model to an LP file
     * @param lpName name of the LP file (used only if writeLP=true)
     * @return A feasible placement Solution extracted from the MILP.
     */
    Solution solver(bool writeLP = false, const std::string& lpName = "warehouse_model.lp");

private:
    /** @brief Builds variables, constraints, and objective. */
    void buildModel();

    /** @brief Returns the value of alpha used in the objective/constraints. */
    double alphaValue() const;
};