#ifndef _structtest_h
#define _structtest_h

#include <stdio.h>

// Umgang mit Pointern in Strukturen... kompliziert!

#define DEFAULTSENSORNAME "Sensor"

typedef struct Entries
{
    char *SensorID;
    double Temperatur;
    char *DateTime;
} Entries;

typedef struct EntryList
{
    Entries **Sensors;
    int EntryCount;
} EntryList;

Entries *GetEntry(char *sensorID, double temperatur, char *datetime);
EntryList *GetEntries(char **sensorNames, int sensorNamesCount);
void ReadEntryList(EntryList *entryListe);
void FreeEntryList(EntryList *entryList);
void FreeEntries(Entries *entry);
#endif