#include <gtk/gtk.h>
#include "DrawTemp.h"

void DrawTemp(cairo_t *cr, float temp, char *name)
{
    time_t currentTime;
    time(&currentTime);
    char temperatureString[28];
    snprintf(temperatureString, 28, "%s : %.2f°C", name,temp);
    char dateTimeStringBuffer[32];
    strftime(dateTimeStringBuffer, 32, "%d.%m.%Y %H:%M:%S",localtime(&currentTime));
    char allStr[64];
    snprintf(allStr,64,"%s - %s",dateTimeStringBuffer,temperatureString);
    printf("DrawTemp: %s\n", allStr);
    cairo_set_source_rgb(cr,0,0,0);
    char text[50];
    snprintf(text,50,"%s : %.2f",name,temp);
    static PangoLayout *layout = NULL;
    PangoFontDescription *desc;
    int width,height;
    if(layout==NULL)
    {
        layout=pango_cairo_create_layout(cr);
        desc=pango_font_description_from_string("Sans Bold 8");
        pango_layout_set_font_description(layout,desc);
        pango_font_description_free(desc);
    }
    //pango_layout_set_text(layout,text,-1);
    pango_layout_set_text(layout,allStr,-1);
    pango_cairo_update_layout(cr,layout);
    pango_layout_get_size(layout,&width,&height);
        
    cairo_move_to(cr,10,100);
    pango_cairo_show_layout(cr,layout);
    // funktioniert nicht!
    //cairo_destroy(cr);
}