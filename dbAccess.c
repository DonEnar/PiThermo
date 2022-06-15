#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
//#include <stdbool.h>
//#include <time.h>
#include <locale.h>
//#include <time.h>
//#include <gtk/gtk.h>
#include "dbAccess.h"

sqlite3 *db;
const char *file= "test.db";
int callback(void *, int, char **, char **);
TEMPENTRYLIST *tempEntryList;
TEMPENTRY **teArray;
TEMPENTRY tempentry;
int count=0;

// return 0 = alles OK ; 1 = kann DB nicht öffnen; 3 = Fehler
// DateTime-Format : YYYY-MM-DD HH:MM:SS.SSS
int CreateNewDB(__attribute__((unused))char *ID, __attribute__((unused))float *Temperatur, __attribute__((unused))char DateTime[], __attribute__((unused))GtkWidget *ErrorWindow)
{
    char *err_msg = 0;
    int rc = sqlite3_open((char*)file,&db);
    if(rc != SQLITE_OK)
    {
        sqlite3_close(db);
        return 1;
    }
    //char text1[4000];
    //snprintf(text1, 4000, "DROP TABLE IF EXISTS TemperaturStatistik;"
    //                "CREATE TABLE TemperaturStatistik(Id INT NOT NULL, Temp FLOAT, DateTime DATETIME);" // Id = Thermometer-Nr(ohne 28-)
    //                "INSERT INTO TemperaturStatistik VALUES(123455, '20.5', '2022-05-23 11:36:30');"
    //                "INSERT INTO TemperaturStatistik VALUES(%i, %f, '%s');", *ID, *Temperatur, DateTime);
    char *text = "DROP TABLE IF EXISTS TemperaturStatistik;CREATE TABLE TemperaturStatistik(Name TEXT, Temp FLOAT, DateTime DATETIME);";
    //printf(text);
    char *sql = text;
    rc = sqlite3_exec(db, sql,0,0,&err_msg);
    if(rc != SQLITE_OK)
    {
        // ErrorWindow :: set Error-> &err_msg
        fprintf(stderr,"SQL Fehler: %s\n", err_msg);
        sqlite3_free(err_msg);
        sqlite3_close(db);
        return 3;
    }
    sqlite3_close(db);
    return 0;
}
// Last entry by DateTime
TEMPENTRY* GetLastEntry()
{
    TEMPENTRY *te = malloc(sizeof(TEMPENTRY));

    int rc = sqlite3_open((char*)file,&db); // 118
    if(rc != SQLITE_OK) { goto end; }       // 119
    char *text1 = "SELECT * FROM TemperaturStatistik WHERE datetime BETWEEN '2022-06-11 02:00:00' AND '2022-06-12 02:00:00';"; // 120
    //char *text1 = "SELECT * FROM TemperaturStatistik WHERE DateTime=(SELECT MAX(DateTime) FROM TemperaturStatistik);";
    sqlite3_stmt *res; // 116
    char *sql = text1; // 121
    //printf("%s\n",text);
    rc = sqlite3_prepare_v2(db,sql,-1, &res,0); // 122
    sqlite3_bind_int(res,1,3);                  // 129

    /*int step = */sqlite3_step(res);
//    te->SensorID = (char*)sqlite3_column_text(res,0); 
//    te->Temperatur = sqlite3_column_double(res,1);
//    te->DateTime = (char*)sqlite3_column_text(res,2);
    //printf("res1=%f, te=%f \n",t,te->Temperatur);
    // ; Temp=%f ; Time=%s
    //printf("ID=%i ; Temp=%lf ; Time=%s \n", te->SensorID, te->Temperatur,te->DateTime);
    sqlite3_finalize(res);
    end:
    sqlite3_close(db);
    return te;
}

TEMPENTRYLIST *GetLast24Hours()
{
    char **sensorNames = NULL;
    int sensorNamesCount = 0;
    TEMPENTRYLIST *entryList=GetEntries(sensorNames,sensorNamesCount);
    // ^^ ist alles hier gespeichert ^^
    //ReadEntryList(entryList);
    return entryList;
}

int InsertEntry(char *ID, float *Temperatur, char DateTime[], __attribute__((unused))GtkWidget *ErrorWindow)
{
    char *err_msg = 0;
    int rc = sqlite3_open((char*)file,&db);
    if(rc != SQLITE_OK)
    {
        sqlite3_close(db);
        return 1;
    }
    char text[4000];
    setlocale(LC_NUMERIC,"C");
    snprintf(text, 4000, "INSERT INTO TemperaturStatistik VALUES('%s', %f, '%s');", ID, *Temperatur, DateTime);
    //printf("%s\n",text);
    char *sql = text;
    rc = sqlite3_exec(db, sql,0,0,&err_msg);
    if(rc != SQLITE_OK)
    {
        // ErrorWindow :: set Error-> &err_msg
        fprintf(stderr,"SQL Fehler: %s\n", err_msg);
        sqlite3_free(err_msg);
        sqlite3_close(db);
        return 3;
    }
    sqlite3_close(db);
    return 0;
}

int callback(__attribute__((unused))void *notused, __attribute__((unused))int argc, __attribute__((unused))char **argv, __attribute__((unused))char **azColName)
{
    count++;
    return 0;
}

TEMPENTRY *GetEntry(char *sensorID, double temperatur, char *datetime) // Einen Entry abholen, daten werden darin gespeichert!
{
    TEMPENTRY *entry = malloc(sizeof(TEMPENTRY));
    entry->SensorID   = sensorID;
    entry->Temperatur = temperatur;
    entry->DateTime   = datetime;
    return entry;
}
TEMPENTRYLIST *GetEntries(__attribute__((unused))char **sensorNames, int sensorNamesCount) // EntryListe erstellen; sensorNames = leerer StringArray; sensorNamesCount = leerer int(=0)
{
    TEMPENTRYLIST *entryList = malloc(sizeof(TEMPENTRYLIST)); // neue Liste
    entryList->tempEntryCount=0;                              // count auf 0 setzen

    // jetzt daten aus db einlesen
    char *err_msg = 0;
    int rc = sqlite3_open((char*)file,&db);
    if(rc != SQLITE_OK) { goto end; }
    char *text1 = "SELECT * FROM TemperaturStatistik WHERE datetime BETWEEN '2022-06-11 02:00:00' AND '2022-06-12 02:00:00';";
    sqlite3_exec(db,text1, callback,0,&err_msg);
    sqlite3_stmt *res;
    char *sql = text1;
    rc = sqlite3_prepare_v2(db,sql,-1, &res,0);
    sqlite3_bind_int(res,1,3);

    entryList->tempEntryCount+=count;

    entryList->Sensors=malloc(sizeof(TEMPENTRY*) * entryList->tempEntryCount);
    TEMPENTRY **currentEntry = entryList->Sensors;
    int sensorNamesAllocated=0; // ??

    //for-Schleife 3 mal durchlaufen
    for(int i = 0; i < count; i++)
    {
        rc = sqlite3_step(res);
        if(rc==SQLITE_DONE) break; // Alles eingelesen
        else if(rc!=SQLITE_ROW)
        {
            fprintf(stderr, "Problem: %s\n", sqlite3_errmsg(db));
            goto end;
        }
        //char *sensorName;
        char *sensorID;
        double temperatur = 0;
        char *datetime = NULL;
            if(sensorNamesCount > sensorNamesAllocated)
            {
                //sensorName = strdup(*sensorNames);
                //sensorNames++;
                sensorNamesAllocated++;
            }else
            {
                //sensorName=strdup(DEFAULTSENSORNAME);
            }
            // Zuweisung von DB zu entry
            sensorID = strdup((char*)sqlite3_column_text(res,0));
            temperatur= sqlite3_column_double(res,1);
            datetime = strdup((char*)sqlite3_column_text(res,2));
            *currentEntry=GetEntry(sensorID,temperatur, datetime);
            currentEntry++;
    }
    sqlite3_finalize(res);
    end:
    sqlite3_close(db);
    return entryList;
}
void ReadEntryList(TEMPENTRYLIST *entryListe)
{
    TEMPENTRYLIST *entryList;
    entryList= entryListe;
    // while wird vorerst nicht benötigt
    //while (!sigintFlag)
    //{
        for(int i=0;i<entryList->tempEntryCount;i++)
        {
            printf("ID: %s ; DT: %s ; T: %f \n", entryList->Sensors[i]->SensorID, entryList->Sensors[i]->DateTime, entryList->Sensors[i]->Temperatur);
        }
    //    sleep(1);
    //}
}
void FreeEntryList(TEMPENTRYLIST *entryList)
{
    if(!entryList)
        return;
    for(int i = 0; i< entryList->tempEntryCount;i++)
        FreeEntries(entryList->Sensors[i]);
    free(entryList->Sensors);
    free(entryList);
}
void FreeEntries(TEMPENTRY *entry)
{
    if(!entry)
        return;
    free(entry->DateTime);
    free(entry->SensorID);
    entry->Temperatur=0;
    free(entry);
}