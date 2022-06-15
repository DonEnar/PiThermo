#ifndef _chart_h
#define _chart_h

#include <stdio.h>
#include <time.h>
#include <gtk/gtk.h>

typedef struct CHARTENTRY
{
    time_t *currentTime;
    float *temperature;
} CHARTENTRY;

typedef struct CHARTDATA
{
    CHARTENTRY ** chartdata;
    int id;
} CHARTDATA;

typedef struct Size
{
    float *width,*height;
} Size;

void DrawCoord(cairo_t *cr);
void DrawCoordDynamicly(cairo_t *cr,Size *size);
gboolean ConfigEventCB(GtkWidget *widget, GdkEventConfigure *event, gpointer data);
void ClearSurface(void);
#endif