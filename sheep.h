#ifndef SHEEP_H
#define SHEEP_H

#include "animal.h"
#include "patch.h"
#include <vector>

class Sheep : public Animal {
    public:
        static std::vector<Sheep> sheepFlock;

        Sheep();
        Sheep(int energy, int rows);
        Sheep(int energy, int x, int y);

        void eatGrass(Patch &patch);
        Sheep reproduceSheep(int my_rank, int world_size, int rows_per_rank);
};


#endif