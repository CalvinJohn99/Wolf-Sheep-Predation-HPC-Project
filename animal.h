#ifndef ANIMAL_H
#define ANIMAL_H

#include <vector>

class Animal {
    public:
        int energy;
        int x, y;
        
        Animal();
        Animal(int energy_, int rows);
        Animal(int energy_, int x_, int y_);

        void move(int my_rank, int world_size, int rows_per_rank);
};

#endif