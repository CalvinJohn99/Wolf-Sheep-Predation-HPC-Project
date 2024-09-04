#ifndef ANIMAL_H
#define ANIMAL_H

#include <vector>

class Animal {
    public:
        int x, y;
        int energy;

        Animal(int energy);
        Animal(int energy, int x, int y);

        void move();
};

#endif