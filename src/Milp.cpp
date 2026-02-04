#include "Milp.hpp"
#include <sstream>
#include <cmath>

WarehouseMILP::WarehouseMILP(const Data& data)
    : env(nullptr), model(nullptr),
      x(nullptr), z(nullptr), y(nullptr),
      instance(data)
{
    env = new GRBEnv(true);
    env->set(GRB_IntParam_LogToConsole, 0);
    env->start();
    model = new GRBModel(*env);
    model->set(GRB_DoubleParam_TimeLimit, 3600.0);

    buildModel();
}

WarehouseMILP::~WarehouseMILP()
{
    // Free vars arrays
    if(y){
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

    if(x){
        for(int r = 0; r < instance.nbRacks; r++){
            delete[] x[r];
        }
        delete[] x;
        x = nullptr;
    }

    if(z){
        for(int f = 0; f < instance.nbFam; f++){
            delete[] z[f];
        }
        delete[] z;
        z = nullptr;
    }

    // Free model/env
    if(model){ delete model; model = nullptr; }
    if(env){ delete env; env = nullptr; }
}

/**
 * Convert instance.aeration to alpha in [0,1] as best-effort:
 * - if aeration <= 1 : assume it's already a fraction (e.g. 0.2)
 * - if aeration > 1  : assume it's a percentage (e.g. 20 means 0.20)
 */
double WarehouseMILP::alphaValue() const
{
    double a = (double)instance.aeration;
    a /= 100.0;
    return a;
}

void WarehouseMILP::buildModel()
{
    const int R = instance.nbRacks;  // racks: 0..R-1
    const int P = instance.nbProd;   // products: 0..P-1
    const int F = instance.nbFam;    // families: 0..F-1
    const int O = instance.nbOrd;    // orders: 0..O-1

    const int startRack = 0;
    const int endRack   = R - 1;

    // -----------------------------
    // Variables x[r][p]
    // -----------------------------
    x = new GRBVar*[R];
    for(int r = 0; r < R; r++){
        x[r] = new GRBVar[P];
        for(int p = 0; p < P; p++){
            std::stringstream ss;
            ss << "x(" << r << "," << p << ")";
            x[r][p] = model->addVar(0.0, 1.0, 0.0, GRB_BINARY, ss.str());
        }
    }

    // -----------------------------
    // Variables z[f][r]
    // -----------------------------
    z = new GRBVar*[F];
    for(int f = 0; f < F; f++){
        z[f] = new GRBVar[R];
        for(int r = 0; r < R; r++){
            std::stringstream ss;
            ss << "z(" << f << "," << r << ")";
            z[f][r] = model->addVar(0.0, 1.0, 0.0, GRB_BINARY, ss.str());
        }
    }

    // -----------------------------
    // Variables y[o][r][l] only for r<l
    // We allocate full cube but fix to 0 when r>=l.
    // -----------------------------
    y = new GRBVar**[O];
    for(int o = 0; o < O; o++){
        y[o] = new GRBVar*[R];
        for(int r = 0; r < R; r++){
            y[o][r] = new GRBVar[R];
            for(int l = 0; l < R; l++){
                if(r < l){
                    std::stringstream ss;
                    ss << "y(" << o << "," << r << "," << l << ")";
                    y[o][r][l] = model->addVar(0.0, 1.0, 0.0, GRB_BINARY, ss.str());
                } else {
                    // Not allowed by definition (r<l), fixed to 0
                    y[o][r][l] = model->addVar(0.0, 0.0, 0.0, GRB_CONTINUOUS, "y_dummy");
                }
            }
        }
    }

    // -----------------------------
    // Objective: min sum_{o,r<l} d_rl * y_orl
    // data.dists is assumed 0-based: dists[r][l]
    // -----------------------------
    GRBLinExpr obj = 0;
    for(int o = 0; o < O; o++){
        for(int r = 0; r < R; r++){
            for(int l = r + 1; l < R; l++){
                obj += (double)instance.dists[r][l] * y[o][r][l];
            }
        }
    }
    model->setObjective(obj, GRB_MINIMIZE);

    // =========================================================
    // Constraints in EXACT same order (1) ... (15)
    // =========================================================

    // (1) Capacity: sum_p x[r][p] <= C_r   forall r
    for(int r = 0; r < R; r++){
        GRBLinExpr lhs = 0;
        for(int p = 0; p < P; p++){
            lhs += x[r][p];
        }
        model->addConstr(lhs <= instance.capRacks[r]);
    }

    // (2) Each product placed once: sum_r x[r][p] = 1  forall p
    for(int p = 0; p < P; p++){
        GRBLinExpr lhs = 0;
        for(int r = 0; r < R; r++){
            lhs += x[r][p];
        }
        model->addConstr(lhs == 1);
    }

    // (3) Aeration per aisle:
    // sum_{r in aisle} sum_p x[r][p] <= ceil((1-alpha) * sum_{r in aisle} C_r)
    // instance.aisle contains the biggest rack id of each aisle (0-based).
    {
        const double alpha = alphaValue();
        int start = 0;
        for(int a = 0; a < (int)instance.aisle.size(); a++){
            int end = instance.aisle[a]; // inclusive, 0-based rack id

            GRBLinExpr lhs = 0;
            double sumCap = 0.0;

            for(int r = start; r <= end; r++){
                for(int p = 0; p < P; p++){
                    lhs += x[r][p];
                }
                sumCap += (double)instance.capRacks[r];
            }

            int rhs = (1 - alpha) * sumCap;
            std::cout << "aeration " << rhs << " capacite "<<sumCap<<std::endl;
            model->addConstr(lhs <= rhs);

            start = end + 1;
        }
    }

    // (4) Start at rack 0: sum_{l>0} y[o][0][l] = 1  forall o
    for(int o = 0; o < O; o++){
        GRBLinExpr lhs = 0;
        for(int l = startRack + 1; l < R; l++){
            lhs += y[o][startRack][l];
        }
        model->addConstr(lhs == 1);
    }

    // (5) End at rack R-1: sum_{r<R-1} y[o][r][R-1] = 1  forall o
    for(int o = 0; o < O; o++){
        GRBLinExpr lhs = 0;
        for(int r = 0; r < endRack; r++){
            lhs += y[o][r][endRack];
        }
        model->addConstr(lhs == 1);
    }

    // (6) y[o][r][0] = 0  forall o,r
    // Not needed: y only exists for r<l, so l=0 impossible unless r<0.
    // (7) y[o][R-1][l] = 0 forall o,l
    // Not needed: r=R-1 cannot be < l.

    // Define R* = {1, ..., R-2} (intermediate racks)
    // (8) For l in R*: sum_{r<l} y[o][r][l] <= 1  forall o,l
    for(int o = 0; o < O; o++){
        for(int l = 1; l <= R - 2; l++){
            GRBLinExpr lhs = 0;
            for(int r = 0; r < l; r++){
                lhs += y[o][r][l];
            }
            model->addConstr(lhs <= 1);
        }
    }

    // (9) Flow conservation on R*:
    // sum_{r<l} y[o][r][l] = sum_{l<r} y[o][l][r]  forall o,l in R*
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
            model->addConstr(inFlow == outFlow);
        }
    }

    // (10) Activation if order uses products stored in rack r:
    // C_r * sum_{l>r} y[o][r][l] >= sum_{p in order o} x[r][p]
    for(int o = 0; o < O; o++){
        const auto& setP = instance.ords.ordsToProd[o]; // products ids are 0-based

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
            model->addConstr(lhs >= rhs);
        }
    }

    // (11) Contiguity of racks for each family:
    // z[f][r] + z[f][s] - 1 <= sum_{t=r+1}^{s-1} z[f][t]  forall f, forall r<s
    for(int f = 0; f < F; f++){
        for(int r = 0; r < R; r++){
            for(int s = r + 2; s < R; s++){
                GRBLinExpr mid = 0;
                for(int t = r + 1; t <= s - 1; t++){
                    mid += z[f][t];
                }
                model->addConstr(z[f][r] + z[f][s] - 1 <= mid);
            }
        }
    }

    // (11-bis) No interleaving between families: if family f uses racks r and s,
    // then no other family g can use racks strictly between r and s.
    for(int f = 0; f < F; ++f){
        for(int g = 0; g < F; ++g){
            if(g == f) continue;
            for(int r = 0; r < R; ++r){
                for(int s = r + 2; s < R; ++s){ // IMPORTANT: s >= r+2
                    GRBLinExpr mid = 0;
                    for(int t = r + 1; t <= s - 1; ++t){
                        mid += z[g][t];
                    }
                    int M = (s - r - 1);
                    model->addConstr(mid <= M * (2 - z[f][r] - z[f][s]));
                }
            }
        }
    }


    for(int r = 0; r < R; r++){
        for(int f = 0; f < F; f++){
            for(int m = 0; m < F; m++){}
        }
    }

    // (12) x binary -> already declared binary
    // (13) y binary -> already declared binary (for r<l), fixed 0 otherwise
    // (14) z binary -> already declared binary

    // (15) Link product-family:
    // x[r][p] <= z[f_p][r]  forall r,p
    for(int f = 0; f < F; f++){
        for(int p = 0; p < P; p++){
            if(f == instance.fam.prodToFam[p]){
                for(int r = 0; r < R; r++){
                    model->addConstr(x[r][p] <= z[f][r]);
                }
            } // family id 0-based
        }
    }

}

Solution WarehouseMILP::solver(bool writeLP, const std::string& lpName)
{
    if(writeLP){
        model->write(lpName); // utile même avant optimise
    }

    model->optimize();

    int status = model->get(GRB_IntAttr_Status);
    int solCount = 0;

    // SolCount existe même si pas optimal
    try {
        solCount = model->get(GRB_IntAttr_SolCount);
    } catch(...) {
        solCount = 0;
    }

    // Cas OK: on a au moins une solution
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
                if(x[r][p].get(GRB_DoubleAttr_X) > 0.5){
                    sol.prodToRack[p] = r;
                    sol.RackToProd[r].push_back(p);
                    break;
                }
            }
        }
        return sol;
    }

    // Sinon: pas de solution => NE PAS lire les vars
    if(status == GRB_INFEASIBLE){
        std::cerr << "Model infeasible. Computing IIS...\n";
        model->computeIIS();
        model->write("warehouse_model.ilp"); // IIS file
    } else if(status == GRB_UNBOUNDED){
        std::cerr << "Model unbounded.\n";
    } else {
        std::cerr << "No solution available. Status = " << status << "\n";
    }

    // retourne une solution vide
    Solution empty;
    empty.prodToRack.assign(instance.nbProd, -1);
    empty.RackToProd.assign(instance.nbRacks, {});
    return empty;
}
