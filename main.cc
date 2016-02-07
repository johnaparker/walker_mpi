#include <iostream>
#include <mpi.h>
#include "matrix.h"

using namespace std;

struct walker {
    int x;
    int y;

    walker(int x, int y): x(x), y(y) {};
};

struct sub_grid {
    matrix<int> grid;
    int xc, yc;

    sub_grid() = default;
    sub_grid(int Nx, int Ny, int xc, int yc): xc(xc), yc(yc) {
        int* data = new int[Nx*Ny];
        grid = matrix<int>(data, Nx, Ny);
    }
};

void initialize_grid(int Lx, int Ly, int world_size, int world_rank, sub_grid & new_grid) {
    int Nx = Lx/2;
    int Ny = Ly/2;
    int xc = (world_rank == 1 || world_rank == 3) ? Nx: 0;
    int yc = (world_rank == 2 || world_rank == 3) ? Ny: 0;
    new_grid = sub_grid(Nx,Ny,xc,yc);
}


int main(int argc, char* argv[]) {
    MPI_Init(&argc, &argv);

    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    int Lx = 10;
    int Ly = 10;

    sub_grid my_grid;
    initialize_grid(Lx, Ly, world_size, world_rank, my_grid);
    cout << world_rank << ": My corner is " << my_grid.xc << ", " << my_grid.yc << endl;

    MPI_Finalize();
}
