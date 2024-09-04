#include "patch.h"
#include "declare_env.h"
#include <vector>

Patch::Patch() : pcolor(Color::Green), countdown(0) {}

void Patch::setColor(Color color) {
    this->pcolor = color;
}

void Patch::setCountdown(int time) {
    this->countdown = time;
}

void Patch::growGrass(int grassRegrowthTime) {
    if (pcolor == Color::Brown) {
        if (countdown <= 0) {
            this->pcolor = Color::Green;
            this->countdown = grassRegrowthTime;
        } else {
            this->countdown--;
        }
    }
}

Patch::Color Patch::getColor() const {
    return pcolor;
}
