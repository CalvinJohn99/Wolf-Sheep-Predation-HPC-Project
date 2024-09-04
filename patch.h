#ifndef PATCH_H
#define PATCH_H

#include <vector>

class Patch {
    public: 
        enum class Color { Green, Brown };
        Color pcolor;
        int countdown;

        Patch();

        void setColor(Color color);
        void setCountdown(int time);
        void growGrass(int grassRegrowthTime);
        Color getColor() const;
};

#endif