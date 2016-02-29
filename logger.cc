#include <iostream>
#include "logger.h"
#include "grid.h"
#include <vector>

using namespace std;

logger::logger(walker& w, sub_grid& grid): wp(&w), gridp(&grid), prev_x(w.x), prev_y(w.y), has_moved(true) {};

void logger::log() {
    int x = wp->x;
    int y = wp->y;
    if (x != prev_x || y != prev_y)
        has_moved = true;
    else
        has_moved = false;
    prev_x = x;
    prev_y = y;
    print();
}

void logger::print() {
    if (!has_moved) {
        vector<bool> a;
        int x = wp->x + gridp->xc;
        int y = wp->y + gridp->yc;
        if (gridp->on_shared_border(wp->x, wp->y, a))
            return;

        cout << "Walker " << wp->index << " has not moved, at postiion " << "(" << x  << ","
            << y << ")" << " at time " << gridp->tStep << endl;
    }
}
