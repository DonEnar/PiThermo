#ifndef _sensor_h
#define _sensor_h

#include <stdio.h>

#define ONEWIREDEVICELOCATION "/sys/bus/w1/devices/"
#define OneWIRESLAVEDEVICE "/w1_slave"
#define DS18B20FAMILYCODE "28"
#define DEFAULTSENSORNAME "Sensor"

typedef struct Sensor
{
    char *SensorName;
    FILE *SensorFile;
    char *SensorNR;
} Sensor; 

typedef struct SensorList
{
    Sensor **Sensors;
    int SensorCount;
} SensorList;

SensorList *GetSensors(char **sensorName,int sensorNamesCount);
Sensor *GetSensor(char *sensorID,char *sensorName, char *sensorNR);
float ReadTemperature(const Sensor *sensor);
void FreeSensors(SensorList *sensorList);
void FreeSensor(Sensor *sensor);

#endif