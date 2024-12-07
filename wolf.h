#ifndef WOLF_H
#define WOLF_H

#include "animal.h"
#include "declare_env.h"
#include "sheep.h"
#include <vector>

class Wolf : public Animal {
    public:
        static std::vector<Wolf> wolfPack;

        Wolf();
        Wolf(int energy, int rows);
        Wolf(int energy , int x, int y);

        void eatSheep(Sheep &sheep);

        Wolf reproduceWolf(int my_rank, int world_size, int rows_per_rank);
};

#endif