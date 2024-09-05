#include "animal.h"
#include "declare_env.h"
#include <cstdlib>
#include <vector>
#include <iostream>
#include "util.h"

Animal::Animal() {}

Animal::Animal(int energy_) : energy(energy_) {
    this->x = rand_int(0, ROWS-1);
    this->y = rand_int(0, COLS-1);
    // std::cout << "Random placement: x = " << this->x << ", y = " << this->y << std::endl;
}

Animal::Animal(int energy_, int x_, int y_) : energy(energy_), x(x_), y(y_) {}

void Animal::move() {
    this->x = (x + (rand_int(-1, 1) ) + ROWS) % ROWS;
    this->y = (y + (rand_int(-1, 1)) + COLS) % COLS;
}
