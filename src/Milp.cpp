#include "Milp.hpp"
#include <sstream>
#include <cmath>

WarehouseMILP::WarehouseMILP(const Data& data)
    : env(nullptr), model(nullptr),
      x(nullptr), z(nullptr), y(nullptr),
      instance(data)
{
    env = new GRBEnv(true);// Create Gurobi environment
    env->set(GRB_IntParam_LogToConsole, 0);// Disable solver log
    env->start();// Start environment
    model = new GRBModel(*env);// Create model
    model->set(GRB_DoubleParam_TimeLimit, 3600.0); // Set time limit (1 hour)

    buildModel();// Build MILP formulation
}

WarehouseMILP::~WarehouseMILP()
{
    if(y){// Free routing variables
        for(int o = 0; o < instance.nbOrd; o++){
            if(y[o]){
                for(int r = 0; r < instance.nbRacks; r++){
                    delete[] y[o][r];
                }
                delete[] y[o];
            }
        }
        delete[] y;
        y = nullptr;
    }

    if(x){$// Free placement variables
        for(int r = 0; r < instance.nbRacks; r++){
            delete[] x[r];
        }
        delete[] x;
        x = nullptr;
    }

    if(z){$// Free family variables
        for(int f = 0; f < instance.nbFam; f++){
            delete[] z[f];
        }
        delete[] z;
        z = nullptr;
    }

    if(model){ delete model; model = nullptr; }// Delete model
    if(env){ delete env; env = nullptr; }// Delete environment
}

double WarehouseMILP::alphaValue() const
{
    double a = (double)instance.aeration;// Retrieve aeration parameter
    a /= 100.0;// Convert percentage to fraction
    return a;// Return alpha in [0,1]
}

void WarehouseMILP::buildModel()
{
    const int R = instance.nbRacks;// Number of racks
    const int P = instance.nbProd;// Number of products
    const int F = instance.nbFam;// Number of families
    const int O = instance.nbOrd;// Number of orders

    const int startRack = 0;// Start rack index
    const int endRack   = R - 1;// End rack index

    x = new GRBVar*[R];// Allocate x[r][p]
    for(int r = 0; r < R; r++){
        x[r] = new GRBVar[P];
        for(int p = 0; p < P; p++){
            std::stringstream ss;
            ss << "x(" << r << "," << p << ")";
            x[r][p] = model->addVar(0.0, 1.0, 0.0, GRB_BINARY, ss.str());  // Product placement
        }
    }

    z = new GRBVar*[F];// Allocate z[f][r]
    for(int f = 0; f < F; f++){
        z[f] = new GRBVar[R];
        for(int r = 0; r < R; r++){
            std::stringstream ss;
            ss << "z(" << f << "," << r << ")";
            z[f][r] = model->addVar(0.0, 1.0, 0.0, GRB_BINARY, ss.str());  // Family activation
        }
    }

    y = new GRBVar**[O];// Allocate y[o][r][l]
    for(int o = 0; o < O; o++){
        y[o] = new GRBVar*[R];
        for(int r = 0; r < R; r++){
            y[o][r] = new GRBVar[R];
            for(int l = 0; l < R; l++){
                if(r < l){
                    std::stringstream ss;
                    ss << "y(" << o << "," << r << "," << l << ")";
                    y[o][r][l] = model->addVar(0.0, 1.0, 0.0, GRB_BINARY, ss.str());  // Routing arc
                } else {
                    y[o][r][l] = model->addVar(0.0, 0.0, 0.0, GRB_CONTINUOUS, "y_dummy");  // Fixed to 0
                }
            }
        }
    }

    GRBLinExpr obj = 0;// Objective initialization
    for(int o = 0; o < O; o++){
        for(int r = 0; r < R; r++){
            for(int l = r + 1; l < R; l++){
                obj += (double)instance.dists[r][l] * y[o][r][l];// Distance cost
            }
        }
    }
    model->setObjective(obj, GRB_MINIMIZE);// Minimize routing distance

    for(int r = 0; r < R; r++){
        GRBLinExpr lhs = 0;
        for(int p = 0; p < P; p++){
            lhs += x[r][p];
        }
        model->addConstr(lhs <= instance.capRacks[r]);// Rack capacity
    }

    for(int p = 0; p < P; p++){
        GRBLinExpr lhs = 0;
        for(int r = 0; r < R; r++){
            lhs += x[r][p];
        }
        model->addConstr(lhs == 1);// Each product assigned once
    }

    {
        const double alpha = alphaValue();// Aeration factor
        int start = 0;
        for(int a = 0; a < (int)instance.aisle.size(); a++){
            int end = instance.aisle[a];

            GRBLinExpr lhs = 0;
            double sumCap = 0.0;

            for(int r = start; r <= end; r++){
                for(int p = 0; p < P; p++){
                    lhs += x[r][p];
                }
                sumCap += (double)instance.capRacks[r];
            }

            int rhs = (1 - alpha) * sumCap;
            model->addConstr(lhs <= rhs); // Aeration constraint per aisle
            start = end + 1;
        }
    }

    for(int o = 0; o < O; o++){
        GRBLinExpr lhs = 0;
        for(int l = startRack + 1; l < R; l++){
            lhs += y[o][startRack][l];
        }
        model->addConstr(lhs == 1);// One departure from start
    }

    for(int o = 0; o < O; o++){
        GRBLinExpr lhs = 0;
        for(int r = 0; r < endRack; r++){
            lhs += y[o][r][endRack];
        }
        model->addConstr(lhs == 1);// One arrival at end
    }

    for(int o = 0; o < O; o++){
        for(int l = 1; l <= R - 2; l++){
            GRBLinExpr lhs = 0;
            for(int r = 0; r < l; r++){
                lhs += y[o][r][l];
            }
            model->addConstr(lhs <= 1);// At most one visit
        }
    }

    for(int o = 0; o < O; o++){
        for(int l = 1; l <= R - 2; l++){
            GRBLinExpr inFlow = 0;
            GRBLinExpr outFlow = 0;

            for(int r = 0; r < l; r++){
                inFlow += y[o][r][l];
            }
            for(int r = l + 1; r < R; r++){
                outFlow += y[o][l][r];
            }
            model->addConstr(inFlow == outFlow);// Flow conservation
        }
    }

    for(int o = 0; o < O; o++){
        const auto& setP = instance.ords.ordsToProd[o];
        for(int r = 0; r < R-1; r++){
            GRBLinExpr lhs = 0;
            for(int l = r + 1; l < R-1; l++){
                lhs += y[o][r][l];
            }
            lhs *= (double)instance.capRacks[r];

            GRBLinExpr rhs = 0;
            for(int p : setP){
                rhs += x[r][p];
            }
            model->addConstr(lhs >= rhs);// Activate rack if order uses it
        }
    }

    for(int f = 0; f < F; f++){
        for(int r = 0; r < R; r++){
            for(int s = r + 2; s < R; s++){
                GRBLinExpr mid = 0;
                for(int t = r + 1; t <= s - 1; t++){
                    mid += z[f][t];
                }
                model->addConstr(z[f][r] + z[f][s] - 1 <= mid);// Contiguous family racks
            }
        }
    }

    for(int f = 0; f < F; ++f){
        for(int g = 0; g < F; ++g){
            if(g == f) continue;
            for(int r = 0; r < R; ++r){
                for(int s = r + 2; s < R; ++s){
                    GRBLinExpr mid = 0;
                    for(int t = r + 1; t <= s - 1; ++t){
                        mid += z[g][t];
                    }
                    int M = (s - r - 1);
                    model->addConstr(mid <= M * (2 - z[f][r] - z[f][s]));// No interleaving
                }
            }
        }
    }

    for(int f = 0; f < F; f++){
        for(int p = 0; p < P; p++){
            if(f == instance.fam.prodToFam[p]){
                for(int r = 0; r < R; r++){
                    model->addConstr(x[r][p] <= z[f][r]);// Product-family linking
                }
            }
        }
    }
}

Solution WarehouseMILP::solver(bool writeLP, const std::string& lpName)
{
    if(writeLP){
        model->write(lpName);// Export LP if requested
    }

    model->optimize();// Solve MILP

    int status = model->get(GRB_IntAttr_Status);
    int solCount = 0;

    try {
        solCount = model->get(GRB_IntAttr_SolCount);// Retrieve solution count
    } catch(...) {
        solCount = 0;
    }

    if(solCount > 0 &&
       (status == GRB_OPTIMAL || status == GRB_SUBOPTIMAL || status == GRB_TIME_LIMIT))
    {
        Solution sol;
        sol.prodToRack.assign(instance.nbProd, -1);
        sol.RackToProd.assign(instance.nbRacks, {});

        const int R = instance.nbRacks;
        const int P = instance.nbProd;

        for(int p = 0; p < P; p++){
            for(int r = 0; r < R; r++){
                if(x[r][p].get(GRB_DoubleAttr_X) > 0.5){ // Extract placement
                    sol.prodToRack[p] = r;
                    sol.RackToProd[r].push_back(p);
                    break;
                }
            }
        }
        return sol;
    }

    if(status == GRB_INFEASIBLE){
        model->computeIIS();// Compute IIS if infeasible
        model->write("warehouse_model.ilp");
    }

    Solution empty;
    empty.prodToRack.assign(instance.nbProd, -1);
    empty.RackToProd.assign(instance.nbRacks, {});
    return empty;// Return empty solution
}