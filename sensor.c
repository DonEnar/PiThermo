#include "sensor.h"
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

SensorList *GetSensors(char **sensorNames, int sensorNamesCount)
{
    DIR *dir;
    struct dirent *dirEntry;

    SensorList *sensorList = malloc(sizeof(SensorList));
    sensorList->SensorCount=0;

    if(!(dir = opendir(ONEWIREDEVICELOCATION)))
        return sensorList;

    while((dirEntry = readdir(dir))!=NULL)
    {
        if(strncmp(dirEntry->d_name,DS18B20FAMILYCODE,strlen(DS18B20FAMILYCODE))==0)
        {
            sensorList->SensorCount++;
        }
    }
    if(sensorList->SensorCount==0)
        return sensorList;
    rewinddir(dir);

    sensorList->Sensors=malloc(sizeof(Sensor*) * sensorList->SensorCount);
    Sensor **currentSensor = sensorList->Sensors;
    int sensorNamesAllocated=0;

    while ((dirEntry=readdir(dir)))
    {
        if(strncmp(dirEntry->d_name, DS18B20FAMILYCODE, strlen(DS18B20FAMILYCODE))==0)
        {
            char *sensorName;
            char *sensorNR;
            if(sensorNamesCount > sensorNamesAllocated)
            {
                sensorName = strdup(*sensorNames);
                sensorNames++;
                sensorNamesAllocated++;
            }else
            {
                sensorName=strdup(DEFAULTSENSORNAME);
            }
            sensorNR = strdup(dirEntry->d_name);
            // so setzt man einen String zusammen!
            char sensorFilePath[285];
            sprintf(sensorFilePath, "%s%s%s", ONEWIREDEVICELOCATION, dirEntry->d_name, OneWIRESLAVEDEVICE);
            *currentSensor=GetSensor(sensorFilePath,sensorName, sensorNR);//dirEntry->d_name);
            currentSensor++;
        }
    }
    closedir(dir);
    return sensorList;
}
Sensor *GetSensor(char *sensorID, char *sensorName, char *sensorNR) 
{
    Sensor *sensor = malloc(sizeof(Sensor));
    sensor->SensorFile=fopen(sensorID,"r");
    sensor->SensorName=sensorName;
    sensor->SensorNR=sensorNR;
    //printf("%s",sensorNR);
    return sensor;
}
float ReadTemperature(const Sensor *sensor)
{
    long deviceFileSize;
    char *buffer;
    FILE *deviceFile = sensor->SensorFile;
    fseek(deviceFile, 0, SEEK_END);
    //printf("Sensor.c/readtemp: %s\n",sensor->SensorNR);
    deviceFileSize=ftell(deviceFile);
    fseek(deviceFile, 0, SEEK_SET);
    buffer=calloc(deviceFileSize, sizeof(char));
    fread(buffer, sizeof(char), deviceFileSize, deviceFile);
    char *temperatureComponent=strstr(buffer, "t=");
    if(!temperatureComponent)
    {
        printf("!temperatureComponent\n");
        free(buffer);
        return -1;
    }
    temperatureComponent += 2;
    float temperatureFloat = atof(temperatureComponent);
    temperatureFloat = temperatureFloat/1000;
    if(temperatureFloat<-55)
        temperatureFloat=-55;
    else if(temperatureFloat>125)
        temperatureFloat=125;
    free(buffer);
    return temperatureFloat;
}
void FreeSensors(SensorList *sensorList)
{
    if(!sensorList)
        return;
    for(int i = 0; i< sensorList->SensorCount;i++)
        FreeSensor(sensorList->Sensors[i]);
    free(sensorList->Sensors);
    free(sensorList);
}
void FreeSensor(Sensor *sensor)
{
    if(!sensor)
        return;
    if(sensor->SensorFile)
        fclose(sensor->SensorFile);
    free(sensor->SensorName);
    free(sensor);
}