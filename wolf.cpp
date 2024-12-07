#include "wolf.h"
#include "animal.h"
#include <cstdlib>
#include <vector>
#include "util.h"

std::vector<Wolf> Wolf::wolfPack;

Wolf::Wolf() : Animal() {}
Wolf::Wolf(int energy, int rows) : Animal(energy, rows) {}
Wolf::Wolf(int energy, int x, int y) : Animal(energy, x, y) {}

void Wolf::eatSheep(Sheep &sheep) {
    this->energy += wolfGainFromFood;
    sheep.energy = -1; // sheep dies
}

Wolf Wolf::reproduceWolf(int my_rank, int world_size, int rows_per_rank) {
    int offspringEnergy = this->energy / 2;
    int offspringX = this->x;
    int offspringY = this->y;
    this->energy /= 2; 

    Wolf offspring(offspringEnergy, offspringX, offspringY);
    offspring.move(my_rank, world_size, rows_per_rank);
    return offspring;
}
