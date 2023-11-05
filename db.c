#include "db.h"
#include "sqlite3.h"
#include "webhudgit.h"

#include <stdio.h>
#include <sys/stat.h>
#include <stdint.h>
#pragma optimize("", off)
void init() {
    struct stat buffer;
    int exist = stat("data.db", &buffer);
    
    sqlite3* db;
    char* errMsg = 0;

    // Open the database (or create it if it doesn't exist)
    int rc = sqlite3_open("data.db", &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        return 1;
    }
    else {
        fprintf(stdout, "Opened database successfully\n");
    }
    /*
    char* sql2 = "DELETE FROM BestLaps;";
    rc = sqlite3_exec(db, sql2, 0, 0, &errMsg);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", errMsg);
        sqlite3_free(errMsg);
    }//*/

    if (exist == 0)
        return;
    
    // Create a table
    //const char* createTableSQL = "CREATE TABLE WindowsSettings( data TEXT );";
    //rc = sqlite3_exec(db, createTableSQL, 0, 0, &errMsg);

    char* sql = "CREATE TABLE WindowsSettings( data BLOB );";
    rc = sqlite3_exec(db, sql, 0, 0, &errMsg);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", errMsg);
        sqlite3_free(errMsg);
    }
    else {
        fprintf(stdout, "Table created successfully\n");
    }

    char* sql1 = "CREATE TABLE BestLaps( track INT, car INT, lap FLOAT, fuel FLOAT, PRIMARY KEY(track, car) );";
    rc = sqlite3_exec(db, sql1, 0, 0, &errMsg);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", errMsg);
        sqlite3_free(errMsg);
    }
    else {
        fprintf(stdout, "Table created successfully\n");
    }


    // Insert data into the table
    
    //const char* insertSQL = "INSERT INTO WindowsSettings (data) VALUES (0x0);";
    //const char* insertSQL = "INSERT INTO WindowsSettings (data) VALUES ();";
    /*int sequence[] = {1, 0, 100, 0, 156, 1, 56, 1, 0, 0, 100, 0, 156, 1, 56, 1, 33, 4, 236, 2, 156, 1, 56, 1, 136, 0, 254, 2, 241, 1, 6, 1, 201, 5, 12, 1, 156, 1, 56, 1, 52, 3, 224, 0, 75, 1, 71, 0, 186, 1, 131, 0, 108, 1, 68, 0, 145, 4, 226, 0, 22, 1, 61, 0, 207, 1, 213, 0, 251, 0, 60, 0, 231, 5, 174, 3, 56, 0, 112, 0, 0, 204
    };
    int size = sizeof(sequence) / sizeof(sequence[0]);

    char* hexString = (char*)malloc(size * sizeof(char) * 2 + 1);
    for (int i = 0; i < size; i++) {
        sprintf_s(hexString + i * 2, size * 2 + 1 - i * 2, "%02X", sequence[i]);
    }

    char* insertSQL = (char*)malloc(1000 * sizeof(char));
    sprintf_s(insertSQL, 1000, "INSERT INTO WindowsSettings (data) VALUES (0x%s);", hexString);*/
    
    const char* insertSQL = "INSERT INTO WindowsSettings (data) VALUES (0x0);";
    rc = sqlite3_exec(db, insertSQL, 0, 0, &errMsg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", errMsg);
        sqlite3_free(errMsg);
    }
    /*else {
        fprintf(stdout, "Record inserted successfully\n");
    }*/


    // Close the database
    sqlite3_close(db);

}

void writeWindowsSettings(unsigned char* value, size_t size) {
    sqlite3* db;
    char* errMsg = 0;

    // Open the database (or create it if it doesn't exist)
    int rc = sqlite3_open("data.db", &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        return 1;
    }

    sqlite3_stmt* stmt;
    //printf("%d", size);
    //rc = sqlite3_prepare_v2(db, "INSERT INTO WindowsSettings(data) VALUES(?)", -1, &stmt, 0);
    rc = sqlite3_prepare_v2(db, "UPDATE WindowsSettings SET data=?", -1, &stmt, 0);

    if (rc != SQLITE_OK) {
        printf("Failed to prepare statement\n");
        return rc;
    }

    rc = sqlite3_bind_blob(stmt, 1, value, size, SQLITE_STATIC);

    if (rc != SQLITE_OK) {
        printf("Failed to bind blob\n");
        return rc;
    }

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_DONE) {
        printf("Failed to execute statement js\n");
        return rc;
    }

    sqlite3_finalize(stmt);

    // Close the database
    sqlite3_close(db);
    /*rc = sqlite3_exec(db, "SELECT 1", 0, 0, 0);

    if (rc == SQLITE_OK)
        printf("Database is open\n");
    else
        printf("Database is closed\n");*/
    //printf("\n\n\n\n\nZAPIS\n\n\n\n\n");
}

BlobResult readWindowsSettings() {

    sqlite3* db;
    char* errMsg = 0;
    sqlite3_stmt* stmt;

    // Open the database (or create it if it doesn't exist)
    int rc = sqlite3_open("data.db", &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        return;
    }
    
    char* sql = "SELECT * FROM WindowsSettings";
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        printf("Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        return;
    }
    BlobResult result;
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        //int colCount = sqlite3_column_count(stmt);
        /*for (int i = 0; i < colCount; i++) {
            printf("%s = %s\n", sqlite3_column_name(stmt, i), sqlite3_column_text(stmt, i));
        }*/
        //printf("%s = %s\n", sqlite3_column_name(stmt, 0), sqlite3_column_text(stmt, 0));
        //result = sqlite3_column_text(stmt, 0);
        //result = sqlite3_column_blob(stmt, 0);

        //result.data = (unsigned char*)sqlite3_column_blob(stmt, 0);
        //printf("%s = %s\n", sqlite3_column_name(stmt, 0), sqlite3_column_text(stmt, 0));
        //const unsigned char* column_text = sqlite3_column_text(stmt, 0);
        //const unsigned char neww = strndup((const unsigned char)column_text, sqlite3_column_bytes(stmt, 0));
        size_t size = sqlite3_column_bytes(stmt, 0);
        /*const unsigned char* src = sqlite3_column_blob(stmt, 0);  // size of the source byte array
        unsigned char dest[] = malloc(size);  // allocate memory for the destination byte array
        if (dest != NULL) {
            dest = *src;
            //memcpy(dest, src, size);  // copy the bytes
        }*/
        const unsigned char* src = sqlite3_column_blob(stmt, 0);

        //unsigned char dest = malloc(size * sizeof(unsigned char));
        //memcpy(dest, src, size);
        /*unsigned char dest[200];
        dest = *src;*/
        unsigned char dest[200];
        memcpy(dest, src, size);

        result.data = &dest;
        //const unsigned char* new_var = result.data;
        result.size = size;
        //const unsigned char* test = result.data;
        //printf("lololo\n");
    }

    if (rc != SQLITE_DONE) {
        printf("Failed to execute statement: %s\n", sqlite3_errmsg(db));
        return;
    }

    rc = sqlite3_finalize(stmt);
    if (rc != SQLITE_OK) {
        printf("Failed to finalize statement: %s\n", sqlite3_errmsg(db));
        return;
    }
    /*printf("\nper\n");
    for (size_t i = 0; i < 100; i++)
    {
        printf("%d ", (int)result.data[i]);
        //offse += 2;
    }
    printf("\nftr\n");*/
    sqlite3_close(db);
    /*for (size_t i = 0; i < result.size; i++)
    {

        printf("%c", result.data[i]);
    }
    printf("\n");*/
    //size_t array_size = sizeof(result) / sizeof(result[0]);
    /*for (size_t i = 0; i < 100; i++)
    {
        printf("%d ", (int)result.data[i]);
        //offse += 2;
    }*/
    //printf("\n");
    /*rc = sqlite3_exec(db, "SELECT 1", 0, 0, 0);

    if (rc == SQLITE_OK)
        printf("Database is open\n");
    else
        printf("Database is closed\n");*/
    return result;
}


void readBestLapFuel(LapsAndFuel* lapsAndFuel, int32_t track, int32_t car) {
    sqlite3* db;
    sqlite3_stmt* stmt;
    //int track = 1;  // replace with your track number
    //int car = 2;  // replace with your car number

    // Open the database
    if (sqlite3_open("data.db", &db) != SQLITE_OK) {
        printf("Can't open database: %s readBestLapFuel\n", sqlite3_errmsg(db));
        return 1;
    }

    // Prepare the SQL statement
    char sql[] = "SELECT * FROM BestLaps WHERE track = ? AND car = ?";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK) {
        //printf("Failed to prepare statement readBestLapFuel\n");
        printf("Failed to prepare statement: %s readBestLapFuel\n", sqlite3_errmsg(db));
        return 1;
    }

    // Bind the parameters
    sqlite3_bind_int(stmt, 1, track);
    sqlite3_bind_int(stmt, 2, car);

    // Step through the result set
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        lapsAndFuel->allBestLap = sqlite3_column_double(stmt, 2);
        lapsAndFuel->allBestFuel = sqlite3_column_double(stmt, 3);
        /*printf("track: %d, car: %d, lap: %f, fuel: %f\n",
            sqlite3_column_int(stmt, 0),
            sqlite3_column_int(stmt, 1),
            sqlite3_column_double(stmt, 2),
            sqlite3_column_double(stmt, 3));*/
    }

    // Finalize the statement and close the database
    sqlite3_finalize(stmt);
    sqlite3_close(db);
}

void writeBestLapFuel(LapsAndFuel* lapsAndFuel, int32_t track, int32_t car) {
    sqlite3* db;
    sqlite3_stmt* stmt;
    //int track = ;  // replace with your track number
    //int car = 2;  // replace with your car number
    float lap = lapsAndFuel->allBestLap;  // replace with your lap value
    float fuel = lapsAndFuel->allBestFuel;  // replace with your fuel value

    // Open the database
    if (sqlite3_open("data.db", &db) != SQLITE_OK) {
        printf("Can't open database: %s writeBestLapFuel\n", sqlite3_errmsg(db));
        return 1;
    }

    // Prepare the SQL statement
    char sql[] = "INSERT OR REPLACE INTO BestLaps (track, car, lap, fuel) VALUES (?, ?, ?, ?)";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK) {
        //printf("Failed to prepare statement writeBestLapFuel\n");
        printf("Failed to prepare statement: %s writeBestLapFuel\n", sqlite3_errmsg(db));
        return 1;
    }

    // Bind the parameters
    sqlite3_bind_int(stmt, 1, track);
    sqlite3_bind_int(stmt, 2, car);
    sqlite3_bind_double(stmt, 3, lap);
    sqlite3_bind_double(stmt, 4, fuel);

    // Execute the statement
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        printf("Failed to execute statement writeBestLapFuel\n");
        return 1;
    }

    //printf("Row inserted or updated successfully\n");

    // Finalize the statement and close the database
    sqlite3_finalize(stmt);
    sqlite3_close(db);
}
#pragma optimize("", on)
/*
-----------------------INSERT

    // Insert data into the table
    const char* insertSQL = "INSERT INTO WindowsSettings (data) VALUES (1, 'John Doe');";
    rc = sqlite3_exec(db, insertSQL, 0, 0, &errMsg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", errMsg);
        sqlite3_free(errMsg);
    }
-----------------------
-----------------------SELECT

    // Assume a table named YourTable with a column named YourColumn
    const char* selectSQL = "SELECT * FROM YourTable;";
    rc = sqlite3_exec(db, selectSQL, callback, "Select result", &errMsg);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", errMsg);
        sqlite3_free(errMsg);
    }
-----------------------
-----------------------UPDATE

    // Assume a table named YourTable with a column named YourColumn
    const char* updateSQL = "UPDATE YourTable SET YourColumn = 'NewValue' WHERE Condition;";

    rc = sqlite3_exec(db, updateSQL, 0, 0, &errMsg);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", errMsg);
        sqlite3_free(errMsg);
    }
-----------------------
*/
/*
int callback(void* data, int argc, char** argv, char** azColName) {
    int i;
    printf("%s: ", (const char*)data);

    for (i = 0; i < argc; i++) {
        printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    }

    printf("\n");
    return 0;
}
*/

/*
void writeBlob() {
    sqlite3* db;
    char* err_msg = 0;

    int rc = sqlite3_open("data.db", &db);

    if (rc != SQLITE_OK) {
        return;
    }

    char* sql = "CREATE TABLE Test(Id INTEGER PRIMARY KEY, Data BLOB);";

    rc = sqlite3_exec(db, sql, 0, 0, &err_msg);

    if (rc != SQLITE_OK) {
        printf("Failed to create table\n");
        return rc;
    }

    sqlite3_stmt* stmt;

    rc = sqlite3_prepare_v2(db, "INSERT INTO Test(Data) VALUES(?)", -1, &stmt, 0);

    if (rc != SQLITE_OK) {
        printf("Failed to prepare statement\n");
        return rc;
    }

    unsigned char bytes_array[] = { 0x12, 0x34, 0x56, 0x78 };

    rc = sqlite3_bind_blob(stmt, 1, bytes_array, sizeof(bytes_array), SQLITE_STATIC);

    if (rc != SQLITE_OK) {
        printf("Failed to bind blob\n");
        return rc;
    }

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_DONE) {
        printf("Failed to execute statement\n");
        return rc;
    }

    printf("Bytes stored successfully\n");

    sqlite3_finalize(stmt);
    sqlite3_close(db);


}

*/