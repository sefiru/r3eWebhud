#pragma once
#include <stdint.h>
#include "webhudgit.h"

typedef struct {
    int size;
    unsigned char* data;
} BlobResult;
void init();
void writeWindowsSettings(unsigned char* value, size_t size);
BlobResult readWindowsSettings();
void writeBestLapFuel(LapsAndFuel* lapsAndFuel, int32_t layout, int32_t car);
void readBestLapFuel(LapsAndFuel* lapsAndFuel, int32_t layout, int32_t car);
//void writeBestLapFuel(LapsAndFuel* lapsAndFuel, int32_t track, int32_t car);
//void readBestLapFuel(LapsAndFuel* lapsAndFuel);
//void readWindowsSettings();
