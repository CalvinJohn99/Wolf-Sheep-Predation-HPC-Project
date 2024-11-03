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

void exchangeAnimals (std::vector<std::vector<Patch>>& local_patches, int my_rank, int world_size, int rows_per_rank, std::vector<Sheep>& sendSheepUp, std::vector<Sheep>& sendSheepDown, std::vector<Wolf>& sendWolfUp, std::vector<Wolf>& sendWolfDown, MPI_Status& status, MPI_Datatype MPI_Sheep, MPI_Datatype MPI_Wolf) {
    std::vector<Sheep> newSheepFlock;
    int sheepUpSize = sendSheepUp.size();
    int sheepDownSize = sendSheepDown.size();
    int wolfUpSize = sendWolfUp.size();
    int wolfDownSize = sendWolfDown.size();

    if (my_rank < (world_size - 1)) {
        // Send Sheep down
        MPI_Send(&sheepDownSize, 1, MPI_INT, my_rank + 1, 0, MPI_COMM_WORLD);
        MPI_Send(sendSheepDown.data(), sheepDownSize, MPI_Sheep, my_rank + 1, my_rank + 100, MPI_COMM_WORLD);

        // Recieve Sheep from below
        int numSheepRecieved;
        MPI_Recv(&numSheepRecieved, 1, MPI_INT, my_rank + 1, 1, MPI_COMM_WORLD, &status);
        std::vector<Sheep> sheepFromBelow(numSheepRecieved);
        MPI_Recv(sheepFromBelow.data(), numSheepRecieved, MPI_Sheep, my_rank + 1, (my_rank + 1 + 101), MPI_COMM_WORLD, &status);
        if (numSheepRecieved > 0) {
            for (Sheep &sheep : sheepFromBelow) {
                // std::cout << "**Process: " << my_rank << " recieved sheep at (" << sheep.x << "," << sheep.y << ") with energy: " << sheep.energy << " from below\n";
                Patch &currentPatch = local_patches[sheep.x][sheep.y];
                sheep.eatGrass(currentPatch);
                if (sheep.energy >= 0) {
                    Sheep::sheepFlock.push_back(sheep);
                    // handling reproduction
                    if (rand_double() < sheepReproduce) {
                        Sheep sheepOffspring = sheep.reproduceSheep(my_rank, world_size, rows_per_rank);
                        if (sheepOffspring.x >= rows_per_rank) {
                            sheepOffspring.x = rows_per_rank - 1;
                        }
                        Sheep::sheepFlock.push_back(sheepOffspring);
                    }
                } 
            }
        }

        // Send Wolves down
        MPI_Send(&wolfDownSize, 1, MPI_INT, my_rank + 1, 2, MPI_COMM_WORLD);
        MPI_Send(sendWolfDown.data(), wolfDownSize, MPI_Wolf, my_rank + 1, my_rank + 102, MPI_COMM_WORLD);

        // Revieve Wolves  
        int numWolfRecieved;
        MPI_Recv(&numWolfRecieved, 1, MPI_INT, my_rank + 1, 3, MPI_COMM_WORLD, &status);
        std::vector<Wolf> wolfFromBelow(numWolfRecieved);
        MPI_Recv(wolfFromBelow.data(), numWolfRecieved, MPI_Wolf, my_rank + 1, (my_rank + 1 + 103), MPI_COMM_WORLD, &status);
        if (numWolfRecieved > 0) {
            for (Wolf &wolf : wolfFromBelow) {
                // std::cout << "**Process: " << my_rank << " recieved wolf at (" << wolf.x << "," << wolf.y << ") with energy: " << wolf.energy << " from below\n";
                newSheepFlock.clear();
                for (Sheep &sheep : Sheep::sheepFlock) {
                    if (sheep.x == wolf.x && sheep.y == wolf.y) {
                        wolf.eatSheep(sheep);
                    } else {
                        newSheepFlock.push_back(sheep);
                    }
                }
                Sheep::sheepFlock = newSheepFlock;
                if (wolf.energy >= 0) {
                    Wolf::wolfPack.push_back(wolf);
                    // Handling wolf reproduction
                    if (rand_double() < wolfReproduce) {
                        Wolf wolfOffspring = wolf.reproduceWolf(my_rank, world_size, rows_per_rank);
                        if (wolfOffspring.x >= rows_per_rank) {
                            wolfOffspring.x = rows_per_rank - 1;
                        }
                        Wolf::wolfPack.push_back(wolfOffspring);
                    } 
                }
            }
        }
    }

    if (my_rank > 0) {
        // Send Sheep up
        MPI_Send(&sheepUpSize, 1, MPI_INT, my_rank - 1, 1, MPI_COMM_WORLD);
        MPI_Send(sendSheepUp.data(), sheepUpSize, MPI_Sheep, my_rank - 1, my_rank + 101, MPI_COMM_WORLD);

        // recieve sheep from above
        int numSheepRecieved;
        MPI_Recv(&numSheepRecieved, 1, MPI_INT, my_rank - 1, 0, MPI_COMM_WORLD, &status);
        std::vector<Sheep> sheepFromAbove(numSheepRecieved);
        MPI_Recv(sheepFromAbove.data(), numSheepRecieved, MPI_Sheep, my_rank - 1, (my_rank - 1 + 100), MPI_COMM_WORLD, &status);
        if (numSheepRecieved > 0) {
            for (Sheep &sheep : sheepFromAbove) {
                // std::cout << "**Process: " << my_rank << " recieved sheep at (" << sheep.x << "," << sheep.y << ") with energy: " << sheep.energy << " from above\n";
                Patch &currentPatch = local_patches[sheep.x][sheep.y];
                sheep.eatGrass(currentPatch);
                if (sheep.energy >= 0) {
                    Sheep::sheepFlock.push_back(sheep);
                    // handling reproduction
                    if (rand_double() < sheepReproduce) {
                        Sheep sheepOffspring = sheep.reproduceSheep(my_rank, world_size, rows_per_rank);
                        if (sheepOffspring.x < 0) {
                            sheepOffspring.x = 0;
                        }
                        Sheep::sheepFlock.push_back(sheepOffspring);
                    }
                } 
            }
        }

        // Send Wolves up
        MPI_Send(&wolfUpSize, 1, MPI_INT, my_rank - 1, 3, MPI_COMM_WORLD);
        MPI_Send(sendWolfUp.data(), wolfUpSize, MPI_Wolf, my_rank - 1, my_rank + 103, MPI_COMM_WORLD);

        // Revieve Wolves from above
        int numWolfRecieved;
        MPI_Recv(&numWolfRecieved, 1, MPI_INT, my_rank - 1, 2, MPI_COMM_WORLD, &status);
        std::vector<Wolf> wolfFromAbove(numWolfRecieved);
        MPI_Recv(wolfFromAbove.data(), numWolfRecieved, MPI_Wolf, my_rank - 1, (my_rank - 1 + 102), MPI_COMM_WORLD, &status);
        if (numWolfRecieved > 0) {
            for (Wolf &wolf : wolfFromAbove) {
                // std::cout << "**Process: " << my_rank << " recieved wolf at (" << wolf.x << "," << wolf.y << ") with energy: " << wolf.energy << " from above\n";
                newSheepFlock.clear();
                for (Sheep &sheep : Sheep::sheepFlock) {
                    if (sheep.x == wolf.x && sheep.y == wolf.y) {
                        wolf.eatSheep(sheep);
                    } else {
                        newSheepFlock.push_back(sheep);
                    }
                }
                Sheep::sheepFlock = newSheepFlock;
                if (wolf.energy >= 0) {
                    Wolf::wolfPack.push_back(wolf);
                    // Handling wolf reproduction
                    if (rand_double() < wolfReproduce) {
                        Wolf wolfOffspring = wolf.reproduceWolf(my_rank, world_size, rows_per_rank);
                        if (wolfOffspring.x < 0) {
                            wolfOffspring.x = 0;
                        }
                        Wolf::wolfPack.push_back(wolfOffspring);
                    } 
                }
            }
        }

    }
}

void go (std::vector<std::vector<Patch>>& local_patches, int my_rank, int world_size, int rows_per_rank, MPI_Status& status, MPI_Datatype MPI_Sheep, MPI_Datatype MPI_Wolf) {
    ++ticks;

    // Simulate the sheep
    std::vector<Sheep> newSheepFlock;
    std::vector<Sheep> sendSheepUp;
    std::vector<Sheep> sendSheepDown;
    for (Sheep &sheep : Sheep::sheepFlock) {
        // std::cout << "Process: " << my_rank << " about to move sheep at (" << sheep.x << "," << sheep.y << ")" << " with energy: " << sheep.energy << "\n";
        sheep.move(my_rank, world_size, rows_per_rank);
        if (modelVersion == SHEEP_WOLVES_GRASS) {
            sheep.energy -= 1;
        }
        // std::cout << "Process: " << my_rank << " moved sheep to (" << sheep.x << "," << sheep.y << ")" << " with energy: " << sheep.energy << "\n";

        // Conditionals for sending sheep between processes
        if (sheep.x < 0 || sheep.x >= rows_per_rank) {
            if (my_rank > 0 && sheep.x < 0) {
                // std::cout << "**Process: " << my_rank << " sent sheep at (" << sheep.x << "," << sheep.y << ") with energy: " << sheep.energy << " upward\n";
                sheep.x = rows_per_rank - 1;
                sendSheepUp.push_back(sheep);
            } 
            if (my_rank < (world_size - 1) && sheep.x >= rows_per_rank) {
                // std::cout << "**Process: " << my_rank << " sent sheep at (" << sheep.x << "," << sheep.y << ") with energy: " << sheep.energy << " downward\n";
                sheep.x = 0;
                sendSheepDown.push_back(sheep);
            } 
        }
        // handling sheep behavior when it doesn't need to be sent to another process
        else {
            if (modelVersion == SHEEP_WOLVES_GRASS) {
                Patch &currentPatch = local_patches[sheep.x][sheep.y];
                sheep.eatGrass(currentPatch); 
                if (sheep.energy >= 0) {
                    newSheepFlock.push_back(sheep);
                    // handling reproduction
                    if (rand_double() < sheepReproduce) {
                        Sheep sheepOffspring = sheep.reproduceSheep(my_rank, world_size, rows_per_rank);
                        if (my_rank > 0 && sheepOffspring.x < 0) {
                            // std::cout << "**Process: " << my_rank << " sent child sheep at (" << sheepOffspring.x << "," << sheepOffspring.y << ") with energy: " << sheepOffspring.energy << " upward\n";
                            sheepOffspring.x = rows_per_rank - 1;
                            sendSheepUp.push_back(sheepOffspring);
                        } else if (my_rank < (world_size -1) && sheepOffspring.x >= rows_per_rank) {
                            // std::cout << "**Process: " << my_rank << " sent child sheep at (" << sheepOffspring.x << "," << sheepOffspring.y << ") with energy: " << sheepOffspring.energy << " downward\n";
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
                    Sheep sheepOffspring = sheep.reproduceSheep(my_rank, world_size, rows_per_rank);
                    if (my_rank > 0 && sheepOffspring.x < 0) {
                        sheepOffspring.x = rows_per_rank - 1;
                        sendSheepUp.push_back(sheepOffspring);
                    } else if (my_rank < (world_size -1) && sheepOffspring.x >= rows_per_rank) {
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
    std::vector<Wolf> sendWolfUp;
    std::vector<Wolf> sendWolfDown;
    for (Wolf &wolf: Wolf::wolfPack) {
        // std::cout << "Process: " << my_rank << " moved wolf at (" << wolf.x << "," << wolf.y << ")" << " with energy: " << wolf.energy << "\n";
        wolf.move(my_rank, world_size, rows_per_rank);
        wolf.energy -= 1; 
        // std::cout << "Process: " << my_rank << " moved wolf to (" << wolf.x << "," << wolf.y << ")" << " with energy: " << wolf.energy << "\n";

        if (wolf.x < 0 || wolf.x >= rows_per_rank) {
            if (my_rank > 0 && wolf.x < 0) {
                // std::cout << "**Process: " << my_rank << " sent wolf at (" << wolf.x << "," << wolf.y << ") with energy: " << wolf.energy << " upward\n";
                wolf.x = rows_per_rank - 1;
                sendWolfUp.push_back(wolf);
            }
            if (my_rank < (world_size - 1) && wolf.x >= rows_per_rank) {
                // std::cout << "**Process: " << my_rank << " sent wolf at (" << wolf.x << "," << wolf.y << ") with energy: " << wolf.energy << " downward\n";
                wolf.x = 0;
                sendWolfDown.push_back(wolf);
            }
        }
        // Handling wolf behavior when it doesn't need to be sent to another process
        else {
            newSheepFlock.clear();
            for (Sheep &sheep : Sheep::sheepFlock) {
                if (sheep.x == wolf.x && sheep.y == wolf.y) {
                    wolf.eatSheep(sheep);
                } else {
                    newSheepFlock.push_back(sheep);
                }
            }
            Sheep::sheepFlock = newSheepFlock;
            // Wolf alive condition
            if (wolf.energy >= 0) {
                newWolfPack.push_back(wolf);
                // Handling wolf reproduction
                if (rand_double() < wolfReproduce) {
                    Wolf wolfOffspring = wolf.reproduceWolf(my_rank, world_size, rows_per_rank);
                    if (my_rank > 0 && wolfOffspring.x < 0) {
                        // std::cout << "**Process: " << my_rank << " sent child wolf at (" << wolf.x << "," << wolf.y << ") with energy: " << wolf.energy << " upward\n";
                        wolfOffspring.x = rows_per_rank - 1;
                        sendWolfUp.push_back(wolfOffspring);
                    } else if (my_rank < (world_size -1) && wolfOffspring.x >= rows_per_rank) {
                        // std::cout << "**Process: " << my_rank << " sent child wolf at (" << wolf.x << "," << wolf.y << ") with energy: " << wolf.energy << " downward\n";
                        wolfOffspring.x = 0;
                        sendWolfDown.push_back(wolfOffspring);
                    } else {
                        newWolfPack.push_back(wolfOffspring);
                    }
                }
            } 
        }
    }
    Wolf::wolfPack = newWolfPack;
    // Exchange the animals between/across the processes
    exchangeAnimals(local_patches, my_rank, world_size, rows_per_rank, sendSheepUp, sendSheepDown, sendWolfUp, sendWolfDown, status, MPI_Sheep, MPI_Wolf);

    // Grow the local grass
    if (modelVersion == SHEEP_WOLVES_GRASS) {
        for (int i = 0; i < rows_per_rank; ++i) {
            for (int j = 0; j < COLS; ++j) {
                local_patches[i][j].growGrass(grassRegrowthTime);
            }
        }
    }

    displayLabels(my_rank);
    int local_green_count = grass(local_patches, rows_per_rank);
    int total_green_count = 0;
    MPI_Allreduce(&local_green_count, &total_green_count, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
    if (my_rank == 0) {
        std::cout << "Total Green Patch Count: " << total_green_count << "\n";
        std::cout << "Ticks: " << ticks << "\n";
    }
}

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
    displayLabels(my_rank);
    // MPI_Barrier(MPI_COMM_WORLD);

    int local_sheep_count = Sheep::sheepFlock.size();
    int local_wolf_count = Wolf::wolfPack.size();
    // std::cout << "process: " << my_rank << " has " << local_sheep_count << " sheep" << "\n";
    // std::cout << "process: " << my_rank << " has " << local_wolf_count << " wolves" << "\n";
    int total_sheep_count = 0;
    int total_wolf_count = 0;
    MPI_Allreduce(&local_sheep_count, &total_sheep_count, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
    MPI_Allreduce(&local_wolf_count, &total_wolf_count, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);

    int counter = 0;
    while (true) {
        if (my_rank == 0) {
            std::cout << "Checking conditions...\n";
        }
        if (total_wolf_count == 0 && total_sheep_count > maxSheep) {
            std::cout << "The sheep have inherited the earth" << "\n";
            break;
        }
        if (total_sheep_count == 0 && total_wolf_count == 0) {
            std::cout << "All sheep and wolves are gone!" << "\n";
            break;
        }
        go(local_patches, my_rank, world_size, rows_per_rank, status, MPI_Sheep, MPI_Wolf);
        // MPI_Barrier(MPI_COMM_WORLD);

        local_sheep_count = Sheep::sheepFlock.size();
        local_wolf_count = Wolf::wolfPack.size();
        total_sheep_count = 0;
        total_wolf_count = 0;
        MPI_Allreduce(&local_sheep_count, &total_sheep_count, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
        MPI_Allreduce(&local_wolf_count, &total_wolf_count, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
        counter++;
        if (counter >= 2500) { //2500 was benchmark for non-MPI
            break;
        }
    }

    std::chrono::steady_clock::time_point stopTime = std::chrono::steady_clock::now();
    std::chrono::duration<double> time_span = (std::chrono::duration_cast<std::chrono::duration<double>>(stopTime - startTime));
    double dt = time_span.count();

    if (my_rank == 0) {
        std::cout << "Counter: " << counter << "\n";
        std::cout << "Time to run main (timespan dt): " << dt << "\n"; 
        std::cout << "sheepFlock size: " << total_sheep_count << "\n";
        std::cout << "wolfPack size: " << total_wolf_count << "\n";
    }

    MPI_Type_free(&MPI_Sheep);
    MPI_Type_free(&MPI_Wolf);
    MPI_Finalize();
    return 0;
}






/*
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
 */