#include <iostream>
#include "logger.h"
#include "grid.h"

using namespace std;

logger::logger(walker& w): wp(&w), prev_x(w.x), prev_y(w.y), has_moved(true) {};

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
        cout << "Walker " << wp->index << " has not moved, at postiion " << "(" << wp->x  << ","
            << wp->y << ")" <<endl;
    }
}
