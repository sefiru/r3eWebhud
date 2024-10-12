#include "webhudgit.h"
#include "r3e.h"
#include "utils.h"
#include "web.h"
#include "db.h"

#define _USE_MATH_DEFINES

#include <math.h>
#include <stdio.h>
#include <time.h>
//#include <Windows.h>
#include <windows.h>
#include <tchar.h>
#include <string.h>

//#define ALIVE_SEC 60000
#define INTERVAL_MS 1
#define INTERVAL_MS_WEB_DELTA_RADAR 10
#define INTERVAL_MS_WEB_INPUTS 33
#define INTERVAL_MS_WEB_WHEELS 200
#define INTERVAL_MS_WEB_RELATIVE_FUEL 200
#define INTERVAL_MS_WEB_PLRS_INFO 5000

HANDLE map_handle = INVALID_HANDLE_VALUE;
r3e_shared* map_buffer = NULL;
BOOL isFilled = FALSE;
//r3e_driver_data all_d_[300];
float yaw_;                       // Global yaw angle (car orientation)
float actual_angle_;              // Actual movement angle
float steer_angle_rad_;
/*
#include <process.h>  // For _beginthreadex
#define WINDOW_WIDTH 300
#define WINDOW_HEIGHT 300

HWND hwnd = NULL;

// Function prototypes
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
void DrawCarVectors(HDC hdc, int centerX, int centerY);
unsigned __stdcall WindowThread(void* param);


// Window thread function
unsigned __stdcall WindowThread(void* param) {
    // Register window class
    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = WndProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = "CarVectorWindow";

    RegisterClass(&wc);

    // Create the window in the new thread
    hwnd = CreateWindowEx(
        0, "CarVectorWindow", "Car Dynamics Visualization",
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        NULL, NULL, wc.hInstance, NULL
    );

    ShowWindow(hwnd, SW_SHOW);

    // Message loop for the window
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

// Window procedure
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        RECT rect;
        GetClientRect(hwnd, &rect);

        // Fill the entire client area with a white brush to clear old drawings
        HBRUSH hBrush = (HBRUSH)GetStockObject(WHITE_BRUSH);
        FillRect(hdc, &rect, hBrush);
        // Center of the window
        int centerX = WINDOW_WIDTH / 2;
        int centerY = WINDOW_HEIGHT / 2;

        // Draw the car vectors based on current car data
        DrawCarVectors(hdc, centerX, centerY);

        EndPaint(hwnd, &ps);
    }
                 break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

// Function to draw car vectors on the screen
void DrawCarVectors(HDC hdc, int centerX, int centerY) {
    int length = 100;  // Length of each line

    // Calculate each line's endpoint using trigonometry
    int endX_yaw = centerX + (int)(length * cos(yaw_));   // Global yaw direction
    int endY_yaw = centerY + (int)(length * sin(yaw_));

    int endX_steering = centerX + (int)(length * cos(steer_angle_rad_));  // Steering wheel angle
    int endY_steering = centerY + (int)(length * sin(steer_angle_rad_));

    int endX_actual = centerX + (int)(length * cos(actual_angle_));   // Actual movement direction
    int endY_actual = centerY + (int)(length * sin(actual_angle_));

    // Draw yaw direction (red)
    HPEN hPenYaw = CreatePen(PS_SOLID, 2, RGB(255, 0, 0));
    SelectObject(hdc, hPenYaw);
    MoveToEx(hdc, centerX, centerY, NULL);
    LineTo(hdc, endX_yaw, endY_yaw);
    DeleteObject(hPenYaw);

    // Draw steering wheel direction (green)
    HPEN hPenSteer = CreatePen(PS_SOLID, 2, RGB(0, 255, 0));
    SelectObject(hdc, hPenSteer);
    MoveToEx(hdc, centerX, centerY, NULL);
    LineTo(hdc, endX_steering, endY_steering);
    DeleteObject(hPenSteer);

    // Draw actual movement direction (blue)
    HPEN hPenActual = CreatePen(PS_SOLID, 2, RGB(0, 0, 255));
    SelectObject(hdc, hPenActual);
    MoveToEx(hdc, centerX, centerY, NULL);
    LineTo(hdc, endX_actual, endY_actual);
    DeleteObject(hPenActual);

    // Optional: Label each line with text
    TextOut(hdc, endX_yaw, endY_yaw, "Yaw", 3);
    TextOut(hdc, endX_steering, endY_steering, "Steer", 5);
    TextOut(hdc, endX_actual, endY_actual, "Movement", 8);
}*/

float16_t float32_to_float16(float value) {
    uint32_t f_bits = *((uint32_t*)&value);  // Get the raw bits of the float32 value
    uint16_t sign = (f_bits >> 16) & 0x8000; // Extract the sign (1 bit)

    // Extract and compute the exponent and mantissa for float32
    uint32_t exponent = (f_bits >> 23) & 0xFF;
    uint32_t mantissa = f_bits & 0x007FFFFF;

    float16_t f16;
    f16.raw_bits = 0; // Default to zero

    // Handle zero, denormals, and small values
    if (exponent == 0) {
        f16.raw_bits = sign;  // Zero is represented with only the sign bit
    }
    // Handle infinities and NaNs
    else if (exponent == 0xFF) {
        if (mantissa == 0) {
            // Infinity
            f16.raw_bits = sign | 0x7C00;  // Exponent = 11111 (all 1s), Mantissa = 0
        }
        else {
            // NaN
            f16.raw_bits = sign | 0x7C00 | (mantissa >> 13);  // Preserve some NaN payload bits
        }
    }
    // Normalized numbers
    else {
        // Convert exponent from float32 to float16
        int16_t new_exp = (int16_t)(exponent - 127 + 15);

        // Check for overflow/underflow
        if (new_exp >= 0x1F) {
            // Overflow: Represent as infinity
            f16.raw_bits = sign | 0x7C00;
        }
        else if (new_exp <= 0) {
            // Underflow: Represent as zero or denormalized number
            if (new_exp < -10) {
                // Too small to be represented as a denormal, set to zero
                f16.raw_bits = sign;
            }
            else {
                // Convert to denormalized number (adjust mantissa)
                mantissa = (mantissa | 0x00800000) >> (1 - new_exp);
                f16.raw_bits = sign | (mantissa >> 13);
            }
        }
        else {
            // Normalized float16 value
            f16.raw_bits = sign | (new_exp << 10) | (mantissa >> 13);
        }
    }

    return f16;
}

float brake[4][1500];
float brakeA[4][1500];
byte writeWheelDataFlag = 0;
char carsId[30000][100];
char classesId[30000][100];
float currentBb = 0;
float setupPressure = 1;

void carsList() {
    long length;

    FILE* file;
    FILE* file1;
    char* filename = "brake_pressure_folder\\cars_id.txt";
    char* filenameClasses = "brake_pressure_folder\\classes_id.txt";

    errno_t err = fopen_s(&file, filename, "rb");

    if (err != 0) {
        char folderNameCars[] = "brake_pressure_folder"; // Folder name
        char folderNameData[] = "brake_pressure_folder\\data"; // Folder name

        // Create the directory (folder)
        if (_mkdir(folderNameCars) == -1) {
            if (errno == EEXIST) {
                printf("Folder '%s' already exists.\n", folderNameCars);
            }
            else {
                //printf("Error creating folder: %s\n", strerror(errno));
                return 1;
            }
        }
        else {
            printf("Folder '%s' created successfully.\n", folderNameCars);
        }
        // Create the directory (folder)
        if (_mkdir(folderNameData) == -1) {
            if (errno == EEXIST) {
                printf("Folder '%s' already exists.\n", folderNameData);
            }
            else {
                //printf("Error creating folder: %s\n", strerror(errno));
                return 1;
            }
        }
        else {
            printf("Folder '%s' created successfully.\n", folderNameData);
        }

        // Create the file path
        //snprintf(filePath, sizeof(filePath), "%s\\cars.txt", folderName);
        printf("File does not exist. Creating and writing data to the file.\n");
        char* str = "252 Aquila CR1 Sports GT\n258 134 Judd V8\n1686 Saleen S7R\n1694 Canhard R51\n1695 BMW Alpina B6 GT3\n1700 BMW 320 Turbo\n1733 BMW M3 GT2\n1738 BMW Z4 GT3\n1741 Canhard R52\n1744 Carlsson C25 GT\n1747 Chevrolet Corvette C6R GT2\n1750 Cougar C14-1\n1753 Cougar C14-2\n1756 DMD P20\n1759 DMD P21\n1779 Ford GT GT1\n1782 Ford Mustang GT3\n1788 Gumpert Apollo Sport\n1794 Koenigsegg CCGT\n1797 McLaren-Mercedes SLR 722 GT\n1800 Mistral M530\n1803 Mistral M531\n1807 Nissan GT-R GT1\n1811 Nissan GT-R GT3\n1815 Nissan Silvia Turbo\n1818 Nissan Skyline 2000RS\n1821 P4-5 Competizione GT3\n1824 Pagani Zonda R\n1827 Radical SR9 AER\n1831 Radical SR9 Judd\n1834 RUF CTR3\n1837 RUF RT12R\n1862 BMW 635 CSI\n1909 McLaren MP4-12C GT3\n2037 Audi R8 LMS Ultra\n2044 Zakspeed Capri\n2116 BMW M3 E30 Gr.A\n2275 Volvo 240 Turbo\n2333 Chevrolet Daytona Prototype\n2338 Chevrolet Camaro GT3\n2339 BMW M1 Procar\n2405 Audi 90 Quattro\n2409 Ford GT GT3\n2410 Cadillac CTS-V.R\n2413 DTM Mercedes AMG C-Coupé\n2417 BMW M3 DTM \n2420 Audi RS 5 DTM\n2533 Mercedes-Benz SLS AMG GT3\n2611 Chevrolet Corvette Z06.R GT3\n2705 BMW E90 320 TC\n2728 Honda Civic WTCC\n2804 SEAT León WTCC\n2842 Lada Granta WTCC\n2846 Chevrolet Cruze WTCC\n2849 Carlsson SLK 340 JUDD\n2859 Fabcar 935\n2868 Nissan 300ZX Z32\n2923 Audi R8 LMS Ultra\n2924 BMW Z4 GT3\n2925 Chevrolet Camaro GT3\n2926 Nissan GT-R GT3\n2927 Mercedes-Benz SLS AMG GT3\n3380 Audi RS 5 DTM 2014\n3384 Mercedes-Benz SLS AMG GT3\n3408 BMW M4 DTM\n3479 DTM Mercedes AMG C Coupe 14\n3500 Audi R8 LMS Ultra\n3508 Chevrolet Corvette Z06.R GT3\n3516 RUF RT12R GTR3\n3527 McLaren MP4-12C GT3\n3530 BMW Z4 GT3\n3534 Chevrolet Camaro GT3\n3539 Mercedes 190E Evo II DTM\n3549 BMW M3 Sport Evolution\n3626 Chevrolet Corvette Z06.R GT3\n3662 Audi V8 DTM\n3754 Ford GT GT3\n3842 Ford Mustang GT DTM\n3874 Opel Omega 3000 Evo500\n3899 Citroen C Elysee WTCC\n3964 Nissan Skyline GTR R32 Group N\n4046 Chevrolet RML Cruze TC1\n4067 LADA Granta 1.6T\n4075 Chevrolet Corvette Greenwood 1977\n4114 Nissan R90CK\n4145 Chevrolet Dekon Monza\n4197 Honda Civic WTCC 2014\n4261 Audi RS 5 DTM 2015\n4264 Mercedes-AMG C63 DTM\n4267 BMW M4 DTM 2015\n4299 Audi TT RS VLN\n4308 Audi R18\n4386 Ford Mustang IMSA GTO\n4518 Audi R8 LMS Ultra\n4544 BMW Z4 GT3\n4551 Mercedes-Benz SLS AMG GT3\n4554 Honda Civic WTCC 2015\n4557 Nissan GT-R GT3\n4570 Chevrolet Corvette Z06.R GT3 GTM15\n4579 Chevrolet RML Cruze TC1 2015\n4582 Chevrolet Camaro GT3\n4585 Citroen C Elysee WTCC 2015\n4598 Formula RaceRoom 2\n4675 Porsche 911 GT3 Cup (991.2) Endurance\n4681 Audi TT cup 2015\n4720 Lada Vesta WTCC 2015\n4810 NSU TTS\n4865 Formula RaceRoom Junior\n5051 Tatuus F4\n5152 Bentley Continental GT3\n5170 Bentley Continental GT3\n5214 P4-5 Competizione GT2\n5259 Mercedes-AMG C 63 DTM\n5342 KTM X-Bow RR\n5348 Formula RaceRoom US\n5396 Formula RaceRoom 3\n5399 BMW M4 DTM 2016\n5402 Mercedes-AMG C 63 DTM 2016\n5411 Audi RS 5 DTM 2016\n5588 Audi TT cup 2016\n5767 KTM X-Bow GT4\n5786 McLaren 650S GT3\n5818 BMW M6 GT3\n5821 Formula Raceroom X-17\n6002 Volvo S60 Polestar TC1\n6011 Chevrolet RML Cruze TC1 2016\n6017 Citroen C Elysee WTCC 2016\n6024 Honda Civic WTCC 2016\n6031 Lada Vesta WTCC 2016\n6057 Audi R8 LMS\n6168 Mercedes AMG GT3\n6174 Callaway Corvette C7 GT3-R\n6177 Chevrolet RML Cruze eSports\n6187 Citroen C Elysee eSports\n6195 Honda Civic eSports\n6203 Lada Vesta eSports\n6208 Volvo S60 Polestar eSports\n6262 BMW M235i Racing\n6310 Chevrolet RML Cruze TC1 2017\n6314 Citroen C Elysee WTCC 2017\n6321 Honda Civic WTCC 2017\n6329 Lada Vesta WTCC 2017\n6334 Volvo S60 Polestar TC1\n6349 Porsche 911 GT3 R\n6568 Porsche Cayman GT4 Clubsport\n6588 Audi RS 3 LMS\n6623 Porsche Cayman GT4 CS MR\n6791 AMG-Mercedes CLK DTM 2003\n6849 AMG-Mercedes C-Klasse DTM 2005\n6860 AMG-Mercedes 190 E 2.5-16 Evolution II 1992\n6949 Mercedes-AMG C 63 DTM 2015\n6978 AMG-Mercedes C-Klasse DTM 1995\n7005 CUPRA TCR\n7011 Audi RS 3 LMS\n7029 Alfa Romeo Giulietta TCR\n7036 Peugeot 308 TCR\n7040 Porsche Cayman GT4 Clubsport\n7076 AMG-Mercedes C-Klasse DTM\n7105 Volkswagen Golf GTI TCR\n7117 Hyundai i30 N TCR\n7125 Honda Civic TCR\n7162 Lotus Evora GT4\n7169 AMG-Mercedes CLK DTM\n7183 AMG-Mercedes C-Klasse DTM\n7215 Formula RR 90 V8\n7218 Formula RR 90 V12\n7220 Formula RR 90 V10\n7282 Porsche 934 Turbo RSR\n7396 Porsche 911 Carrera Cup (964)\n7399 Porsche 962 C Team Joest\n7629 BMW M6 GT3\n7630 Audi R8 LMS\n7640 Callaway Corvette C7 GT3-R\n7643 Mercedes AMG GT3\n7647 Porsche 911 GT3 R\n7805 BMW M1 Gr. 4\n7810 Volkswagen ID. R\n7845 Alfa Romeo Giulietta TCR\n7847 Audi RS 3 LMS\n7850 CUPRA TCR\n7853 Honda Civic TCR\n7856 Hyundai i30 N TCR\n7859 Volkswagen Golf GTI TCR\n7926 Lynk & Co 03 TCR\n7983 Porsche 911 GT3 Cup\n8006 CUPRA TCR\n8053 BMW M4 GT4 (F82)\n8115 E36 V8 JUDD\n8149 Opel Astra\n8166 Porsche 911 GT3 Cup (991.2)\n8201 Porsche 718 Cayman GT4 Clubsport\n8242 Porsche 911 GT2 RS Clubsport\n8257 Porsche 911 GT3 R (2019)\n8467 Honda Civic Type R\n8487 Porsche 911 RSR 2019\n8498 Volkswagen Scirocco Gr2\n8593 Bentley Continental GT3 EVO\n8604 Audi R8 LMS GT3 EVO\n8652 Audi R8 LMS GT2 2019\n8657 Audi RS 3 LMS\n8661 Hyundai i30 N TCR\n8677 Shopping Cart\n8698 Lada Vesta\n8715 Hyundai i30 N TCR\n8719 Lynk & Co 03 TCR\n8726 CUPRA Leon Competición\n8757 CUPRA Leon e-Racer\n8776 Honda Civic TCR\n8782 Alfa Romeo Giulietta TCR\n8785 Peugeot 308\n8788 CUPRA Leon Competición\n8792 Porsche 911 GT3 Cup (991.2)\n8900 Audi R8 LMS GT4 2020\n8936 BMW M6 GT3\n8938 Porsche 911 GT3 R (2019)\n8940 Mercedes AMG GT3\n8942 Callaway Corvette C7 GT3-R\n9031 Audi R8 LMS\n9094 BMW M4 DTM 2020e\n9102 Audi RS 5 DTM 2020e\n9154 Lynk & Co 03 TCR\n9206 Audi RS 5 DTM 2020\n9220 BMW M4 DTM 2020\n9234 Hyundai i30 N TCR\n9241 Lynk & Co 03 TCR\n9252 Alfa Romeo Giulietta TCR\n9256 Audi RS 3 LMS\n9266 Honda Civic TCR\n9278 CUPRA Leon Competición\n9467 Renault Mégane RS TCR\n9548 Audi R8 LMS GT3 EVO\n9565 Bentley Continental GT3 EVO\n9575 McLaren 720S GT3\n9581 Mercedes-AMG GT3 2020\n9603 Mercedes-AMG GT3 2020\n9644 Mercedes AMG GT4 2020\n9660 KTM X-BOW GTX\n9761 BMW E30 M3 Drift\n9986 RaceRoom Truck\n10018 Formula Raceroom X-22\n10051 Audi R8 LMS GT3 EVO\n10076 BMW M6 GT3\n10085 Bentley Continental GT3 EVO\n10110 Mercedes-AMG GT3 2020\n10119 Porsche 911 GT3 R (2019)\n10127 Callaway Corvette C7 GT3-R\n10263 Ford Mustang Mach-E 1400\n10345 Audi RS 3 LMS 2021\n10368 Lynk & Co 03 TCR\n10371 CUPRA Leon Competición\n10387 Ferrari 488 GT3 EVO 2020 DTM\n10397 Hyundai Elantra TCR 2021\n10401 Honda Civic TCR\n10441 Honda Civic TCR\n10541 Audi R8 LMS GT3 EVO DTM\n10546 BMW M6 GT3 DTM\n10552 McLaren 720S GT3 DTM\n10556 Mercedes-AMG GT3 2020 DTM\n10564 Porsche 911 GT3 R (2019) DTM\n10578 Audi RS 3 LMS\n10599 BMW M2 CS Racing 2021\n10674 Lada Vesta TCR\n10721 S.C. Cupra TCR\n10744 McLaren 570s GT4\n10787 Audi R8 LMS GT3 EVO\n10790 Bentley Continental GT3 EVO\n10795 BMW M6 GT3\n10804 McLaren 720S GT3\n10806 Mercedes-AMG GT3 2020\n10808 Porsche 911 GT3 R (2019)\n10890 Ferrari 488 GT3 EVO 2020\n10896 Crosslé 90F\n10914 Mazda MX-5 CUP 2019 ND2\n10918 Audi R8 LMS GT3 EVO eDTM\n10920 BMW M6 GT3 eDTM\n10924 Ferrari 488 GT3 EVO 2020 eDTM\n10945 Mercedes-AMG GT3 2020 eDTM\n10950 Porsche 911 GT3 R (2019) eDTM\n11025 Callaway Corvette C7 GT3-R\n11051 Praga R1\n11285 Crosslé 9S\n11318 Hyundai Elantra TCR 2022\n11320 Honda Civic TCR\n11323 CUPRA Leon Competición\n11325 Audi RS 3 LMS 2021\n11328 Lynk & Co 03 TCR\n11342 Mazda RT24 P DPi\n11474 Formula Raceroom X-22 Esports\n11536 BMW M4 GT3\n11561 Porsche 944 Turbo Cup\n11567 Audi R8 LMS GT3 EVO\n11586 BMW M6 GT3\n11589 Mercedes-AMG GT3 2020\n11597 Porsche 911 GT3 R (2019)\n11917 Audi RS 3 LMS 2021\n11922 Hyundai Elantra TCR 2022\n11991 KTM X-BOW GT2\n12012 Porsche 911 GT3 Cup (992) Endurance\n12090 Porsche 911 GT3 R (992)\n12163 Porsche 911 GT3 Cup (992)\n12197 Porsche 911 GT3 R (992) DTM\n12237 BMW M4 GT3 DTM\n12240 Mercedes-AMG GT3 2020 DTM\n12319 Audi R8 LMS GT3 EVO II\n12352 Audi R8 LMS GT3 EVO II DTM\n12405 Ferrari 296 GT3\n12421 Ferrari 296 GT3 DTM\n12430 BMW M8 GTE\n12509 BMW M4 GT4 (G82)\n12537 S.C. Audi R8 LMS\n12788 Audi R8 LMS GT3 EVO II DTM\n12790 BMW M4 GT3 DTM\n12792 Ferrari 296 GT3 DTM\n12795 Lamborghini Huracán GT3 EVO II DTM\n12800 Mercedes-AMG GT3 2020 DTM\n12807 Porsche 911 GT3 R (992) DTM\n12815 McLaren 720S GT3 EVO DTM\n12839 Opel Calibra V6 DTM\n12850 Alfa Romeo 155 TI DTM 1995\n";
        err = fopen_s(&file, filename, "wb");
        if (err == 0) {
            fwrite(str, sizeof(char), strlen(str), file);
            fclose(file);
        }
        else {
            printf("Error opening file!\n");
        }
        char* strC = "253 FRJ Cup\n255 Aquila CR1 Cup\n1685 Hillclimb Icons\n1687 GTR 1\n1703 GTR 3\n1704 GTR 2\n1706 German Nationals\n1708 Group 5\n1711 Sideways\n1712 Touring Classics\n1713 GTO Classics\n1714 P1\n1717 Silhouette Series\n1921 DTM 2013\n1922 WTCC 2013\n1923 P2\n2378 Procar\n2922 ADAC GT Masters 2013\n3086 DTM 2014\n3375 ADAC GT Masters 2014\n3499 DTM 1992\n3905 WTCC 2014\n4121 Group C\n4260 DTM 2015\n4516 ADAC GT Masters 2015\n4517 WTCC 2015\n4597 FR2 Cup\n4680 Audi Sport TT Cup 2015\n4813 NSU TTS Cup\n4867 Tatuus F4 Cup\n5234 Audi TT RS cup\n5262 DTM 2016\n5383 FR US Cup\n5385 KTM X-Bow RR Cup\n5634 Mercedes-AMG Motorsport\n5652 FR3 Cup\n5726 Audi Sport TT Cup 2016\n5824 FR X-17 Cup\n5825 GTR 4\n6036 WTCC 2016\n6172 RaceRoom Esports\n6309 WTCC 2017\n6344 BMW M235i Racing Cup\n6345 Porsche 991.2 GT3 Cup\n6648 Cayman GT4 Trophy by Manthey-Racing\n6783 Esports WTCR Prologue\n6794 Mercedes-AMG Motorsport 30 Years of DTM\n7009 WTCR 2018\n7041 Super Racer\n7075 DTM 1995\n7110 Zonda R Cup\n7167 CLK DTM 2003\n7168 C-Klasse DTM 2005\n7214 FR X-90 Cup\n7278 ADAC GT Masters 2018\n7287 Porsche 964 Cup\n7304 Group 4\n7765 Volkswagen ID. R\n7767 ADAC GT Masters 2020\n7844 WTCR 2019\n7982 Porsche Carrera Cup Deutschland 2019\n8165 Porsche Carrera Cup Scandinavia\n8248 GT2\n8483 Group 2\n8600 GTE\n8656 eSports WTCR\n8660 Touring Cars Cup\n8680 Shopping Cart\n8681 Porsche Carrera Cup Deutschland 2020\n8682 CUPRA Leon e-Racer\n9101 DTM 2020 Esport\n9205 DTM 2020\n9233 WTCR 2020\n9989 Truck Racing\n10049 ADAC Esports GT Masters 2021\n10050 FR X-22 Cup\n10266 Ford Mustang Mach E\n10344 WTCR 2021\n10396 DTM 2021\n10743 Safety Cars\n10786 ADAC Esports GT Masters\n10899 Crosslé 90F\n10909 BMW M2 Cup\n10917 DTM Esports 2022\n10977 Mazda MX-5 Cup\n11055 Praga R1\n11317 WTCR 2022\n11485 FRX-22 Esports\n11564 Porsche 944 Turbo Cup\n11566 ADAC GT Masters 2021\n11844 Crosslé 9S\n11990 KTM GTX\n12003 Mazda Dpi\n12015 Porsche Carrera Cup Deutschland 2023\n12196 DTM 2023\n12302 Porsche 992 GT3 Cup\n12770 DTM 2024\n";
        err = fopen_s(&file1, filenameClasses, "wb");
        if (err == 0) {
            fwrite(strC, sizeof(char), strlen(strC), file1);
            fclose(file1);
        }
        else {
            printf("Error opening file!\n");
        }

    }
    err = fopen_s(&file, filename, "rb");
    if (err != 0) {
        return;
    }

    char line[100];  // Buffer for reading lines
    while (fgets(line, sizeof(line), file) != NULL) {
        // Remove trailing newline if present
        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\n') {
            line[len - 1] = '\0';
        }

        // Split the line into number and string
        char* context = NULL;
        char* numberPart = strtok_s(line, " ", &context);
        char* stringPart = strtok_s(NULL, "", &context);

        if (numberPart && stringPart) {
            /*printf("Number: %s\n", numberPart);
            printf("String: %s\n", stringPart);*/
            //strncpy(carsId[atoi(numberPart)], stringPart, 100 - 1);
            strncpy_s(carsId[atoi(numberPart)], 100, stringPart, _TRUNCATE);
            // Ensure null termination
            carsId[atoi(numberPart)][100 - 1] = '\0';
        }
        else {
            printf("Invalid line format.\n");
        }
    }

    fclose(file);
    err = fopen_s(&file1, filenameClasses, "rb");
    if (err != 0) {
        return;
    }

    while (fgets(line, sizeof(line), file1) != NULL) {
        // Remove trailing newline if present
        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\n') {
            line[len - 1] = '\0';
        }

        // Split the line into number and string
        char* context = NULL;
        char* numberPart = strtok_s(line, " ", &context);
        char* stringPart = strtok_s(NULL, "", &context);

        if (numberPart && stringPart) {
            /*printf("Number: %s\n", numberPart);
            printf("String: %s\n", stringPart);*/
            //strncpy(carsId[atoi(numberPart)], stringPart, 100 - 1);
            strncpy_s(classesId[atoi(numberPart)], 100, stringPart, _TRUNCATE);
            // Ensure null termination
            classesId[atoi(numberPart)][100 - 1] = '\0';
        }
        else {
            printf("Invalid line format.\n");
        }
    }

    fclose(file1);
   /* for (int i = 0; i < count; i++) {
        printf("%s\n", carsId[i]);
    }*/
}

void setBb() {
    int optt;
    if (map_buffer->brake_temp[0].optimal_temp < map_buffer->brake_temp[2].optimal_temp)
        optt = map_buffer->brake_temp[0].optimal_temp;
    else
        optt = map_buffer->brake_temp[2].optimal_temp;

    float f = 2 * (1 - currentBb);
    float r = 2 * currentBb;
    //brake[0][optt];

    for (int j = 0; j < 1500; j++) {
        brakeA[0][j] = brake[0][j] * f * setupPressure;
        brakeA[1][j] = brake[1][j] * f * setupPressure;
        brakeA[2][j] = brake[2][j] * r * setupPressure;
        brakeA[3][j] = brake[3][j] * r * setupPressure;
    }
    /*printf("%f ", brakeA[0][optt] + brakeA[1][optt] + brakeA[2][optt] + brakeA[3][optt]);
    printf("%f ", brakeA[0][optt]);
    printf("%f ", brakeA[1][optt]);
    printf("%f ", brakeA[2][optt]);
    printf("%f\n", brakeA[3][optt]);*/
}

DWORD WINAPI writeData(LPVOID lpParam) {
    if (currentBb != 0.5f) {
        printf("Set the break bias to 50%% and try again.");
        return;
    }
    if (map_buffer->aid_settings.abs != 0) {
        printf("Set the abs assist off and try again.");
        return;
    }
    //memset(brake, 0, sizeof(brake));
    printf("record started\n");
    writeWheelDataFlag = 1;
    float p;
    int tmp;
    while (writeWheelDataFlag) {
        if (map_buffer) {
            //printf("%d\n", counti++);
            if (map_buffer->brake_raw > 0.1f) {
                p = map_buffer->brake_pressure[0] / map_buffer->brake_raw;
                tmp = map_buffer->brake_temp[0].current_temp;
                if (brake[0][tmp] == 0 && tmp > 0 && tmp < 1500 && !isnan(p) && !isinf(p))
                    brake[0][tmp] = p;

                p = map_buffer->brake_pressure[1] / map_buffer->brake_raw;
                tmp = map_buffer->brake_temp[1].current_temp;
                if (brake[1][tmp] == 0 && tmp > 0 && tmp < 1500 && !isnan(p) && !isinf(p))
                    brake[1][tmp] = p;

                p = map_buffer->brake_pressure[2] / map_buffer->brake_raw;
                tmp = map_buffer->brake_temp[2].current_temp;
                if (brake[2][tmp] == 0 && tmp > 0 && tmp < 1500 && !isnan(p) && !isinf(p))
                    brake[2][tmp] = p;

                p = map_buffer->brake_pressure[3] / map_buffer->brake_raw;
                tmp = map_buffer->brake_temp[3].current_temp;
                if (brake[3][tmp] == 0 && tmp > 0 && tmp < 1500 && !isnan(p) && !isinf(p))
                    brake[3][tmp] = p;
            }
        }
        Sleep(1);
    }
    char folderNameData[] = "brake_pressure_folder\\data\\carclass"; // Folder name
    char* pos1 = strstr(folderNameData, "carclass");
    char newPathC[500];

    if (pos1) {
        // Calculate the length before and after the word "brake"
        size_t lenBefore = pos1 - folderNameData;
        size_t lenAfter = strlen(pos1 + strlen("carclass"));

        // Construct the new path
        snprintf(newPathC, sizeof(newPathC), "%.*s%s%s",
            (int)lenBefore, folderNameData, classesId[map_buffer->vehicle_info.class_id], pos1 + strlen("carclass"));

        // Output the new path
        //printf("New path: %s\n", newPathC);
    }
    else {
        printf("The word 'brake' was not found in the path.\n");
    }
    // Create the directory (folder)
    if (_mkdir(newPathC) == -1) {
        if (errno == EEXIST) {
            printf("Folder '%s' already exists.\n", newPathC);
        }
        else {
            //printf("Error creating folder: %s\n", strerror(errno));
            return 1;
        }
    }
    else {
        printf("Folder '%s' created successfully.\n", newPathC);
    }
    const char* stringToAdd = "\\carname.txt";

    // Check if there is enough space to append the new string
    if (strlen(newPathC) + strlen(stringToAdd) + 1 < MAX_PATH) {
        // Use strcat_s to safely append the string
        strcat_s(newPathC, MAX_PATH, stringToAdd);
    }
    else {
        printf("Not enough space to append the string.\n");
    }
    //char path[] = "brake_pressure_folder\\data\\carclass\\carname.txt";
    char* pos = strstr(newPathC, "carname");
    char newPath[500];

    if (pos) {
        // Calculate the length before and after the word "brake"
        size_t lenBefore = pos - newPathC;
        size_t lenAfter = strlen(pos + strlen("carname"));

        // Construct the new path
        snprintf(newPath, sizeof(newPath), "%.*s%s%s",
            (int)lenBefore, newPathC, carsId[map_buffer->vehicle_info.model_id], pos + strlen("carname"));

        // Output the new path
        //printf("New path: %s\n", newPath);
    }
    else {
        printf("The word 'brake' was not found in the path.\n");
    }
    FILE* file;
    //errno_t err = fopen_s(&file, "brake_pressure_folder\\data\\brake.txt", "w");  // Use fopen_s
    errno_t err = fopen_s(&file, newPath, "w");  // Use fopen_s

    if (err != 0 || file == NULL) {
        printf("Error opening file for writing.\n");
        return;
    }

    for (int j = 0; j < 1500; j++) {
        for (int i = 0; i < 4; i++) {
            fprintf(file, "%f ", brake[i][j]);
        }
        fprintf(file, "\n");
    }

    fclose(file);
    printf("done %s\n", newPath);

    for (int j = 20; j < 1500; j++) {

        if (brake[0][j] == 0 || brake[1][j] == 0 || brake[2][j] == 0 || brake[3][j] == 0) {
            printf("%d temperature missed for:", j);
        }
        if (brake[0][j] == 0) {
            printf(" front left ");
        }
        if (brake[1][j] == 0) {
            printf(" front right ");
        }
        if (brake[2][j] == 0) {
            printf(" rear left ");
        }
        if (brake[3][j] == 0) {
            printf(" rear right ");
        }
        if (brake[0][j] == 0 || brake[1][j] == 0 || brake[2][j] == 0 || brake[3][j] == 0) {
            printf("\n");
        }
    }

    setBb();
    return 0;
}

DWORD WINAPI read_input(LPVOID lpParam) {
    carsList();
    char input[100];

    printf("Thread is waiting for input...\n");
    while (1) {
        fgets(input, sizeof(input), stdin);

        input[strcspn(input, "\n")] = 0;

        // Command recognition logic
        if (strcmp(input, "record") == 0 || strcmp(input, "r") == 0) {
            HANDLE thread;
            DWORD threadId;

            // Create a new thread
            thread = CreateThread(
                NULL,               // Default security attributes
                0,                  // Default stack size
                writeData,         // Thread function
                NULL,               // No arguments to thread function
                0,                  // Default creation flags
                &threadId);         // Thread identifier

            // Check if thread creation was successful
            if (thread == NULL) {
                printf("Error creating thread\n");
                return 1;
            }
        }
        else if (strcmp(input, "stop") == 0 || strcmp(input, "s") == 0) {
            writeWheelDataFlag = 0;
        }
        else {
            char* endptr;
            int number = strtol(input, &endptr, 10);

            if (*endptr == '\0') {
                if (number < 80 || number > 100) {
                    printf("Incorrect brake pressure\n");
                    continue;
                }
                printf("Brake pressure set to %d%%\n", number);
                setupPressure = (float)number / 100;
                setBb();
            }
            else {
                printf("Unknown command: %s\n", input);
            }
        }

        
    }
    // Read input from console
    //fgets(input, sizeof(input), stdin);

    // Print the input
    printf("You entered: %s\n", input);

    return 0;
}

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
unsigned char chrsRadar[200];
struct KeyValue radarPlayers[R3E_NUM_DRIVERS_MAX];
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
    //_beginthreadex(NULL, 0, WindowThread, NULL, 0, NULL);

    

    // Initialize the critical section
    //InitializeCriticalSection(&cs);

    //// Step 1: Create a new thread to run the GUI
    //hThread = (HANDLE)_beginthreadex(NULL, 0, &RunGuiThread, NULL, 0, NULL);
    //if (hThread == NULL) {
    //    printf("Failed to create GUI thread\n");
    //    return -1;
    //}

    //printf("GUI is running in a separate thread...\n");

    // Step 2: Main function can continue doing other work without being blocked
    //Sleep(1000);  // Wait for GUI to initialize

    //// Step 3: Add some lines dynamically
    //AddLine(50, 50, 200, 50, RGB(255, 0, 0));  // Red line
    //AddLine(200, 50, 200, 200, RGB(0, 255, 0));  // Green line
    //AddLine(200, 200, 50, 200, RGB(0, 0, 255));  // Blue line
    //AddLine(50, 200, 50, 50, RGB(255, 255, 0));  // Yellow line

    //// Step 4: Trigger a redraw to show the lines
    //TriggerRedraw();

    //// Simulate dynamic updates by adding more lines
    //Sleep(2000);
    //AddLine(100, 100, 300, 300, RGB(128, 0, 128));  // Purple diagonal line
    //TriggerRedraw();

    HANDLE thread;
    DWORD threadId;

    // Create a new thread
    thread = CreateThread(
        NULL,               // Default security attributes
        0,                  // Default stack size
        read_input,         // Thread function
        NULL,               // No arguments to thread function
        0,                  // Default creation flags
        &threadId);         // Thread identifier

    // Check if thread creation was successful
    if (thread == NULL) {
        printf("Error creating thread\n");
        return 1;
    }

    //Beep(440, 5000);
    init();
    initSettings();
    lapsAndFuelData.allBestLap = 9999;
    lapsAndFuelData.allBestFuel = 9999;
    lapsAndFuelData.leaderBestLap = 9999;
    lapsAndFuelData.currentBestLap = 9999;
    lapsAndFuelData.previousLap = 9999;
    playerRatingInfoDb.size++;
    playerRatingInfoDb.player[0].id = 1085106;
    playerRatingInfoDb.player[0].name = "s. anti";
    playerRatingInfoDb.player[0].rating = 1862;
    playerRatingInfoDb.player[0].reputation = 88;
    playerRatingInfoSended.ids[0] = 1085106;
    playerRatingInfoSended.size++;
    //BlobResult res = readWindowsSettings();
    //unsigned char bytes_array[] = { 0x12, 0x34, 0x56, 0x78 };
    //unsigned char bytes_array1[10] = { 'h', 'e', 'l', 'l', 'o','h', 'e', 'l', 'l', 'o' };
    //writeWindowsSettings(bytes_array1, sizeof(bytes_array1) / sizeof(bytes_array1[0]));
    for (int i = 0; i < 60; ++i) {
        radarPlayers[i].key = INT_MAX;
        radarPlayers[i].value = INT_MAX;
    }
    chrsRadar[2] = 111;
    chrsRadar[0] = 130;
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

            for (int i = 0; i < R3E_NUM_DRIVERS_MAX; i++) {
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
            //printf("%d\n", map_buffer->layout_id);
            //printf("%d\n", map_buffer->layout_id);
            /*printf("%d, %d\n", map_buffer->session_type, currentState);*/
            /*printf("%d\n", map_buffer->vehicle_info.model_id);*/
            /*if (currentState != map_buffer->session_type) {
                if (lastpr1 != currentState || lastpr2 != map_buffer->session_type) {
                    printf("\n%d, %d, %d\n", currentState, map_buffer->session_type, map_buffer->vehicle_info.model_id);
                    lastpr1 = currentState;
                    lastpr2 = map_buffer->session_type;
                }
            }*/
            if (map_buffer->session_type != -1) {
                if (currentState == -2) {
                    //printf("\n%d\n", map_buffer->vehicle_info.model_id);
                    currentBb = map_buffer->brake_bias;
                    memset(brake, 0, sizeof(brake));
                    char folderNameData[] = "brake_pressure_folder\\data\\carclass"; // Folder name
                    char* pos1 = strstr(folderNameData, "carclass");
                    char newPathC[500];

                    if (pos1) {
                        // Calculate the length before and after the word "brake"
                        size_t lenBefore = pos1 - folderNameData;
                        size_t lenAfter = strlen(pos1 + strlen("carclass"));

                        // Construct the new path
                        snprintf(newPathC, sizeof(newPathC), "%.*s%s%s",
                            (int)lenBefore, folderNameData, classesId[map_buffer->vehicle_info.class_id], pos1 + strlen("carclass"));

                        // Output the new path
                        //printf("New path: %s\n", newPathC);
                    }
                    else {
                        printf("The word 'brake' was not found in the path.\n");
                    }
                    
                    const char* stringToAdd = "\\carname.txt";

                    // Check if there is enough space to append the new string
                    if (strlen(newPathC) + strlen(stringToAdd) + 1 < MAX_PATH) {
                        // Use strcat_s to safely append the string
                        strcat_s(newPathC, MAX_PATH, stringToAdd);
                    }
                    else {
                        //printf("Not enough space to append the string.\n");
                    }

                    char* pos = strstr(newPathC, "carname");
                    char newPath[500];

                    if (pos) {
                        // Calculate the length before and after the word "brake"
                        size_t lenBefore = pos - newPathC;
                        size_t lenAfter = strlen(pos + strlen("carname"));

                        // Construct the new path
                        snprintf(newPath, sizeof(newPath), "%.*s%s%s",
                            (int)lenBefore, newPathC, carsId[map_buffer->vehicle_info.model_id], pos + strlen("carname"));

                        // Output the new path
                        //printf("New path: %s\n", newPath);
                    }
                    else {
                        printf("The word 'brake' was not found in the path.\n");
                    }
                    FILE* file;
                    //errno_t err = fopen_s(&file, "brake_pressure_folder\\data\\brake.txt", "w");  // Use fopen_s
                    errno_t err = fopen_s(&file, newPath, "r");  // Use fopen_s

                    if (err != 0 || file == NULL) {
                        if (map_buffer->aid_settings.abs > 0)
                            printf("No brake pressure data for the car. ABS graph no available. %s\n", newPath);
                        goto skipbb;
                    }

                    // Read the data from the file and restore it into the array
                    for (int j = 0; j < 1500; j++) {
                        for (int i = 0; i < 4; i++) {
                            if (fscanf_s(file, "%f", &brake[i][j]) != 1) {
                                printf("Error reading value at [%d][%d]\n", i, j);
                                fclose(file);
                                return EXIT_FAILURE;
                            }
                        }
                    }

                    fclose(file);

                    setBb();

                    // Example: Print restored array to verify
                    /*for (int j = 0; j < 1500; j++) {
                        for (int i = 0; i < 4; i++) {
                            printf("%f ", brake[i][j]);
                        }
                        printf("\n");
                    }*/
                    skipbb:
                    //load from db
                    readBestLapFuel(&lapsAndFuelData, map_buffer->layout_id, map_buffer->vehicle_info.model_id);
                    //wprintf_s(L"bestloaded\n");
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
            else {
                if (currentState != -2) {
                    currentState = map_buffer->session_type;
                }
            }
        }
    }

    map_close();

    wprintf_s(L"All done!");
    system("PAUSE");

    return 0;
}
void resetSort() {
    for (int i = 0; i < R3E_NUM_DRIVERS_MAX; i++) {
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
        fscanf_s(file, "http port = %9s\n", &settings.http_port, (unsigned)_countof(settings.http_port));
        fscanf_s(file, "websocket port = %9s\n", &settings.wesocket_port, (unsigned)_countof(settings.wesocket_port));
        fscanf_s(file, "last lap = %d\n", &settings.last_lap);
        fscanf_s(file, "best lap = %d\n", &settings.best_lap);
        fscanf_s(file, "best lap leader = %d\n", &settings.best_lap_leader);
        fscanf_s(file, "delta = %d\n", &settings.delta);
        fscanf_s(file, "radar = %d\n", &settings.radar);
        fscanf_s(file, "all time best table = %d\n", &settings.all_time_best_table);
        fscanf_s(file, "relative table = %d\n", &settings.relative_table);
        fscanf_s(file, "wheels = %d\n", &settings.wheels);
        fscanf_s(file, "absgrip graph = %d\n", &settings.absgrip_graph);
        fscanf_s(file, "input graph = %d\n", &settings.input_graph);
        fscanf_s(file, "slip graph = %d\n", &settings.slip_graph);
        fscanf_s(file, "input percents = %d\n", &settings.input_percents);
        fscanf_s(file, "standings = %d\n", &settings.standings);
        fscanf_s(file, "starting lights = %d\n", &settings.startingLights);
        fscanf_s(file, "dev = %d\n", &settings.dev);
        fclose(file);
    } else {
        settings = (Settings){ "8081", "8082", 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0 };
        FILE* file;
        errno_t err = fopen_s(&file, "settings.txt", "w");
        if (err == 0) {
            fprintf(file, "http port = %s\n", settings.http_port);
            fprintf(file, "websocket port = %s\n", settings.wesocket_port);
            fprintf(file, "last lap = %d\n", settings.last_lap);
            fprintf(file, "best lap = %d\n", settings.best_lap);
            fprintf(file, "best lap leader = %d\n", settings.best_lap_leader);
            fprintf(file, "delta = %d\n", settings.delta);
            fprintf(file, "radar = %d\n", settings.radar);
            fprintf(file, "all time best table = %d\n", settings.all_time_best_table);
            fprintf(file, "relative table = %d\n", settings.relative_table);
            fprintf(file, "wheels = %d\n", settings.wheels);
            fprintf(file, "absgrip graph = %d\n", settings.absgrip_graph);
            fprintf(file, "input graph = %d\n", settings.input_graph);
            fprintf(file, "slip graph = %d\n", settings.slip_graph);
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
    chrsRadar[1] = 1;
    chrs[1] = 5;
    chrs[2] = 1;
    memcpy(&chrs[3], &map_buffer->time_delta_best_self, sizeof(r3e_float32));
    //memcpy(&chrs[3], &map_buffer->lap_time_delta_leader_class, sizeof(r3e_float32));


    /*printf("(%f,%f,%f)", map_buffer->all_drivers_data_1[1].position.x, map_buffer->all_drivers_data_1[1].position.z, map_buffer->all_drivers_data_1[1].orientation.y);
    printf("(%f,%f,%f)\n", map_buffer->all_drivers_data_1[0].position.x, map_buffer->all_drivers_data_1[0].position.z, map_buffer->all_drivers_data_1[0].orientation.y);*/
    /* printf("(%f,%f)", map_buffer->all_drivers_data_1[1].driver_info.car_width, map_buffer->all_drivers_data_1[1].driver_info.car_length);
        printf("(%f,%f)\n", map_buffer->all_drivers_data_1[0].driver_info.car_width, map_buffer->all_drivers_data_1[0].driver_info.car_length);*/
        //printf("%d ",map_buffer->num_cars);
    if (map_buffer->position <= map_buffer->num_cars) {
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
    }
    else {
        r3e_float32 locx = map_buffer->player.position.x;
        r3e_float32 locz = map_buffer->player.position.z;
        r3e_float32 locy = map_buffer->player.orientation.y;
        r3e_float32 locw = map_buffer->vehicle_info.car_width;
        r3e_float32 locl = map_buffer->vehicle_info.car_length;
        memcpy(&chrs[2 + chrs[1]], &locx, sizeof(r3e_float32));
        chrs[1] += 4;
        memcpy(&chrs[2 + chrs[1]], &locz, sizeof(r3e_float32));
        chrs[1] += 4;
        memcpy(&chrs[2 + chrs[1]], &locy, sizeof(r3e_float32));
        chrs[1] += 4;
        memcpy(&chrs[2 + chrs[1]], &locw, sizeof(r3e_float32));
        chrs[1] += 4;
        memcpy(&chrs[2 + chrs[1]], &locl, sizeof(r3e_float32));
        chrs[1] += 4;
    }
    //r3e_driver_data asd = map_buffer->all_drivers_data_1[map_buffer->position - 1];
    //printf("%d ", map_buffer->position);
    /*for (size_t i = 0; i < 134; i++)
    {
        //printf("%d ", map_buffer->all_drivers_data_1[i].driver_info.user_id);
        if (all_d_[i] != NULL)
            printf("%d ", all_d_[i]->driver_info.user_id);
        /*if (map_buffer->all_drivers_data_1[i].driver_info.user_id == 1085106) {
            printf("%d ", i);
        }*/
    //}
    //printf("wl(%f %f)\n", map_buffer->all_drivers_data_1[map_buffer->position - 1].driver_info.car_width, map_buffer->all_drivers_data_1[map_buffer->position - 1].driver_info.car_length);
    //printf("x(%f %f %f) ", all_d_[12].position.x, map_buffer->player.position.x, map_buffer->all_drivers_data_1[map_buffer->position - 1].position.x - map_buffer->player.position.x);
    /*printf("x(%f %f %f) ", map_buffer->all_drivers_data_1[map_buffer->position - 1].position.x, map_buffer->player.position.x, map_buffer->all_drivers_data_1[map_buffer->position - 1].position.x - map_buffer->player.position.x);
    printf("z(%f %f %f) ", map_buffer->all_drivers_data_1[map_buffer->position - 1].position.z, map_buffer->player.position.z, map_buffer->all_drivers_data_1[map_buffer->position - 1].position.z- map_buffer->player.position.z);
    printf("y(%f %f %f)\n", map_buffer->all_drivers_data_1[map_buffer->position - 1].orientation.y, map_buffer->player.orientation.y, map_buffer->all_drivers_data_1[map_buffer->position - 1].orientation.y- map_buffer->player.orientation.y);
    */
    for (int i = 0; i < R3E_NUM_DRIVERS_MAX; ++i) {
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
    for (size_t i = 0; i < (map_buffer->num_cars > 10 ? 10 : map_buffer->num_cars); i++) {
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
        if (i < 5) {
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
        } else {
            memcpy(&chrsRadar[2 + chrsRadar[1]], &radarPlayers[i].data.position.x, sizeof(r3e_float32));
            chrsRadar[1] += 4;
            memcpy(&chrsRadar[2 + chrsRadar[1]], &radarPlayers[i].data.position.z, sizeof(r3e_float32));
            chrsRadar[1] += 4;
            memcpy(&chrsRadar[2 + chrsRadar[1]], &radarPlayers[i].data.orientation.y, sizeof(r3e_float32));
            chrsRadar[1] += 4;
            memcpy(&chrsRadar[2 + chrsRadar[1]], &radarPlayers[i].data.driver_info.car_width, sizeof(r3e_float32));
            chrsRadar[1] += 4;
            memcpy(&chrsRadar[2 + chrsRadar[1]], &radarPlayers[i].data.driver_info.car_length, sizeof(r3e_float32));
            chrsRadar[1] += 4;
        }

    }
    //printf("the rad %d\n", chrsRadar[1]);
    //printf("the cha %d\n", chrs[1]);
    //printf("\n\nthe rad beins\n");
    //for (int i = 0; i < 134; i++) {
    //    printf("%d: %d\n", i, (signed char)chrsRadar[i]);  // Print as integer
    //}
    //printf("\nthe rad ends\n\n");
    //printf("\n\nthe cha beins\n");
    //for (int i = 0; i < 134; i++) {
    //    printf("%d: %d\n", i, (char)chrs[i]);  // Print as integer
    //}
    //printf("\n\nthe cha ends\n");
    sendMessage(chrsRadar, chrsRadar[1] + 2);
    sendMessage(chrs, chrs[1] + 2);


}

void doPlayersInfo() {
    if (isClientNew) {
        isClientNew = FALSE;
        for (size_t j = 0; j < 1000; j++) {
            if (playerRatingInfoDb.player[j].id == 0)
                break;

            //InterlockedIncrement(playerRatingInfoSended.size);

            //int length = snprintf(NULL, 0, "%lu,%s,%d,%d", temp->id, temp[playerRatingInfoDb.size].name, temp[playerRatingInfoDb.size].rating, temp[playerRatingInfoDb.size].reputation);
            int length = snprintf(NULL, 0, "%lu,%s,%d,%d", playerRatingInfoDb.player[j].id, playerRatingInfoDb.player[j].name, playerRatingInfoDb.player[j].rating, playerRatingInfoDb.player[j].reputation);

            // Allocate memory for the string
            char* str = malloc(length + 4);

            sprintf_s(&str[3], length + 1, "%lu,%s,%d,%d", playerRatingInfoDb.player[j].id, playerRatingInfoDb.player[j].name, playerRatingInfoDb.player[j].rating, playerRatingInfoDb.player[j].reputation);

            //printf("%s\n", str);

            str[0] = 130;
            str[1] = length + 2;
            str[2] = 5;
            sendMessage(str, length + 4);
            free(str);
        }
    }
    /*if (!isFilled) {
        for (size_t i = 0; i < 300; i++) {
            all_d_[i] = map_buffer->all_drivers_data_1[i];
        }
        isFilled = TRUE;
    }*/
    /*printf("%d ", map_buffer->all_drivers_offset);
    printf("%d ", map_buffer->driver_data_size);
    printf("%d\n", map_buffer->num_cars);*/
    if (map_buffer->session_type == -1)
        return;
    unsigned long playersIdSumTemp = 0;
    for (size_t i = 0; i < map_buffer->num_cars; i++) {
        //map_buffer->all_drivers_data_1[i];
        //map_buffer->all_drivers_data_1[i].driver_info.user_id;
        playersIdSumTemp += map_buffer->all_drivers_data_1[i].driver_info.user_id;
    }
    if (currentState != map_buffer->session_type || playersIdSum != playersIdSumTemp) {
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
                        if (temp->name == NULL) {
                            temp->name = malloc(64);
                            char* input = map_buffer->all_drivers_data_1[i].driver_info.name;
                            int k = 0, j = 0;

                            // Step 1: Get the first letter of the first part and add ". " to result
                            temp->name[j++] = input[0];  // Add the first character
                            temp->name[j++] = '.';       // Add the period
                            temp->name[j++] = ' ';       // Add a space

                            // Step 2: Find the first space in the input
                            while (input[k] != ' ' && input[k] != '\0') {
                                k++;
                            }

                            // Step 3: Move `i` to point to the start of the second part
                            if (input[k] == ' ') {
                                k++;  // Move past the space character
                            }

                            // Step 4: Add up to 15 characters of the second part to `result`
                            int count = 0;
                            while (input[k] != '\0' && count < 15) {
                                temp->name[j++] = input[k++];
                                count++;
                            }

                            // Step 5: Null-terminate the result string
                            temp->name[j] = '\0';
                            //printf("%s\n", temp->name);
                        }

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

                        //printf("%s\n", str);

                        playerRatingInfoSended.size++;
                        playersIdSum = playersIdSumTemp;
                        str[0] = 130;
                        str[1] = length + 2;
                        str[2] = 5;
                        sendMessage(str, length + 4);
                        free(str);
                        isClientNew = FALSE;
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
                        //printf("%d\n", map_buffer->all_drivers_data_1[i].driver_info.user_id);
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
    //if (isClientNew) {
    //    isClientNew = FALSE;
    //    for (size_t i = 0; i < map_buffer->num_cars; i++) {
    //        if (map_buffer->all_drivers_data_1[i].driver_info.user_id == -1)
    //            continue;
    //        PlayerRatingInfo* temp = NULL;
    //        for (size_t j = 0; j < 1000; j++) {
    //            if (playerRatingInfoDb.player[j].id == 0)
    //                break;
    //            if (playerRatingInfoDb.player[j].id == map_buffer->all_drivers_data_1[i].driver_info.user_id)
    //                temp = &playerRatingInfoDb.player[j];
    //        }
    //        if (temp != NULL) {

    //            //InterlockedIncrement(playerRatingInfoSended.size);

    //            //int length = snprintf(NULL, 0, "%lu,%s,%d,%d", temp->id, temp[playerRatingInfoDb.size].name, temp[playerRatingInfoDb.size].rating, temp[playerRatingInfoDb.size].reputation);
    //            int length = snprintf(NULL, 0, "%lu,%s,%d,%d", temp->id, temp->name, temp->rating, temp->reputation);

    //            // Allocate memory for the string
    //            char* str = malloc(length + 4);

    //            sprintf_s(&str[3], length + 1, "%lu,%s,%d,%d", temp->id, temp->name, temp->rating, temp->reputation);

    //            //printf("%s\n", str);

    //            playersIdSum = playersIdSumTemp;
    //            str[0] = 130;
    //            str[1] = length + 2;
    //            str[2] = 5;
    //            sendMessage(str, length + 4);
    //            free(str);
    //        }
    //    }
    //}
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
        unsigned char chrp = 100;
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

                float timeza = ((map_buffer->lap_distance / map_buffer->layout_length * 100) / 100 * lapsAndFuelData.allBestLap) - (all_drivers_data_copy[currentCar]->lap_distance / map_buffer->layout_length * 100) / 100 * lapsAndFuelData.allBestLap;
                
                float16_t half = float32_to_float16(timeza);
                memcpy(&chrs[2 + chrs[1]], &half, sizeof(float16_t));
                chrs[1] += 2;
                memcpy(&chrs[2 + chrs[1]], (char*)&all_drivers_data_copy[currentCar]->penaltyType, 1);
                chrs[1] += 1;
                unsigned char toClient = (all_drivers_data_copy[currentCar]->completed_laps - map_buffer->completed_laps);
                memcpy(&chrs[2 + chrs[1]], &toClient, 1);
                chrs[1] += 1;
            }
            else {
                memcpy(&chrs[2 + chrs[1]], &num, sizeof(r3e_int32));
                chrs[1] += 4;
                memcpy(&chrs[2 + chrs[1]], &chr, sizeof(unsigned char));
                chrs[1] += 1;
                memcpy(&chrs[2 + chrs[1]], &num, sizeof(float16_t));
                chrs[1] += 2;
                memcpy(&chrs[2 + chrs[1]], &chrp, sizeof(unsigned char));
                chrs[1] += 1;
                memcpy(&chrs[2 + chrs[1]], &chrp, sizeof(unsigned char));
                chrs[1] += 1;
            }


        }
        if (notFilled != 0) {
            for (size_t i = notFilled; i < 6; i++) {
                memcpy(&chrs[2 + chrs[1]], &num, sizeof(r3e_int32));
                chrs[1] += 4;
                memcpy(&chrs[2 + chrs[1]], &chr, sizeof(unsigned char));
                chrs[1] += 1;
                memcpy(&chrs[2 + chrs[1]], &num, sizeof(float16_t));
                chrs[1] += 2;
                memcpy(&chrs[2 + chrs[1]], &chrp, sizeof(unsigned char));
                chrs[1] += 1;
                memcpy(&chrs[2 + chrs[1]], &chrp, sizeof(unsigned char));
                chrs[1] += 1;
            }
            notFilled = 0;
        }

        if (chrs[1] < 62) {
            for (size_t i = 0; i < skiped; i++) {
                memcpy(&chrs[2 + chrs[1]], &num, sizeof(r3e_int32));
                chrs[1] += 4;
                memcpy(&chrs[2 + chrs[1]], &chr, sizeof(unsigned char));
                chrs[1] += 1;
                memcpy(&chrs[2 + chrs[1]], &num, sizeof(float16_t));
                chrs[1] += 2;
                memcpy(&chrs[2 + chrs[1]], &chrp, sizeof(unsigned char));
                chrs[1] += 1;
                memcpy(&chrs[2 + chrs[1]], &chrp, sizeof(unsigned char));
                chrs[1] += 1;
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
                
                float timeza = ((map_buffer->lap_distance / map_buffer->layout_length * 100) / 100 * lapsAndFuelData.allBestLap) - (all_drivers_data_copy[currentCar]->lap_distance / map_buffer->layout_length * 100) / 100 * lapsAndFuelData.allBestLap;
                
                float16_t half = float32_to_float16(timeza);
                memcpy(&chrs[2 + chrs[1]], &half, sizeof(float16_t));
                chrs[1] += 2;
                memcpy(&chrs[2 + chrs[1]], (char*)&all_drivers_data_copy[currentCar]->penaltyType, 1);
                chrs[1] += 1;
                unsigned char toClient = (all_drivers_data_copy[currentCar]->completed_laps - map_buffer->completed_laps);
                //printf("%d", toClient);
                memcpy(&chrs[2 + chrs[1]], &toClient, 1);
                chrs[1] += 1;
            }
            else {
                memcpy(&chrs[2 + chrs[1]], &num, sizeof(r3e_int32));
                chrs[1] += 4;
                memcpy(&chrs[2 + chrs[1]], &chr, sizeof(unsigned char));
                chrs[1] += 1;
                memcpy(&chrs[2 + chrs[1]], &num, sizeof(float16_t));
                chrs[1] += 2;
                memcpy(&chrs[2 + chrs[1]], &chrp, sizeof(unsigned char));
                chrs[1] += 1;
                memcpy(&chrs[2 + chrs[1]], &chrp, sizeof(unsigned char));
                chrs[1] += 1;
            }

        }
        if (notFilled != 0) {
            for (size_t i = notFilled; i < 6; i++) {
                memcpy(&chrs[2 + chrs[1]], &num, sizeof(r3e_int32));
                chrs[1] += 4;
                memcpy(&chrs[2 + chrs[1]], &chr, sizeof(unsigned char));
                chrs[1] += 1;
                memcpy(&chrs[2 + chrs[1]], &num, sizeof(float16_t));
                chrs[1] += 2;
                memcpy(&chrs[2 + chrs[1]], &chrp, sizeof(unsigned char));
                chrs[1] += 1;
                memcpy(&chrs[2 + chrs[1]], &chrp, sizeof(unsigned char));
                chrs[1] += 1;
            }
            notFilled = 0;
        }

        if (chrs[1] < 107) {
            for (size_t i = 0; i < skiped; i++) {
                memcpy(&chrs[2 + chrs[1]], &num, sizeof(r3e_int32));
                chrs[1] += 4;
                memcpy(&chrs[2 + chrs[1]], &chr, sizeof(unsigned char));
                chrs[1] += 1;
                memcpy(&chrs[2 + chrs[1]], &num, sizeof(float16_t));
                chrs[1] += 2;
                memcpy(&chrs[2 + chrs[1]], &chrp, sizeof(unsigned char));
                chrs[1] += 1;
                memcpy(&chrs[2 + chrs[1]], &chrp, sizeof(unsigned char));
                chrs[1] += 1;
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
    
    /*
    chrs[1] = 2;
    chrs[2] = 11;
    chrs[3] = 0;
    int compLaps = map_buffer->all_drivers_data_1[0].completed_laps + 1;
    memcpy(&chrs[2 + chrs[1]], &compLaps, sizeof(r3e_float32));
    chrs[1] += 4;
    memcpy(&chrs[2 + chrs[1]], &map_buffer->session_time_remaining, sizeof(r3e_float32));
    chrs[1] += 4;
    sendMessage(chrs, chrs[1] + 2);
    r3e_float32 gap = 0;


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
        sendMessage(chrs, chrs[1] + 2);
    }

    //if (map_buffer->position)
    for (size_t i = 10; i < 140; i += 10) {
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
            memcpy(&chrs[2 + chrs[1]], &map_buffer->all_drivers_data_1[j].driver_info.user_id, sizeof(r3e_float32));
            chrs[1] += 4;
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
        sendMessage(chrs, chrs[1] + 2);
    }*/
}

void doWheels() {
    //printf("%f\n", map_buffer->brake_pressure[0] + map_buffer->brake_pressure[1] + map_buffer->brake_pressure[2] + map_buffer->brake_pressure[3]);
    //printf("front left wheel brake: pressure %f, temperature %f, optimal temp %f;\n", map_buffer->brake_pressure, map_buffer->brake_temp[0].current_temp, map_buffer->brake_temp[0].optimal_temp);
    //printf("front left wheel brake: pressure %f, predicted %f, temperature %f;\n", map_buffer->brake_pressure[0], map_buffer->brake_pressure[0] / map_buffer->brake_raw, map_buffer->brake_temp[0].current_temp);
    //if (map_buffer->brake_raw == 1.0f /*&& floor(map_buffer->brake_temp[0].current_temp) == floor(map_buffer->brake_temp[0].optimal_temp)*/) {
        //printf("front left wheel brake: pressure %f, temperature %f, optimal temp %f;\n", map_buffer->brake_pressure[0], map_buffer->brake_temp[0].current_temp, map_buffer->brake_temp[0].optimal_temp);
        //printf("rear left wheel brake: pressure %f, temperature %f, optimal temp %f;\n", map_buffer->brake_pressure[2], map_buffer->brake_temp[2].current_temp, map_buffer->brake_temp[2].optimal_temp);
        /*printf("front left wheel brake: pressure %f, temperature %f, optimal temp %f;", map_buffer->brake_pressure[0], map_buffer->brake_temp[0].current_temp, map_buffer->brake_temp[0].optimal_temp);
        printf("front right wheel brake: pressure %f, temperature %f, optimal temp %f;", map_buffer->brake_pressure[1], map_buffer->brake_temp[1].current_temp, map_buffer->brake_temp[1].optimal_temp);
        printf("rear left wheel brake: pressure %f, temperature %f, optimal temp %f;", map_buffer->brake_pressure[2], map_buffer->brake_temp[2].current_temp, map_buffer->brake_temp[2].optimal_temp);
        printf("rear right wheel brake: pressure %f, temperature %f, optimal temp %f;\n", map_buffer->brake_pressure[3], map_buffer->brake_temp[3].current_temp, map_buffer->brake_temp[3].optimal_temp);*/
    //}
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
    //printf("map_buffer->player.orientation.x: %f, ", map_buffer->player.orientation.x);
    //printf("map_buffer->all_drivers_data_1[player].orientation.x: %f, ", map_buffer->all_drivers_data_1[map_buffer->position - 1].orientation.x);
    //printf("diff: %f\n", map_buffer->player.orientation.x - map_buffer->all_drivers_data_1[map_buffer->position - 1].orientation.x);
    //printf("map_buffer->player.orientation.y: %f, ", map_buffer->player.orientation.y);
    //printf("map_buffer->all_drivers_data_1[player].orientation.y: %f", map_buffer->all_drivers_data_1[map_buffer->position - 1].orientation.y);
    //printf("diff: %f\n", map_buffer->player.orientation.y - map_buffer->all_drivers_data_1[map_buffer->position - 1].orientation.y);//
    //printf("map_buffer->player.orientation.z: %f, ", map_buffer->player.orientation.z);
    //printf("map_buffer->all_drivers_data_1[player].orientation.z: %f", map_buffer->all_drivers_data_1[map_buffer->position - 1].orientation.z);
    //printf("diff: %f\n", map_buffer->player.orientation.z - map_buffer->all_drivers_data_1[map_buffer->position - 1].orientation.z);

    //printf("map_buffer->player.position.x: %f, ", map_buffer->player.position.x);
    //printf("map_buffer->all_drivers_data_1[player].position.x: %f, ", map_buffer->all_drivers_data_1[map_buffer->position - 1].position.x);
    //printf("diff: %f\n", map_buffer->player.position.x - map_buffer->all_drivers_data_1[map_buffer->position - 1].position.x);
    //printf("map_buffer->player.position.y: %f, ", map_buffer->player.position.y);
    //printf("map_buffer->all_drivers_data_1[player].position.y: %f, ", map_buffer->all_drivers_data_1[map_buffer->position - 1].position.y);
    //printf("diff: %f\n", map_buffer->player.position.y - map_buffer->all_drivers_data_1[map_buffer->position - 1].position.y);//
    //printf("map_buffer->player.position.z: %f, ", map_buffer->player.position.z);
    //printf("map_buffer->all_drivers_data_1[player].position.z: %f, ", map_buffer->all_drivers_data_1[map_buffer->position - 1].position.z);
    //printf("diff: %f\n", map_buffer->player.position.z - map_buffer->all_drivers_data_1[map_buffer->position - 1].position.z);
    float yaw = map_buffer->car_orientation.yaw;
    float forward_x = cos(yaw);
    float forward_z = sin(yaw);
    float actual_velocity_x = -map_buffer->player.local_velocity.x;
    float actual_velocity_z = -map_buffer->player.local_velocity.z;
    // Get the angle of the actual movement vector
    float actual_angle = atan2(actual_velocity_x, actual_velocity_z);
    float forward_angle = atan2(forward_x, forward_z);

    float angle_difference = actual_angle - forward_angle;

    if (angle_difference > M_PI) {
        angle_difference -= 2 * M_PI;
    }
    else if (angle_difference < -M_PI) {
        angle_difference += 2 * M_PI;
    }

    float d_fl = 1.5, d_fr = 1.5, d_rl = 1.5, d_rr = 1.5;
    float yaw_rate = map_buffer->player.angular_velocity.y;


    float v_lateral_fl = actual_velocity_x + yaw_rate * d_fl;
    float v_lateral_fr = actual_velocity_x + yaw_rate * d_fr;
    float v_lateral_rl = actual_velocity_x + yaw_rate * d_rl;
    float v_lateral_rr = actual_velocity_x + yaw_rate * d_rr;
    float steer_angle_deg = map_buffer->steer_lock_degrees * map_buffer->steer_input_raw;
    float steer_angle_rad = steer_angle_deg * (M_PI / 180.0);

    float slip_angle_fl = atan2(v_lateral_fl, actual_velocity_z) - steer_angle_rad;
    float slip_angle_fr = atan2(v_lateral_fr, actual_velocity_z) - steer_angle_rad;
    float slip_angle_rl = atan2(v_lateral_rl, actual_velocity_z);
    float slip_angle_rr = atan2(v_lateral_rr, actual_velocity_z);

    slip_angle_fl -= actual_angle;
    slip_angle_fr -= actual_angle;
    slip_angle_rl -= actual_angle;
    slip_angle_rr -= actual_angle;

    float threshold = 0.02f;
    
    char ch = 0;
    if (fabs(slip_angle_rl) > threshold || fabs(slip_angle_rr) > threshold) {
        //printf("Oversteering\n");
        ch = 1;
    }
    else if (fabs(slip_angle_fl) > threshold || fabs(slip_angle_fr) > threshold) {
        //printf("Understeering\n");
        ch = 2;
    }
    else {
        //printf("Neutral Steer\n");
        ch = 3;
    }
    chrs[1] = 1;
    chrs[2] = 101;
    float max_slip_angle = 1;
    float min_slip_angle = 0;
    float epsilon = 1e-6;
    float range = max_slip_angle - min_slip_angle;
    if (range < epsilon) range = epsilon;
    slip_angle_fl = fabs(slip_angle_fl);
    slip_angle_fr = fabs(slip_angle_fr);
    slip_angle_rl = fabs(slip_angle_rl);
    slip_angle_rr = fabs(slip_angle_rr);
    float slip_angle_fl_normalized = (slip_angle_fl * 10 - min_slip_angle) / range;
    float slip_angle_fr_normalized = (slip_angle_fr * 10 - min_slip_angle) / range;
    float slip_angle_rl_normalized = (slip_angle_rl * 10 - min_slip_angle) / range;
    float slip_angle_rr_normalized = (slip_angle_rr * 10 - min_slip_angle) / range;
    slip_angle_fl_normalized = fmax(0.0f, fmin(1.0f, slip_angle_fl_normalized));
    slip_angle_fr_normalized = fmax(0.0f, fmin(1.0f, slip_angle_fr_normalized));
    slip_angle_rl_normalized = fmax(0.0f, fmin(1.0f, slip_angle_rl_normalized));
    slip_angle_rr_normalized = fmax(0.0f, fmin(1.0f, slip_angle_rr_normalized));

    yaw_ = -1.5708;
    actual_angle_ = actual_angle - 1.5708;
    steer_angle_rad_ = steer_angle_rad - 1.5708;

    /*if (hwnd) {
        InvalidateRect(hwnd, NULL, TRUE);
    }*/
    memcpy(&chrs[2 + chrs[1]], &slip_angle_fl_normalized, sizeof(r3e_float32));
    chrs[1] += 4;
    memcpy(&chrs[2 + chrs[1]], &slip_angle_rl_normalized, sizeof(r3e_float32));
    chrs[1] += 4;
    memcpy(&chrs[2 + chrs[1]], &actual_angle_, sizeof(r3e_float32));
    chrs[1] += 4;
    memcpy(&chrs[2 + chrs[1]], &steer_angle_rad_, sizeof(r3e_float32));
    chrs[1] += 4;
    memcpy(&chrs[2 + chrs[1]], &ch, sizeof(char));
    chrs[1] += 1;

    sendMessage(chrs, chrs[1] + 2);

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
    memcpy(&chrs[2 + chrs[1]], &map_buffer->steer_input_raw, sizeof(r3e_float32));
    chrs[1] += 4;
    float percentage0 = 0;
    float percentage1 = 0;
    float percentage2 = 0;
    float percentage3 = 0;

    if (currentBb != map_buffer->brake_bias) {
        currentBb = map_buffer->brake_bias;
        if (currentBb != 0.5f)
            setBb();
    }
    if (map_buffer->brake_raw > 0.01f) {
        //printf("%f ", map_buffer->brake_bias);
        //printf("%f\n", map_buffer->brake_temp[0].hot_temp);
        /*printf("%f ", map_buffer->brake_pressure[0] / map_buffer->brake_raw + map_buffer->brake_pressure[1] / map_buffer->brake_raw + map_buffer->brake_pressure[2] / map_buffer->brake_raw + map_buffer->brake_pressure[3] / map_buffer->brake_raw);
        printf("%f ", map_buffer->brake_pressure[0] / map_buffer->brake_raw);
        printf("%f ", map_buffer->brake_pressure[1] / map_buffer->brake_raw);
        printf("%f ", map_buffer->brake_pressure[2] / map_buffer->brake_raw);
        printf("%f\n", map_buffer->brake_pressure[3] / map_buffer->brake_raw);

        printf("%f ", brakeA[0][(int)map_buffer->brake_temp[0].current_temp] + brakeA[1][(int)map_buffer->brake_temp[0].current_temp] + brakeA[2][(int)map_buffer->brake_temp[0].current_temp] + brakeA[3][(int)map_buffer->brake_temp[0].current_temp]);
        printf("%f ", brakeA[0][(int)map_buffer->brake_temp[0].current_temp]);
        printf("%f ", brakeA[1][(int)map_buffer->brake_temp[0].current_temp]);
        printf("%f ", brakeA[2][(int)map_buffer->brake_temp[0].current_temp]);
        printf("%f\n", brakeA[3][(int)map_buffer->brake_temp[0].current_temp]);*/
        /*printf("%.5f ", map_buffer->brake_temp[0].current_temp);
        printf("%.5f ", map_buffer->brake_temp[1].current_temp);
        printf("%.5f ", map_buffer->brake_temp[2].current_temp);
        printf("%.5f ", map_buffer->brake_temp[3].current_temp);
        printf("%.5f ", map_buffer->brake_temp[0].optimal_temp);
        printf("%.5f\n", map_buffer->brake_temp[2].optimal_temp);*/
        //if (map_buffer->aid_settings.abs == 1 && brake[0][(int)map_buffer->brake_temp[0].current_temp] == 0) {
        //    /*brake[0][(int)map_buffer->brake_temp[0].current_temp / 10] = map_buffer->brake_pressure[0] / map_buffer->brake_raw;
        //    brake[1][(int)map_buffer->brake_temp[1].current_temp / 10] = map_buffer->brake_pressure[1] / map_buffer->brake_raw;
        //    brake[2][(int)map_buffer->brake_temp[2].current_temp / 10] = map_buffer->brake_pressure[2] / map_buffer->brake_raw;
        //    brake[3][(int)map_buffer->brake_temp[3].current_temp / 10] = map_buffer->brake_pressure[3] / map_buffer->brake_raw;*/
        //    if (brake[0][(int)map_buffer->brake_temp[0].current_temp] == 0)
        //        brake[0][(int)map_buffer->brake_temp[0].current_temp] = map_buffer->brake_pressure[0] / map_buffer->brake_raw;

        //    if (brake[1][(int)map_buffer->brake_temp[0].current_temp] == 0)
        //        brake[1][(int)map_buffer->brake_temp[1].current_temp] = map_buffer->brake_pressure[1] / map_buffer->brake_raw;

        //    if (brake[2][(int)map_buffer->brake_temp[0].current_temp] == 0)
        //        brake[2][(int)map_buffer->brake_temp[2].current_temp] = map_buffer->brake_pressure[2] / map_buffer->brake_raw;

        //    if (brake[3][(int)map_buffer->brake_temp[0].current_temp] == 0)
        //        brake[3][(int)map_buffer->brake_temp[3].current_temp] = map_buffer->brake_pressure[3] / map_buffer->brake_raw;
        //}
        if (map_buffer->aid_settings.abs == 5) {
            /*float percentage0 = 100 - (map_buffer->brake_pressure[0] / brake[0][(int)map_buffer->brake_temp[0].current_temp / 10]) * 100;
            float percentage1 = 100 - (map_buffer->brake_pressure[1] / brake[1][(int)map_buffer->brake_temp[1].current_temp / 10]) * 100;
            float percentage2 = 100 - (map_buffer->brake_pressure[2] / brake[2][(int)map_buffer->brake_temp[2].current_temp / 10]) * 100;
            float percentage3 = 100 - (map_buffer->brake_pressure[3] / brake[3][(int)map_buffer->brake_temp[3].current_temp / 10]) * 100;*/
            /*float percentage0 = 100 - (map_buffer->brake_pressure[0] / brake[0][(int)map_buffer->brake_temp[0].current_temp]) * 100;
            float percentage1 = 100 - (map_buffer->brake_pressure[1] / brake[1][(int)map_buffer->brake_temp[1].current_temp]) * 100;
            float percentage2 = 100 - (map_buffer->brake_pressure[2] / brake[2][(int)map_buffer->brake_temp[2].current_temp]) * 100;
            float percentage3 = 100 - (map_buffer->brake_pressure[3] / brake[3][(int)map_buffer->brake_temp[3].current_temp]) * 100;*/
            percentage0 = 100 - (map_buffer->brake_pressure[0] / map_buffer->brake_raw / brakeA[0][(int)map_buffer->brake_temp[0].current_temp]) * 100;
            percentage1 = 100 - (map_buffer->brake_pressure[1] / map_buffer->brake_raw / brakeA[1][(int)map_buffer->brake_temp[1].current_temp]) * 100;
            percentage2 = 100 - (map_buffer->brake_pressure[2] / map_buffer->brake_raw / brakeA[2][(int)map_buffer->brake_temp[2].current_temp]) * 100;
            percentage3 = 100 - (map_buffer->brake_pressure[3] / map_buffer->brake_raw / brakeA[3][(int)map_buffer->brake_temp[3].current_temp]) * 100;
            /*memcpy(&chrs[2 + chrs[1]], &percentage0, sizeof(r3e_float32));
            memcpy(&chrs[2 + chrs[1]], &percentage1, sizeof(r3e_float32));
            memcpy(&chrs[2 + chrs[1]], &percentage2, sizeof(r3e_float32));
            memcpy(&chrs[2 + chrs[1]], &percentage3, sizeof(r3e_float32));*/
        }
    }
    memcpy(&chrs[2 + chrs[1]], &percentage0, sizeof(r3e_float32));
    chrs[1] += 4;
    memcpy(&chrs[2 + chrs[1]], &percentage1, sizeof(r3e_float32));
    chrs[1] += 4;
    memcpy(&chrs[2 + chrs[1]], &percentage2, sizeof(r3e_float32));
    chrs[1] += 4;
    memcpy(&chrs[2 + chrs[1]], &percentage3, sizeof(r3e_float32));
    chrs[1] += 4;

    memcpy(&chrs[2 + chrs[1]], &map_buffer->tire_grip[0], sizeof(r3e_float32));
    chrs[1] += 4;
    memcpy(&chrs[2 + chrs[1]], &map_buffer->tire_grip[1], sizeof(r3e_float32));
    chrs[1] += 4;
    memcpy(&chrs[2 + chrs[1]], &map_buffer->tire_grip[2], sizeof(r3e_float32));
    chrs[1] += 4;
    memcpy(&chrs[2 + chrs[1]], &map_buffer->tire_grip[3], sizeof(r3e_float32));
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
    if (lapsAndFuelData.allBestLap == -1)
        lapsAndFuelData.allBestLap = 99999;
    if (lastSavedLap != map_buffer->completed_laps && map_buffer->lap_time_previous_self != -1) {

        /*printf("\n---------");
        printf("\ncompleted_laps %d", map_buffer->completed_laps);
        printf("\nallBestLap %f", lapsAndFuelData.allBestLap);
        printf("\nallBestFue %f", lapsAndFuelData.allBestFuel);
        printf("\nleaderBest %f", lapsAndFuelData.leaderBestLap);
        printf("\ncurrentBes %f", lapsAndFuelData.currentBestLap);
        printf("\npreviousLa %f", lapsAndFuelData.previousLap);*/
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

            //printf("\nsomethbest");
            //if ((lapsAndFuelData.currentBestLap > map_buffer->lap_time_best_self || lapsAndFuelData.allBestFuel > map_buffer->fuel_per_lap || lapsAndFuelData.leaderBestLap > map_buffer->lap_time_best_leader_class)) {
            if (map_buffer->completed_laps > 0 && lapsAndFuelData.currentBestLap > lapsAndFuelData.previousLap) {
                //if (map_buffer->lap_time_best_self != -1 && map_buffer->completed_laps > 0 && lapsAndFuelData.currentBestLap > map_buffer->lap_time_best_self) {
                lapsAndFuelData.currentBestLap = lapsAndFuelData.previousLap;
                //printf("\ncurrentBestLap %f", lapsAndFuelData.currentBestLap);
            }
            //lapsAndFuelData.leaderBestLap = map_buffer->lap_time_best_leader_class;
            /*/////////////////////*memcpy(&chrs[2 + chrs[1]], &map_buffer->lap_time_best_self, sizeof(float));
            //memcpy(&chrs[2 + chrs[1]], &lapsAndFuelData.currentBestLap, sizeof(float));
            chrs[1] += 4;
            memcpy(&chrs[2 + chrs[1]], &lapsAndFuelData.leaderBestLap, sizeof(float));
            chrs[1] += 4;*/////////////////////
            //if (map_buffer->completed_laps > 1 && (lapsAndFuelData.allBestLap > lapsAndFuelData.currentBestLap || lapsAndFuelData.allBestFuel > map_buffer->fuel_per_lap)) {
            if (((map_buffer->session_type != 1 && map_buffer->completed_laps > 1) || (map_buffer->session_type == 1 && map_buffer->completed_laps > 0)) && ((lapsAndFuelData.currentBestLap != -1 && lapsAndFuelData.allBestLap > lapsAndFuelData.currentBestLap) || lapsAndFuelData.allBestFuel > map_buffer->fuel_per_lap)) {
                if (map_buffer->lap_time_best_self != -1 && lapsAndFuelData.currentBestLap != -1 && lapsAndFuelData.allBestLap > lapsAndFuelData.currentBestLap) {
                    lapsAndFuelData.allBestLap = map_buffer->lap_time_best_self;//lapsAndFuelData.currentBestLap;
                    lapsAndFuelData.allBestFuel = map_buffer->fuel_per_lap;
                    //printf("\currentBestLap %f", lapsAndFuelData.currentBestLap);
                }
                /*/////////////////////*memcpy(&chrs[2 + chrs[1]], &lapsAndFuelData.allBestLap, sizeof(float));
                chrs[1] += 4;*/////////////////////
                /*if (lapsAndFuelData.allBestFuel > map_buffer->fuel_per_lap && map_buffer->fuel_per_lap != 0) {
                    lapsAndFuelData.allBestFuel = map_buffer->fuel_per_lap;
                    //printf("\currentBestLap %f", lapsAndFuelData.allBestFuel);
                }*/
                /*/////////////////////*memcpy(&chrs[2 + chrs[1]], &lapsAndFuelData.allBestFuel, sizeof(float));
                chrs[1] += 4;*/////////////////////

                writeBestLapFuel(&lapsAndFuelData, map_buffer->layout_id, map_buffer->vehicle_info.model_id);

            }
        }

        lapsAndFuelData.leaderBestLap = map_buffer->lap_time_best_leader_class;
        memcpy(&chrs[2 + chrs[1]], &map_buffer->lap_time_best_self, sizeof(float));
        chrs[1] += 4;
        memcpy(&chrs[2 + chrs[1]], &lapsAndFuelData.leaderBestLap, sizeof(float));
        chrs[1] += 4;
        memcpy(&chrs[2 + chrs[1]], &lapsAndFuelData.allBestLap, sizeof(float));
        chrs[1] += 4;
        memcpy(&chrs[2 + chrs[1]], &lapsAndFuelData.allBestFuel, sizeof(float));
        chrs[1] += 4;
        /*printf("\n-");
        printf("\nallBestLap %f", lapsAndFuelData.allBestLap);
        printf("\nallBestFue %f", lapsAndFuelData.allBestFuel);
        printf("\nleaderBest %f", lapsAndFuelData.leaderBestLap);
        printf("\ncurrentBes %f", lapsAndFuelData.currentBestLap);
        printf("\npreviousLa %f", lapsAndFuelData.previousLap);*/
        sendMessage(chrs, chrs[1] + 2);
        lastSavedLap = map_buffer->completed_laps;
    }
}
