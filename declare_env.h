#ifndef DECLARE_ENV_H
#define DECLARE_ENV_H

#include <string>
#include <vector>
#include "sheep.h"
#include "wolf.h"
#include "animal.h"

extern std::string SHEEP_WOLVES_GRASS;
extern std::string SHEEP_WOLVES;

extern int ROWS;
extern int COLS;

extern int sheepGainFromFood;
extern int wolfGainFromFood;
extern int grassRegrowthTime;
extern int initialNumberSheep;
extern int initialNumberWolves;
// extern const int maxSheep;
// extern const int maxWolf;
extern float sheepReproduce;
extern float wolfReproduce;
extern bool showEnergy;

#endif