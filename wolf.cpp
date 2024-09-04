#include "wolf.h"
#include "animal.h"
#include <cstdlib>
#include <vector>

std::vector<Wolf> Wolf::wolfPack;

Wolf::Wolf(int energy) : Animal(energy) {}
Wolf::Wolf(int energy, int x, int y) : Animal(energy, x, y) {}

void Wolf::eatSheep(Sheep &sheep) {
    this->energy += wolfGainFromFood;
    sheep.energy = 0; // sheep dies
}

void Wolf::reproduceWolf() {
    if (static_cast<float>(std::rand()) / RAND_MAX < wolfReproduce) { // check rand float 0-1 < sheepReproduce
        int offspringEnergy = this->energy / 2;
        int offspringX = this->x;
        int offspringY = this->y;
        this->energy /= 2; 

        Wolf offspring(offspringEnergy, offspringX, offspringY);
        offspring.move();
        wolfPack.push_back(offspring);  
    }
}
