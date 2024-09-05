#ifndef SHEEP_H
#define SHEEP_H

#include "animal.h"
#include "patch.h"
#include <vector>

class Sheep : public Animal {
    public:
        Sheep();
        Sheep(int energy);
        Sheep(int energy, int x, int y);

        void eatGrass(Patch &patch);
        Sheep reproduceSheep();
};


#endif