#include <iostream>
#include <mpi.h>
#include <stdexcept>
#include <stdlib.h>
#include <time.h>
#include "matrix.h"

using namespace std;

struct walker {
    int x;
    int y;

    walker(int x, int y): x(x), y(y) {};
    void move(int dx, int dy) {
        x += dx;
        y += dy;
    }
};

struct sub_grid {
    matrix<walker*> grid;
    int xc, yc;
    int Nx, Ny;

    sub_grid() = default;
    sub_grid(int Nx, int Ny, int xc, int yc): xc(xc), yc(yc), Nx(Nx), Ny(Ny) {
        walker** data = new walker*[Nx*Ny];
        grid = matrix<walker*>(data, Nx, Ny);
    }

    void create_walker(int xp, int yp) {
        check_out_of_bounds(xp,yp);
        walker* new_walker = new walker(xp,yp);
        grid[xp][yp] = new_walker;
    }
    
    void move_walker(int xp, int yp) {
        check_out_of_bounds(xp,yp);
        walker* cur_walker = grid[xp][yp];
        if (cur_walker == nullptr)
            return;
        srand(time(NULL));       
        int mov = rand() % 4 - 1 ;
        int dx = (mov == -1 || mov == 1) ? mov : 0;
        int dy = (mov ==  0 || mov == 2) ? mov-1: 0;
        
        if (valid_pos(xp+dx, yp+dy)) 
            cur_walker->move(dx,dy);
        cout << "Final Postion: " << xc+cur_walker->x << ", : " << 
                     yc+cur_walker->y << endl;
    }


    void update() {
        for (int i = 0; i != Nx; i++) {
            for (int j = 0; j != Ny; j++) {
                move_walker(i,j);
            }
        }
    }

    bool valid_pos(int xp, int yp) const{
        if (xp < 0 || xp > Nx-1 || yp < 0 || yp > Ny-1)
            return false;
        return true; 
    }

    void check_out_of_bounds(int xp, int yp) const{
        if (!valid_pos(xp,yp))
            throw invalid_argument("new walker position is out of bounds");
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
    my_grid.create_walker(2,2);
    my_grid.update();

    MPI_Finalize();
}

