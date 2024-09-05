#include "wolf.h"
#include "animal.h"
#include <cstdlib>
#include <vector>
#include "util.h"

Wolf::Wolf() : Animal() {
    this->energy = rand_int(0, 2 * wolfGainFromFood - 1);
    this->x = rand_int(0, ROWS-1);
    this->y = rand_int(0, COLS-1);
}
Wolf::Wolf(int energy) : Animal(energy) {}
Wolf::Wolf(int energy, int x, int y) : Animal(energy, x, y) {}

void Wolf::eatSheep(Sheep &sheep) {
    this->energy += wolfGainFromFood;
    sheep.energy = -1; // sheep dies
}

Wolf Wolf::reproduceWolf() {
    int offspringEnergy = this->energy / 2;
    int offspringX = this->x;
    int offspringY = this->y;
    this->energy /= 2; 

    Wolf offspring(offspringEnergy, offspringX, offspringY);
    offspring.move();
    return offspring;
}
