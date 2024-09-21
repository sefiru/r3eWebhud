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
        sqlite3_stmt* stmt;
        char sql[256];
        int exists = 0;
        int exists1 = 0;

        // Query the SQLite schema to check for the existence of the column
        snprintf(sql, sizeof(sql),
            "PRAGMA table_info(%s);", "BestLaps");

        if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) == SQLITE_OK) {
            while (sqlite3_step(stmt) == SQLITE_ROW) {
                const char* col_name = (const char*)sqlite3_column_text(stmt, 1);
                if (strcmp(col_name, "track") == 0) {
                    exists = 1;
                }
                if (strcmp(col_name, "layout") == 0) {
                    exists1 = 1;
                }
            }
        }
        sqlite3_finalize(stmt);

        if (exists && exists1) {
            char* sql_begin = "BEGIN TRANSACTION;";
            rc = sqlite3_exec(db, sql_begin, 0, 0, &errMsg);
            if (rc != SQLITE_OK) {
                fprintf(stderr, "SQL error (begin transaction): %s\n", errMsg);
                sqlite3_free(errMsg);
                return 1;
            }

            // Create the new table without 'track' and with the new primary key
            char* sql_create = "CREATE TABLE BestLaps_new ("
                "layout INT, "
                "car INT, "
                "lap FLOAT, "
                "fuel FLOAT, "
                "PRIMARY KEY(layout, car));";
            rc = sqlite3_exec(db, sql_create, 0, 0, &errMsg);
            if (rc != SQLITE_OK) {
                fprintf(stderr, "SQL error (creating new table): %s\n", errMsg);
                sqlite3_free(errMsg);
                return 1;
            }

            // Copy data from the old table to the new table (excluding 'track')
            char* sql_copy = "INSERT INTO BestLaps_new(layout, car, lap, fuel) "
                "SELECT layout, car, lap, fuel FROM BestLaps;";
            rc = sqlite3_exec(db, sql_copy, 0, 0, &errMsg);
            if (rc != SQLITE_OK) {
                fprintf(stderr, "SQL error (copying data): %s\n", errMsg);
                sqlite3_free(errMsg);
                return 1;
            }

            // Drop the old table
            char* sql_drop = "DROP TABLE BestLaps;";
            rc = sqlite3_exec(db, sql_drop, 0, 0, &errMsg);
            if (rc != SQLITE_OK) {
                fprintf(stderr, "SQL error (dropping old table): %s\n", errMsg);
                sqlite3_free(errMsg);
                return 1;
            }

            // Rename the new table to the original table name
            char* sql_rename = "ALTER TABLE BestLaps_new RENAME TO BestLaps;";
            rc = sqlite3_exec(db, sql_rename, 0, 0, &errMsg);
            if (rc != SQLITE_OK) {
                fprintf(stderr, "SQL error (renaming new table): %s\n", errMsg);
                sqlite3_free(errMsg);
                return 1;
            }

            // Commit the transaction
            char* sql_commit = "COMMIT;";
            rc = sqlite3_exec(db, sql_commit, 0, 0, &errMsg);
            if (rc != SQLITE_OK) {
                fprintf(stderr, "SQL error (committing transaction): %s\n", errMsg);
                sqlite3_free(errMsg);
                return 1;
            }

            fprintf(stdout, "Table altered successfully\n");
        } else
        if (exists) {
            char* sql_rename_column = "ALTER TABLE BestLaps RENAME COLUMN track TO layout;";
            rc = sqlite3_exec(db, sql_rename_column, 0, 0, &errMsg);
            if (rc != SQLITE_OK) {
                fprintf(stderr, "SQL error (renaming column): %s\n", errMsg);
                sqlite3_free(errMsg);
            }
            else {
                fprintf(stdout, "Column renamed successfully\n");
            }
        }
        fprintf(stdout, "Opened database successfully\n");
    }

    /*char* sql14 = "DELETE FROM `BestLaps` WHERE `track`=1678;";
    rc = sqlite3_exec(db, sql14, 0, 0, &errMsg);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", errMsg);
        sqlite3_free(errMsg);
    }
    else {
        fprintf(stdout, "Table created successfully\n");
    }*/
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

    char* sql1 = "CREATE TABLE BestLaps( layout INT, car INT, lap FLOAT, fuel FLOAT, PRIMARY KEY(layout, car) );";
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
    /*int sequence[] = {122, 3, 238, 3, 0, 0, 71, 0, 171, 2, 143, 3, 0, 0, 79, 0, 201, 4, 8, 2, 0, 0, 66, 0, 197, 3, 168, 2, 1, 0, 0, 0, 250, 4, 118, 0, 1, 0, 0, 0, 167, 5, 113, 0, 1, 0, 0, 0, 249, 4, 187, 0, 1, 0, 0, 0, 248, 5, 221, 3, 1, 0, 0, 0, 203, 4, 201, 3, 0, 0, 34, 0, 53, 1, 136, 0, 0, 0, 96, 0, 72, 2, 63, 2, 1, 0, 0, 0, 68, 0, 64, 1, 0, 0, 47, 0, 0, 0
    };
    int size = sizeof(sequence) / sizeof(sequence[0]);

    char* hexString = (char*)malloc(size * sizeof(char) * 2 + 1);
    for (int i = 0; i < size; i++) {
        sprintf_s(hexString + i * 2, size * 2 + 1 - i * 2, "%02X", sequence[i]);
    }

    char* insertSQL = (char*)malloc(1000 * sizeof(char));
    sprintf_s(insertSQL, 1000, "INSERT INTO WindowsSettings (data) VALUES (0x%s);", hexString);*/
    /*const char* insertSQL = "INSERT INTO WindowsSettings (data) VALUES (122, 3, 238, 3, 0, 0, 71, 0, 171, 2, 143, 3, 0, 0, 79, 0, 201, 4, 8, 2, 0, 0, 66, 0, 197, 3, 168, 2, 1, 0, 0, 0, 250, 4, 118, 0, 1, 0, 0, 0, 167, 5, 113, 0, 1, 0, 0, 0, 249, 4, 187, 0, 1, 0, 0, 0, 248, 5, 221, 3, 1, 0, 0, 0, 203, 4, 201, 3, 0, 0, 34, 0, 53, 1, 136, 0, 0, 0, 96, 0, 72, 2, 63, 2, 1, 0, 0, 0, 68, 0, 64, 1, 0, 0, 47, 0, 0, 0);";
    */

    /* const char* insertSQL = "INSERT INTO WindowsSettings (data) VALUES (0x7A, 0x03, 0xEE, 0x03, 0x00, 0x00, 0x47, 0x00, 0xAB, 0x02, 0x8F, 0x03, 0x00, 0x00, 0x4F, 0x00, 0xC9, 0x04, 0x08, 0x02, 0x00, 0x00, 0x42, 0x00, 0xC5, 0x03, 0xA8, 0x02, 0x01, 0x00, 0x00, 0x00, 0xFA, 0x04, 0x76, 0x00, 0x01, 0x00, 0x00, 0x00, 0xA7, 0x05, 0x71, 0x00, 0x01, 0x00, 0x00, 0x00, 0xF9, 0x04, 0xBB, 0x00, 0x01, 0x00, 0x00, 0x00, 0xF8, 0x05, 0xDD, 0x03, 0x01, 0x00, 0x00, 0x00, 0xCB, 0x04, 0xC9, 0x03, 0x00, 0x00, 0x22, 0x00, 0x35, 0x01, 0x88, 0x00, 0x00, 0x00, 0x60, 0x00, 0x48, 0x02, 0x3F, 0x02, 0x01, 0x00, 0x00, 0x00, 0x44, 0x00, 0x40, 0x01, 0x00, 0x00, 0x2F, 0x00, 0x00, 0x00);";
     */

     /*const char* insertSQL = "INSERT INTO WindowsSettings (data) VALUES (x'7A03EE0300004700AB028F0300004F00C904080200004200C503A802010000FA047600010000A7057100010000F904BB00010000F805DD03010000CB04C903000022003501880000600048023F020100004400400100002F000000');";*/

    const char* insertSQL = "INSERT INTO WindowsSettings (data) VALUES (0x0);";
    rc = sqlite3_exec(db, insertSQL, 0, 0, &errMsg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", errMsg);
        sqlite3_free(errMsg);
    }
    else {
        fprintf(stdout, "Record inserted successfully\n");
    }


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
    BlobResult result = { 0, (unsigned char*)malloc(200 * sizeof(unsigned char)) };
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
        //for (int i = 0; i < size; i++) {
        //    printf("%d: %d\n", i, src[i]);  // Print as integer
        //}
        unsigned char dest[200];
        memcpy(dest, src, size);
        result.data = &dest;
        //memcpy(result.data, src, size);
        //const unsigned char* new_var = result.data;
        result.size = size;
        //for (int i = 0; i < size; i++) {
        //    printf("%d: %d\n", i, result.data[i]);  // Print as integer
        //}
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


void readBestLapFuel(LapsAndFuel* lapsAndFuel, int32_t layout, int32_t car) {
    sqlite3* db;
    sqlite3_stmt* stmt;
    //int track = 1;  // replace with your track number
    //int car = 2;  // replace with your car number
    //printf("read: %d, %d, %d\n", track, layout, car);

    // Open the database
    if (sqlite3_open("data.db", &db) != SQLITE_OK) {
        printf("Can't open database: %s readBestLapFuel\n", sqlite3_errmsg(db));
        return 1;
    }

    // Prepare the SQL statement
    char sql[] = "SELECT * FROM BestLaps WHERE layout = ? AND car = ?";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK) {
        //printf("Failed to prepare statement readBestLapFuel\n");
        printf("Failed to prepare statement: %s readBestLapFuel\n", sqlite3_errmsg(db));
        return 1;
    }

    // Bind the parameters
    sqlite3_bind_int(stmt, 1, layout);
    sqlite3_bind_int(stmt, 2, car);
    lapsAndFuel->allBestLap = 9999;
    lapsAndFuel->allBestFuel = 9999;
    // Step through the result set
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        lapsAndFuel->allBestLap = sqlite3_column_double(stmt, 2);
        lapsAndFuel->allBestFuel = sqlite3_column_double(stmt, 3);
        /*printf("track: %d, layout: %d, car: %d, lap: %f, fuel: %f\n",
            sqlite3_column_int(stmt, 0),
            sqlite3_column_int(stmt, 1),
            sqlite3_column_int(stmt, 2),
            sqlite3_column_double(stmt, 3),
            sqlite3_column_double(stmt, 4));*/
    }

    // Finalize the statement and close the database
    sqlite3_finalize(stmt);

    /*char* sql2 = "CREATE TABLE BestLaps( track INT, layout INT, car INT, lap FLOAT, fuel FLOAT, PRIMARY KEY(track, layout, car) );";
    char* sql4 = "DROP TABLE BestLaps;";

    char* err_msg = 0;


    if (sqlite3_exec(db, sql4, 0, 0, &err_msg) != SQLITE_OK) {
        printf("SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
        sqlite3_close(db);
        return 1;
    }
    if (sqlite3_exec(db, sql2, 0, 0, &err_msg) != SQLITE_OK) {
        printf("SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
        sqlite3_close(db);
        return 1;
    }*/
    sqlite3_close(db);
}

void writeBestLapFuel(LapsAndFuel* lapsAndFuel, int32_t layout, int32_t car) {
    //printf("write: %d, %d, %d\n", track, layout, car);
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
    char sql[] = "INSERT OR REPLACE INTO BestLaps (layout, car, lap, fuel) VALUES (?, ?, ?, ?)";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK) {
        //printf("Failed to prepare statement writeBestLapFuel\n");
        printf("Failed to prepare statement: %s writeBestLapFuel\n", sqlite3_errmsg(db));
        return 1;
    }
    // Bind the parameters
    sqlite3_bind_int(stmt, 1, layout);
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
//#pragma optimize("", on)
