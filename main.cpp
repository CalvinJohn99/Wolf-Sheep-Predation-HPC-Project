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

Patch **patches;
Sheep *sheepFlock;
Wolf *wolfPack;
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
                if (patches[i][j].getColor() == Patch::Color::Green) {
                    ++greenCount;
                }
            }
        }
    }
    return greenCount;
}

void setup () {
    // Memory allocation    
    patches = new Patch*[ROWS];
    for (int i = 0; i < ROWS; ++i) {
        patches[i] = new Patch[COLS];
    }

    sheepCount = initialNumberSheep;
    wolfCount = initialNumberWolves;
    sheepFlock = new Sheep[sheepCount];
    wolfPack = new Wolf[wolfCount];

    // setup grass patches if grass needs to regrow and be consumed by sheep
    for (int i = 0; i < ROWS; ++i) {
        for (int j = 0; j < COLS; ++j) {
            if (modelVersion == SHEEP_WOLVES_GRASS) {
                if (rand_int(0, 1) == 0) {
                    patches[i][j].setColor(Patch::Color::Green);
                    patches[i][j].setCountdown(grassRegrowthTime);
                } else {
                    patches[i][j].setColor(Patch::Color::Brown);
                    patches[i][j].setCountdown(rand_int(0, grassRegrowthTime-1));
                }
            } else {
                patches[i][j].setColor(Patch::Color::Green);
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
    Sheep *newSheepFlock = new Sheep[sheepCount];
    int newSheepCount = 0;

    for (int i = 0; i < sheepCount; ++i) {
        Sheep &sheep = sheepFlock[i];
        sheep.move();
        if (modelVersion == SHEEP_WOLVES_GRASS) {
            sheep.energy -= 1; 

            // std::cerr << "Sheep (wants to eat grass patch) at : " << it->x << ", " << it->y << "\n";
            Patch &currentPatch = patches[sheep.x][sheep.y];
            sheep.eatGrass(currentPatch); 

            if (sheep.energy >= 0) {
                newSheepFlock[newSheepCount++] = sheep;
                if (rand_double() < sheepReproduce) {
                    newSheepFlock[newSheepCount++] = sheep.reproduceSheep();
                }
            } 
        } else {
            if (rand_double() < sheepReproduce) {
                newSheepFlock[newSheepCount++] = sheep.reproduceSheep();
            }
        }
    }
    delete[] sheepFlock;
    sheepFlock = newSheepFlock;
    sheepCount = newSheepCount;
    
    // Simulate the wolves
    Wolf *newWolfPack = new Wolf[wolfCount];
    int newWolfCount = 0;
    for (int i = 0; i < wolfCount; ++i) {
        Wolf &wolf = wolfPack[i];
        wolf.move();
        wolf.energy -= 1; 

        // Wolves eat sheep
        Sheep* updatedSheepFlock = new Sheep[sheepCount];
        int updatedSheepCount = 0;
        for (int j = 0; j < sheepCount; ++j) {
            Sheep &sheep = sheepFlock[i];
            if (sheep.x == wolf.x && sheep.y == wolf.y) {
                wolf.eatSheep(sheep);
                // sheepIt = sheepFlock.erase(sheepIt); 
            } else {
                updatedSheepFlock[updatedSheepCount++] = sheep;
            }
        }
        delete[] sheepFlock;
        sheepFlock = updatedSheepFlock;
        sheepCount = updatedSheepCount;

        // Wolves death condition
        if (wolf.energy >= 0) {
            newWolfPack[newWolfCount++] = wolf;
            if (rand_double() < wolfReproduce) {
                newWolfPack[newWolfCount++] = wolf.reproduceWolf();
            }
        } 
    }
    delete[] wolfPack;
    wolfPack = newWolfPack;
    wolfCount = newWolfCount;

    // Grow the grass
    if (modelVersion == SHEEP_WOLVES_GRASS) {
        for (int i = 0; i < ROWS; ++i) {
            for (int j = 0; j < COLS; ++j) {
                // std::cout << "i, j: " << i << ", " << j << "\n";
                patches[i][j].growGrass(grassRegrowthTime);
            }
        }
    }
    displayLabels();
    std::cout << "Green Patch Count: " << grass() << "\n";
    std::cout << "Ticks: " << ticks << "\n";
}

int main () {
    std::chrono::steady_clock::time_point startTime = std::chrono::steady_clock::now();

    setup();

    int counter = 0;
    while (true) {
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

/*     if (sheepFlock.empty() && wolfPack.empty()) {
      std::cout << "Both Sheep and Wolves are dead" << "\n";
    }
 */
    return 0;
}
