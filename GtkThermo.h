#ifndef _gtkThermo_h
#define _gtkThermo_h

#include "sensor.h"
#include <gtk/gtk.h>

void Cleanup(SensorList *sensorList);
void* ReadTemperatureLoop(void* sensorList);
void LogTemperature(Sensor *sensor, float temperature);
void PrintTemperatureToTextViewLine(Sensor *sensor, int lineToPrintDataOn, float temperature);
void PrintTemperatureToTextView(Sensor *sensor, float temperature);
void PrintTempToDA(Sensor *sensor, float temperature);
void on_DrawArea_map_event(GtkWidget *widget, GdkEvent *event, gpointer user_data);
void on_DrawArea_size_allocate(GtkWidget *widget, GdkRectangle *allocation, gpointer user_data);
void DBTest();

#endif
