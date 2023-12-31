#pragma once

#include <time.h>
#include "r3e.h";

#ifndef WEBHUD_H
#define WEBHUD_H

typedef struct {
	int last_lap;
	int best_lap;
	int best_lap_leader;
	int delta;
	int radar;
	int all_time_best_table;
	int relative_table;
	int wheels;
	int input_graph;
	int input_percents;
	int standings;
	int startingLights;
	int dev;
} Settings;

typedef int (*func_ptr_t)();

typedef struct {
	int intervalMs;
	clock_t	clkLast;
	clock_t clkDelta;
	func_ptr_t doJob;
} Job;

typedef struct {
	unsigned long id;
	unsigned char* name;
	int rating;
	int reputation;
} PlayerRatingInfo;

typedef struct {
	unsigned long ids[500];
	unsigned int size;
} PlayerRatingInfoSended;

typedef struct {
	PlayerRatingInfo player[1000];
	unsigned int size;
} PlayerRatingInfoDb;

typedef struct {
	float allBestLap;
	float allBestFuel;
	float leaderBestLap;
	float currentBestLap;
	float previousLap;
} LapsAndFuel;

extern PlayerRatingInfoDb playerRatingInfoDb;
extern PlayerRatingInfoSended playerRatingInfoSended;
extern LapsAndFuel lapsAndFuelData;
extern int currentState;
extern r3e_shared* map_buffer;
extern Settings settings;
//extern lapsAndFuel currentState;


void doStartLightsAndBestLapSaves();
void doDeltaRadar();
void doInputs();
void doWheels();
void doRelativeFuel();
void doPlayersInfo();

void initSettings();
void doThings();

#endif // WEBHUD_H