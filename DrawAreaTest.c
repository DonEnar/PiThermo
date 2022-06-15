
#include <pthread.h>
#include <unistd.h>
#include <gtk/gtk.h>
#include <stdio.h>
#include <time.h>
#include <signal.h>

GtkWidget *da;
GtkWidget *textView;
static pthread_t drawingThread;
static pthread_mutex_t mutex;
static cairo_surface_t *surface = NULL;
static int surfaceW;
static int surfaceH;

void DeleteWindow(__attribute__((unused)) GtkWidget *widget, __attribute__((unused)) gpointer   data)
{
    g_print("Exit\n");
    gtk_main_quit();
}

//OK :: drawing_area_draw_cb
gboolean DrawCB(__attribute__((unused))GtkWidget *widget, cairo_t *cr, __attribute__((unused))gpointer data)
{
    pthread_mutex_lock(&mutex);
    if(surface!=(cairo_surface_t*)NULL)
    {
        cairo_set_source_surface(cr,surface,0,0);
        cairo_paint(cr);
    }
    pthread_mutex_unlock(&mutex);
    return FALSE;
}

void ClearSurface(void)
{
    cairo_t *cr;
    cr =cairo_create(surface);
    cairo_set_source_rgb(cr,1,1,1);
    cairo_paint(cr);
    cairo_destroy(cr);
}

//OK :: drawing_area_configure_cb
gboolean ConfigEventCB(GtkWidget *widget, GdkEventConfigure *event, __attribute__((unused))gpointer data)
{
    if(event->type == GDK_CONFIGURE)
    {
        pthread_mutex_lock(&mutex);
        if(surface!=(cairo_surface_t*)NULL)
        {
            cairo_surface_destroy(surface);
        }
        GtkAllocation allocation;
        gtk_widget_get_allocation(widget,&allocation);
        surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,allocation.width,allocation.height);
        surfaceW = allocation.width;
        surfaceH = allocation.height;
        pthread_mutex_unlock(&mutex);
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

void* ThreadDraw(__attribute__((unused))void *ptr)
{
    while(1)
    {
        sleep(1);
        if(surface==(cairo_surface_t*)NULL){continue;}
        pthread_mutex_lock(&mutex);
        cairo_t *cr=cairo_create(surface);
        // Hintergrund, nur für den Fall
        cairo_set_source_rgb(cr,0.92,0.92,0.92);
        cairo_rectangle(cr,0,0,surfaceW,surfaceH);
        cairo_fill(cr);

        // Zeichnen der Zeit
        time_t currentTime;
        time(&currentTime);
        char dateTimeStringBuffer[32];
        strftime(dateTimeStringBuffer, 32, "%d.%m.%Y %H:%M:%S",localtime(&currentTime));
        cairo_set_source_rgb(cr,0,0,0);
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
        pango_layout_set_text(layout,dateTimeStringBuffer,-1);
        pango_cairo_update_layout(cr,layout);
        pango_layout_get_size(layout,&width,&height);
        
        cairo_move_to(cr,10,10);
        pango_cairo_show_layout(cr,layout);

        cairo_destroy(cr);
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}

void SizeChanged(__attribute__((unused))GtkWidget *widget,__attribute__((unused))GtkAllocation *allocation, __attribute__((unused)) void *data){}
void on_DrawArea_map_event(__attribute__((unused))GtkWidget *widget, __attribute__((unused))GdkEvent *event, __attribute__((unused))gpointer user_data){}
void on_DrawArea_size_allocate(__attribute__((unused))GtkWidget *widget, __attribute__((unused))GdkRectangle *allocation, __attribute__((unused))gpointer user_data){}

void SearchThermo(__attribute__((unused)) GtkWidget *widget, __attribute__((unused)) gpointer   data)
{
    // Neuer Thread
    pthread_mutex_init(&mutex,NULL);
    pthread_create(&drawingThread,NULL,ThreadDraw,NULL);// thread_draw = ThreadDraw
    // Timer um Fenster neu zu zeichnen, 1Hz
    g_timeout_add(1,InvalidateCB,da);
}

int main(int argc, char **argv)
{
    GtkWidget *window;
    GtkBuilder *builder = NULL;
    //cairo_surface_t *s;

    gtk_init(&argc,&argv);
    builder = gtk_builder_new();
    if(gtk_builder_add_from_file(builder,"GladeThermo1.glade",NULL)==0)
    {
        printf("gtk_builder_add_from_file  FAILED\n");
        return(0);
    }
    window = GTK_WIDGET(gtk_builder_get_object(builder,"window1"));
    textView = GTK_WIDGET(gtk_builder_get_object(builder,"TextView1"));
    da = GTK_WIDGET(gtk_builder_get_object(builder,"DrawArea"));
    // configure-event = ConfigEventCB :: drawing_area_configure_cb
    // draw = drawCB :: drawing_area_draw_cb

    gtk_builder_connect_signals(builder,NULL);
    g_object_unref(builder);
    gtk_widget_show(window);

    

    gtk_main();
    return 0;
}