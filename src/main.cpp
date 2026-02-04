#include"Datas.hpp"

int main(){

    allFiles af ;
    af.f1 = "../data/warehouse_toy/metadata.txt" ;
    af.f2 = "../data/warehouse_toy/rack_capacity.txt" ;
    af.f3 = "../data/warehouse_toy/rack_adjacency_matrix.txt" ;
    af.f4 = "../data/warehouse_toy/product_circuit.txt" ;
    af.f5 = "../data/warehouse_toy/aisle_racks.txt" ;
    af.f6 = "../data/warehouse_toy/orders.txt" ;
    
    Data data(af); 
    data.displayData(); 

    std::vector<int> aer = data.InitcapRacksAer() ;

    return 0 ;
}