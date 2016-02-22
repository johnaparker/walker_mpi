#include "grid.h"

using namespace std;

int main(int argc, char* argv[]) {
    MPI_Init(&argc, &argv);

    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    srand(time(NULL)+world_rank);       
    int Lx = 10;
    int Ly = 10;

    sub_grid my_grid;
    initialize_grid(Lx, Ly, world_size, world_rank, my_grid);
    my_grid.create_walker(2,2, world_rank);
    for (int i = 0; i != 100; i++)
        my_grid.update();
    my_grid.display(world_size);

    if (world_rank == 0) {
        h5out output("test.h5");
        output.create_node("0", {2}); 
        output.create_node("1", {2}); 
        output.create_node("2", {2}); 
        output.create_node("3", {2}); 
    }
    
    
    MPI_Finalize();
}

