#ifndef _dbAccess_h
#define _dbAccess_h

#include <sqlite3.h>
#include <stdio.h>
#include <gtk/gtk.h>

#define DEFAULTSENSORNAME "Sensor"

typedef struct TEMPENTRY
{
    char *SensorID;
    double Temperatur;
    char *DateTime;
} TEMPENTRY;
typedef struct TEMPENTRYLIST
{
    TEMPENTRY **Sensors;
    int tempEntryCount;
}TEMPENTRYLIST;

int CreateNewDB(char *ID, float *Temperatur, char DateTime[], GtkWidget *ErrorWindow); //
int InsertEntry(char *ID, float *Temperatur, char DateTime[], GtkWidget *ErrorWindow); //
TEMPENTRY* GetLastEntry();                                                             //
TEMPENTRYLIST *GetLast24Hours();                                                       //
int callback(void *notused, int argc, char **argv, char **azColName);

//Alles um eine Liste zu erstellen
TEMPENTRY *GetEntry(char *sensorID, double temperatur, char *datetime);
TEMPENTRYLIST *GetEntries(char **sensorNames, int sensorNamesCount);
void ReadEntryList(TEMPENTRYLIST*entryListe);
void FreeEntryList(TEMPENTRYLIST *entryList);
void FreeEntries(TEMPENTRY *entry);
#endif