#ifndef logger_GUARD
#define logger_GUARD

#include "grid.h"

class logger {
public:
    bool has_moved;
    int prev_x, prev_y;
    walker* wp;
    sub_grid* gridp;

public:
    logger(walker& w, sub_grid& grid);
    void log();   //to be called after an update cycle
    void print();
};



#endif
