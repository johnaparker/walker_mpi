#include <iostream>
#include <mpi.h>
#include <stdexcept>
#include <stdlib.h>
#include <time.h>
#include <vector>
#include <algorithm>
#include "matrix.h"
#include "h5out.h"
#include "grid.h"

using namespace std;
using namespace H5;

void walker::move(int dx, int dy) {
    x += dx;
    y += dy;
    moved = true;
}


sub_grid::sub_grid(int Lx, int Ly, int Nx, int Ny, int xc, int yc, int world_rank):
                        Lx(Lx), Ly(Ly), xc(xc), yc(yc), Nx(Nx), Ny(Ny), world_rank(world_rank), tStep(0) {
    walker** data = new walker*[Nx*Ny];
    for(int i = 0; i != Nx*Ny; i++)
        data[i] = nullptr;
    grid = matrix<walker*>(data, Nx, Ny);
}

void sub_grid::create_walker(int xp, int yp, int index) {
    check_out_of_bounds_error(xp,yp);
    walker* new_walker = new walker(xp,yp, index);
    grid[xp][yp] = new_walker;
    current_walkers[index] = new_walker;
}
    
vector<int> sub_grid::allowed_movements(int xp, int yp) {
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
        else {
            if (grid[xp][yp+1] == nullptr)
                movements.push_back(2);
            if (grid[xp][yp-1] == nullptr)
                movements.push_back(0);
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
        else {
            if (grid[xp-1][yp] == nullptr)
                movements.push_back(3);
            if (grid[xp+1][yp] == nullptr)
                movements.push_back(1);
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

void sub_grid::move_walker(int xp, int yp) {
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

void sub_grid::update() {
    for (const auto& w: current_walkers) {
        move_walker(w.second->x,w.second->y);
    }
    reset_moved_walkers();
    tStep += 1;
}

void sub_grid::remove(int xp, int yp) {
    if (!occupied(xp,yp))
        throw invalid_argument("Nothing to remove at this position");
    walker* wp = grid[xp][yp];
    current_walkers.erase(wp->index);
    delete wp;
    grid[xp][yp] = nullptr;  
}

//resset the moved boolean after all walkers have been moved
void sub_grid::reset_moved_walkers() {
    for (auto & w: moved_walkers)
        w->moved = false;
    moved_walkers.clear();
}

bool sub_grid::on_shared_border(int xp, int yp, vector<bool>& dirs) {
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
    
    if ((xp == 0 && yp == 0) || (xp == 0 && yp == Ny-1) || (xp == Nx-1 && yp == 0) || (xp == Nx-1 && yp == Ny-1)) {
        if ((x == 0 && y == 0) || (x == 0 && y == Ly-1) || (x == Lx-1 && y == 0) || (x == Lx-1 && y == Ly-1))
            return false;
        return true;
    }
    return false; 
}

bool sub_grid::on_shared_border(int xp, int yp) {
    vector<bool> dirs;
    return on_shared_border(xp,yp,dirs);
}

bool sub_grid::on_my_border(int xp, int yp, int& locx, int& locy) {
    bool ret = false;
    if (xp == 0) {
        locx = 3;
        ret = true;
    }
    else if (xp == Nx-1) {
        locx = 1;
        ret = true;
    }
    else locx = -1;
    if (yp == 0) {
        locy = 0;
        ret = true;
    }
    else if (yp == Ny-1) {
        locy = 2;
        ret = true;
    }
    else locy= -1;
    return ret;
}

bool sub_grid::on_my_border(int xp, int yp) {
    int a,b;
    return on_my_border(xp,yp,a,b);
}

bool sub_grid::on_outer_border(int xp, int yp, int& locx, int& locy) const {
    //0 bottom, 1 right, 2 top, 3 left, -1 interior
    bool ret = false;
    int x = xp + xc;
    int y = yp + yc;
    if (x == 0) {
        locx = 3;
        ret = true;
    }
    else if (x == Lx-1) {
        locx = 1;
        ret = true;
    }
    else locx = -1;
    if (y == 0) {
        locy = 0;
        ret = true;
    }
    else if (y == Ly-1) {
        locy = 2;
        ret = true;
    }
    else locy= -1;
    return ret;
}

bool sub_grid::on_outer_border(int xp, int yp) const {
    int a,b;
    return on_outer_border(xp,yp,a,b);
}

bool sub_grid::valid_pos(int xp, int yp) {
    if (check_out_of_bounds(xp,yp))
        return false; 
    if (occupied(xp,yp))
        return false;
    return true;
}

bool sub_grid::occupied(int xp, int yp) {
    if (grid[xp][yp] == nullptr)
        return false;
    return true;
}

bool sub_grid::check_out_of_bounds(int xp, int yp) const{
    if (xp < 0 || xp > Nx-1 || yp < 0 || yp > Ny-1)
        return true;
    return false;
}

void sub_grid::check_out_of_bounds_error(int xp, int yp) const{
    if (check_out_of_bounds(xp,yp))
        throw invalid_argument("new walker position is out of bounds");
}

void sub_grid::display(int world_size) {
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

void sub_grid::collect_at_main(int *wi, int *wx, int *wy, int world_size, int num_walkers) {
    //int *wi, *wx, *wy;
    int size = current_walkers.size();
    int i = 0;

    if (world_rank != 0) {
        wi = new int[size];
        wx = new int[size];
        wy = new int[size];
        for (const auto& walker_i: current_walkers) {
            const walker& w = *walker_i.second;
            wi[i] = w.index;
            wx[i] = w.x+xc;
            wy[i] = w.y+yc;
            i++;
        }

        MPI_Send(wi, size, MPI_INT, 0, 1, MPI_COMM_WORLD);     
        delete [] wi;
        MPI_Send(wx, size, MPI_INT, 0, 2, MPI_COMM_WORLD);     
        delete [] wx;
        MPI_Send(wy, size, MPI_INT, 0, 3, MPI_COMM_WORLD);     
        delete [] wy;
    }
    else {
        for (const auto& walker_i: current_walkers) {
            const walker& w = *walker_i.second;
            wi[i] = w.index;
            wx[i] = w.x+xc;
            wy[i] = w.y+yc;
            i++;
        }

        for (int j = 1; j != world_size; j++) {
            MPI_Status status;
            int next_size;
            MPI_Probe(j, 1, MPI_COMM_WORLD, &status);
            MPI_Get_count(&status, MPI_INT, &next_size);
            MPI_Recv(wi + size, next_size, MPI_INT, j, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Recv(wx + size, next_size, MPI_INT, j, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Recv(wy + size, next_size, MPI_INT, j, 3, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            size += next_size;
        }  
    }
}

void sub_grid::share(int to_rank, int xpos, int ypos) {
    if (!occupied(xpos,ypos))
        throw invalid_argument("Nothing to share at this position");
    int index = grid[xpos][ypos]->index;
    int data[] = {xpos, ypos, index};
    MPI_Send(&data, 3, MPI_INT, to_rank, 4, MPI_COMM_WORLD);
    remove(xpos, ypos);
}

void sub_grid::receive(int from_rank) {
    int *data;
    MPI_Recv(data, 3, MPI_INT, from_rank, 4, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
}

void initialize_grid(int Lx, int Ly, int world_size, int world_rank, sub_grid & new_grid) {
    int Nx = Lx/2;
    int Ny = Ly/2;
    int xc = (world_rank == 1 || world_rank == 3) ? Nx: 0;
    int yc = (world_rank == 2 || world_rank == 3) ? Ny: 0;
    new_grid = sub_grid(Lx,Ly,Nx,Ny,xc,yc, world_rank);
}

