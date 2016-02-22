#include "grid.h"
#include <string>

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
    const int num_walkers = 4;

    sub_grid my_grid;
    initialize_grid(Lx, Ly, world_size, world_rank, my_grid);
    my_grid.create_walker(2,2, world_rank);

    h5out* output;
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

    for (int i = 0; i != 10; i++) {
        my_grid.update();
        my_grid.collect_at_main(wi, wx, wy, world_size, num_walkers);
        if (world_rank == 0) {
            for (int j = 0; j != num_walkers; j++) {
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
    }
    my_grid.display(world_size);

    MPI_Finalize();
}

