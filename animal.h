#ifndef ANIMAL_H
#define ANIMAL_H

#include <vector>

class Animal {
    public:
        int energy;
        int x, y;
        
        Animal(int energy_, int rows);
        Animal(int energy_, int x_, int y_);

        void move();
};

#endif