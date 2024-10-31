#include <mpi.h>
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

int ticks = 0;
std::string modelVersion = SHEEP_WOLVES_GRASS;

MPI_Datatype createSheepMPIType() {
    const int nitems = 3;
    
    int blocklengths[nitems] = {1, 1, 1}; 
    MPI_Datatype types[nitems] = {MPI_INT, MPI_INT, MPI_INT};  
    
    MPI_Aint offsets[nitems];
    offsets[0] = offsetof(Sheep, energy);
    offsets[1] = offsetof(Sheep, x);
    offsets[2] = offsetof(Sheep, y);

    MPI_Datatype mpi_sheep_type;
    MPI_Type_create_struct(nitems, blocklengths, offsets, types, &mpi_sheep_type);
    MPI_Type_commit(&mpi_sheep_type);

    return mpi_sheep_type;
}

MPI_Datatype createWolfMPIType() {
    const int nitems = 3;
    
    int blocklengths[nitems] = {1, 1, 1}; 
    MPI_Datatype types[nitems] = {MPI_INT, MPI_INT, MPI_INT};  
    
    MPI_Aint offsets[nitems];
    offsets[0] = offsetof(Wolf, energy);
    offsets[1] = offsetof(Wolf, x);
    offsets[2] = offsetof(Wolf, y);

    MPI_Datatype mpi_wolf_type;
    MPI_Type_create_struct(nitems, blocklengths, offsets, types, &mpi_wolf_type);
    MPI_Type_commit(&mpi_wolf_type);

    return mpi_wolf_type;
}

void displayLabels(int my_rank) {
    if (showEnergy) {
        std::cout << "\nDisplay Wolf from process " << my_rank << ":\n";
        for (const auto &wolf : Wolf::wolfPack) {
            std::cout << "Wolf (process: " << my_rank << ") at (" << wolf.x << ", " << wolf.y << ") with energy: " << wolf.energy << "\n";
        }
        std::cout << "Display Sheep from process " << my_rank << ":\n";
        for (const auto &sheep : Sheep::sheepFlock) {
            if (modelVersion == SHEEP_WOLVES_GRASS) {
                std::cout << "Sheep (process: " << my_rank << ") at (" << sheep.x << ", " << sheep.y << ") with energy: " << sheep.energy << "\n";
            }
        }
    } else {
        std::cout << "Not showing Energy.\n";
    }
    std::cout << "\n"; 
}

int grass(const std::vector<std::vector<Patch>>& local_patches, int rows_per_rank) {
    int greenCount = 0;

    // Only count green patches if we're using the sheep-wolves-grass model
    if (modelVersion == SHEEP_WOLVES_GRASS) {
        for (int i = 0; i < rows_per_rank; ++i) {
            for (int j = 0; j < COLS; ++j) {
                if (local_patches[i][j].getColor() == Patch::Color::Green) {
                    ++greenCount;
                }
            }
        }
    }
    return greenCount;
}

void setup (std::vector<std::vector<Patch>>& local_patches, int rows_per_rank, int world_size) {

    // setup grass patches if grass needs to regrow and be consumed by sheep
    for (int i = 0; i < rows_per_rank; ++i) {
        for (int j = 0; j < COLS; ++j) {
            if (modelVersion == SHEEP_WOLVES_GRASS) {
                if (rand_int(0, 1) == 0) {
                    local_patches[i][j].setColor(Patch::Color::Green);
                    local_patches[i][j].setCountdown(grassRegrowthTime);
                } else {
                    local_patches[i][j].setColor(Patch::Color::Brown);
                    local_patches[i][j].setCountdown(rand_int(0, grassRegrowthTime-1));
                }
            } else {
                local_patches[i][j].setColor(Patch::Color::Green);
            }
        }
    }

    // create the sheep
    for (int i = 0; i < (initialNumberSheep/world_size); ++i) {
        int initialEnergy = rand_int(0, 2 * sheepGainFromFood - 1);
        Sheep::sheepFlock.push_back(Sheep(initialEnergy, rows_per_rank));
    }

    // create the wolves
    for (int i = 0; i < (initialNumberWolves/world_size); ++i) { // pre-increment for performance and best practice
        int initialEnergy = rand_int(0, 2 * wolfGainFromFood - 1);
        Wolf::wolfPack.push_back(Wolf(initialEnergy, rows_per_rank));
    }

    //displayLabels();
    ticks = 0;
}

/* void go () {
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
            newSheepFlock.push_back(sheep);
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
    // displayLabels();
    // std::cout << "Green Patch Count: " << grass() << "\n";
    std::cout << "Ticks: " << ticks << "\n";
} */

int main (int argc, char** argv) {
    MPI_Init(&argc, &argv);
    
    std::chrono::steady_clock::time_point startTime = std::chrono::steady_clock::now();
    MPI_Datatype MPI_Sheep = createSheepMPIType();
    MPI_Datatype MPI_Wolf = createWolfMPIType();

    int my_rank, world_size;
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    int rows_per_rank = ROWS / world_size;
    int my_start_row = my_rank * rows_per_rank;
    int my_end_row = (my_rank == world_size - 1) ? ROWS : (my_rank + 1) * rows_per_rank;


    std::vector<std::vector<Patch>> local_patches(rows_per_rank, std::vector<Patch>(COLS));

    setup(local_patches, rows_per_rank, world_size);
    int local_sheep_count = Sheep::sheepFlock.size();
    int local_wolf_count = Wolf::wolfPack.size();
    int total_sheep_count = 0;
    int total_wolf_count = 0;
    MPI_Allreduce(&local_sheep_count, &total_sheep_count, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
    MPI_Allreduce(&local_wolf_count, &total_wolf_count, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);

    std::cout << "Hello from process: " << my_rank << " out of " << world_size << " processes\n";
    std::cout << "process: " << my_rank << " has " << local_sheep_count << " sheep out of a total " << total_sheep_count << " sheep." << "\n";
    std::cout << "process: " << my_rank << " has " << local_wolf_count << " wolves out of a total " << total_wolf_count << " wolf count." << "\n";

    displayLabels(my_rank);


    int counter = 0;
/*     while (true) {
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

        if (counter >= 2) { //2500 was benchmark for non-MPI
            break;
        }
    } */

    std::chrono::steady_clock::time_point stopTime = std::chrono::steady_clock::now();
    std::chrono::duration<double> time_span = (std::chrono::duration_cast<std::chrono::duration<double>>(stopTime - startTime));
    double dt = time_span.count();

    MPI_Barrier(MPI_COMM_WORLD);
    if (my_rank == 0) {
        std::cout << "Time to run main (timespan dt): " << dt << "\n"; 
        std::cout << "sheepFlock size: " << total_sheep_count << "\n";
        std::cout << "wolfPack size: " << total_wolf_count << "\n";
    }

    MPI_Type_free(&MPI_Sheep);
    MPI_Type_free(&MPI_Wolf);
    MPI_Finalize();
    
    return 0;
}
