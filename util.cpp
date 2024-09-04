#include "util.h"
#include <cstdlib>
#include <random>

int random(int end) {
    return std::rand() % end;
}

const int seed = 0;
std::mt19937 generator(seed); // generator


int rand_int(int start, int end) { //inclusive
    std::uniform_int_distribution<int> dist(start, end); // distribution
    return dist(generator);
}

double rand_double() {
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    return dist(generator);
}