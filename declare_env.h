#ifndef DECLARE_ENV_H
#define DECLARE_ENV_H

#include <string>
#include <vector>
#include "sheep.h"
#include "wolf.h"
#include "animal.h"

const std::string SHEEP_WOLVES_GRASS = "sheep-wolves-grass";
const std::string SHEEP_WOLVES = "sheep-wolves";

const int ROWS = 1000;
const int COLS = 1000;

const int sheepGainFromFood = 4;
const int wolfGainFromFood = 20;
const int grassRegrowthTime = 30;
const int initialNumberSheep = 100;
const int initialNumberWolves = 50;
const int maxSheep = 1000;
const float sheepReproduce = 0.04f;
const float wolfReproduce = 0.05f;
const bool showEnergy = true;

#endif