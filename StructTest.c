#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include "StructTest.h"

sqlite3 *db;
const char *file= "test.db";

Entries *GetEntry(char *sensorID, double temperatur, char *datetime) // Einen Entry abholen, daten werden darin gespeichert!
{
    Entries *entry = malloc(sizeof(Entries));
    entry->SensorID   = sensorID;
    entry->Temperatur = temperatur;
    entry->DateTime   = datetime;
    return entry;
}
EntryList *GetEntries(char **sensorNames, int sensorNamesCount) // EntryListe erstellen; sensorNames = leerer StringArray; sensorNamesCount = leerer int(=0)
{
    EntryList *entryList = malloc(sizeof(EntryList)); // neue Liste
    entryList->EntryCount=0;                          // count auf 0 setzen


    // jetzt daten aus db einlesen
    int rc = sqlite3_open((char*)file,&db);
    if(rc != SQLITE_OK) { goto end; }
    char *text1 = "SELECT * FROM TemperaturStatistik WHERE datetime BETWEEN '2022-06-11 02:00:00' AND '2022-06-12 02:00:00';";
    sqlite3_stmt *res;
    char *sql = text1;
    rc = sqlite3_prepare_v2(db,sql,-1, &res,0);
    sqlite3_bind_int(res,1,3);

    // als Test wird der count auf 3 gesetzt
    entryList->EntryCount+=3;

    entryList->Sensors=malloc(sizeof(Entries*) * entryList->EntryCount);
    Entries **currentEntry = entryList->Sensors;
    int sensorNamesAllocated=0; // ??


    //for-Schleife 3 mal durchlaufen
    for(int i = 0; i < 3; i++)
    {
        rc = sqlite3_step(res);
        if(rc==SQLITE_DONE) break; // Alles eingelesen
        else if(rc!=SQLITE_ROW)
        {
            fprintf(stderr, "Problem: %s\n", sqlite3_errmsg(db));
            goto end;
        }
        char *sensorName;

        char *sensorID;
        double temperatur = 0;
        char *datetime = NULL;
            if(sensorNamesCount > sensorNamesAllocated)
            {
                sensorName = strdup(*sensorNames);
                sensorNames++;
                sensorNamesAllocated++;
            }else
            {
                sensorName=strdup(DEFAULTSENSORNAME);
            }
            // Zuweisung von DB zu entry
            sensorID = strdup((char*)sqlite3_column_text(res,0));
            temperatur= sqlite3_column_double(res,1);
            datetime = strdup((char*)sqlite3_column_text(res,2));
            printf("%s \n",datetime);
            
            *currentEntry=GetEntry(sensorID,temperatur, datetime);
            currentEntry++;
    }

    sqlite3_finalize(res);
    end:
    sqlite3_close(db);
    return entryList;
}
void ReadEntryList(EntryList *entryListe)
{
    EntryList *entryList;
    entryList= entryListe;
    // while wird vorerst nicht benötigt
    //while (!sigintFlag)
    //{
        for(int i=0;i<entryList->EntryCount;i++)
        {
            printf("ID: %s ; DT: %s ;T: %f \n", entryList->Sensors[i]->SensorID, entryList->Sensors[i]->DateTime, entryList->Sensors[i]->Temperatur);
            //printf("DT: %s \n", entryList->Sensors[i]->DateTime);
            //float temperature = ReadTemperature(entryList->Sensors[i]);
            //PrintTempToDA(sensorList->Sensors[i], temperature);
        }
    //    sleep(1);
    //}
}

int main(void)
{
    // Sensoren holen (von PiThermo kopiert)
    char **sensorNames = NULL;
    int sensorNamesCount = 0;
    EntryList *entryList=GetEntries(sensorNames,sensorNamesCount);
    // ^^ sollte hier gespeichert sein ^^
    ReadEntryList(entryList);
    return 0;
}