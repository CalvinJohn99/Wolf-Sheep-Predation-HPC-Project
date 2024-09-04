#include "declare_env.h"

std::string SHEEP_WOLVES_GRASS = "sheep-wolves-grass";
std::string SHEEP_WOLVES = "sheep-wolves";

int ROWS = 60; // 1000 ends in wolves death
int COLS = 60; // 60 meands stabilisation

int sheepGainFromFood = 4;
int wolfGainFromFood = 20;
int grassRegrowthTime = 30;
int initialNumberSheep = 100;
int initialNumberWolves = 50;
int maxSheep = 10000;
float sheepReproduce = 0.04f;
float wolfReproduce = 0.05f;
bool showEnergy = true;