#pragma once

#include "gurobi_c++.h"
#include "Datas.hpp"          
#include <string>

/**
 * MILP for the warehouse placement + routing model (0-based indexing).
 * - products: 0..P-1
 * - racks   : 0..R-1  (start rack = 0, end rack = R-1)
 * - families: 0..F-1
 * - orders  : 0..O-1
 *
 * y[o][r][l] is created only for r < l (else fixed to 0).
 */
struct WarehouseMILP {

    GRBEnv*  env;
    GRBModel* model;

    // Decision variables
    GRBVar**  x;     // x[r][p] in {0,1}
    GRBVar**  z;     // z[f][r] in {0,1}
    GRBVar*** y;     // y[o][r][l] in {0,1} if r<l, else fixed 0

    Data instance;

    WarehouseMILP(const Data& data);
    ~WarehouseMILP();

    /**
     * Solve the MILP and return a placement Solution (prodToRack + RackToProd).
     * If writeLP=true, writes the LP model to lpName.
     */
    Solution solver(bool writeLP = false, const std::string& lpName = "warehouse_model.lp");

private:
    void buildModel();
    double alphaValue() const;
};
