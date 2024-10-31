#include "wolf.h"
#include "animal.h"
#include <cstdlib>
#include <vector>
#include "util.h"

std::vector<Wolf> Wolf::wolfPack;

Wolf::Wolf(int energy, int rows) : Animal(energy, rows) {}
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
