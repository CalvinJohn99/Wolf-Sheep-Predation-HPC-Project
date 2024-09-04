#include "animal.h"
#include "declare_env.h"
#include <cstdlib>
#include <vector>
#include <iostream>

Animal::Animal(int energy) : energy(energy) {
    this->x = std::rand() % ROWS;
    this->y = std::rand() % COLS;
    // std::cout << "Random placement: x = " << this->x << ", y = " << this->y << std::endl;
}

Animal::Animal(int energy, int x, int y) : energy(energy), x(x), y(y) {}

void Animal::move() {
    this->x = (x + (std::rand() % 3 - 1) + ROWS) % ROWS;
    this->y = (y + (std::rand() % 3 - 1) + COLS) % COLS;
}
