#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include "patch.h"
#include "animal.h"
#include "sheep.h"
#include "wolf.h"
#include <thread>
#include <chrono>
#include "declare_env.h"
#include "util.h"
#include <cmath>

const int maxSheep = 100;
const int maxWolf = 100;
Patch *patches;
Sheep sheepFlock[maxSheep];
Sheep nextSheepFlock[maxSheep];
Wolf wolfPack[maxSheep];
Wolf nextWolfPack[maxSheep];
int sheepCount, wolfCount;
int ticks = 0;
std::string modelVersion = SHEEP_WOLVES_GRASS;


void displayLabels() {
    if (showEnergy) {
        std::cout << "\nDisplay Wolf:\n";
        for (int i = 0; i < wolfCount; ++i) {
            std::cout << "Wolf at (" << wolfPack[i].x << ", " << wolfPack[i].y << ") with energy: " << wolfPack[i].energy << "\n";
        }
        std::cout << "Display Sheep:\n";
        for (int i = 0; i < sheepCount; ++i) {
            if (modelVersion == SHEEP_WOLVES_GRASS) {
                std::cout << "Sheep at (" << sheepFlock[i].x << ", " << sheepFlock[i].y << ") with energy: " << sheepFlock[i].energy << "\n";
            }
        }
    } else {
        std::cout << "Not showing Energy.\n";
    }
    std::cout << "\n"; 
}

int grass() {
    int greenCount = 0;

    // Only count green patches if we're using the sheep-wolves-grass model
    if (modelVersion == SHEEP_WOLVES_GRASS) {
        for (int i = 0; i < ROWS; ++i) {
            for (int j = 0; j < COLS; ++j) {
                if (patches[i * COLS + j].getColor() == Patch::Color::Green) {
                    ++greenCount;
                }
            }
        }
    }
    return greenCount;
}

void setup () {
    std::cerr << "setup\n";
    // Memory allocation    
    patches = new Patch[ROWS * COLS];
    // for (int i = 0; i < ROWS; ++i) {
    //     patches[i] = new Patch[COLS];
    // }

    sheepCount = initialNumberSheep;
    wolfCount = initialNumberWolves;
    // sheepFlock = new Sheep[maxSheep];
    // nextSheepFlock = new Sheep[maxSheep];
    // wolfPack = new Wolf[maxWolf];
    // nextWolfPack = new Wolf[maxWolf];

    // setup grass patches if grass needs to regrow and be consumed by sheep
    for (int i = 0; i < ROWS; ++i) {
        for (int j = 0; j < COLS; ++j) {
            Patch &patch = patches[i * COLS + j];
            if (modelVersion == SHEEP_WOLVES_GRASS) {
                if (rand_int(0, 1) == 0) {
                    patch.setColor(Patch::Color::Green);
                    patch.setCountdown(grassRegrowthTime);
                } else {
                    patch.setColor(Patch::Color::Brown);
                    patch.setCountdown(rand_int(0, grassRegrowthTime-1));
                }
            } else {
                patch.setColor(Patch::Color::Green);
            }
        }
    }

    // initialize sheepFlock
    for (int i = 0; i < sheepCount; ++i) {
        sheepFlock[i] = Sheep();
    }

    // initialize the wolfPack
    for (int i = 0; i < wolfCount; ++i) { // pre-increment for performance and best practice
        wolfPack[i] = Wolf();
    }

    displayLabels();
    ticks = 0;
}

void go () {
    ++ticks;

    // Simulate the sheep
    // Sheep *newSheepFlock = new Sheep[static_cast<int>(std::ceil(sheepCount * 1.25))];
    int nextSheepCount = 0;
    for (int i = 0; i < sheepCount; ++i) {
        Sheep &sheep = sheepFlock[i];
        sheep.move();
        if (modelVersion == SHEEP_WOLVES_GRASS) {
            sheep.energy -= 1; 

            // std::cerr << "Sheep (wants to eat grass patch) at : " << it->x << ", " << it->y << "\n";
            Patch &currentPatch = patches[sheep.x * COLS + sheep.y];
            sheep.eatGrass(currentPatch); 

            if (sheep.energy >= 0) {
                nextSheepFlock[nextSheepCount++] = sheep;
                if (rand_double() < sheepReproduce) {
                    nextSheepFlock[nextSheepCount++] = sheep.reproduceSheep();
                }
            } 
        } else {
            if (rand_double() < sheepReproduce) {
                nextSheepFlock[nextSheepCount++] = sheep.reproduceSheep();
            }
        }
    }
    // delete[] sheepFlock;
    // sheepFlock = nextSheepFlock;
    std::swap(sheepFlock, nextSheepFlock);
    sheepCount = nextSheepCount;
    
    // Simulate the wolves
    // Wolf *newWolfPack = new Wolf[static_cast<int>(std::ceil(wolfCount * 1.25))];
    int nextWolfCount = 0;
    for (int i = 0; i < wolfCount; ++i) {
        Wolf &wolf = wolfPack[i];
        wolf.move();
        wolf.energy -= 1; 

        // Wolves eat sheep
        nextSheepCount = 0;
        for (int j = 0; j < sheepCount; ++j) {
            Sheep &sheep = sheepFlock[j];
            if (sheep.x == wolf.x && sheep.y == wolf.y) {
                wolf.eatSheep(sheep);
                // sheepIt = sheepFlock.erase(sheepIt); 
            } else {
                nextSheepFlock[nextSheepCount++] = sheep;
            }
        }
        // delete[] sheepFlock;
        std::swap(sheepFlock, nextSheepFlock);
        sheepCount = nextSheepCount;

        // Wolves death condition
        if (wolf.energy >= 0) {
            nextWolfPack[nextWolfCount++] = wolf;
            if (rand_double() < wolfReproduce) {
                nextWolfPack[nextWolfCount++] = wolf.reproduceWolf();
            }
        } 
    }
    // delete[] wolfPack;
    std::swap(wolfPack, nextWolfPack);
    wolfCount = nextWolfCount;

    // Grow the grass
    if (modelVersion == SHEEP_WOLVES_GRASS) {
        for (int i = 0; i < ROWS; ++i) {
            for (int j = 0; j < COLS; ++j) {
                // std::cout << "i, j: " << i << ", " << j << "\n";
                patches[i * COLS + j].growGrass(grassRegrowthTime);
            }
        }
    }
    displayLabels();
    std::cout << "Green Patch Count: " << grass() << "\n";
    std::cout << "Ticks: " << ticks << "\n";
}

int main() {
    std::cout << "Hello\n";
}

int main_2 () {
    std::cerr << "Main \n";

    std::chrono::steady_clock::time_point startTime = std::chrono::steady_clock::now();

    setup();

    int counter = 0;
    while (true) {
        std::cerr << "Counter: " << counter << "\n";
        std::cout << "Checking conditions...\n";

        if (wolfCount == 0 && sheepCount > maxSheep) {
        std::cout << "The sheep have inherited the earth" << "\n";
        break;
        }
        if (sheepCount == 0 && wolfCount == 0) {
            std::cout << "All sheep and wolves are gone!" << "\n";
            break;
        }

        go();
        counter++;

        if (counter >= 500) {
        break;
        }

    // sleep between ticks to simulate
    //   std::this_thread::sleep_for(std::chrono::milliseconds(250));
    }

    std::chrono::steady_clock::time_point stopTime = std::chrono::steady_clock::now();
    std::chrono::duration<double> time_span = (std::chrono::duration_cast<std::chrono::duration<double>>(stopTime - startTime));
    double dt = time_span.count();
    std::cout << "Time to run main (timespan dt): " << dt << "\n"; 


    std::cout << "sheepFlock size: " << sheepCount << "\n";
    std::cout << "wolfPack size: " << wolfCount << "\n";

    // Cleanup
    delete[] patches;
    // delete[] sheepFlock;
    // delete[] wolfPack;

/*     if (sheepFlock.empty() && wolfPack.empty()) {
      std::cout << "Both Sheep and Wolves are dead" << "\n";
    }
 */
    return 0;
}
