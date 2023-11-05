#pragma once

#ifndef WEBHUD_H
#define WEBHUD_H


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
//extern lapsAndFuel currentState;

#endif // WEBHUD_H