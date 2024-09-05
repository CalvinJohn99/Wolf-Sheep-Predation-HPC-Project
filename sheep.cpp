#include "sheep.h"
#include "patch.h"
#include "declare_env.h"
#include "animal.h"
#include <cstdlib>
#include <vector>
#include <iostream>
#include <cstdlib>
#include "util.h"

Sheep::Sheep() : Animal() {
    this->energy = rand_int(0, 2 * sheepGainFromFood - 1);
    this->x = rand_int(0, ROWS-1);
    this->y = rand_int(0, COLS-1);
}
Sheep::Sheep(int energy) : Animal(energy) {}
Sheep::Sheep(int energy, int x, int y) : Animal(energy, x, y) {}

void Sheep::eatGrass(Patch &patch) {
    // if (&patch == nullptr) {
    //     std::cerr << "Error: Patch reference is null." << std::endl;
    //     return;
    // }

    // std::cout << "Sheep at (" << this->x << ", " << this->y << ") attempting to eat grass." << std::endl;
    // std::cout << "Patch color: " << (patch.pcolor == Patch::Color::Green ? "Green" : "Brown") << std::endl;


    if (patch.pcolor == Patch::Color::Green) {
        patch.setColor(Patch::Color::Brown);
        this->energy += sheepGainFromFood; 
    }
}

Sheep Sheep::reproduceSheep() {
    int offspringEnergy = this->energy / 2;
    int offspringX = this->x;
    int offspringY = this->y;
    this->energy /= 2; 

    Sheep offspring(offspringEnergy, offspringX, offspringY);
    offspring.move();
    return offspring;  

}


