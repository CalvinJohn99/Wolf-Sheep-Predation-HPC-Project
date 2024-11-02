#include "animal.h"
#include "declare_env.h"
#include <cstdlib>
#include <vector>
#include <iostream>
#include "util.h"

Animal::Animal() {
    this->energy = 0;
    this->x = 0;
    this->y = 0;
}

Animal::Animal(int energy_, int rows) : energy(energy_) {
    this->x = rand_int(0, rows-1);
    this->y = rand_int(0, COLS-1);
    // std::cout << "Random placement: x = " << this->x << ", y = " << this->y << std::endl;
}

Animal::Animal(int energy_, int x_, int y_) : energy(energy_), x(x_), y(y_) {}

void Animal::move() {
    // this->x = (x + (rand_int(-1, 1)));
    this->x = (x + 1);
    // this->y = (y + (rand_int(-1, 1)) + COLS) % COLS;
}
