#include <iostream>
#include <mpi.h>
#include <stdexcept>
#include <stdlib.h>
#include <time.h>
#include <vector>
#include "matrix.h"

using namespace std;

struct walker {
    int x;
    int y;
    bool moved;

    walker(int x, int y): x(x), y(y), moved(false) {};
    void move(int dx, int dy) {
        x += dx;
        y += dy;
        moved = true;
    }
};

struct sub_grid {
    matrix<walker*> grid;
    vector<walker*> moved_walkers;
    int xc, yc;
    int Nx, Ny;
    int world_rank;

    sub_grid() = default;
    sub_grid(int Nx, int Ny, int xc, int yc, int world_rank):
                        xc(xc), yc(yc), Nx(Nx), Ny(Ny), world_rank(world_rank) {
        walker** data = new walker*[Nx*Ny];
        grid = matrix<walker*>(data, Nx, Ny);
    }

    void create_walker(int xp, int yp) {
        check_out_of_bounds_error(xp,yp);
        walker* new_walker = new walker(xp,yp);
        grid[xp][yp] = new_walker;
    }
    
    void move_walker(int xp, int yp) {
        //check_out_of_bounds_error(xp,yp);
        walker* cur_walker = grid[xp][yp];
        if (cur_walker == nullptr)
            return;

        int mov = rand() % 4 - 1 ;
        cout << mov << endl;
        int dx = (mov == -1 || mov == 1) ? mov : 0;
        int dy = (mov ==  0 || mov == 2) ? mov-1: 0;
        int newx = xp + dx;
        int newy = yp + dy;
        
        if (valid_pos(newx, newy) && !cur_walker->moved) {
            cur_walker->move(dx,dy);
            grid[xp][yp] = nullptr;
            grid[newx][newy] = cur_walker;
            moved_walkers.push_back(cur_walker);
        }
        
    }


    void update() {
        for (int i = 0; i != Nx; i++) {
            for (int j = 0; j != Ny; j++) {
                move_walker(i,j);
            }
        }
        update_moved_walkers();
    }

    void update_moved_walkers() {
        for (auto & w: moved_walkers)
            w->moved = false;
        moved_walkers.clear();
    }

    bool valid_pos(int xp, int yp) {
        if (check_out_of_bounds(xp,yp))
            return false; 
        if (occupied(xp,yp))
            return false;
        return true;
    }

    bool occupied(int xp, int yp) {
        if (grid[xp][yp] == nullptr)
            return false;
        return true;
    }

    bool check_out_of_bounds(int xp, int yp) const{
        if (xp < 0 || xp > Nx-1 || yp < 0 || yp > Ny-1)
            return true;
        return false;
    }

    void check_out_of_bounds_error(int xp, int yp) const{
        if (check_out_of_bounds(xp,yp))
            throw invalid_argument("new walker position is out of bounds");
    }

    void display(int world_size) {
        int ready = 0;
        if (world_rank != 0)
            MPI_Recv(&ready, 1, MPI_INT, world_rank-1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        
        cout << "Rank " << world_rank << ": \n";
        for (int i = 0; i != Nx; i++) {
            for (int j = 0; j != Ny; j++) {
                if(occupied(i,j)) {
                    walker* w = grid[i][j];
                    cout << "\t" << i << ", " << j << endl;
                }
            }
        }
        if (world_rank != world_size-1)
            MPI_Send(&ready, 1, MPI_INT, world_rank+1, 0, MPI_COMM_WORLD);
    }
};

void initialize_grid(int Lx, int Ly, int world_size, int world_rank, sub_grid & new_grid) {
    int Nx = Lx/2;
    int Ny = Ly/2;
    int xc = (world_rank == 1 || world_rank == 3) ? Nx: 0;
    int yc = (world_rank == 2 || world_rank == 3) ? Ny: 0;
    new_grid = sub_grid(Nx,Ny,xc,yc, world_rank);
}


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
    my_grid.create_walker(2,2);
    my_grid.update();
    my_grid.update();
    my_grid.update();
    my_grid.update();
    my_grid.display(world_size);

    MPI_Finalize();
}

