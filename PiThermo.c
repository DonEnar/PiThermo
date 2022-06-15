// gcc -Wno-unknown-pragmas -Wall -Wextra -o Main3 PiThermo.c sensor.c dbAccess.c $(pkg-config gtk+-3.0 --cflags --libs) -lpigpio -lwiringPi -lbcm2835 -lsqlite3 -rdynamic
// gcc -Wno-unknown-pragmas -Wall -Wextra -o Main3 PiThermo.c sensor.c dbAccess.c MCP3008.c chart.c $(pkg-config gtk+-3.0 --cflags --libs) -lm -lbcm2835 -lsqlite3 -rdynamic

//github name : DonEnar pw:m6.yv_Cs42XPWtL

// In DrawCB ist der Verweis zum Zeichen der Gauge

#include <pthread.h>
#include <unistd.h>
#include <gtk/gtk.h>
#include <stdio.h>
#include <time.h>
#include <signal.h>
#include <math.h>
#include <locale.h>
//#include <wiringPi.h>
//#include <pigpio.h>
#include <bcm2835.h>
#include "sensor.h"
#include "MCP3008.h"
#include "dbAccess.h"
#include "chart.h"
//#include "ColorGauge.h"

#pragma region Struct
typedef struct Brush
{
    double RED;
    double GREEN;
    double BLUE;
}Brush ;
#pragma endregion

#pragma region Variablen
GtkWidget *da1; // Thermo
GtkWidget *da2; // Druck
GtkWidget *da3; // Flow
GtkWidget *da4; // Chart
GtkWidget *notebook; // Notebook
gboolean relais=FALSE;
static pthread_t drawingThread;
static pthread_t thermoThread;
static pthread_t mcp3008Thread;
static pthread_mutex_t mutex1, mutex2, mutex3, mutex4, mutex5, mutex6, mutex7, mutex8, mutex9;
//static pthread_mutex_t mutex2;
//static pthread_mutex_t mutex3;
//static pthread_mutex_t mutex4;
//static pthread_mutex_t mutex5;
static cairo_surface_t *surface1 = NULL;
static cairo_surface_t *surface2 = NULL;
static cairo_surface_t *surface3 = NULL;
static cairo_surface_t *surface4 = NULL;
static int surface1W, surface1H;
static int surface2W, surface2H;
static int surface3W, surface3H;
static int surface4W, surface4H;
volatile sig_atomic_t sigintFlag = 0;
float Da1Width, Da1Height,Da2Width, Da2Height,Da3Width, Da3Height,Da4Width, Da4Height;
float Temp=19.750;
//int ID;
int Pressure=0;
int Flow=0;
char Name[40]; //funktioniert ohne *
#pragma endregion

#pragma region Gauge-Init-Variablen
enum BarStyle { Flowing, Expanding, Blocking };
float g_Value=19.5;
float g_Minimum=0;
float g_Maximum=100;
static int g_BorderWidth=2;
enum BarStyle g_Style = Expanding;
Brush *lstBrushes;
int brushLenght;
int smooth = 6; //smooth zwischen 0 und 7 | 6 ist super
#pragma endregion

#pragma region Methoden-Init
void BuildGtkFromBuilder(int argc, char **argv);
void BuildGtkInC(int argc, char **argv);
void DeleteWindow(GtkWidget *widget, gpointer data);
gboolean DrawNB(GtkWidget *widget, cairo_t *cr, gpointer data);
gboolean DrawCB1(GtkWidget *widget, cairo_t *cr, gpointer data);
gboolean ConfigEventCB1(GtkWidget *widget, GdkEventConfigure *event, gpointer data);
gboolean DrawCB2(GtkWidget *widget, cairo_t *cr, gpointer data);
gboolean ConfigEventCB2(GtkWidget *widget, GdkEventConfigure *event, gpointer data);
static gboolean InvalidateCB(void *ptr);
void SearchThermo(GtkWidget *widget, gpointer data);
void on_DrawArea_map_event_da1(GtkWidget *widget, GdkEvent *event, gpointer user_data);
void on_DrawArea_map_event_da2(GtkWidget *widget, GdkEvent *event, gpointer user_data);
void on_DrawArea_map_event_da3(GtkWidget *widget, GdkEvent *event, gpointer user_data);
void on_DrawArea_map_event_da4(GtkWidget *widget, GdkEvent *event, gpointer user_data);
void on_DrawArea_size_allocate_da1(GtkWidget *widget, GdkRectangle *allocation, gpointer user_data);
void on_DrawArea_size_allocate_da2(GtkWidget *widget, GdkRectangle *allocation, gpointer user_data);
void on_DrawArea_size_allocate_da3(GtkWidget *widget, GdkRectangle *allocation, gpointer user_data);
void on_DrawArea_size_allocate_da4(GtkWidget *widget, GdkRectangle *allocation, gpointer user_data);
void SizeChanged(GtkWidget *widget, GtkAllocation *allocation, void *data);
gboolean SelectPage(GtkNotebook *notebook, gboolean *arg1, gpointer *data);
void* DrawTempOld(__attribute__((unused))void *ptr);
void* DrawTemp(void *ptr);
void* SaveThermos(void *ptr);
void* ReadMCP3008(void *ptr);
void onSigInt(int signum);
void ReadTemperatureLoop(SensorList *sensorListe);
void PrintTempToDA(Sensor *sensor, float temperature);
//void TestGpio();
//void TestPiGpio();
void TestBCM2835();
void DrawBlackGauge();
void DrawColorGauge();
void BuildColorList();
gboolean InsertColor(Brush neu, int position);
Brush InterpolateColors(Brush color1, Brush color2);
void DrawChart(cairo_t *cr, time_t startTime);
void DBTest();
void WriteToDB();
void AddPageToNotebook(GtkComboBox *widget, gpointer data);
#pragma endregion

int main(int argc, char **argv)
{
    //GetLastEntry();
    TEMPENTRYLIST *entryList = GetLast24Hours(); // Daten jetzt bearbeiten
    //ReadEntryList(entryList);
    //BuildGtkFromBuilder(argc,argv);
    BuildGtkInC(argc,argv);
    return 0;
}

#pragma region Events
gboolean DrawNB(__attribute__((unused))GtkWidget *widget, __attribute__((unused))cairo_t *cr, __attribute__((unused))gpointer data) { return FALSE; }
void DeleteWindow(__attribute__((unused)) GtkWidget *widget, __attribute__((unused)) gpointer   data)
{
    g_print("Exit\n");
    gtk_main_quit();
}
gboolean DrawCB1(__attribute__((unused))GtkWidget *widget, cairo_t *cr, __attribute__((unused))gpointer data)
{
    pthread_mutex_lock(&mutex1);
    if(surface1!=(cairo_surface_t*)NULL)
    {
        cairo_set_source_surface(cr,surface1,0,0);
        DrawColorGauge();
        cairo_paint(cr);
    }
    //printf("widged %s\n",widget->parent_instance.g_type_instance.g_class->g_type);
    
    //gtk_widget_show_all(widget);
    pthread_mutex_unlock(&mutex1);

    return FALSE;
}
gboolean DrawCB2(__attribute__((unused))GtkWidget *widget, cairo_t *cr, __attribute__((unused))gpointer data)
{
    pthread_mutex_lock(&mutex5);
    if(surface2!=(cairo_surface_t*)NULL)
    {
        cairo_set_source_surface(cr,surface2,0,0);
        //DrawChart(cr, NULL);
        cairo_paint(cr);
    }
    pthread_mutex_unlock(&mutex5);

    return FALSE;
}
gboolean DrawCB3(__attribute__((unused))GtkWidget *widget, cairo_t *cr, __attribute__((unused))gpointer data)
{
    pthread_mutex_lock(&mutex6);
    if(surface3!=(cairo_surface_t*)NULL)
    {
        cairo_set_source_surface(cr,surface3,0,0);
        //DrawChart(cr, NULL);
        cairo_paint(cr);
    }
    pthread_mutex_unlock(&mutex6);

    return FALSE;
}
gboolean DrawCB4(__attribute__((unused))GtkWidget *widget, cairo_t *cr, __attribute__((unused))gpointer data)
{
    pthread_mutex_lock(&mutex7);
    if(surface4!=(cairo_surface_t*)NULL)
    {
        cairo_set_source_surface(cr,surface4,0,0);
        DrawChart(cr, NULL);
        cairo_paint(cr);
    }
    pthread_mutex_unlock(&mutex7);

    return FALSE;
}
gboolean ConfigEventCB1(GtkWidget *widget, GdkEventConfigure *event, __attribute__((unused))gpointer data)
{
    if(event->type == GDK_CONFIGURE)
    {
        pthread_mutex_lock(&mutex1);
        if(surface1!=(cairo_surface_t*)NULL)
        {
            cairo_surface_destroy(surface1);
        }
        GtkAllocation allocation;
        gtk_widget_get_allocation(widget,&allocation);
        surface1 = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,allocation.width,allocation.height);
        surface1W = allocation.width;
        surface1H = allocation.height;
        pthread_mutex_unlock(&mutex1);
    }
    return TRUE;
}
gboolean ConfigEventCB2(GtkWidget *widget, GdkEventConfigure *event, __attribute__((unused))gpointer data)
{
    if(event->type == GDK_CONFIGURE)
    {
        pthread_mutex_lock(&mutex4);
        if(surface2!=(cairo_surface_t*)NULL)
        {
            cairo_surface_destroy(surface2);
        }
        GtkAllocation allocation;
        gtk_widget_get_allocation(widget,&allocation);
        surface2 = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,allocation.width,allocation.height);
        surface2W = allocation.width;
        surface2H = allocation.height;
        pthread_mutex_unlock(&mutex4);
    }
    return TRUE;
}
gboolean ConfigEventCB3(GtkWidget *widget, GdkEventConfigure *event, __attribute__((unused))gpointer data)
{
    if(event->type == GDK_CONFIGURE)
    {
        pthread_mutex_lock(&mutex8);
        if(surface3!=(cairo_surface_t*)NULL)
        {
            cairo_surface_destroy(surface3);
        }
        GtkAllocation allocation;
        gtk_widget_get_allocation(widget,&allocation);
        surface3 = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,allocation.width,allocation.height);
        surface3W = allocation.width;
        surface3H = allocation.height;
        pthread_mutex_unlock(&mutex8);
    }
    return TRUE;
}
gboolean ConfigEventCB4(GtkWidget *widget, GdkEventConfigure *event, __attribute__((unused))gpointer data)
{
    if(event->type == GDK_CONFIGURE)
    {
        pthread_mutex_lock(&mutex9);
        if(surface4!=(cairo_surface_t*)NULL)
        {
            cairo_surface_destroy(surface4);
        }
        GtkAllocation allocation;
        gtk_widget_get_allocation(widget,&allocation);
        surface4 = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,allocation.width,allocation.height);
        surface4W = allocation.width;
        surface4H = allocation.height;
        pthread_mutex_unlock(&mutex9);
    }
    return TRUE;
}
static gboolean InvalidateCB(void *ptr)
{
    if(GTK_IS_WIDGET(ptr))
    {
        gtk_widget_queue_draw(GTK_WIDGET(ptr));
        return TRUE;
    }
    return FALSE;
}
void SearchThermo(__attribute__((unused)) GtkWidget *widget, __attribute__((unused)) gpointer   data)
{
    // Neuer Zeichen-Thread
    pthread_mutex_init(&mutex1,NULL);
    pthread_create(&drawingThread,NULL,DrawTemp,NULL);
    // Timer um Fenster neu zu zeichnen, 1Hz
    g_timeout_add(1,InvalidateCB,da1);
    //g_timeout_add(1,InvalidateCB,nb);

    //Neuer Thermometer-Thread
    pthread_mutex_init(&mutex2,NULL);
    pthread_create(&thermoThread,NULL,SaveThermos,NULL);

    //Neuer MCP3008-Thread
    pthread_mutex_init(&mutex3,NULL);
    pthread_create(&mcp3008Thread,NULL,ReadMCP3008,NULL);
}
void on_DrawArea_map_event_da1(GtkWidget *widget, __attribute__((unused))GdkEvent *event, __attribute__((unused))gpointer user_data)
{
    GtkAllocation  allocation;
    gtk_widget_get_allocation(widget, &allocation);
    //speichere größen global
    Da1Width = allocation.width;
    Da1Height = allocation.height;
    return;// FALSE;
}
void on_DrawArea_map_event_da2(GtkWidget *widget, __attribute__((unused))GdkEvent *event, __attribute__((unused))gpointer user_data)
{
    GtkAllocation  allocation;
    gtk_widget_get_allocation(widget, &allocation);
    //speichere größen global
    Da2Width = allocation.width;
    Da2Height = allocation.height;
    return;// FALSE;
}
void on_DrawArea_map_event_da3(GtkWidget *widget, __attribute__((unused))GdkEvent *event, __attribute__((unused))gpointer user_data)
{
    GtkAllocation  allocation;
    gtk_widget_get_allocation(widget, &allocation);
    //speichere größen global
    Da3Width = allocation.width;
    Da3Height = allocation.height;
    return;// FALSE;
}
void on_DrawArea_map_event_da4(GtkWidget *widget, __attribute__((unused))GdkEvent *event, __attribute__((unused))gpointer user_data)
{
    GtkAllocation  allocation;
    gtk_widget_get_allocation(widget, &allocation);
    //speichere größen global
    Da4Width = allocation.width;
    Da4Height = allocation.height;
    return;// FALSE;
}
void on_DrawArea_size_allocate_da1(__attribute__((unused))GtkWidget *widget, GdkRectangle *allocation, __attribute__((unused))gpointer user_data)
{
    Da1Width = allocation->width;
    Da1Height = allocation->height;
}
void on_DrawArea_size_allocate_da2(__attribute__((unused))GtkWidget *widget, GdkRectangle *allocation, __attribute__((unused))gpointer user_data)
{
    Da2Width = allocation->width;
    Da2Height = allocation->height;
}
void on_DrawArea_size_allocate_da3(__attribute__((unused))GtkWidget *widget, GdkRectangle *allocation, __attribute__((unused))gpointer user_data)
{
    Da3Width = allocation->width;
    Da3Height = allocation->height;
}
void on_DrawArea_size_allocate_da4(__attribute__((unused))GtkWidget *widget, GdkRectangle *allocation, __attribute__((unused))gpointer user_data)
{
    Da4Width = allocation->width;
    Da4Height = allocation->height;
}
void SizeChanged(__attribute__((unused))GtkWidget *widget, __attribute__((unused))GtkAllocation *allocation, __attribute__((unused)) void *data)
{
    //gtk_widget_set_size_request(da,allocation->width-20,allocation->height-170);
}
gboolean SelectPage(GtkNotebook *notebook, __attribute__((unused))gboolean *arg1, __attribute__((unused))gpointer *data)
{
    g_signal_emit_by_name(notebook,"switch-page");
    return TRUE;
}
#pragma endregion

#pragma region Threads
void* DrawTempOld(__attribute__((unused))void *ptr)
{
    while(1)
    {
        sleep(1);
        if(surface1==(cairo_surface_t*)NULL){continue;}
        pthread_mutex_lock(&mutex1);
        cairo_t *cr=cairo_create(surface1);

        // Hintergrund
        cairo_set_source_rgb(cr,0.92,0.92,0.92);
        cairo_rectangle(cr,0,0,surface1W,surface1H);
        cairo_fill(cr);

        // Strings erstellen für Zeit, Temperature, Druck und Durchfluss
        time_t currentTime;
        time(&currentTime);
        char temperatureString[479];
        snprintf(temperatureString, 479, "%s : %.2f°C", Name, Temp);
        char dateTimeStringBuffer[32];
        strftime(dateTimeStringBuffer, 32, "%d.%m.%Y %H:%M:%S",localtime(&currentTime));
        char dttStr[513];
        snprintf(dttStr,513,"%s - %s",dateTimeStringBuffer,temperatureString);
        cairo_set_source_rgb(cr,0,0,0);

        char pressureString[11];
        snprintf(pressureString,11,"Druck: %i",Pressure);

        char flowString[24];
        snprintf(flowString,24,"Durchfluss: %i",Flow);

        //Layout erstellen
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
        //Zeit und Temp
        pango_layout_set_text(layout,dttStr,-1);
        pango_cairo_update_layout(cr,layout);
        pango_layout_get_size(layout,&width,&height);
        cairo_move_to(cr,10,10);
        pango_cairo_show_layout(cr,layout);

        // Druck
        pango_layout_set_text(layout,pressureString,-1);
        pango_cairo_update_layout(cr,layout);
        pango_layout_get_size(layout,&width,&height);
        cairo_move_to(cr,10,50);
        pango_cairo_show_layout(cr,layout);

        // Durchfluss
        pango_layout_set_text(layout,flowString,-1);
        pango_cairo_update_layout(cr,layout);
        pango_layout_get_size(layout,&width,&height);
        cairo_move_to(cr,10,90);
        pango_cairo_show_layout(cr,layout);

        cairo_destroy(cr);
        pthread_mutex_unlock(&mutex1);
    }
    return NULL;
}
void* DrawTemp(__attribute__((unused))void *ptr)
{
    // Temperatur in Gauge darstellen; Zeiger und Text
    while (1)
    {
        sleep(1);
        pthread_mutex_lock(&mutex1);
        DrawColorGauge();
        pthread_mutex_unlock(&mutex1);
    }
    return NULL;
}
void* SaveThermos(__attribute__((unused))void *ptr)
{
    pthread_mutex_lock(&mutex2);
    signal(SIGINT,onSigInt);
    char **sensorNames = NULL;
    int sensorNamesCount = 0;
    SensorList *sensorList=GetSensors(sensorNames,sensorNamesCount);
    if(sensorList->SensorCount==0)
    {
        printf("Keine Sensoren gefunden!\n");
        return NULL;
    }
    printf("%d Sensor(en) gefunden!\n", sensorList->SensorCount);
    ReadTemperatureLoop(sensorList);
    pthread_mutex_unlock(&mutex2);
    return NULL;
}
void* ReadMCP3008(__attribute__((unused))void *ptr)
{
    pthread_mutex_lock(&mutex3);
    int *p;
    for(;;)
    {
        p = RunMCP();
        Pressure= p[0];
        Flow=p[7];
        sleep(1);
    }
    pthread_mutex_unlock(&mutex3);
    return NULL;
}
#pragma endregion
#pragma region Methoden
void BuildGtkFromBuilder(int argc, char **argv)
{
    setlocale(LC_NUMERIC,"C");
    //DBTest();
    GtkWidget *window;
    GtkBuilder *builder = NULL;

    gtk_init(&argc,&argv);
    builder = gtk_builder_new();
    if(gtk_builder_add_from_file(builder,"GladeThermo1.glade",NULL)==0)
    {
        printf("gtk_builder_add_from_file  FAILED\n");
        return;
    }
    window = GTK_WIDGET(gtk_builder_get_object(builder,"window1"));
    da1 = GTK_WIDGET(gtk_builder_get_object(builder,"DrawArea1"));
    notebook = GTK_WIDGET(gtk_builder_get_object(builder,"Notebook"));
    gtk_builder_connect_signals(builder,NULL);
    g_object_unref(builder);
    gtk_widget_show(window);
    gtk_main();
    return;
}
void BuildGtkInC(int argc, char **argv)
{
    GtkWidget *window, *searchButton, *fixed;//, *notebook;
    gtk_init(&argc,&argv);
    //window
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window),"Pi Thermo");
    gtk_window_set_default_size(GTK_WINDOW(window),402,398);
    gtk_window_set_position(GTK_WINDOW(window),GTK_WIN_POS_CENTER);
    //fixedf
    fixed = gtk_fixed_new();
    gtk_container_add(GTK_CONTAINER(window),fixed);
    //button
    searchButton = gtk_button_new_with_label("Thermometer suchen");
    gtk_widget_set_size_request(searchButton,382,30);
    gtk_fixed_put(GTK_FIXED(fixed),searchButton,10,10);
    //notebook
    notebook = gtk_notebook_new();
    gtk_widget_set_size_request(notebook,382,338);
    AddPageToNotebook(NULL,notebook);
    gtk_fixed_put(GTK_FIXED(fixed),notebook,10,50);
    g_signal_connect(searchButton,"clicked",G_CALLBACK(SearchThermo),NULL);
    g_signal_connect(window,"delete-event",G_CALLBACK(DeleteWindow),NULL);
    g_signal_connect(notebook,"draw",G_CALLBACK(DrawNB),NULL);
    gtk_widget_show_all(window);
    gtk_main();
    return;
}
void onSigInt(int signum)
{
    signum++;
    sigintFlag=1;
}
void ReadTemperatureLoop(SensorList *sensorListe)
{
    SensorList *sensorList;
    sensorList= sensorListe;
    while (!sigintFlag)
    {
        for(int i=0;i<sensorList->SensorCount;i++)
        {
            float temperature = ReadTemperature(sensorList->Sensors[i]);
            PrintTempToDA(sensorList->Sensors[i], temperature);
        }
        sleep(1);
    }
}
void PrintTempToDA(Sensor *sensor, float temperature)
{
    Temp= temperature;
    //printf("%.2f\n",temperature);
    if(!bcm2835_init())
        return;
    bcm2835_gpio_fsel(RPI_GPIO_P1_11, BCM2835_GPIO_FSEL_OUTP);
    if(((bcm2835_gpio_lev(RPI_GPIO_P1_11))==HIGH) &!relais) //LED aus & relais = FALSE
        bcm2835_gpio_write(RPI_GPIO_P1_11,LOW); // LED geht an
    if(Temp>20)
    {
        if(!relais)
        {
            bcm2835_gpio_write(RPI_GPIO_P1_11,HIGH);
            //printf("größer\n");
            relais=TRUE;
        }
        //else
        //    bcm2835_gpio_write(RPI_GPIO_P1_11,LOW);
    }
    else
    {
        if(relais)
        {
            bcm2835_gpio_write(RPI_GPIO_P1_11,LOW);
            //printf("kleiner\n");
            relais=FALSE;
        }
        //else
        //    bcm2835_gpio_write(RPI_GPIO_P1_11,HIGH);
    }
    bcm2835_close();
    snprintf(Name, 40, "%s",sensor->SensorNR);
    WriteToDB();
}
/*
void TestGpio()
{
    wiringPiSetup();
    int pins[]={0,1,2,3,4,5,6,21,22,23,24,25,26,27,28,29}; // 7 nicht, ist Thermometer
    for(int i = 0;i<(int)(sizeof(pins)/sizeof(int));i++)
    {
        pinMode(pins[i],OUTPUT);
        digitalWrite(i,LOW);
    }
}
void TestPiGpio()
{
    if(gpioInitialise()<0)
        return;
    gpioSetMode(17,PI_OUTPUT);
    gpioWrite(17,0);
    gpioTerminate();
}
*/
void TestBCM2835()
{
    if(!bcm2835_init())
        return;
    bcm2835_gpio_fsel(RPI_GPIO_P1_11, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_write(RPI_GPIO_P1_11,HIGH);
    sleep(1);
    bcm2835_gpio_write(RPI_GPIO_P1_11,LOW);
    bcm2835_close();
}
// Array des Farbverlaufs erstellen
void BuildColorList()
{
    int idx, cnt,sdc; // lstBrush index, itemcount, sub-divine count
    brushLenght = ((int)pow(2,smooth+2)+1);
    // Arraylänge: 2hoch7 + 1 = 257
    lstBrushes = malloc(sizeof(Brush)*brushLenght);
    
    //int len = sizeof(lstBrushes)/sizeof(lstBrushes[0]);
    //printf("Insert %i\n",brushLenght);
    //printf("t: %i\n",t);
    // evtl. 5 Farben für besseren Grünbereich
    // die 3 Hauptfarben: links BLAU(kalt),mitte GRÜN(normal), rechts ROT(heiß)
    //ROT 255,40,0
    Brush b1;
    b1.RED=(128./255.); b1.GREEN=0; b1.BLUE=0;
    //printf("Brush 1 r= %lf , g=%lf , b=%lf\n",b1.RED,b1.GREEN,b1.BLUE);
    Brush b5;
    b5.RED=1; b5.GREEN=(40./255.); b5.BLUE=0;
    //GRÜN 11,102,35 | 57,255,20
    Brush b2;
    b2.RED=(57./255.); b2.GREEN=255./255.; b2.BLUE=(20./255.);
    //printf("Brush 2 r= %lf , g=%lf , b=%lf\n",b2.RED,b2.GREEN,b2.BLUE);
    //BLAU 0,87,217 | 0,48,143
    Brush b3;
    b3.RED=8./255.; b3.GREEN=37./255.; b3.BLUE=103./255.;
    Brush b4;
    b4.RED=15./255.; b4.GREEN=82./255.; b4.BLUE=186./255.;
    //printf("Brush 3 r= %lf , g=%lf , b=%lf\n",b3.RED,b3.GREEN,b3.BLUE);
    // dem Array zuweisen
    lstBrushes[0]=b3; lstBrushes[1] = b4; lstBrushes[2] = b2; lstBrushes[3] = b5; lstBrushes[4]=b1;
    //for(int i=0;i<brushLenght;i++)
    //    printf("Nach Zuweisung : Pos %i: R->%lf , G->%lf , B->%lf\n",i,lstBrushes[i].RED, lstBrushes[i].GREEN, lstBrushes[i].BLUE);
    // Farbverlauf erstellen
    // sdc : je größer, desto mehr Farben; 0->3 (Grund-) Farben; 1-> 5 Farben
    for(sdc=0;sdc<smooth; sdc++) // 3 means smoothness , gibts in c nicht, deswegen 3 pauschal, keine Ahnung für was das gut sein soll
    {
        idx =0;
        cnt=(pow(2,sdc+2));
        //printf("While %i\n",idx);
            // 0 < 2 beim ersten mal
        while(idx<cnt) // in erster for soll 2x 
        {
            //printf("While1 %i\n",cnt);
            // insert eine mittlere Farbe swischen 2 Array-Items
            // InterpolateColors(x,y) gibt die Farbe zurück
            Brush b =InterpolateColors((Brush)lstBrushes[idx],(Brush)lstBrushes[idx+1]);
            InsertColor(b,idx+1);
            //InsertColor(InterpolateColors(lstBrushes[idx],lstBrushes[idx+1]),idx+1);

            idx +=2;
            cnt++;
        }
    }
}
gboolean InsertColor(Brush neu, int position)
{
    //printf("Insert %i\n", position);
    for(int i=brushLenght-1;i>=position+1;i--)
    {
        lstBrushes[i]=lstBrushes[i-1];
    }
    // neuer Brush
    lstBrushes[position]=neu;
    //for(int i=0;i<brushLenght;i++)
    //    printf("InsertColor : Pos %i: R->%lf , G->%lf , B->%lf\n",i,lstBrushes[i].RED, lstBrushes[i].GREEN, lstBrushes[i].BLUE);
    return TRUE;
}
Brush InterpolateColors(Brush color1, Brush color2)
{
    Brush temp;
    //double r = color1.RED;
    double r = (color1.RED + color2.RED) /2;
    double g = (color1.GREEN + color2.GREEN)/2;
    double b = (color1.BLUE + color2.BLUE)/2;
    //printf("Interpol r=%lf\n",r);
    temp.RED = r;
    temp.GREEN = g;
    temp.BLUE = b;
    return temp;
}
//BlackGauge
void DrawBlackGauge()
{
    //if(surface!=(cairo_surface_t*)NULL){ printf("STOP"); return;}
    cairo_t *cr=cairo_create(surface1);

    // Hintergrund
    cairo_set_source_rgb(cr,0.92,0.92,0.92);
    cairo_rectangle(cr,0,0,surface1W,surface1H);
    cairo_fill(cr);
    
    // Gauge-Path, in Schwarz
    cairo_set_source_rgb(cr,0,0,0);
    cairo_arc(cr, surface1W/2, surface1H/2, surface1H/4*1.5, (135*(M_PI/180)), (405*(M_PI/180)));

    float x=((surface1H/4*1.5) * sinf(135*(M_PI/180)) + surface1W/2-25);
    float y=(fabs((surface1H/4*1.5) * cosf(135*(M_PI/180)))+surface1H/2-25);
    cairo_line_to(cr,x,y);
    cairo_arc_negative(cr, surface1W/2, surface1H/2, surface1H/4*1.5-25, (405*(M_PI/180)), (135*(M_PI/180)));
    cairo_close_path(cr);
    cairo_fill(cr);
    cairo_destroy(cr);
}
// übernommen von CircularGauge
void DrawColorGauge()
{
    setlocale(LC_NUMERIC,"C");
    BuildColorList();
    cairo_t *cr=cairo_create(surface1);

    #pragma region Hintergrund
    cairo_set_source_rgb(cr,0.92,0.92,0.92);
    cairo_rectangle(cr,0,0,surface1W,surface1H);
    cairo_fill(cr);
    #pragma endregion
    #pragma region Basiswerte
    float percentComplete = (g_Value-g_Minimum)/(g_Maximum-g_Minimum);//Standart 0.195 , OK
    if(percentComplete<=0.0f) return;
    if(percentComplete>1.0f) percentComplete = 1.0f;
    float totalWidth,fullwidth;
    fullwidth = (surface1W - g_BorderWidth);
    totalWidth = fullwidth * percentComplete;
    float barwidth=0;
    if(g_Style==Expanding) barwidth = totalWidth;
    else barwidth = fullwidth;
    barwidth /= brushLenght;
    int idxColor = 0;
    float angle=45;//startwinkel 45° ok
    float endAngle=315; // 45+270= 305° ok
    float step=270./brushLenght; // 270° / Anzahl Farben
    #pragma endregion
    #pragma region Gauge Zeichnen
    do
    {
        angle += step;
        // Farben setzen
        cairo_set_antialias(cr, CAIRO_ANTIALIAS_SUBPIXEL);
        cairo_set_source_rgb(cr,lstBrushes[idxColor].RED,lstBrushes[idxColor].GREEN,lstBrushes[idxColor].BLUE);
        // an ticken weiter gehen
        double tick = 0.75;
        cairo_arc(cr, surface1W/2, surface1H/2, surface1H/4*1.5, ((angle-step+90)*(M_PI/180)), ((angle+90+tick)*(M_PI/180)));//AUSSENRING
        cairo_arc_negative(cr, surface1W/2, surface1H/2, surface1H/4*1.5-25, ((angle+90+tick)*(M_PI/180)), ((angle-step+90)*(M_PI/180)));
        cairo_close_path(cr);
        cairo_fill(cr);
        idxColor++;
    }while (angle<endAngle);
    #pragma endregion ^^ bis hier läuft es super!!
    #pragma region Skalierung und Text
    // 16°C - 26°C // 27°(Kreis) pro °C
    cairo_select_font_face(cr, "Serif", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_text_extents_t extents;
    for(int i=0;i<101;i++)
    {
        float deg = (45+(i*2.7));
        float x1,x2,x3,y1,y2,y3;//,x4,y4;
        char Text[15];
        snprintf(Text,15,"%i°C",(26-(i/10)));

        // Textzentrum finden
        cairo_text_extents(cr,Text,&extents);

        //punkt innen; radius=radgauge+1pixel (versuch)
        float radA = surface1H/4*1.5 + 2; // Innen
        float radB = surface1H/4*1.5 + 12; // Außen
        float radC = surface1H/4*1.5 + 6; // Mitte

        float xa=(radA * sinf(deg*(M_PI/180)));
        x1=xa+surface1W/2;

        float ya=(radA * cosf(deg*(M_PI/180)));
        y1=ya+surface1H/2;

        float xb=(radB * sinf(deg*(M_PI/180)));
        x2=xb+surface1W/2;

        float yb=(radB * cosf(deg*(M_PI/180)));
        y2=yb+surface1H/2;

        float xc=(radC * sinf(deg*(M_PI/180)));
        x3=xc+surface1W/2;

        float yc=(radC * cosf(deg*(M_PI/180)));
        y3=yc+surface1H/2;

        //float xd=(radD *sinf(deg*(M_PI/180)));
        //x4=xd+surfaceW/2;//-extents.width/2;

        //float yd=(radD * cosf(deg*(M_PI/180)));
        //y4=yd+surfaceH/2;//-extents.height/2;

        int temp=i;
        while (temp>10)
        {
            temp-=10;
        }
        cairo_set_source_rgb(cr,0,0,0);
        if((temp%10)==0||i==0) // Ganze Gradzahlen
        {
            cairo_set_line_width(cr,2);
            cairo_move_to(cr,x1,y1);
            cairo_line_to(cr,x2,y2);
        }
        else
        {
            cairo_set_line_width(cr,1);
            cairo_move_to(cr,x1,y1);
            cairo_line_to(cr,x3,y3);
        }
        cairo_stroke(cr);
    }
    // ^^ bis hier läuft es!!
    cairo_set_font_size(cr,20);
    cairo_text_extents_t extents1;

    char Text1[15];
    snprintf(Text1,15,"%.1lf°C",g_Value);
    cairo_text_extents(cr,Text1,&extents1);
    cairo_move_to(cr, surface1W/2 - extents1.width/2, surface1H/2 - extents1.height);
    cairo_show_text(cr,Text1);

    snprintf(Text1,15,"%.1lf°C",Temp);
    cairo_text_extents(cr,Text1,&extents1);
    cairo_move_to(cr, surface1W/2 - extents1.width/2, surface1H/2 + extents1.height);
    cairo_show_text(cr,Text1);

    cairo_set_font_size(cr,10);
    snprintf(Text1,15,"Soll");
    cairo_text_extents(cr,Text1,&extents1);
    cairo_move_to(cr, surface1W/2 - extents1.width/2, surface1H/2 - extents1.height*6);
    cairo_show_text(cr,Text1);
    snprintf(Text1,15,"Ist");
    cairo_text_extents(cr,Text1,&extents1);
    cairo_move_to(cr, surface1W/2 - extents1.width/2, surface1H/2);// - extents1.height*6);
    cairo_show_text(cr,Text1);
    #pragma endregion
    #pragma region Zeiger
    // °C in Winkel umrechnen
    // 1.: Temp<16°C -> Temp=16°C ; 2.: Temp>26°C -> Temp=26°C
    if(Temp<16) Temp=16;
    if(Temp>26) Temp=26;
    //printf("DrawCG: Temp %f\n",Temp);
    // 315°(Winkel) - (pro 1°C 27°Winkel)
    float deg = (315-((Temp-16)*27)); // paßt

    float radA = surface1H/4*1.5 + 1; // Innen
    float radB = surface1H/4*1.5 - 26; // Außen

    float xa=(radA * sinf(deg*(M_PI/180)));
    float x1=xa+surface1W/2;

    float ya=(radA * cosf(deg*(M_PI/180)));
    float y1=ya+surface1H/2;

    float xb=(radB * sinf(deg*(M_PI/180)));
    float x2=xb+surface1W/2;

    float yb=(radB * cosf(deg*(M_PI/180)));
    float y2=yb+surface1H/2;
    cairo_set_line_width(cr,5);
    cairo_move_to(cr,x1,y1);
    cairo_line_to(cr,x2,y2);
    cairo_stroke(cr);
    #pragma endregion
    //GdkDrawingContext *cont; cont= gdk_wi;

    gtk_widget_queue_draw(GTK_WIDGET(notebook));
    //gtk_widget_draw(GTK_WIDGET(notebook),NBCR);
    
    //gtk_widget_draw(GTK_WIDGET(notebook),area);
    cairo_destroy(cr);
    //printf("test: %f",percentComplete);
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
    //int id = 123456;
    char id[20];
    snprintf(id,20,"123456");
    //printf("Wert = %s",test);
    //PrintS(test);
    int ret = CreateNewDB(id, &temp, dateTimeStringBuffer,NULL);
    if(ret==0)
        printf("ret OK\n");
    else if(ret==3)
        printf("ret Fehler 3\n");
    else printf("Anderes");
    //TEMPENTRY *te = GetLastEntry();
    //printf("ID=%s ; Temp=%f ; Time=%s \n", te->SensorID, te->Temperatur,te->DateTime);
    //id=234567;
    //snprintf(id,20,"28-3c01e076e89c");
    //temp+=5;
    //snprintf(dateTimeStringBuffer, 32, "2022-05-24 21:30:00.000");
    //ret = InsertEntry(id, &Temp,dateTimeStringBuffer,NULL);
    //if(ret==0)
    //    printf("ret OK\n");
    //else if(ret==3)
    //    printf("ret Fehler 3\n");

    //te = GetLastEntry();
    //printf("ID=%s ; Temp=%f ; Time=%s \n", te->SensorID, te->Temperatur,te->DateTime);
    return;
}
void WriteToDB()
{
    time_t currentTime;
    time(&currentTime);
    char dateTimeStringBuffer[32];
    strftime(dateTimeStringBuffer, 32, "%Y-%m-%d %H:%M:%S.000",localtime(&currentTime));
    setlocale(LC_NUMERIC,"C");
    int ret = InsertEntry(Name, &Temp,dateTimeStringBuffer,NULL);
    if(ret !=0)
        printf("FEHLER");
    TEMPENTRY *te = GetLastEntry();
    printf("ID=%s ; Temp=%f ; Time=%s \n", te->SensorID, te->Temperatur,te->DateTime);
}
// DrawChart: Daten übernehmen von dbAccess und in chart darstellen
void DrawChart(cairo_t *cr, __attribute__((unused))time_t startTime)
{
    printf("Size w:%i , h:%i \n", surface4W, surface4H); // paßt
    //DrawCoordDynamicly(cr, Size *size);
}

void AddPageToNotebook(__attribute__((unused))GtkComboBox *widget, gpointer data)
{
    GtkWidget *box, *tab_label;

    tab_label = gtk_label_new("Thermometer");

    da1 = gtk_drawing_area_new();
    g_signal_connect(da1,"configure-event",G_CALLBACK(ConfigEventCB1),NULL);
    g_signal_connect(da1,"draw",G_CALLBACK(DrawCB1),NULL);
    box = gtk_box_new(FALSE,0);
    gtk_box_pack_start(GTK_BOX(box),da1,TRUE,TRUE,0);
    gtk_widget_set_name(GTK_WIDGET(tab_label),"thermo tab");
    gtk_notebook_insert_page(GTK_NOTEBOOK(data), GTK_WIDGET(box), GTK_WIDGET(tab_label),-1);

    tab_label = gtk_label_new("Druck");
    da2 = gtk_drawing_area_new();
    g_signal_connect(da2,"configure-event",G_CALLBACK(ConfigEventCB2),NULL);
    g_signal_connect(da2,"draw",G_CALLBACK(DrawCB2),NULL);
    box = gtk_box_new(FALSE,0);
    gtk_box_pack_start(GTK_BOX(box),da2,TRUE,TRUE,0);
    gtk_widget_set_name(GTK_WIDGET(tab_label),"druck tab");
    gtk_notebook_insert_page(GTK_NOTEBOOK(data), GTK_WIDGET(box), GTK_WIDGET(tab_label),-1);

    tab_label = gtk_label_new("Durchfluß");
    da3 = gtk_drawing_area_new();
    g_signal_connect(da3,"configure-event",G_CALLBACK(ConfigEventCB3),NULL);
    g_signal_connect(da3,"draw",G_CALLBACK(DrawCB3),NULL);
    box = gtk_box_new(FALSE,0);
    gtk_box_pack_start(GTK_BOX(box),da3,TRUE,TRUE,0);
    gtk_widget_set_name(GTK_WIDGET(tab_label),"druck tab");
    gtk_notebook_insert_page(GTK_NOTEBOOK(data), GTK_WIDGET(box), GTK_WIDGET(tab_label),-1); 

    tab_label = gtk_label_new("Chart");
    da4 = gtk_drawing_area_new();
    g_signal_connect(da4,"configure-event",G_CALLBACK(ConfigEventCB4),NULL);
    g_signal_connect(da4,"draw",G_CALLBACK(DrawCB4),NULL);
    box = gtk_box_new(FALSE,0);
    gtk_box_pack_start(GTK_BOX(box),da4,TRUE,TRUE,0);
    gtk_widget_set_name(GTK_WIDGET(tab_label),"druck tab");
    gtk_notebook_insert_page(GTK_NOTEBOOK(data), GTK_WIDGET(box), GTK_WIDGET(tab_label),-1);
    //page_label = gtk_label_new("Test");
    //gtk_box_pack_start(GTK_BOX(box),page_label,TRUE,TRUE,0);


}
#pragma endregion