#include <pthread.h>
#include <unistd.h>
#include <gtk/gtk.h>
#include <stdio.h>
#include <time.h>
#include <signal.h>
#include <time.h>
#include "chart.h"
#include "dbAccess.h"

static cairo_surface_t *surface = NULL;


// Sven: Statisches Zeichnen der Koordinaten
void DrawCoord(cairo_t *cr)
{
    #pragma region Basis
    cairo_set_source_rgb(cr,0,0,0);
    cairo_set_line_width(cr,1);
    cairo_move_to(cr,20,10);
    cairo_line_to(cr,20,320);
    cairo_line_to(cr,290,320);
    cairo_stroke(cr);
    #pragma endregion
    #pragma region Linien
    cairo_set_line_width(cr,0.2);
    // Senkrechte Reihe
    for(int i=0;i<31;i++)
    {
        cairo_move_to(cr,17,((i*10)+10));
        if(i==15)
        {
            cairo_set_line_width(cr,0.7);
            cairo_line_to(cr,290,((i*10)+10));
        }
        else
        {
            cairo_set_line_width(cr,0.2);
            cairo_line_to(cr,23,((i*10)+10));
        }
        cairo_stroke(cr);
    }
    // Waagerechte Reihe
    for(int i=2;i<29;i++)
    {
        cairo_move_to(cr,((i*10)+10),317);
        cairo_line_to(cr,((i*10)+10),323);
        cairo_stroke(cr);
    }
    #pragma endregion
    #pragma region Temperatur
    static PangoLayout *layout = NULL;
    PangoFontDescription *desc;
    int width,height;
    if(layout==NULL)
    {
        layout=pango_cairo_create_layout(cr);
        desc=pango_font_description_from_string("Sans Bold 6");
        pango_layout_set_font_description(layout,desc);
        pango_font_description_free(desc);
    }
    // Senkrechte ; -30 bis +30
    for(int j=0;j<62;j+=2)
    {
        char text[4];
        snprintf(text,4,"%i",(j-30));
        pango_layout_set_text(layout,text,-1);
        pango_cairo_update_layout(cr,layout);
        pango_layout_get_size(layout,&width,&height);
        if(j<22)
            cairo_move_to(cr,0,305-((j)*5));
        else if(j<30)
            cairo_move_to(cr,6,305-((j)*5));
        else if(j<40)
            cairo_move_to(cr,9,305-((j)*5));
        else
            cairo_move_to(cr,3,305-((j)*5));
        pango_cairo_show_layout(cr,layout);
    }
    #pragma endregion
    #pragma region Zeit
    int h=0;//Startzeit
    for(int j=0+h;j<26+h;j+=4)
    {
        char text[6];
        if(j<24)
            snprintf(text,5,"%i:00",(j));
        else
            snprintf(text,6,"%i:00",(j-24));
        pango_layout_set_text(layout,text,-1);
        pango_cairo_update_layout(cr,layout);
        pango_layout_get_size(layout,&width,&height);
        if(j<10)
            cairo_move_to(cr,13+(j*10),320); // j*10 + 40
        else
            cairo_move_to(cr,10+(j*10),320); // j*10 + 40
        pango_cairo_show_layout(cr,layout);
    }
    #pragma endregion
}
// Sven: Dynamisches Zeichnen der Koordinaten
// entsprechend der jeweiligen Uhrzeit (zentriert)
// und der durchschnittlichen Temperatur (zentriert)
// cr = cairo_t
// size = Größe des Zeichenbereichs (dynamisch, veränderbar)
void DrawCoordDynamicly(cairo_t *cr, Size *size)
{
    #pragma region Basis
    float w,h;
    w = *size->width;
    h = *size->height;
    //printf("Chart: width = %f, height = %f \n", w, h);
    cairo_set_source_rgb(cr,0,0,0);
    cairo_set_line_width(cr,1);
    cairo_move_to(cr,20,10);
    cairo_line_to(cr,20,h-10); // 10 für Beschriftung lassen
    cairo_line_to(cr,w-10,h-10);
    cairo_stroke(cr);
    #pragma endregion

    time_t currentTime=time(NULL);
    struct tm *tm_struct=localtime(&currentTime);
    int hour = tm_struct->tm_hour;
    // currentTime soll in der mitte der chart sein (senkrechte Linie)
    #pragma region Linien
    cairo_set_line_width(cr,0.2);
    float breite = (w-40)/29; // w- 30 (links) - 10 (rechts) / 29 (Striche)
    // Waagerechte Reihe
    for(int i=hour-15;i<hour+15;i++)
    {
        int xpos= (((i-hour+15)*breite)+29);
        if(i==hour)
        {
            cairo_set_line_width(cr,1);
            cairo_set_source_rgb(cr,255,0,0);
            cairo_move_to(cr,xpos,h-7);
            cairo_line_to(cr,xpos,10);
        }
        else
        {
            cairo_set_line_width(cr,0.2);
            cairo_set_source_rgb(cr,0,0,0);
            cairo_move_to(cr,xpos,h-7);
            cairo_line_to(cr,xpos,h-13);
        }
        cairo_stroke(cr);
    }
    // Senkrechte Reihe
    int averageTemp = 0; // Durchschnittliche Temperatur aller CHARTENTRY's
    float hoehe = (h-10)/32;
    for(int i=averageTemp-15;i<averageTemp+15;i++)
    {
        int ypos=(((i-averageTemp+15)*hoehe)+10);
        cairo_move_to(cr,17,ypos);
        if(i==averageTemp)
        {
            cairo_set_line_width(cr,0.7);
            cairo_line_to(cr,w-10,ypos);
        }
        else
        {
            cairo_set_line_width(cr,0.2);
            cairo_line_to(cr,23,ypos);
        }
        cairo_stroke(cr);
    }
    #pragma endregion
    #pragma region Temperatur
    static PangoLayout *layout = NULL;
    PangoFontDescription *desc;
    int width,height;
    if(layout==NULL)
    {
        layout=pango_cairo_create_layout(cr);
        desc=pango_font_description_from_string("Sans Bold 6");
        pango_layout_set_font_description(layout,desc);
        pango_font_description_free(desc);
    }
    // Senkrechte ; averageTemp -30°C bis averageTemp +30°C
    //hoehe = (h-10)/31;
    hoehe = (h-20)/13; // nicht ok
    for(int i=averageTemp-30;i<averageTemp+31;i+=5)
    {
        int ypos=(h-((i-averageTemp+30)*hoehe/2)-17); // i=-30; avT=0; höhe=26.5; :: 10, soll 302
        char text[4];
        snprintf(text,4,"%i",(i-averageTemp)); // i=-30 , avTemp=0 : -30 , pos soll unten sein
        printf("i=%i; ypos=%i; text=%s \n",i,ypos,text);
        pango_layout_set_text(layout,text,-1);
        pango_cairo_update_layout(cr,layout);
        pango_layout_get_size(layout,&width,&height);
        if(i<-9) // bis -10 x=0
            cairo_move_to(cr,0,ypos);
            //cairo_move_to(cr,0,h-((i)*5));
        else if(i<0) // von -9 bis-1
            cairo_move_to(cr,6,ypos);
        else if(i<9) // von 0 bis 9
            cairo_move_to(cr,9,ypos);
        else // über 10
            cairo_move_to(cr,3,ypos);
        pango_cairo_show_layout(cr,layout);
    }
    #pragma endregion
}

void ClearSurface(void)
{
    cairo_t *cr;
    cr =cairo_create(surface);
    cairo_set_source_rgb(cr,1,1,1);
    cairo_paint(cr);
    cairo_destroy(cr);
}

// Neue Oberfläche zum Zeichnen schaffen
gboolean ConfigEventCB(GtkWidget *widget, __attribute__((unused))GdkEventConfigure *event, __attribute__((unused))gpointer data)
{
    if(surface)
        cairo_surface_destroy(surface);
    surface = gdk_window_create_similar_surface(gtk_widget_get_window(widget),CAIRO_CONTENT_COLOR,gtk_widget_get_allocated_width(widget),gtk_widget_get_allocated_height(widget));
    //make surface white
    ClearSurface();
    return TRUE;
}
