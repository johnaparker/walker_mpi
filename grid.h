#ifndef grid_GUARD
#define grid_GUARD

#include <iostream>
#include <mpi.h>
#include <stdexcept>
#include <stdlib.h>
#include <time.h>
#include <vector>
#include <map>
#include <algorithm>
#include "matrix.h"
#include "h5out.h"

//a walker moves randomly on a 2D grid
struct walker {
    int x;
    int y;
    bool moved;         //has the walker been moved for the given timestep
    int index;          //unique index

    walker(int x, int y, int index): x(x), y(y), index(index), moved(false) {};
    void move(int dx, int dy);
};


//the grid upon which walkers move
class sub_grid {
public:
    matrix<walker*> grid;               //the grid of walker pointers. nullptr's are empty
    std::map<int, walker*> current_walkers;   //a map of walkers currently in this grid
    std::vector<walker*> moved_walkers; //a list of walkers that have been moved
    int xc, yc;                         //xc,yc = bottom left corner relative to the whole grid
    int Nx, Ny;                         //number of cells in each direction
    int world_rank;
    int Lx, Ly;                         //number of cells in each direction for the whole grid
    int tStep;                          //current number of steps taken

public: 
    //constructors
    sub_grid() = default;
    sub_grid(int Lx, int Ly, int Nx, int Ny, int xc, int yc, int world_rank);  //initialize an empty grid

    //creating and moving walkers
    void create_walker(int xp, int yp, int index);  //create new walker at given (relative) position.
    void move_walker(int xp, int yp);   //move the walker at this position
    std::vector<int> allowed_movements(int xp, int yp);  //determine the allowed movements a walker can make
    void update();  //loop through grid, moving any walkers
    void reset_moved_walkers();   //resset the moved boolean after all walkers have been moved
    void remove(int xp, int yp);  //remove walker at positon xp,yp

    //determining the border
    bool on_shared_border(int xp, int yp, std::vector<bool>& dirs);  //determine whether a point lies on a shared border
    bool on_shared_border(int xp, int yp); 
    bool on_my_border(int xp, int yp, int& locx, int& locy);  //determine whether a point is on the border of the subgrid
    bool on_my_border(int xp, int yp);
    bool on_outer_border(int xp, int yp, int& locx, int& locy) const;  //determine whether a point is on the border of the whole grid
    bool on_outer_border(int xp, int yp) const;

    //probing a space
    bool valid_pos(int xp, int yp);  //determine whether a walker is allowed to move to this position
    bool occupied(int xp, int yp);  //determine if this position is occupied
    bool check_out_of_bounds(int xp, int yp) const;  //check if position is out of sub-grid
    void check_out_of_bounds_error(int xp, int yp) const;  //above, but throw an error

    //outputing and sharing positions 
    void display(int world_size);  //display the positions of all walkers
    void collect_at_main(int* wi, int* wx, int* wy, int world_size, int num_walkers);
    void share(int to_rank, int xpos, int ypos);  //share the information at position x,y with rank to_rank
    void receive(int from_rank);  //receive infromation from an adjacent rank
};

//Initialize sub_grid for a given worl_rank
void initialize_grid(int Lx, int Ly, int world_size, int world_rank, sub_grid & new_grid);

#endif
