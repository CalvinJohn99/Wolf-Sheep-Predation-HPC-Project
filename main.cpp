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
                if (std::rand() % 2 == 0) {
                    patches[i][j].setColor(Patch::Color::Green);
                    patches[i][j].setCountdown(grassRegrowthTime);
                } else {
                    patches[i][j].setColor(Patch::Color::Brown);
                    patches[i][j].setCountdown(std::rand() % grassRegrowthTime);
                }
            } else {
                patches[i][j].setColor(Patch::Color::Green);
            }
        }
    }

    // create the sheep
    for (int i = 0; i < initialNumberSheep; ++i) {
        int initialEnergy = std::rand() % (2 * sheepGainFromFood);
        Sheep::sheepFlock.push_back(Sheep(initialEnergy));
    }

    // create the wolves
    for (int i = 0; i < initialNumberWolves; ++i) { // pre-increment for performance and best practice
        int initialEnergy = std::rand() % (2 * wolfGainFromFood);
        Wolf::wolfPack.push_back(Wolf(initialEnergy));
    }

    displayLabels();
    ticks = 0;
}

void go () {

    ++ticks;

    // Using iterator loops, since we need to remove sheep/wolves when they die
    // Simulate the sheep
    for (auto it = Sheep::sheepFlock.begin(); it != Sheep::sheepFlock.end();) { // "it" is of type std::vector<Sheep>::iterator
        it->move(); // it behaves like pointer, needs dereferenced usage of methods

        if (modelVersion == SHEEP_WOLVES_GRASS) {
            it->energy -= 1; 

            std::cerr << "Sheep (wants to eat grass patch) at : " << it->x << ", " << it->y << "\n";
            // std::cout << "Move placement: x = " << it->x << ", y = " << it->y << std::endl;
            Patch &currentPatch = patches[it->x][it->y];
            it->eatGrass(currentPatch); 

            if (it->energy < 0) {
                it = Sheep::sheepFlock.erase(it); // Sheep dies, next sheep passed to "it"
                continue;
            }
        }
        it->reproduceSheep();
        ++it; // increment only happens is no sheep was removed
    }

    // Simulate the wolves
    for (auto it = Wolf::wolfPack.begin(); it != Wolf::wolfPack.end();) {
        it->move();
        it->energy -= 1; 

        // Wolves eat sheep
        for (auto sheepIt = Sheep::sheepFlock.begin(); sheepIt != Sheep::sheepFlock.end();) {
            if (sheepIt->x == it->x && sheepIt->y == it->y) {
                it->eatSheep(*sheepIt);
                sheepIt = Sheep::sheepFlock.erase(sheepIt); // Sheep dies
            } else {
                ++sheepIt;
            }
        }

        // Wolves death condition
        if (it->energy < 0) {
            it = Wolf::wolfPack.erase(it);
        } else {
            it->reproduceWolf();
            ++it;
        }
    }

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
    setup();

    while (true) {
      std::cout << "Checking conditions...\n";

      if (Wolf::wolfPack.empty() && Sheep::sheepFlock.size() > maxSheep) {
        std::cout << "The sheep have inherited the earth" << "\n";
        break;
      }
      if (Sheep::sheepFlock.empty() && Wolf::wolfPack.empty()) {
            std::cout << "All sheep and wolves are gone!" << "\n";
            break;
      }

      go();

      // sleep between ticks
      std::this_thread::sleep_for(std::chrono::milliseconds(250));
    }

    std::cout << "sheepFlock size: " << Sheep::sheepFlock.size() << "\n";
    std::cout << "wolfPack size: " << Wolf::wolfPack.size() << "\n";

/*     if (Sheep::sheepFlock.empty() && Wolf::wolfPack.empty()) {
      std::cout << "Both Sheep and Wolves are dead" << "\n";
    }
 */
    return 0;
}
