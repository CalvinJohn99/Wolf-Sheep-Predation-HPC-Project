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

/* void go (std::vector<std::vector<Patch>>& local_patches, int my_rank, int world_size, int rows_per_rank, MPI_Datatype MPI_Sheep, MPI_Datatype MPI_Wolf) {
    ++ticks;

    // Simulate the sheep
    std::vector<Sheep> newSheepFlock;
    std::vector<Sheep> sendSheepUp;
    std::vector<Sheep> sendSheepDown;
    for (Sheep &sheep : Sheep::sheepFlock) {
        sheep.move();
        // Conditionals for sending sheep between processes
        if (my_rank > 0 && sheep.x < 0) {
            sheep.x = rows_per_rank - 1;
            if (modelVersion == SHEEP_WOLVES_GRASS) {
                sheep.energy -= 1;
            }
            sendSheepUp.push_back(sheep);
        } else if (my_rank < (world_size - 1) && sheep.x == rows_per_rank) {
            sheep.x = 0;
            if (modelVersion == SHEEP_WOLVES_GRASS) {
                sheep.energy -= 1;
            }
            sendSheepDown.push_back(sheep);

        // handling sheep behavior when it doesn't need to be sent to another process
        } else {
            if (modelVersion == SHEEP_WOLVES_GRASS) {
                sheep.energy -= 1; 
                Patch &currentPatch = local_patches[sheep.x][sheep.y];
                sheep.eatGrass(currentPatch); 
                if (sheep.energy >= 0) {
                    newSheepFlock.push_back(sheep);
                    // handling reproduction
                    if (rand_double() < sheepReproduce) {
                        Sheep &sheepOffspring = sheep.reproduceSheep();
                        if (my_rank > 0 && sheepOffspring.x < 0) {
                            sheepOffspring.x = rows_per_rank - 1;
                            sendSheepUp.push_back(sheepOffspring);
                        } else if (my_rank < (world_size -1) && sheepOffspring.x == rows_per_rank) {
                            sheepOffspring.x = 0;
                            sendSheepDown.push_back(sheepOffspring);
                        } else {
                            newSheepFlock.push_back(sheepOffspring);
                        }
                    }
                } 
            } else {
                newSheepFlock.push_back(sheep);
                // handling sheep reproduction
                if (rand_double() < sheepReproduce) {
                    Sheep &sheepOffspring = sheep.reproduceSheep();
                    if (my_rank > 0 && sheepOffspring.x < 0) {
                        sheepOffspring.x = rows_per_rank;
                        sendSheepUp.push_back(sheepOffspring);
                    } else if (my_rank < (world_size -1) && sheepOffspring.x == rows_per_rank) {
                        sheepOffspring.x = 0;
                        sendSheepDown.push_back(sheepOffspring);
                    } else {
                        newSheepFlock.push_back(sheepOffspring);
                    }
                }
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

    MPI_Status status;
    int my_rank, world_size;
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    int rows_per_rank = ROWS / world_size;
    int my_start_row = my_rank * rows_per_rank;
    int my_end_row = (my_rank == world_size - 1) ? ROWS : (my_rank + 1) * rows_per_rank;


    std::vector<std::vector<Patch>> local_patches(rows_per_rank, std::vector<Patch>(COLS));

    setup(local_patches, rows_per_rank, world_size);

    // std::cout << "Hello from process: " << my_rank << " out of " << world_size << " processes\n";
    // std::cout << "process: " << my_rank << " has " << local_sheep_count << " sheep out of a total " << total_sheep_count << " sheep." << "\n";
    // ::cout << "process: " << my_rank << " has " << local_wolf_count << " wolves out of a total " << total_wolf_count << " wolf count." << "\n";
    // displayLabels(my_rank);

    if (my_rank < world_size - 1) {
        // Populate sheep to send down
        std::vector<Sheep> sendSheepDown;
        Sheep sheepToTransfer = Sheep(1000000, 49, 0);
        Sheep::sheepFlock.push_back(sheepToTransfer);
        for (Sheep &sheep : Sheep::sheepFlock) {
            if (sheep.x == 49 && sheep.energy == 1000000) {
                std::cout << "Sheep to send down: \n";
                std::cout << "Sheep (process: " << my_rank << ") at (" << sheep.x << ", " << sheep.y << ") with energy: " << sheep.energy << "\n";
                sheep.move();
                sheep.energy -= 1;
            }
        }
        std::vector<Sheep> newSheepFlock;
        for (Sheep &sheep : Sheep::sheepFlock) {
            if (sheep.x >= rows_per_rank) {
                sheep.x = 0;
                sendSheepDown.push_back(sheep);
            } else {
                newSheepFlock.push_back(sheep);
            }
        }
        Sheep::sheepFlock = newSheepFlock;
        // Send sheep down
        int sheep_count = sendSheepDown.size();
        MPI_Send(&sheep_count, 1, MPI_INT, my_rank + 1, 0, MPI_COMM_WORLD);
        MPI_Send(sendSheepDown.data(), sheep_count, MPI_Sheep, my_rank + 1, my_rank + 100, MPI_COMM_WORLD);

        // recieve sheep from below
        int numSheepRecieved;
        MPI_Recv(&numSheepRecieved, 1, MPI_INT, my_rank + 1, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        std::vector<Sheep> sheepFromBelow(numSheepRecieved);
        MPI_Recv(sheepFromBelow.data(), numSheepRecieved, MPI_Sheep, my_rank + 1, (my_rank + 1 + 100), MPI_COMM_WORLD, &status);
        for (Sheep &sheep : sheepFromBelow) {
            Patch &currentPatch = local_patches[sheep.x][sheep.y];
            sheep.eatGrass(currentPatch);
            if (sheep.energy >= 0) {
                Sheep::sheepFlock.push_back(sheep);
                // handling reproduction
                if (rand_double() < sheepReproduce) {
                    Sheep sheepOffspring = sheep.reproduceSheep();
                    if (sheepOffspring.x >= rows_per_rank) {
                        sheepOffspring.x = rows_per_rank - 1;
                    }
                    Sheep::sheepFlock.push_back(sheepOffspring);
                }
            } 
        }
        // print sheep recieved
        for (Sheep &sheep : Sheep::sheepFlock) {
            if (sheep.energy > 10000) {
                std::cout << "Sheep recieved from below: \n";
                std::cout << "Sheep (process: " << my_rank << ") at (" << sheep.x << ", " << sheep.y << ") with energy: " << sheep.energy << "\n";
            }
        }
    }

    if (my_rank > 0) {
        // Populate sheep to send up
        std::vector<Sheep> sendSheepUp;
        Sheep sheepToTransfer = Sheep(1000001, 0, 1);
        Sheep::sheepFlock.push_back(sheepToTransfer);
        for (Sheep &sheep : Sheep::sheepFlock) {
            if (sheep.x == 0 && sheep.energy == 1000001) {
                std::cout << "Sheep to send up: \n";
                std::cout << "Sheep (process: " << my_rank << ") at (" << sheep.x << ", " << sheep.y << ") with energy: " << sheep.energy << "\n";
                sheep.x -= 1;
                sheep.energy -= 1;
            }
        }
        std::vector<Sheep> newSheepFlock;
        for (Sheep &sheep : Sheep::sheepFlock) {
            if (sheep.x < 0) {
                sheep.x = 49;
                sendSheepUp.push_back(sheep);
            } else {
                newSheepFlock.push_back(sheep);
            }
        }

        Sheep::sheepFlock = newSheepFlock;
        // Send sheep up
        int sheep_count = sendSheepUp.size();
        MPI_Send(&sheep_count, 1, MPI_INT, my_rank - 1, 1, MPI_COMM_WORLD);
        MPI_Send(sendSheepUp.data(), sheep_count, MPI_Sheep, my_rank - 1, my_rank + 100, MPI_COMM_WORLD);

        // recieve sheep from above
        int numSheepRecieved;
        MPI_Recv(&numSheepRecieved, 1, MPI_INT, my_rank - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        std::vector<Sheep> sheepFromAbove(numSheepRecieved);
        MPI_Recv(sheepFromAbove.data(), numSheepRecieved, MPI_Sheep, my_rank - 1, (my_rank - 1 + 100), MPI_COMM_WORLD, &status);
        for (Sheep &sheep : sheepFromAbove) {
            Patch &currentPatch = local_patches[sheep.x][sheep.y];
            sheep.eatGrass(currentPatch);
            if (sheep.energy >= 0) {
                Sheep::sheepFlock.push_back(sheep);
                // handling reproduction
                if (rand_double() < sheepReproduce) {
                    Sheep sheepOffspring = sheep.reproduceSheep();
                    if (sheepOffspring.x < 0) {
                        sheepOffspring.x = 0;
                    }
                    Sheep::sheepFlock.push_back(sheepOffspring);
                }
            } 
        }
        // print sheep recieved
        for (Sheep &sheep : Sheep::sheepFlock) {
            if (sheep.energy > 10000) {
                std::cout << "Sheep recieved from above: \n";
                std::cout << "Sheep (process: " << my_rank << ") at (" << sheep.x << ", " << sheep.y << ") with energy: " << sheep.energy << "\n";
            }
        }
    }

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

    int local_sheep_count = Sheep::sheepFlock.size();
    int local_wolf_count = Wolf::wolfPack.size();
    int total_sheep_count = 0;
    int total_wolf_count = 0;
    MPI_Allreduce(&local_sheep_count, &total_sheep_count, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
    MPI_Allreduce(&local_wolf_count, &total_wolf_count, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);

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
