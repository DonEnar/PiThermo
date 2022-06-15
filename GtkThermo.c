//gcc -Wno-unknown-pragmas -Wall -Wextra -o Main GtkThermo.c sensor.c chart.c dbAccess.c DrawTemp.c $(pkg-config gtk+-3.0 --cflags --libs) -lsqlite3 -rdynamic

#include "GtkThermo.h"
#include "sensor.h"
#include "chart.h"
#include "DrawTemp.h"
#include "dbAccess.h"
#include <pthread.h>
#include <unistd.h>
#include <gtk/gtk.h>
#include <stdio.h>
#include <time.h>
#include <signal.h>

volatile sig_atomic_t sigintFlag = 0;
GtkWidget *textView;
GtkWidget *da;
gboolean *resized_ = FALSE;
float DaWidth, DaHeight;
float Temp=19.5;
char *Name[40]; 

//Neuzeichnen des screens
gboolean DrawCB(__attribute__((unused))GtkWidget *widget, cairo_t *cr, __attribute__((unused))gpointer data)
{
    //DrawCoord(cr);
    Size *s = malloc(sizeof(Size));
    s->width= &DaWidth;
    s->height=&DaHeight;
    //printf("DrawCB: width = %f, height = %f \n", DaWidth, DaHeight);
    //DrawCoordDynamicly(cr,s);
    DrawTemp(cr,Temp,Name);
    return FALSE;
}

void InsertText(gpointer data, char *text)
{
    GtkTextBuffer *b;
    const gchar *t;
    GtkTextMark *m;
    GtkTextIter i;
    b=gtk_text_view_get_buffer(GTK_TEXT_VIEW(data));
    t=text;
    m=gtk_text_buffer_get_insert(b);
    gtk_text_buffer_get_iter_at_mark(b,&i,m);
    gtk_text_buffer_insert(b,&i,"\n",-1);
    gtk_text_buffer_insert(b,&i,t,-1);
}

void SetText(gpointer data, char *text)
{
    GtkTextBuffer *b;
    const gchar *t;
    GtkTextMark *m;
    GtkTextIter i;
    b=gtk_text_buffer_new(NULL);
    gtk_text_view_set_buffer(data,b);
    t=text;
    m=gtk_text_buffer_get_insert(b);
    gtk_text_buffer_get_iter_at_mark(b,&i,m);
    gtk_text_buffer_insert(b,&i,t,-1);
}

void onSigInt(int signum)
{
    signum++;
    sigintFlag=1;
}

void SearchThermo(__attribute__((unused)) GtkWidget *widget, __attribute__((unused)) gpointer   data)
{
    SetText(textView,"Auch OK!");
    g_print("Suchen geklickt\n");
    signal(SIGINT,onSigInt);
    char **sensorNames = NULL;
    int sensorNamesCount = 0;
    SensorList *sensorList=GetSensors(sensorNames,sensorNamesCount);
    if(sensorList->SensorCount==0)
    {
        printf("No sensores found!\n");
        return;
    }
    printf("Gefundene Sensoren: %d\n", sensorList->SensorCount);

    pthread_t thread_id;
    pthread_create(&thread_id,NULL,ReadTemperatureLoop,(void*)sensorList);
}

void DeleteWindow(__attribute__((unused)) GtkWidget *widget, __attribute__((unused)) gpointer   data)
{
    g_print("Exit");
    gtk_main_quit();
}

gboolean windowDelete(__attribute__((unused)) GtkWidget *widget, __attribute__((unused)) GdkEvent  *event, __attribute__((unused)) gpointer  data)
{
    g_print("%s called.\n", __FUNCTION__);
    //return TRUE;
    return FALSE;
}

//works very fine!!
void SizeChanged(__attribute__((unused))GtkWidget *widget,GtkAllocation *allocation, __attribute__((unused)) void *data)
{
    //printf("SizeChanged: width = %d, height = %d \n", allocation->width, allocation->height);
    gtk_widget_set_size_request(da,allocation->width-20,allocation->height-170);
}

// DA Größe wenn es erstmals angezeigt wird
void on_DrawArea_map_event(GtkWidget *widget, __attribute__((unused))GdkEvent *event, __attribute__((unused))gpointer user_data)
{
    GtkAllocation  allocation;
    gtk_widget_get_allocation(widget, &allocation);
    //speichere größen global
    DaWidth = allocation.width;
    DaHeight = allocation.height;
    return;// FALSE;
}

// DA Größe nach Größenänderung
void on_DrawArea_size_allocate(__attribute__((unused))GtkWidget *widget, GdkRectangle *allocation, __attribute__((unused))gpointer user_data)
{
    DaWidth = allocation->width;
    DaHeight = allocation->height;
    //printf("on_DrawArea_size_allocate: width = %d, height = %d \n", allocation->width, allocation->height);
    //printf("on_DrawArea_size_allocate: DaWidth = %f, DaHeight = %f \n", DaWidth, DaHeight);
}

int main(int argc, char **argv)
{
    DBTest();
    GtkWidget *window;
    GtkBuilder *builder = NULL;

    gtk_init(&argc,&argv);
    builder = gtk_builder_new();
    if(gtk_builder_add_from_file(builder,"GladeThermo1.glade",NULL)==0)
    {
        printf("gtk_builder_add_from_file  FAILED\n");
        return(0);
    }
    window = GTK_WIDGET(gtk_builder_get_object(builder,"window1"));
    //window =GTK_WINDOW(gtk_builder_get_object(builder,"window1"));
    textView = GTK_WIDGET(gtk_builder_get_object(builder,"TextView1"));
    da = GTK_WIDGET(gtk_builder_get_object(builder,"DrawArea"));
    gtk_builder_connect_signals(builder,NULL);

    
    //SetText(textView,"Test OK°");

    //gtk_builder_connect_signals(builder,textView);
    g_object_unref(builder);
    gtk_widget_show(window);
    //gtk_widget_show_all(window);
    gtk_main();
    return 0;
}

void Cleanup(SensorList *sensorList)
{
    printf("Exiting...\n");
    FreeSensors(sensorList);
}

void* ReadTemperatureLoop(void* sensorListe)
//void ReadTemperatureLoop(SensorList *sensorList)
{
    SensorList *sensorList;
    sensorList= sensorListe;
    while (!sigintFlag)
    {
        for(int i=0;i<sensorList->SensorCount;i++)
        {
            //InsertText(textView,"Immer noch OK?");
            float temperature = ReadTemperature(sensorList->Sensors[i]);
            // LCD1602LINES ist nur bei LCD anzuwenden
            //PrintTemperatureToTextView(sensorList->Sensors[i], i % LCD1602LINES, temperature);
            //PrintTemperatureToTextView(sensorList->Sensors[i], temperature);
            PrintTempToDA(sensorList->Sensors[i], temperature);
            //LogTemperature(sensorList->Sensors[i],temperature);
        }
        sleep(9);
    }
}

void LogTemperature(Sensor *sensor, float temperature)
{
    time_t currentTime;
    time(&currentTime);
    char dateTimeStringBuffer[32];
    strftime(dateTimeStringBuffer, 32, "%d.%m.%Y %H:%M:%S",localtime(&currentTime));
    printf("%s - %s - %.2f°C\n", dateTimeStringBuffer, sensor->SensorName, temperature);
}

void PrintTemperatureToTextViewLine(__attribute__((unused))Sensor *sensor, __attribute__((unused))int lineToPrintDataOn, __attribute__((unused))float temperature)
{}

void PrintTemperatureToTextView(Sensor *sensor, float temperature)
{
    time_t currentTime;
    time(&currentTime);
    char temperatureString[18];
    snprintf(temperatureString, 18, "%s : %.2f°C", sensor->SensorName,temperature);
    char dateTimeStringBuffer[32];
    strftime(dateTimeStringBuffer, 32, "%d.%m.%Y %H:%M:%S",localtime(&currentTime));
    char allStr[54];
    snprintf(allStr,54,"%s - %s",dateTimeStringBuffer,temperatureString);
    printf("Test");
    PrintTempToDA(sensor, temperature);
    //SetText(textView,allStr);
}

void PrintTempToDA(Sensor *sensor, float temperature)
{
    Temp= temperature;
    printf("TEST %s\n",sensor->SensorNR);
    snprintf(Name, 40, "%s",sensor->SensorNR);
    //printf(Name);
    gtk_widget_queue_draw(da);
}

void DBTest()
{
    time_t currentTime;
    time(&currentTime);
    char dateTimeStringBuffer[32];
    strftime(dateTimeStringBuffer, 32, "%Y-%m-%d %H:%M:%S.000",localtime(&currentTime));
    char test[6];
    //printf("String = %s\n",test);
    snprintf(test,6,"test");
    float temp = 19.5;
    int id = 123456;
    //printf("Wert = %s",test);
    //PrintS(test);
    int ret = CreateNewDB(&id, &temp, dateTimeStringBuffer,NULL);
    if(ret==0)
        printf("1 OK 0\n");
    else if(ret==3)
        printf("2 Fehler 3");
    else printf("Anderes");
    TEMPENTRY *te = GetLastEntry();
    printf("ID=%i ; Temp=%f ; Time=%s \n", te->SensorID, te->Temperatur,te->DateTime);
    id=234567;
    temp+=5;
    snprintf(dateTimeStringBuffer, 32, "2022-05-24 21:30:00.000");
    ret = InsertEntry(&id, &temp,dateTimeStringBuffer,NULL);
    if(ret==0)
        printf("2 OK 0\n");
    else if(ret==3)
        printf("2 Fehler 3");

    te = GetLastEntry();
    printf("ID=%i ; Temp=%f ; Time=%s \n", te->SensorID, te->Temperatur,te->DateTime);
    return;
}