#include "sheep.h"
#include "patch.h"
#include "declare_env.h"
#include "animal.h"
#include <cstdlib>
#include <vector>
#include <iostream>
#include <cstdlib>
#include "util.h"

std::vector<Sheep> Sheep::sheepFlock;

Sheep::Sheep() : Animal() {}
Sheep::Sheep(int energy, int rows) : Animal(energy, rows) {}
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


