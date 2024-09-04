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

std::vector<std::vector<Patch>> patches(ROWS, std::vector<Patch>(COLS));
int ticks = 0;
std::string modelVersion = SHEEP_WOLVES_GRASS;


void displayLabels() {
    if (showEnergy) {
        std::cout << "\nDisplay Wolf:\n";
        for (const auto &wolf : Wolf::wolfPack) {
            std::cout << "Wolf at (" << wolf.x << ", " << wolf.y << ") with energy: " << wolf.energy << "\n";
        }
        std::cout << "Display Sheep:\n";
        for (const auto &sheep : Sheep::sheepFlock) {
            if (modelVersion == SHEEP_WOLVES_GRASS) {
                std::cout << "Sheep at (" << sheep.x << ", " << sheep.y << ") with energy: " << sheep.energy << "\n";
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

    // create the sheep
    for (int i = 0; i < initialNumberSheep; ++i) {
        int initialEnergy = rand_int(0, 2 * sheepGainFromFood - 1);
        Sheep::sheepFlock.push_back(Sheep(initialEnergy));
    }

    // create the wolves
    for (int i = 0; i < initialNumberWolves; ++i) { // pre-increment for performance and best practice
        int initialEnergy = rand_int(0, 2 * wolfGainFromFood - 1);
        Wolf::wolfPack.push_back(Wolf(initialEnergy));
    }

    displayLabels();
    ticks = 0;
}

void go () {
    ++ticks;

    // Simulate the sheep
    std::vector<Sheep> newSheepFlock;
    for (Sheep &sheep : Sheep::sheepFlock) {
        sheep.move();
        if (modelVersion == SHEEP_WOLVES_GRASS) {
            sheep.energy -= 1; 

            // std::cerr << "Sheep (wants to eat grass patch) at : " << it->x << ", " << it->y << "\n";
            Patch &currentPatch = patches[sheep.x][sheep.y];
            sheep.eatGrass(currentPatch); 

            if (sheep.energy >= 0) {
                newSheepFlock.push_back(sheep);
                if (rand_double() < sheepReproduce) {
                    newSheepFlock.push_back(sheep.reproduceSheep());
                }
            } 
        } else {
            if (rand_double() < sheepReproduce) {
                newSheepFlock.push_back(sheep.reproduceSheep());
            }
        }
    }
    Sheep::sheepFlock = newSheepFlock;
    
    // Simulate the wolves
    std::vector<Wolf> newWolfPack;
    for (Wolf &wolf: Wolf::wolfPack) {
        wolf.move();
        wolf.energy -= 1; 

        // Wolves eat sheep
        newSheepFlock.clear();
        for (Sheep &sheep : Sheep::sheepFlock) {
            if (sheep.x == wolf.x && sheep.y == wolf.y) {
                wolf.eatSheep(sheep);
                // sheepIt = Sheep::sheepFlock.erase(sheepIt); 
            } else {
                newSheepFlock.push_back(sheep);
            }
        }
        Sheep::sheepFlock = newSheepFlock;

        // Wolves death condition
        if (wolf.energy >= 0) {
            newWolfPack.push_back(wolf);
            if (rand_double() < wolfReproduce) {
                newWolfPack.push_back(wolf.reproduceWolf());
            }
        } 
    }
    Wolf::wolfPack = newWolfPack;

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
    // std::chrono::steady_clock::time_point startTime = std::chrono::steady_clock::now();

    setup();

    int counter = 0;
    while (true) {
      std::cout << "Checking conditions...\n";

      if (Wolf::wolfPack.empty() && (int) Sheep::sheepFlock.size() > maxSheep) {
        std::cout << "The sheep have inherited the earth" << "\n";
        break;
      }
      if (Sheep::sheepFlock.empty() && Wolf::wolfPack.empty()) {
            std::cout << "All sheep and wolves are gone!" << "\n";
            break;
      }

      go();
      counter++;

      if (counter >= 150) {
        break;
      }

    // sleep between ticks to simulate
    //   std::this_thread::sleep_for(std::chrono::milliseconds(250));
    }

    // std::chrono::steady_clock::time_point stopTime = std::chrono::steady_clock::now();
    // std::chrono::duration<double> time_span = (std::chrono::duration_cast<std::chrono::duration<double>>(stopTime - startTime));
    // double dt = time_span.count();
    // std::cout << "Time to run main (timespan dt): " << dt << "\n"; 


    std::cout << "sheepFlock size: " << Sheep::sheepFlock.size() << "\n";
    std::cout << "wolfPack size: " << Wolf::wolfPack.size() << "\n";

/*     if (Sheep::sheepFlock.empty() && Wolf::wolfPack.empty()) {
      std::cout << "Both Sheep and Wolves are dead" << "\n";
    }
 */
    return 0;
}
