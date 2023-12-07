#include "webhudgit.h"
#include "r3e.h"
#include "utils.h"
#include "web.h"
#include "db.h"

#define _USE_MATH_DEFINES

#include <math.h>
#include <stdio.h>
#include <time.h>
#include <windows.h>
#include <tchar.h>
#include <string.h>

#define INTERVAL_MS 1

HANDLE map_handle = INVALID_HANDLE_VALUE;
r3e_shared* map_buffer = NULL;

HANDLE map_open()
{
    return OpenFileMapping(
        FILE_MAP_READ,
        FALSE,
        TEXT(R3E_SHARED_MEMORY_NAME));
}

BOOL map_exists()
{
    HANDLE handle = map_open();

    if (handle != NULL)
        CloseHandle(handle);

    return handle != NULL;
}

int map_init()
{
    map_handle = map_open();

    if (map_handle == NULL)
    {
        wprintf_s(L"Failed to open mapping");
        return 1;
    }

    map_buffer = (r3e_shared*)MapViewOfFile(map_handle, FILE_MAP_READ, 0, 0, sizeof(r3e_shared));
    if (map_buffer == NULL)
    {
        wprintf_s(L"Failed to map buffer");
        return 1;
    }

    return 0;
}

void map_close()
{
    if (map_buffer) UnmapViewOfFile(map_buffer);
    if (map_handle) CloseHandle(map_handle);
}

struct KeyValue {
    int key;
    float value;
    r3e_driver_data data;
};

int compareByValue(const void* a, const void* b) {
    //printf("             %d            ",((struct KeyValue*)a)->value);
    //return ((struct KeyValue*)a)->value - ((struct KeyValue*)b)->value;
    float epsilon = 0.0001;
    float diff = ((struct KeyValue*)a)->value - ((struct KeyValue*)b)->value;

    if (diff < -epsilon) {
        return -1;
    }
    else if (diff > epsilon) {
        return 1;
    }
    else {
        return 0;
    }
}


int compare(const void* a, const void* b) {
    r3e_driver_data* playerA = *(r3e_driver_data**)a;
    r3e_driver_data* playerB = *(r3e_driver_data**)b;


    return (playerB->lap_distance - playerA->lap_distance) > 0 ? 1 : -1;
    // Sort by currentLap first, then by distance
    //if (playerA->completed_laps != playerB->completed_laps)
    //    return playerB->completed_laps - playerA->completed_laps;
    //else
    //    return (playerB->lap_distance - playerA->lap_distance) > 0 ? 1 : -1;
}

unsigned char chrs[200];
struct KeyValue radarPlayers[128];
unsigned long playersIdSum = 0;
PlayerRatingInfoDb playerRatingInfoDb;
PlayerRatingInfoSended playerRatingInfoSended;
LapsAndFuel lapsAndFuelData;
Settings settings;
int currentState = -2;
int lastSavedLap = -2;
int startLights = -1;
r3e_driver_data* all_drivers_data_copy[R3E_NUM_DRIVERS_MAX];
r3e_driver_data* all_drivers_data_copy2[R3E_NUM_DRIVERS_MAX];
int main()
{
    //Beep(440, 5000);
    init();
    initSettings();
    lapsAndFuelData.allBestLap = 9999;
    lapsAndFuelData.allBestFuel = 9999;
    lapsAndFuelData.leaderBestLap = 9999;
    lapsAndFuelData.currentBestLap = 9999;
    lapsAndFuelData.previousLap = 9999;

    for (int i = 0; i < 60; ++i) {
        radarPlayers[i].key = INT_MAX;
        radarPlayers[i].value = INT_MAX;
    }
    chrs[0] = 130;
    startServers();

    clock_t clk_start = 0, clk_last = 0;
    clock_t clk_delta_ms = 0, clk_elapsed = 0;

    clock_t clk_start_web_delta_radar = 0, clk_last_web_delta_radar = 0;
    clock_t clk_delta_ms_web_delta_radar = 0;
    clock_t clk_start_web_inputs = 0, clk_last_web_inputs = 0;
    clock_t clk_delta_ms_web_inputs = 0;
    clock_t clk_start_web_wheels = 0, clk_last_web_wheels = 0;
    clock_t clk_delta_ms_web_wheels = 0;
    clock_t clk_start_web_ralative_fuel = 0, clk_last_web_ralative_fuel = 0;
    clock_t clk_delta_ms_web_ralative_fuel = 0;
    clock_t clk_start_web_plrs_info = 0, clk_last_web_plrs_info = 0;
    clock_t clk_delta_ms_web_plrs_info = 0;
    
    int err_code = 0;
    BOOL mapped_r3e = FALSE;

    clk_start = clock();
    clk_last = clk_start;

    wprintf_s(L"Looking for RRRE.exe...\n");

    for (;;)
    {
        /*clk_elapsed = (clock() - clk_start) / CLOCKS_PER_SEC;
        if (clk_elapsed >= ALIVE_SEC)
            break;*/

        clk_delta_ms = (clock() - clk_last) / CLOCKS_PER_MS;
        if (clk_delta_ms < INTERVAL_MS)
        {
            Sleep(1);
            continue;
        }

        clk_last = clock();

        if (!mapped_r3e && is_r3e_running() && map_exists())
        {
            wprintf_s(L"Found RRRE.exe, mapping shared memory...\n");

            err_code = map_init();
            if (err_code)
                return err_code;

            wprintf_s(L"Memory mapped successfully\n");

            for (int i = 0; i < 128; i++) {
                all_drivers_data_copy[i] = &map_buffer->all_drivers_data_1[i];
                all_drivers_data_copy2[i] = all_drivers_data_copy[i];
            }
            mapped_r3e = TRUE;
            clk_start = clock();
        }
        /*if (isClientConnected) {
            clk_delta_ms_web_delta_radar = (clock() - clk_last_web_delta_radar) / CLOCKS_PER_MS;
            if (clk_delta_ms_web_delta_radar > INTERVAL_MS_WEB_DELTA_RADAR) {
                int random_num = rand() % 26;

                // Map the random number to a lowercase letter (ASCII values 'a' to 'z')
                char random_char = 'a' + random_num;
                char chrs[] = { 130, 3, 0x4d, 0x44, random_char, '\0' };
                sendMessage(chrs);
                clk_last_web_delta_radar = clock();
            }
            
        }*/
        if (mapped_r3e)
        {

            if (map_buffer->session_type != -1) {
                if (currentState == -2) {
                    //load from db
                    readBestLapFuel(&lapsAndFuelData, map_buffer->track_id, map_buffer->vehicle_info.model_id);
                    currentState = -1;
                }
                if (currentState != -1 && currentState != map_buffer->session_type) {
                    lastSavedLap = -2;
                }
            }
            else {
                if (currentState != map_buffer->session_type) {
                    //memset(&lapsAndFuelData, 9999, sizeof(lapsAndFuelData));
                    /*lapsAndFuelData.currentBestFuel = 0;
                    lapsAndFuelData.currentBestLap = 0;
                    lapsAndFuelData.previousLap = 0;*/
                    lapsAndFuelData.allBestLap = 9999;
                    lapsAndFuelData.allBestFuel = 9999;
                    lapsAndFuelData.leaderBestLap = 9999;
                    lapsAndFuelData.currentBestLap = 9999;
                    lapsAndFuelData.previousLap = 9999;

                    lastSavedLap = -2;
                    currentState = -2;
                }
            }
            //if (isClientConnected && map_buffer->session_type != -1) {
            if (isClientConnected) {
                doThings();
            }
        }
    }

    map_close();

    wprintf_s(L"All done!");
    system("PAUSE");

    return 0;
}
void resetSort() {
    for (int i = 0; i < 128; i++) {
        all_drivers_data_copy[i] = all_drivers_data_copy2[i];
    }
}
//all_drivers_data_copy


#define JOBS_SIZE 6
Job jobs[JOBS_SIZE];

void initSettings() {
    FILE* file;
    errno_t err = fopen_s(&file, "settings.txt", "r");
    if (err == 0) {
        /*fscanf_s(file, "last lap = %d\n", &settings.last_lap, sizeof(settings.last_lap));
        fscanf_s(file, "best lap = %d\n", &settings.best_lap, sizeof(settings.best_lap));
        fscanf_s(file, "best lap leader = %d\n", &settings.best_lap_leader, sizeof(settings.best_lap_leader));
        fscanf_s(file, "delta = %d\n", &settings.delta, sizeof(settings.delta));
        fscanf_s(file, "radar = %d\n", &settings.radar, sizeof(settings.radar));
        fscanf_s(file, "all time best table = %d\n", &settings.all_time_best_table, sizeof(settings.all_time_best_table));
        fscanf_s(file, "relative table = %d\n", &settings.relative_table, sizeof(settings.relative_table));
        fscanf_s(file, "wheels = %d\n", &settings.wheels, sizeof(settings.wheels));
        fscanf_s(file, "input graph = %d\n", &settings.input_graph, sizeof(settings.input_graph));
        fscanf_s(file, "input percents = %d\n", &settings.input_percents, sizeof(settings.input_percents));
        fscanf_s(file, "standings = %d\n", &settings.standings, sizeof(settings.standings));
        fscanf_s(file, "starting lights = %d\n", &settings.startingLights, sizeof(settings.startingLights));
        fscanf_s(file, "dev = %d\n", &settings.dev, sizeof(settings.dev));*/
        fscanf_s(file, "last lap = %d\n", &settings.last_lap);
        fscanf_s(file, "best lap = %d\n", &settings.best_lap);
        fscanf_s(file, "best lap leader = %d\n", &settings.best_lap_leader);
        fscanf_s(file, "delta = %d\n", &settings.delta);
        fscanf_s(file, "radar = %d\n", &settings.radar);
        fscanf_s(file, "all time best table = %d\n", &settings.all_time_best_table);
        fscanf_s(file, "relative table = %d\n", &settings.relative_table);
        fscanf_s(file, "wheels = %d\n", &settings.wheels);
        fscanf_s(file, "input graph = %d\n", &settings.input_graph);
        fscanf_s(file, "input percents = %d\n", &settings.input_percents);
        fscanf_s(file, "standings = %d\n", &settings.standings);
        fscanf_s(file, "starting lights = %d\n", &settings.startingLights);
        fscanf_s(file, "dev = %d\n", &settings.dev);
        fclose(file);
    } else {
        settings = (Settings){ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0};
        FILE* file;
        errno_t err = fopen_s(&file, "settings.txt", "w");
        if (err == 0) {
            fprintf(file, "last lap = %d\n", settings.last_lap);
            fprintf(file, "best lap = %d\n", settings.best_lap);
            fprintf(file, "best lap leader = %d\n", settings.best_lap_leader);
            fprintf(file, "delta = %d\n", settings.delta);
            fprintf(file, "radar = %d\n", settings.radar);
            fprintf(file, "all time best table = %d\n", settings.all_time_best_table);
            fprintf(file, "relative table = %d\n", settings.relative_table);
            fprintf(file, "wheels = %d\n", settings.wheels);
            fprintf(file, "input graph = %d\n", settings.input_graph);
            fprintf(file, "input percents = %d\n", settings.input_percents);
            fprintf(file, "standings = %d\n", settings.standings);
            fprintf(file, "starting lights = %d\n", settings.startingLights);
            fprintf(file, "dev = %d\n", settings.dev);
            fclose(file);
        }
    }

    jobs[0] = (Job){ 0, 0, 0, doStartLightsAndBestLapSaves };
    jobs[1] = (Job){ 10, 0, 0, doDeltaRadar };
    jobs[2] = (Job){ 33, 0, 0, doInputs };
    jobs[3] = (Job){ 200, 0, 0, doWheels };
    jobs[4] = (Job){ 200, 0, 0, doRelativeFuel };
    jobs[5] = (Job){ 5000, 0, 0, doPlayersInfo };
}

void checkTimer(Job* job) {
    if (job->intervalMs != 0) {
        job->clkDelta = (clock() - job->clkLast) / CLOCKS_PER_MS;
        if (job->clkDelta > job->intervalMs) {
            job->doJob();
            job->clkLast = clock();
        }
    } else {
        job->doJob();
    }
}

void doThings() {
    for (size_t i = 0; i < JOBS_SIZE; i++) {
        if (jobs[i].doJob != NULL)
            checkTimer(&jobs[i]);
    }
}


void doDeltaRadar() {
    chrs[1] = 5;
    chrs[2] = 1;
    memcpy(&chrs[3], &map_buffer->time_delta_best_self, sizeof(r3e_float32));
    //memcpy(&chrs[3], &map_buffer->lap_time_delta_leader_class, sizeof(r3e_float32));


    /*printf("(%f,%f,%f)", map_buffer->all_drivers_data_1[1].position.x, map_buffer->all_drivers_data_1[1].position.z, map_buffer->all_drivers_data_1[1].orientation.y);
    printf("(%f,%f,%f)\n", map_buffer->all_drivers_data_1[0].position.x, map_buffer->all_drivers_data_1[0].position.z, map_buffer->all_drivers_data_1[0].orientation.y);*/
    /* printf("(%f,%f)", map_buffer->all_drivers_data_1[1].driver_info.car_width, map_buffer->all_drivers_data_1[1].driver_info.car_length);
        printf("(%f,%f)\n", map_buffer->all_drivers_data_1[0].driver_info.car_width, map_buffer->all_drivers_data_1[0].driver_info.car_length);*/
        //printf("%d ",map_buffer->num_cars);
    memcpy(&chrs[2 + chrs[1]], &map_buffer->all_drivers_data_1[map_buffer->position - 1].position.x, sizeof(r3e_float32));
    chrs[1] += 4;
    memcpy(&chrs[2 + chrs[1]], &map_buffer->all_drivers_data_1[map_buffer->position - 1].position.z, sizeof(r3e_float32));
    chrs[1] += 4;
    memcpy(&chrs[2 + chrs[1]], &map_buffer->all_drivers_data_1[map_buffer->position - 1].orientation.y, sizeof(r3e_float32));
    chrs[1] += 4;
    memcpy(&chrs[2 + chrs[1]], &map_buffer->all_drivers_data_1[map_buffer->position - 1].driver_info.car_width, sizeof(r3e_float32));
    chrs[1] += 4;
    memcpy(&chrs[2 + chrs[1]], &map_buffer->all_drivers_data_1[map_buffer->position - 1].driver_info.car_length, sizeof(r3e_float32));
    chrs[1] += 4;
    for (int i = 0; i < 128; ++i) {
        radarPlayers[i].key = i;
        radarPlayers[i].value = INT_MAX;
        radarPlayers[i].data;
    }
    for (int i = 0; i < map_buffer->num_cars; i++) {
        all_drivers_data_copy[i] = &map_buffer->all_drivers_data_1[i];
    }
    for (size_t i = 0; i < map_buffer->num_cars && map_buffer->session_phase != -1; i++) {
        //if (i == map_buffer->position - 1) {
        if (map_buffer->all_drivers_data_1[i].driver_info.slot_id == map_buffer->vehicle_info.slot_id || all_drivers_data_copy[i]->finish_status > 1) {
            continue;
        }
        /*if (all_drivers_data_copy[currentCar]->finish_status > 1) {
            to++;
            skiped++;
            continue;
        }*/
        /*printf("%d", map_buffer->session_phase);
        __try {
            if (map_buffer->all_drivers_data_1[i].driver_info.slot_id == map_buffer->vehicle_info.slot_id) {
                continue;
            }
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            printf("Caught an access violation!\n");
        }*/
        //int dx = (&map_buffer->all_drivers_data_1[i].position.x + 500) - (&map_buffer->all_drivers_data_1[map_buffer->position - 1].position.x + 500);
        //int dy = (&map_buffer->all_drivers_data_1[i].position.y + 500) - (&map_buffer->all_drivers_data_1[map_buffer->position - 1].position.y + 500);
        float dx = (map_buffer->all_drivers_data_1[i].position.x + 500) - (map_buffer->player.position.x + 500);
        float dy = (-map_buffer->all_drivers_data_1[i].position.y + 500) - (-map_buffer->player.position.y + 500);

        float distanceCenter = sqrt(dx * dx + dy * dy);
        radarPlayers[i].key = i;
        radarPlayers[i].value = map_buffer->vehicle_info.slot_id == map_buffer->all_drivers_data_1[i].driver_info.slot_id ? 300000 : distanceCenter;
        radarPlayers[i].data = map_buffer->all_drivers_data_1[i];
        /*__try {
            radarPlayers[i].data = map_buffer->all_drivers_data_1[i];
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            printf("Caught an access violation!\n");
        }*/
    }
    //if (map_buffer->session_phase == -1)
    //    goto nextiteration;
    if (map_buffer->session_phase == -1)
        return;
    qsort(radarPlayers, map_buffer->num_cars, sizeof(struct KeyValue), compareByValue);
    float temp = 0;
    for (size_t i = 0; i < (map_buffer->num_cars > 5 ? 5 : map_buffer->num_cars); i++) {
        //printf("%f\n", radarPlayers[i].value);
        //if (i == map_buffer->position - 1) {
        /*if (map_buffer->all_drivers_data_1[i].driver_info.slot_id == map_buffer->vehicle_info.slot_id) {
            printf("NNNNNNNNNNNNNNNNNNNNNNNNOOOOOOOOOOOOOOOOOO");
            continue;
        }*/
        /* if (temp > radarPlayers[i].value) {
                printf("pizda\n");
                for (size_t i = 0; i < (map_buffer->num_cars > 5 ? 5 : map_buffer->num_cars); i++) {
                    printf("%f\n", radarPlayers[i].value);
                }
            }
            //printf("%f,%f,%f,%f,%f\n", radarPlayers[i].data.position.x, radarPlayers[i].data.position.z, radarPlayers[i].data.orientation.y,
            //    radarPlayers[i].data.driver_info.car_width, radarPlayers[i].data.driver_info.car_length);
            printf("%f ", radarPlayers[i].value);
            for (size_t ii = 0; ii < map_buffer->num_cars; ii++)
            {
                if (map_buffer->all_drivers_data_1[ii].driver_info.slot_id == radarPlayers[i].data.driver_info.slot_id) {
                    printf("%d %s\n", radarPlayers[i].data.driver_info.slot_id, radarPlayers[i].data.driver_info.name);

                }
            }
            temp = radarPlayers[i].value;*/
        memcpy(&chrs[2 + chrs[1]], &radarPlayers[i].data.position.x, sizeof(r3e_float32));
        chrs[1] += 4;
        memcpy(&chrs[2 + chrs[1]], &radarPlayers[i].data.position.z, sizeof(r3e_float32));
        chrs[1] += 4;
        memcpy(&chrs[2 + chrs[1]], &radarPlayers[i].data.orientation.y, sizeof(r3e_float32));
        chrs[1] += 4;
        memcpy(&chrs[2 + chrs[1]], &radarPlayers[i].data.driver_info.car_width, sizeof(r3e_float32));
        chrs[1] += 4;
        memcpy(&chrs[2 + chrs[1]], &radarPlayers[i].data.driver_info.car_length, sizeof(r3e_float32));
        chrs[1] += 4;

    }


    sendMessage(chrs, chrs[1] + 2);

}

void doPlayersInfo() {
    if (currentState != map_buffer->session_type) {
        unsigned long playersIdSumTemp = 0;
        if (map_buffer->session_type == -1 || currentState == -1) {
            /*for (int i = 0; i < sizeof(playerRatingInfoCurrent) / sizeof(playerRatingInfoCurrent[0]); i++) {
                //free(playerRatingInfoCurrent[i].name);
                memset(&playerRatingInfoCurrent[i], 0, sizeof(playerRatingInfoCurrent[i]));
            }*/
            memset(playerRatingInfoSended.ids, 0, sizeof(playerRatingInfoSended.ids));
            playerRatingInfoSended.size = 0;

            if (map_buffer->session_type == -1)
                return;
            else if (currentState == -1) {
                goto dopls;
            }
            /*free(playerRatingInfo[i].name);
            memset(playerRatingInfo, 0, sizeof(playerRatingInfo));*/

        }
        for (size_t i = 0; i < map_buffer->num_cars; i++) {
            //map_buffer->all_drivers_data_1[i];
            //map_buffer->all_drivers_data_1[i].driver_info.user_id;
            playersIdSumTemp += map_buffer->all_drivers_data_1[i].driver_info.user_id;
        }
        if (playersIdSum != playersIdSumTemp) {
        dopls:
            for (size_t i = 0; i < map_buffer->num_cars; i++) {
                if (map_buffer->all_drivers_data_1[i].driver_info.user_id == -1)
                    continue;
                boolean notContains = TRUE;
                for (size_t j = 0; j < playerRatingInfoSended.size; j++) {
                    if (map_buffer->all_drivers_data_1[i].driver_info.user_id == playerRatingInfoSended.ids[j]) {
                        notContains = FALSE;
                        break;
                    }
                }
                if (notContains) {
                    PlayerRatingInfo* temp = NULL;
                    for (size_t j = 0; j < 1000; j++) {
                        if (playerRatingInfoDb.player[j].id == 0)
                            break;
                        if (playerRatingInfoDb.player[j].id == map_buffer->all_drivers_data_1[i].driver_info.user_id)
                            temp = &playerRatingInfoDb.player[j];
                    }

                    if (temp != NULL) {

                        playerRatingInfoSended.ids[playerRatingInfoSended.size] = map_buffer->all_drivers_data_1[i].driver_info.user_id;
                        //InterlockedIncrement(playerRatingInfoSended.size);

                        //int length = snprintf(NULL, 0, "%lu,%s,%d,%d", temp->id, temp[playerRatingInfoDb.size].name, temp[playerRatingInfoDb.size].rating, temp[playerRatingInfoDb.size].reputation);
                        int length = snprintf(NULL, 0, "%lu,%s,%d,%d", temp->id, temp->name, temp->rating, temp->reputation);

                        // Allocate memory for the string
                        char* str = malloc(length + 4);

                        // Create the string
                        //sprintf_s(str, length + 1, "%lu,%s,%d,%d", temp[playerRatingInfoDb.size].id, temp[playerRatingInfoDb.size].name, temp[playerRatingInfoDb.size].rating, temp[playerRatingInfoDb.size].reputation);
                        //sprintf_s(str, length + 1, "%lu,%s,%d,%d", temp->id, temp->name, temp->rating, temp->reputation);
                        //sprintf_s(&str[3], length + 1, "%lu,%s,%d,%d", temp->id, temp->name, temp->rating, temp->reputation);
                        sprintf_s(&str[3], length + 1, "%lu,%s,%d,%d", temp->id, temp->name, temp->rating, temp->reputation);


                        playerRatingInfoSended.size++;
                        playersIdSum = playersIdSumTemp;
                        str[0] = 130;
                        str[1] = length + 2;
                        str[2] = 5;
                        sendMessage(str, length + 4);
                        free(str);
                    }
                    else {
                        //printf("\n\n%s %d\n\n", map_buffer->all_drivers_data_1[i].driver_info.name, map_buffer->all_drivers_data_1[i].driver_info.user_id);
                        playerRatingInfoSended.ids[playerRatingInfoSended.size] = map_buffer->all_drivers_data_1[i].driver_info.user_id;
                        playerRatingInfoSended.size++;
                        unsigned char byte_array[sizeof(unsigned long) + 1 + 2];
                        byte_array[0] = 130;
                        byte_array[1] = sizeof(unsigned long) + 1;
                        byte_array[2] = 8;
                        /*for (int i = 0; i < sizeof(unsigned long); ++i) {
                            byte_array[i + 3] = (map_buffer->all_drivers_data_1[i].driver_info.user_id >> (i * 8)) & 0xFF;
                        }*/
                        memcpy(&byte_array[2 + 1], &map_buffer->all_drivers_data_1[i].driver_info.user_id, sizeof(unsigned long));
                        //printf("sended, %d\n", sizeof(unsigned long) + 1);
                        //printf("sended, %d", sizeof(byte_array));
                        //printf("sended, %d", map_buffer->all_drivers_data_1[i].driver_info.user_id);
                        //printf("sended, %d", *((unsigned long*)&byte_array[3]));
                        sendMessage(byte_array, sizeof(byte_array));

                    }
                }

            }

        }
        currentState = map_buffer->session_type;
    }
}

void doRelativeFuel() {
    //r3e_float32 fuel_left;
    //r3e_float32 fuel_capacity;
    //r3e_float32 fuel_per_lap;
    chrs[1] = 1;
    chrs[2] = 4;
    //printf("%f\n", map_buffer->fuel_capacity);
    memcpy(&chrs[2 + chrs[1]], &map_buffer->fuel_left, sizeof(r3e_float32));
    chrs[1] += 4;
    float fuelPerLap = lapsAndFuelData.allBestFuel < map_buffer->fuel_per_lap ? lapsAndFuelData.allBestFuel : map_buffer->fuel_per_lap;
    float fastestLap = -2;
    if (map_buffer->lap_time_best_leader_class != -1) {
        fastestLap = lapsAndFuelData.allBestLap < map_buffer->lap_time_best_leader_class ? lapsAndFuelData.allBestLap : map_buffer->lap_time_best_leader_class;
    }
    else {
        if (lapsAndFuelData.allBestLap != 9999) {
            fastestLap = lapsAndFuelData.allBestLap;
        }
    }
    //printf("%f %f %f %f\n", ceil(map_buffer->race_session_minutes[0]*60 / fastestLap), fastestLap, fuelPerLap, ceil(ceil(map_buffer->race_session_minutes[0]*60 / fastestLap)* fuelPerLap) + 1);
    float fuelForRace = ceil(ceil(map_buffer->race_session_minutes[0] * 60 / fastestLap) * fuelPerLap) + 2;
    memcpy(&chrs[2 + chrs[1]], &fuelForRace, sizeof(r3e_float32));
    chrs[1] += 4;


    memcpy(&chrs[2 + chrs[1]], &map_buffer->vehicle_info.user_id, sizeof(r3e_int32));
    chrs[1] += 4;
    memcpy(&chrs[2 + chrs[1]], &map_buffer->position, sizeof(r3e_int32));
    chrs[1] += 4;
    
    if (map_buffer->session_phase == -1)
        return;
    for (int i = 0; i < map_buffer->num_cars; i++) {
        all_drivers_data_copy[i] = &map_buffer->all_drivers_data_1[i];
    }
    qsort(all_drivers_data_copy, map_buffer->num_cars, sizeof(r3e_driver_data*), compare);
    int playerInSortedArray = 0;
    //&map_buffer->vehicle_info.user_id;
    for (int i = 0; i < map_buffer->num_cars; i++) {
        if (all_drivers_data_copy[i]->driver_info.user_id == map_buffer->vehicle_info.user_id) {
            playerInSortedArray = i;
            break;
        }
    }
    if (map_buffer->num_cars > 1) {
        int counting = map_buffer->num_cars - 1;
        int num = 0;
        unsigned char chr = 0;
        int notFilled = 0;
        int skiped = 0;
        size_t to = 6;
        for (size_t i = 1; i < to; i++) {
            if (i < map_buffer->num_cars) {

                unsigned char currentCar = ((int)(playerInSortedArray - i)) < 0 ? map_buffer->num_cars + (playerInSortedArray - i) : playerInSortedArray - i;
                if (all_drivers_data_copy[currentCar]->finish_status > 1) {
                    to++;
                    skiped++;
                    continue;
                }
                if (playerInSortedArray == currentCar) {
                    notFilled = i;
                    break;
                }
                unsigned char currentCarNum = all_drivers_data_copy[currentCar]->place;

                memcpy(&chrs[2 + chrs[1]], &all_drivers_data_copy[currentCar]->driver_info.user_id, sizeof(r3e_int32));
                chrs[1] += 4;
                memcpy(&chrs[2 + chrs[1]], &currentCarNum, sizeof(unsigned char));
                chrs[1] += 1;

                if (map_buffer->completed_laps != all_drivers_data_copy[currentCar]->completed_laps) {
                    float toClient = (all_drivers_data_copy[currentCar]->completed_laps - map_buffer->completed_laps) * 1000;
                    memcpy(&chrs[2 + chrs[1]], &toClient, sizeof(r3e_float32));
                    chrs[1] += 4;
                    continue;
                }
                float timeza = ((map_buffer->lap_distance / map_buffer->layout_length * 100) / 100 * lapsAndFuelData.allBestLap) - (all_drivers_data_copy[currentCar]->lap_distance / map_buffer->layout_length * 100) / 100 * lapsAndFuelData.allBestLap;

                memcpy(&chrs[2 + chrs[1]], &timeza, sizeof(r3e_float32));
                chrs[1] += 4;
            }
            else {
                memcpy(&chrs[2 + chrs[1]], &num, sizeof(r3e_int32));
                chrs[1] += 4;
                memcpy(&chrs[2 + chrs[1]], &chr, sizeof(unsigned char));
                chrs[1] += 1;
                memcpy(&chrs[2 + chrs[1]], &num, sizeof(r3e_float32));
                chrs[1] += 4;
            }


        }
        if (notFilled != 0) {
            for (size_t i = notFilled; i < 6; i++) {
                memcpy(&chrs[2 + chrs[1]], &num, sizeof(r3e_int32));
                chrs[1] += 4;
                memcpy(&chrs[2 + chrs[1]], &chr, sizeof(unsigned char));
                chrs[1] += 1;
                memcpy(&chrs[2 + chrs[1]], &num, sizeof(r3e_float32));
                chrs[1] += 4;
            }
            notFilled = 0;
        }

        if (chrs[1] < 62) {
            for (size_t i = 0; i < skiped; i++) {
                memcpy(&chrs[2 + chrs[1]], &num, sizeof(r3e_int32));
                chrs[1] += 4;
                memcpy(&chrs[2 + chrs[1]], &chr, sizeof(unsigned char));
                chrs[1] += 1;
                memcpy(&chrs[2 + chrs[1]], &num, sizeof(r3e_float32));
                chrs[1] += 4;
            }
            skiped = 0;
        }
        to = 6;
        for (size_t i = 1; i < to; i++) {
            if (i < map_buffer->num_cars) {
                unsigned char currentCar = (int)(playerInSortedArray + i) > (int)(map_buffer->num_cars - 1) ? i - (map_buffer->num_cars - 1 - playerInSortedArray) - 1 : playerInSortedArray + i;
                if (all_drivers_data_copy[currentCar]->finish_status > 1) {
                    to++;
                    skiped++;
                    continue;
                }
                if (playerInSortedArray == currentCar) {
                    notFilled = i;
                    break;
                }

                unsigned char currentCarNum = all_drivers_data_copy[currentCar]->place;
                memcpy(&chrs[2 + chrs[1]], &all_drivers_data_copy[currentCar]->driver_info.user_id, sizeof(r3e_int32));
                chrs[1] += 4;
                memcpy(&chrs[2 + chrs[1]], &currentCarNum, sizeof(unsigned char));
                chrs[1] += 1;

                if (map_buffer->completed_laps != all_drivers_data_copy[currentCar]->completed_laps) {
                    float toClient = (all_drivers_data_copy[currentCar]->completed_laps - map_buffer->completed_laps) * 1000;
                    memcpy(&chrs[2 + chrs[1]], &toClient, sizeof(r3e_float32));
                    chrs[1] += 4;
                    continue;
                }
                float timeza = ((map_buffer->lap_distance / map_buffer->layout_length * 100) / 100 * lapsAndFuelData.allBestLap) - (all_drivers_data_copy[currentCar]->lap_distance / map_buffer->layout_length * 100) / 100 * lapsAndFuelData.allBestLap;
                memcpy(&chrs[2 + chrs[1]], &timeza, sizeof(r3e_float32));
                chrs[1] += 4;
            }
            else {
                memcpy(&chrs[2 + chrs[1]], &num, sizeof(r3e_int32));
                chrs[1] += 4;
                memcpy(&chrs[2 + chrs[1]], &chr, sizeof(unsigned char));
                chrs[1] += 1;
                memcpy(&chrs[2 + chrs[1]], &num, sizeof(r3e_float32));
                chrs[1] += 4;
            }

        }
        if (notFilled != 0) {
            for (size_t i = notFilled; i < 6; i++) {
                memcpy(&chrs[2 + chrs[1]], &num, sizeof(r3e_int32));
                chrs[1] += 4;
                memcpy(&chrs[2 + chrs[1]], &chr, sizeof(unsigned char));
                chrs[1] += 1;
                memcpy(&chrs[2 + chrs[1]], &num, sizeof(r3e_float32));
                chrs[1] += 4;
            }
            notFilled = 0;
        }

        if (chrs[1] < 107) {
            for (size_t i = 0; i < skiped; i++) {
                memcpy(&chrs[2 + chrs[1]], &num, sizeof(r3e_int32));
                chrs[1] += 4;
                memcpy(&chrs[2 + chrs[1]], &chr, sizeof(unsigned char));
                chrs[1] += 1;
                memcpy(&chrs[2 + chrs[1]], &num, sizeof(r3e_float32));
                chrs[1] += 4;
            }
            skiped = 0;
        }
    }
    /*if (map_buffer->num_cars > 11) {

    }
    else {
        for (size_t i = 0; i < map_buffer->num_cars; i++) {
            memcpy(&chrs[2 + chrs[1]], &map_buffer->all_drivers_data_1->driver_info.user_id, sizeof(r3e_float32));
            chrs[1] += 4;
            memcpy(&chrs[2 + chrs[1]], &map_buffer->all_drivers_data_1->del, sizeof(r3e_float32));
            chrs[1] += 4;
        }
        memcpy(&chrs[2 + chrs[1]], &map_buffer->fuel_left, sizeof(r3e_float32));
        chrs[1] += 4;
        memcpy(&chrs[2 + chrs[1]], &map_buffer->fuel_capacity, sizeof(r3e_float32));
        chrs[1] += 4;
        memcpy(&chrs[2 + chrs[1]], &map_buffer->fuel_per_lap, sizeof(r3e_float32));
        chrs[1] += 4;

    }*/
    //printf("%d", chrs[1] + 2);
    sendMessage(chrs, chrs[1] + 2);


    chrs[1] = 2;
    chrs[2] = 11;
    chrs[3] = 0;
    int compLaps = map_buffer->all_drivers_data_1[0].completed_laps + 1;
    memcpy(&chrs[2 + chrs[1]], &compLaps, sizeof(r3e_float32));
    chrs[1] += 4;
    if (map_buffer->session_time_remaining > 0) {
        memcpy(&chrs[2 + chrs[1]], &map_buffer->session_time_remaining, sizeof(r3e_float32));
    } else {
        r3e_float32 float_var = (r3e_float32)(map_buffer->race_session_laps[0] - map_buffer->all_drivers_data_1[0].completed_laps);
        memcpy(&chrs[2 + chrs[1]], &float_var, sizeof(r3e_float32));
    }
    chrs[1] += 4;
    //printf("%f %d %d %d %d\n", map_buffer->flags.closest_yellow_distance_into_track, map_buffer->flags.yellow, map_buffer->flags.sector_yellow[0], map_buffer->flags.sector_yellow[1], map_buffer->flags.sector_yellow[2]);
    memcpy(&chrs[2 + chrs[1]], &map_buffer->flags.yellow, sizeof(r3e_float32));
    chrs[1] += 4;
    sendMessage(chrs, chrs[1] + 2);
    r3e_float32 gap = 0;
    
    r3e_float32 firstBestLap = 0;
    if (map_buffer->session_type == 0 || map_buffer->session_type == 1) {
        firstBestLap = map_buffer->all_drivers_data_1[0].sector_time_best_self[2];
        gap = firstBestLap;
    }
    //printf("%f, %f, %f, %f\n", firstBestLap, map_buffer->all_drivers_data_1[0].sector_time_best_self[0], map_buffer->all_drivers_data_1[0].sector_time_best_self[1], map_buffer->all_drivers_data_1[0].sector_time_best_self[2]);
    if (firstBestLap == -3) {
        firstBestLap = 0;
    }
    for (size_t j = 0; j < map_buffer->num_cars; j++) {
        chrs[1] = 2;
        chrs[3] = 1;
        //printf("%d %d\n", j, numCars);
        //memcpy(&chrs[2 + chrs[1]], &map_buffer->all_drivers_data_1[j].driver_info.user_id, sizeof(r3e_float32));
        memcpy(&chrs[2 + chrs[1]], &j, sizeof(r3e_int32));
        chrs[1] += 4;
        memcpy(&chrs[2 + chrs[1]], &map_buffer->all_drivers_data_1[j].driver_info.name, 64);
        //chrs[1] += 4;
        chrs[1] += 64;
        if (j != 0) {
            if (map_buffer->session_type == 0 || map_buffer->session_type == 1) {
                if (gap != -3) {
                    gap = map_buffer->all_drivers_data_1[j].sector_time_best_self[2] - firstBestLap;
                    memcpy(&chrs[2 + chrs[1]], &gap, sizeof(r3e_float32));
                } else {
                    gap = 0;
                    memcpy(&chrs[2 + chrs[1]], &gap, sizeof(r3e_float32));
                    gap = -3;
                }
            } else {
                gap += map_buffer->all_drivers_data_1[j].time_delta_front;
                memcpy(&chrs[2 + chrs[1]], &gap, sizeof(r3e_float32));
            }
        } else {
            if (map_buffer->session_type == 0 || map_buffer->session_type == 1) {
                memcpy(&chrs[2 + chrs[1]], &firstBestLap, sizeof(r3e_float32));
            } else {
                memcpy(&chrs[2 + chrs[1]], &map_buffer->all_drivers_data_1[j].lap_time_current_self, sizeof(r3e_float32));
            }
        }
        chrs[1] += 4;
        //short manuf = map_buffer->all_drivers_data_1[0].driver_info.manufacturer_id;
        unsigned short manuf = map_buffer->all_drivers_data_1[j].driver_info.car_number;
        memcpy(&chrs[2 + chrs[1]], &manuf, 2);
        chrs[1] += 2;
        unsigned char pitStatus = map_buffer->all_drivers_data_1[j].pitstop_status < 2 ? 0 : 1;
        memcpy(&chrs[2 + chrs[1]], &pitStatus, 1);
        chrs[1] += 1;
        sendMessage(chrs, chrs[1] + 2);
    }
    //if (map_buffer->position)
    /*for (size_t i = 10; i < 140; i += 10) {
        if (i > map_buffer->num_cars + 10)
            break;
        chrs[1] = 2;
        chrs[3] = i;// / 10;
        int numCars = map_buffer->num_cars > i ? i : map_buffer->num_cars;
        //printf("%d\n", i - 10);
        //printf("%d\n", numCars);
        //printf("\n\n");
        //printf("%f\n", map_buffer->session_time_remaining);
        for (size_t j = i - 10; j < numCars; j++) {

            //printf("%d %d\n", j, numCars);
            //memcpy(&chrs[2 + chrs[1]], &map_buffer->all_drivers_data_1[j].driver_info.user_id, sizeof(r3e_float32));
            memcpy(&chrs[2 + chrs[1]], &map_buffer->all_drivers_data_1[j].position, sizeof(r3e_float32));
            chrs[1] += 4;
            memcpy(&chrs[2 + chrs[1]], &map_buffer->all_drivers_data_1[j].driver_info.name, 64);
            //chrs[1] += 4;
            chrs[1] += 64;
            printf("%d", chrs[1] + 2);
            if (j != 0) {
                gap += map_buffer->all_drivers_data_1[j].time_delta_front;
            }
            memcpy(&chrs[2 + chrs[1]], &gap, sizeof(r3e_float32));
            chrs[1] += 4;
            //short manuf = map_buffer->all_drivers_data_1[0].driver_info.manufacturer_id;
            unsigned short manuf = map_buffer->all_drivers_data_1[j].driver_info.car_number;
            memcpy(&chrs[2 + chrs[1]], &manuf, 2);
            chrs[1] += 2;
            unsigned char pitStatus = map_buffer->all_drivers_data_1[j].pitstop_status < 2 ? 0 : 1;
            memcpy(&chrs[2 + chrs[1]], &pitStatus, 1);
            chrs[1] += 1;
        }
        //printf("%d", chrs[1] + 2);
        sendMessage(chrs, chrs[1] + 2);
    }*/
}

void doWheels() {
    chrs[1] = 1;
    chrs[2] = 2;

    memcpy(&chrs[2 + chrs[1]], &map_buffer->tire_wear[0], sizeof(r3e_float32));
    chrs[1] += 4;
    memcpy(&chrs[2 + chrs[1]], &map_buffer->tire_wear[1], sizeof(r3e_float32));
    chrs[1] += 4;
    memcpy(&chrs[2 + chrs[1]], &map_buffer->tire_wear[2], sizeof(r3e_float32));
    chrs[1] += 4;
    memcpy(&chrs[2 + chrs[1]], &map_buffer->tire_wear[3], sizeof(r3e_float32));
    chrs[1] += 4;

    memcpy(&chrs[2 + chrs[1]], &map_buffer->tire_pressure[0], sizeof(r3e_float32));
    chrs[1] += 4;
    memcpy(&chrs[2 + chrs[1]], &map_buffer->tire_pressure[1], sizeof(r3e_float32));
    chrs[1] += 4;
    memcpy(&chrs[2 + chrs[1]], &map_buffer->tire_pressure[2], sizeof(r3e_float32));
    chrs[1] += 4;
    memcpy(&chrs[2 + chrs[1]], &map_buffer->tire_pressure[3], sizeof(r3e_float32));
    chrs[1] += 4;

    memcpy(&chrs[2 + chrs[1]], &map_buffer->tire_dirt[0], sizeof(r3e_float32));
    chrs[1] += 4;
    memcpy(&chrs[2 + chrs[1]], &map_buffer->tire_dirt[1], sizeof(r3e_float32));
    chrs[1] += 4;
    memcpy(&chrs[2 + chrs[1]], &map_buffer->tire_dirt[2], sizeof(r3e_float32));
    chrs[1] += 4;
    memcpy(&chrs[2 + chrs[1]], &map_buffer->tire_dirt[3], sizeof(r3e_float32));
    chrs[1] += 4;

    memcpy(&chrs[2 + chrs[1]], &map_buffer->brake_temp[0].current_temp, sizeof(r3e_float32));
    chrs[1] += 4;
    memcpy(&chrs[2 + chrs[1]], &map_buffer->brake_temp[0].optimal_temp, sizeof(r3e_float32));
    chrs[1] += 4;
    memcpy(&chrs[2 + chrs[1]], &map_buffer->brake_temp[0].cold_temp, sizeof(r3e_float32));
    chrs[1] += 4;
    memcpy(&chrs[2 + chrs[1]], &map_buffer->brake_temp[0].hot_temp, sizeof(r3e_float32));
    chrs[1] += 4;

    memcpy(&chrs[2 + chrs[1]], &map_buffer->brake_temp[1].current_temp, sizeof(r3e_float32));
    chrs[1] += 4;
    memcpy(&chrs[2 + chrs[1]], &map_buffer->brake_temp[1].optimal_temp, sizeof(r3e_float32));
    chrs[1] += 4;
    memcpy(&chrs[2 + chrs[1]], &map_buffer->brake_temp[1].cold_temp, sizeof(r3e_float32));
    chrs[1] += 4;
    memcpy(&chrs[2 + chrs[1]], &map_buffer->brake_temp[1].hot_temp, sizeof(r3e_float32));
    chrs[1] += 4;

    memcpy(&chrs[2 + chrs[1]], &map_buffer->brake_temp[2].current_temp, sizeof(r3e_float32));
    chrs[1] += 4;
    memcpy(&chrs[2 + chrs[1]], &map_buffer->brake_temp[2].optimal_temp, sizeof(r3e_float32));
    chrs[1] += 4;
    memcpy(&chrs[2 + chrs[1]], &map_buffer->brake_temp[2].cold_temp, sizeof(r3e_float32));
    chrs[1] += 4;
    memcpy(&chrs[2 + chrs[1]], &map_buffer->brake_temp[2].hot_temp, sizeof(r3e_float32));
    chrs[1] += 4;

    memcpy(&chrs[2 + chrs[1]], &map_buffer->brake_temp[3].current_temp, sizeof(r3e_float32));
    chrs[1] += 4;
    memcpy(&chrs[2 + chrs[1]], &map_buffer->brake_temp[3].optimal_temp, sizeof(r3e_float32));
    chrs[1] += 4;
    memcpy(&chrs[2 + chrs[1]], &map_buffer->brake_temp[3].cold_temp, sizeof(r3e_float32));
    chrs[1] += 4;
    memcpy(&chrs[2 + chrs[1]], &map_buffer->brake_temp[3].hot_temp, sizeof(r3e_float32));
    chrs[1] += 4;

    sendMessage(chrs, chrs[1] + 2);

    chrs[1] = 1;
    chrs[2] = 3;

    memcpy(&chrs[2 + chrs[1]], &map_buffer->tire_temp[0].current_temp[0], sizeof(r3e_float32));
    chrs[1] += 4;
    memcpy(&chrs[2 + chrs[1]], &map_buffer->tire_temp[0].current_temp[1], sizeof(r3e_float32));
    chrs[1] += 4;
    memcpy(&chrs[2 + chrs[1]], &map_buffer->tire_temp[0].current_temp[2], sizeof(r3e_float32));
    chrs[1] += 4;
    memcpy(&chrs[2 + chrs[1]], &map_buffer->tire_temp[0].optimal_temp, sizeof(r3e_float32));
    chrs[1] += 4;
    memcpy(&chrs[2 + chrs[1]], &map_buffer->tire_temp[0].cold_temp, sizeof(r3e_float32));
    chrs[1] += 4;
    memcpy(&chrs[2 + chrs[1]], &map_buffer->tire_temp[0].hot_temp, sizeof(r3e_float32));
    chrs[1] += 4;
    //printf("%f, %f, %f", map_buffer->tire_temp[0].cold_temp, map_buffer->tire_temp[0].optimal_temp, map_buffer->tire_temp[0].hot_temp);

    memcpy(&chrs[2 + chrs[1]], &map_buffer->tire_temp[1].current_temp[0], sizeof(r3e_float32));
    chrs[1] += 4;
    memcpy(&chrs[2 + chrs[1]], &map_buffer->tire_temp[1].current_temp[1], sizeof(r3e_float32));
    chrs[1] += 4;
    memcpy(&chrs[2 + chrs[1]], &map_buffer->tire_temp[1].current_temp[2], sizeof(r3e_float32));
    chrs[1] += 4;
    memcpy(&chrs[2 + chrs[1]], &map_buffer->tire_temp[1].optimal_temp, sizeof(r3e_float32));
    chrs[1] += 4;
    memcpy(&chrs[2 + chrs[1]], &map_buffer->tire_temp[1].cold_temp, sizeof(r3e_float32));
    chrs[1] += 4;
    memcpy(&chrs[2 + chrs[1]], &map_buffer->tire_temp[1].hot_temp, sizeof(r3e_float32));
    chrs[1] += 4;

    memcpy(&chrs[2 + chrs[1]], &map_buffer->tire_temp[2].current_temp[0], sizeof(r3e_float32));
    chrs[1] += 4;
    memcpy(&chrs[2 + chrs[1]], &map_buffer->tire_temp[2].current_temp[1], sizeof(r3e_float32));
    chrs[1] += 4;
    memcpy(&chrs[2 + chrs[1]], &map_buffer->tire_temp[2].current_temp[2], sizeof(r3e_float32));
    chrs[1] += 4;
    memcpy(&chrs[2 + chrs[1]], &map_buffer->tire_temp[2].optimal_temp, sizeof(r3e_float32));
    chrs[1] += 4;
    memcpy(&chrs[2 + chrs[1]], &map_buffer->tire_temp[2].cold_temp, sizeof(r3e_float32));
    chrs[1] += 4;
    memcpy(&chrs[2 + chrs[1]], &map_buffer->tire_temp[2].hot_temp, sizeof(r3e_float32));
    chrs[1] += 4;

    memcpy(&chrs[2 + chrs[1]], &map_buffer->tire_temp[3].current_temp[0], sizeof(r3e_float32));
    chrs[1] += 4;
    memcpy(&chrs[2 + chrs[1]], &map_buffer->tire_temp[3].current_temp[1], sizeof(r3e_float32));
    chrs[1] += 4;
    memcpy(&chrs[2 + chrs[1]], &map_buffer->tire_temp[3].current_temp[2], sizeof(r3e_float32));
    chrs[1] += 4;
    memcpy(&chrs[2 + chrs[1]], &map_buffer->tire_temp[3].optimal_temp, sizeof(r3e_float32));
    chrs[1] += 4;
    memcpy(&chrs[2 + chrs[1]], &map_buffer->tire_temp[3].cold_temp, sizeof(r3e_float32));
    chrs[1] += 4;
    memcpy(&chrs[2 + chrs[1]], &map_buffer->tire_temp[3].hot_temp, sizeof(r3e_float32));
    chrs[1] += 4;

    /*memcpy(&chrs[2 + chrs[1]], &map_buffer->throttle, sizeof(r3e_float32));
    chrs[1] += 4;
    memcpy(&chrs[2 + chrs[1]], &map_buffer->brake, sizeof(r3e_float32));
    chrs[1] += 4;
    memcpy(&chrs[2 + chrs[1]], &map_buffer->throttle_raw, sizeof(r3e_float32));
    chrs[1] += 4;
    memcpy(&chrs[2 + chrs[1]], &map_buffer->brake_raw, sizeof(r3e_float32));
    chrs[1] += 4;*/

    sendMessage(chrs, chrs[1] + 2);
   
}

void doInputs() {
    chrs[1] = 1;
    chrs[2] = 10;

    memcpy(&chrs[2 + chrs[1]], &map_buffer->throttle, sizeof(r3e_float32));
    chrs[1] += 4;
    memcpy(&chrs[2 + chrs[1]], &map_buffer->brake, sizeof(r3e_float32));
    chrs[1] += 4;
    memcpy(&chrs[2 + chrs[1]], &map_buffer->throttle_raw, sizeof(r3e_float32));
    chrs[1] += 4;
    memcpy(&chrs[2 + chrs[1]], &map_buffer->brake_raw, sizeof(r3e_float32));
    chrs[1] += 4;

    sendMessage(chrs, chrs[1] + 2);
}

void doStartLightsAndBestLapSaves() {
    if (map_buffer->start_lights != -1 && startLights != map_buffer->start_lights) {
        startLights = map_buffer->start_lights;
        chrs[1] = 1;
        chrs[2] = 7;
        memcpy(&chrs[2 + chrs[1]], &startLights, sizeof(int));
        chrs[1] += 4;

        sendMessage(chrs, chrs[1] + 2);
    }
    //if (isClientConnected && map_buffer->session_type == 100) {
        //if (map_buffer->completed_laps > 1) {
        //printf("%f\n", map_buffer->lap_time_previous_self);
    if (lastSavedLap != map_buffer->completed_laps && map_buffer->lap_time_previous_self != -1) {

        chrs[1] = 1;
        chrs[2] = 6;

        lapsAndFuelData.previousLap = map_buffer->lap_time_previous_self;
        //map_buffer->time_delta_best_self
        memcpy(&chrs[2 + chrs[1]], &lapsAndFuelData.previousLap, sizeof(float));
        chrs[1] += 4;

        /*if (lapsAndFuelData.currentBestLap > lapsAndFuelData.previousLap) {
            lapsAndFuelData.currentBestLap = lapsAndFuelData.previousLap;
            chrs[2 + chrs[1]] = 1;
            chrs[1] += 1;
        }
        else {
            chrs[2 + chrs[1]] = 0;
            chrs[1] += 1;
        }*/

        /*memcpy(&chrs[2 + chrs[1]], &lapsAndFuelData.currentBestFuel, sizeof(float));
        chrs[1] += 4;*/

        /*if (lapsAndFuelData.currentBestFuel > map_buffer->fuel_per_lap) {
            //lapsAndFuelData.currentBestFuel = map_buffer->fuel_per_lap;
            memcpy(&chrs[2 + chrs[1]], &lapsAndFuelData.currentBestFuel, sizeof(float));
            chrs[1] += 4;
        }*/
        if ((lapsAndFuelData.currentBestLap > lapsAndFuelData.previousLap || lapsAndFuelData.allBestFuel > map_buffer->fuel_per_lap || lapsAndFuelData.leaderBestLap > map_buffer->lap_time_best_leader_class)) {

            //if ((lapsAndFuelData.currentBestLap > map_buffer->lap_time_best_self || lapsAndFuelData.allBestFuel > map_buffer->fuel_per_lap || lapsAndFuelData.leaderBestLap > map_buffer->lap_time_best_leader_class)) {
            if (map_buffer->completed_laps > 0 && lapsAndFuelData.currentBestLap > lapsAndFuelData.previousLap) {
                //if (map_buffer->lap_time_best_self != -1 && map_buffer->completed_laps > 0 && lapsAndFuelData.currentBestLap > map_buffer->lap_time_best_self) {
                lapsAndFuelData.currentBestLap = lapsAndFuelData.previousLap;
            }
            lapsAndFuelData.leaderBestLap = map_buffer->lap_time_best_leader_class;
            //if (map_buffer->completed_laps > 1 && (lapsAndFuelData.allBestLap > lapsAndFuelData.currentBestLap || lapsAndFuelData.allBestFuel > map_buffer->fuel_per_lap)) {
            if (((map_buffer->session_type != 1 && map_buffer->completed_laps > 1) || (map_buffer->session_type == 1 && map_buffer->completed_laps > 0)) && ((map_buffer->lap_time_best_self != -1 && lapsAndFuelData.allBestLap > map_buffer->lap_time_best_self) || lapsAndFuelData.allBestFuel > map_buffer->fuel_per_lap)) {
                if (map_buffer->lap_time_best_self != -1 && lapsAndFuelData.allBestLap > map_buffer->lap_time_best_self) {
                    lapsAndFuelData.allBestLap = map_buffer->lap_time_best_self;
                }
                if (lapsAndFuelData.allBestFuel > map_buffer->fuel_per_lap) {
                    lapsAndFuelData.allBestFuel = map_buffer->fuel_per_lap;
                }

                writeBestLapFuel(&lapsAndFuelData, map_buffer->track_id, map_buffer->vehicle_info.model_id);

            }
        }
        memcpy(&chrs[2 + chrs[1]], &map_buffer->lap_time_best_self, sizeof(float));
        chrs[1] += 4;
        memcpy(&chrs[2 + chrs[1]], &lapsAndFuelData.leaderBestLap, sizeof(float));
        chrs[1] += 4;
        memcpy(&chrs[2 + chrs[1]], &lapsAndFuelData.allBestLap, sizeof(float));
        chrs[1] += 4;
        memcpy(&chrs[2 + chrs[1]], &lapsAndFuelData.allBestFuel, sizeof(float));
        chrs[1] += 4;
        sendMessage(chrs, chrs[1] + 2);
        lastSavedLap = map_buffer->completed_laps;
    }
}
