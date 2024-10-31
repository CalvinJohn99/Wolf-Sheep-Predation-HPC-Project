#ifndef SHEEP_H
#define SHEEP_H

#include "animal.h"
#include "patch.h"
#include <vector>

class Sheep : public Animal {
    public:
        static std::vector<Sheep> sheepFlock;

        Sheep(int energy, int rows);
        Sheep(int energy, int x, int y);

        void eatGrass(Patch &patch);
        Sheep reproduceSheep();
};


#endif