#include "grid.h"
#include <string>
#include "logger.h"

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
    const int num_walkers = 16;
    int T = 500;

    sub_grid my_grid;
    initialize_grid(Lx, Ly, world_size, world_rank, my_grid);
    my_grid.create_walker(2,2, world_rank);
    my_grid.create_walker(2,3, world_rank+world_size);
    my_grid.create_walker(3,2, world_rank+2*world_size);
    my_grid.create_walker(3,3, world_rank+3*world_size);

    logger l1(*my_grid.grid[2][2], my_grid);
    logger l2(*my_grid.grid[2][3], my_grid);
    logger l3(*my_grid.grid[3][2], my_grid);
    logger l4(*my_grid.grid[3][3], my_grid);

    h5out* output = nullptr;
    if (world_rank == 0) {
        output= new h5out("test.h5");
        for (int j = 0; j != num_walkers; j++) {
            string node_name = to_string(j);
            const char* c = node_name.c_str();
            H5std_string d(c);
            output->create_node(d, {2}); 
        }
    }
    
    int *wi, *wx, *wy;
    wi = new int[num_walkers];
    wx = new int[num_walkers];
    wy = new int[num_walkers];

    for (int i = 0; i != T; i++) {
        my_grid.collect_at_main(wi, wx, wy, world_size, num_walkers);
        if (world_rank == 0) { for (int j = 0; j != num_walkers; j++) {
                double* data = new double[2];
                data[0] = wx[j];
                data[1] = wy[j];
                string node_name = to_string(wi[j]);
                const char* c = node_name.c_str();
                H5std_string d(c);
                output->write_to_node(d, data);
                delete data;
            } 
        }
        my_grid.update();
        l1.log();
        l2.log();
        l3.log();
        l4.log();
    }
my_grid.display(world_size);
    delete output;
    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Finalize();
}

