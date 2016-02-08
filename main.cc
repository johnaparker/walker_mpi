#include <iostream>
#include <mpi.h>
#include <stdexcept>
#include <stdlib.h>
#include <time.h>
#include <vector>
#include <algorithm>
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
    int Lx, Ly;

    sub_grid() = default;
    sub_grid(int Lx, int Ly, int Nx, int Ny, int xc, int yc, int world_rank):
                        Lx(Lx), Ly(Ly), xc(xc), yc(yc), Nx(Nx), Ny(Ny), world_rank(world_rank) {
        walker** data = new walker*[Nx*Ny];
        grid = matrix<walker*>(data, Nx, Ny);
    }

    void create_walker(int xp, int yp) {
        check_out_of_bounds_error(xp,yp);
        walker* new_walker = new walker(xp,yp);
        grid[xp][yp] = new_walker;
    }
    
    vector<int> allowed_movements(int xp, int yp) {
        //0 bottom, 1 right, 2 top, 3 left, -1 interior
        vector<int> movements;

        bool my_border,main_border;
        int locx, locy, locx_out, locy_out;
        my_border = on_my_border(xp,yp,locx,locy);
        main_border = on_outer_border(xp,yp,locx_out,locy_out);

        if (my_border) {
            if (locy == 0) {
                if (grid[xp][yp+1] == nullptr)
                    movements.push_back(2);
                if (locy_out != 0)
                    movements.push_back(0);
            }
            else if (locy == 2) {
                if (grid[xp][yp-1] == nullptr)
                    movements.push_back(0);
                if (locy_out != 2)
                    movements.push_back(2);
            }
            if (locx == 1) {
                if (grid[xp-1][yp] == nullptr)
                    movements.push_back(3);
                if (locx_out != 1)
                    movements.push_back(1);
            }
            else if (locx == 3) {
                if (grid[xp+1][yp] == nullptr)
                    movements.push_back(1);
                if (locx_out != 3)
                    movements.push_back(3);
            }
        }
        else {
            if (grid[xp+1][yp] == nullptr)
                movements.push_back(1);
            if (grid[xp-1][yp] == nullptr)
                movements.push_back(3);
            if (grid[xp][yp+1] == nullptr)
                movements.push_back(2);
            if (grid[xp][yp-1] == nullptr)
                movements.push_back(0);
        }
        return movements;
    }

    
    void move_walker(int xp, int yp) {
        walker* cur_walker = grid[xp][yp];
        if (cur_walker == nullptr)
            return;

        vector<int> movements = allowed_movements(xp,yp);
        int mov, n;
        if (!movements.empty()) {
            n = rand() % movements.size();
            mov = movements[n] - 1;
        }
        else {
            n = 0;
            mov = -2;
        }
        int dx = (mov == 0 || mov == 2) ? -1*mov+1 : 0;
        int dy = (mov ==  -1 || mov == 1) ? mov: 0;
        int newx = xp + dx;
        int newy = yp + dy;
        
        if (valid_pos(newx, newy) && !cur_walker->moved) {
            cur_walker->move(dx,dy);
            grid[xp][yp] = nullptr;
            grid[newx][newy] = cur_walker;
            moved_walkers.push_back(cur_walker);
        }
        //else if (check_out_of_bounds(xp,yp) && on_shared_border(xp,yp))
        
        
    }


    void update() {
        for (int i = 0; i != Nx; i++) {
            for (int j = 0; j != Ny; j++) {
                move_walker(i,j);
            }
        }
        reset_moved_walkers();
    }

    void reset_moved_walkers() {
        for (auto & w: moved_walkers)
            w->moved = false;
        moved_walkers.clear();
    }

    bool on_shared_border(int xp, int yp, vector<bool>& dirs) {
        //dirs: false means outer border. True means shared border if returns true
        bool on_sub_border, on_main_border;
        dirs.clear();
        dirs = {true, true, true, true};
        int loc1, loc2;

        if (on_my_border(xp,yp, loc1, loc2))
            on_sub_border = true;
        else
            return false;

        on_main_border = on_outer_border(xp,yp, loc1, loc2);
        if (loc1 != -1)
            dirs[loc1] = false;
        if (loc2 != -1)
            dirs[loc2] = false;
        if (!on_main_border)
            return true;
        
        int x = xp + xc;
        int y = yp + yc;
        
        if ((xp == 0 && yp == 0) || (xp == 0 && yp == Ny) || (xp == Nx && yp == 0) || (xp == Nx && yp == Ny)) {
            if ((x == 0 && y == 0) || (x == 0 && y == Ly) || (x == Lx && y == 0) || (x == Lx && y == Ly))
                return false;
            return true;
        }
        return false; 
    }

    bool on_my_border(int xp, int yp, int& locx, int& locy) {
        if (xp == 0) {
            locx = 3;
            return true;
        }
        else if (xp == Nx) {
            locx = 1;
            return true;
        }
        else locx = -1;
        if (yp == 0) {
            locy = 0;
            return true;
        }
        else if (yp == Ny) {
            locy = 2;
            return true;
        }
        else locy= -1;
        return false;
    }

    bool on_outer_border(int xp, int yp, int& locx, int& locy) const {
        //0 bottom, 1 right, 2 top, 3 left, -1 interior
        int x = xp + xc;
        int y = yp + yc;
        if (x == 0) {
            locx = 3;
            return true;
        }
        else if (x == Lx) {
            locx = 1;
            return true;
        }
        else locx = -1;
        if (y == 0) {
            locy = 0;
            return true;
        }
        else if (y == Ly) {
            locy = 2;
            return true;
        }
        else locy= -1;
        return false;
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
                    cout << "\t" << w->x << ", " << w->y << endl;
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
    new_grid = sub_grid(Lx,Ly,Nx,Ny,xc,yc, world_rank);
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
    for (int i = 0; i != 100; i++)
        my_grid.update();
    my_grid.display(world_size);

    MPI_Finalize();
}

