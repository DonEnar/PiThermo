#include "ColorGauge.h"
#include <math.h>

#pragma region struct
struct MyColorGaugePrivate
{
    gfloat temp;
    GdkWindow *window;
    gint breite;
    gint hoehe;
};
typedef struct CGBrush
{
    double RED;
    double GREEN;
    double BLUE;
}CGBrush ;
#pragma endregion
#pragma region Gauge-Init-Variablen
enum BarStyle { Flowing, Expanding, Blocking };
float cg_Value=19.5;
float cg_Minimum=0;
float cg_Maximum=100;
static int cg_BorderWidth=2;
enum BarStyle cg_Style = Expanding;
CGBrush *cglstBrushes;
int cgbrushLenght;
int cgsmooth = 6; //smooth zwischen 0 und 7 | 6 ist super
#pragma endregion
#pragma region Properties
enum{CG_TEMP};
#pragma endregion
#pragma region variablen
gint WIDTH = 300;
gint HEIGHT = 300;
#pragma endregion
#pragma region Internal API
static void my_colorgauge_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec);
static void my_colorgauge_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec);
//static void my_colorgauge_size_request(GtkWidget *widget, GtkRequisition *requisition);
static void my_colorgauge_get_preferred_height(GtkWidget *widget, gint *minimal_height, gint *natural_height);
static void my_colorgauge_get_preferred_width(GtkWidget *widget, gint *minimal_width, gint *natural_width);
static void my_colorgauge_size_allocate(GtkWidget *widget, GtkAllocation *allocation);
static void my_colorgauge_realize(GtkWidget *widget);
static void BuildColorList();
static CGBrush InterpolateColors(CGBrush color1, CGBrush color2);
static gboolean InsertColor(CGBrush neu, int position);
static gboolean my_colorgauge_draw_event(GtkWidget *widget, cairo_t *cr);
#pragma endregion
#pragma region Define Type
G_DEFINE_TYPE(ColorGauge, my_colorgauge, GTK_TYPE_WIDGET)//my_cpu
#pragma endregion
#pragma region Init
static void my_colorgauge_class_init(ColorGaugeClass *klass)// expose_event habe ich nicht gefunden!
{
    GObjectClass *g_class;
    GtkWidgetClass *w_class;
    GParamSpec *pspec;
    g_class = G_OBJECT_CLASS(klass);
    w_class = GTK_WIDGET_CLASS(klass);
    g_class->set_property         = my_colorgauge_set_property;
    g_class->get_property         = my_colorgauge_get_property;
    w_class->realize              = my_colorgauge_realize;
    w_class->get_preferred_height = my_colorgauge_get_preferred_height;
    w_class->get_preferred_width  = my_colorgauge_get_preferred_width;
    w_class->size_allocate        = my_colorgauge_size_allocate;
    w_class->draw                 = my_colorgauge_draw_event;
    //w_class->expose_event         = my_colorgauge_expose;

    pspec = g_param_spec_float("grad celsius","Grad Celsius","Was die Gauge anzeigen soll",-55.,125.,19.5,G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
    g_object_class_install_property(g_class, CG_TEMP,pspec);
    //g_type_class_add_private(g_class, sizeof(ColorGaugePrivate));
    //G_ADD_PRIVATE(ColorGaugePrivate) G_IMPLEMENT_INTERFACE(GTK_TYPE_BUILDABLE,gtk_test);
}
static void my_colorgauge_init(ColorGauge *colorgauge)
{
    ColorGaugePrivate *priv;
    priv = my_colorgauge_get_instance_private(colorgauge);
    //priv = G_TYPE_INSTANCE_GET_PRIVATE(colorgauge, MY_TYPE_COLORGAUGE, ColorGaugePrivate);

    gtk_widget_set_has_window(GTK_WIDGET(colorgauge), TRUE);
    priv->temp=19.5;
    colorgauge->priv=priv;
}
#pragma endregion
#pragma region override virtual methods
static void my_colorgauge_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
    ColorGauge *colorgauge = MY_COLORGAUGE(object);
    switch(prop_id)
    {
        case CG_TEMP:
            my_colorgauge_set_temp(colorgauge,g_value_get_float(value));
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}
static void my_colorgauge_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
    ColorGauge *colorgauge = MY_COLORGAUGE(object);
    switch(prop_id)
    {
        case CG_TEMP:
            g_value_set_float(value, colorgauge->priv->temp);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}
static void my_colorgauge_realize(GtkWidget *widget)
{
    ColorGaugePrivate *priv = MY_COLORGAUGE(widget)->priv;
    GtkAllocation alloc;
    GdkWindowAttr attrs;
    guint attrs_mask;
    gtk_widget_set_realized(widget, TRUE);
    gtk_widget_get_allocation(widget, &alloc);
    attrs.x             = alloc.x;
    attrs.y             = alloc.y;
    attrs.width         = alloc.width;
    attrs.height        = alloc.height;
    attrs.window_type   = GDK_WINDOW_CHILD;
    attrs.wclass        = GDK_INPUT_OUTPUT;
    attrs.event_mask    = gtk_widget_get_events(widget) | GDK_EXPOSURE_MASK;

    attrs_mask = GDK_WA_X | GDK_WA_Y;

    priv->window = gdk_window_new(gtk_widget_get_parent_window(widget),&attrs, attrs_mask);
    gdk_window_set_user_data(priv->window, widget);
    gtk_widget_set_window(widget, priv->window);
    //gtk_widget_set_style(widget,gtk_style_attach(gtk_widget_get_style(widget),priv->window));
    gtk_widget_get_style_context(widget);
    //widget->StyleContext = gtk_style_attach(gtk_widget_get_style(widget),priv->window);
}
//static void my_colorgauge_size_request(__attribute__((unused)) GtkWidget *widget, GtkRequisition *requisition)
//{
//    requisition->width = WIDTH;
//    requisition->height = HEIGHT;
//}
static void my_colorgauge_get_preferred_height(__attribute__((unused)) GtkWidget *widget, gint *minimal_height, gint *natural_height)
{
    //GtkRequisition *req;
    ColorGaugePrivate *priv;
    priv = MY_COLORGAUGE(widget)->priv;
    priv->hoehe = *natural_height;
    HEIGHT = *minimal_height;
    //minimal_height = HEIGHT;
    //natural_height = HEIGHT;
}
static void my_colorgauge_get_preferred_width(__attribute__((unused)) GtkWidget *widget, gint *minimal_width, gint *natural_width)
{
    *minimal_width = WIDTH;
    *natural_width = WIDTH;
}
static void my_colorgauge_size_allocate(GtkWidget *widget, GtkAllocation *allocation)
{
    ColorGaugePrivate *priv;
    priv = MY_COLORGAUGE(widget)->priv;
    gtk_widget_set_allocation(widget, allocation);
    if(gtk_widget_get_realized(widget))
    {
        gdk_window_move_resize(priv->window, allocation->x, allocation->y, WIDTH, HEIGHT);
    }
}
static gboolean my_colorgauge_draw_event(GtkWidget* widget, cairo_t *cr)
{
    ColorGaugePrivate *priv = MY_COLORGAUGE(widget)->priv;
    BuildColorList();
    int surfaceH = HEIGHT;
    int surfaceW = WIDTH;
    #pragma region Hintergrund
    cairo_set_source_rgb(cr,0.92,0.92,0.92);
    cairo_rectangle(cr,0,0,surfaceW,surfaceH);
    cairo_fill(cr);
    #pragma endregion
    #pragma region Basiswerte
    float percentComplete = (cg_Value-cg_Minimum)/(cg_Maximum-cg_Minimum);//Standart 0.195 , OK
    if(percentComplete<=0.0f) return FALSE;
    if(percentComplete>1.0f) percentComplete = 1.0f;
    float totalWidth,fullwidth;
    fullwidth = (surfaceW - cg_BorderWidth);
    totalWidth = fullwidth * percentComplete;
    float barwidth=0;
    if(cg_Style==Expanding) barwidth = totalWidth;
    else barwidth = fullwidth;
    barwidth /= cgbrushLenght;
    int idxColor = 0;
    float angle=45;//startwinkel 45° ok
    float endAngle=315; // 45+270= 305° ok
    float step=270./cgbrushLenght; // 270° / Anzahl Farben
    #pragma endregion
    #pragma region Gauge Zeichnen
    do
    {
        angle += step;
        // Farben setzen
        cairo_set_antialias(cr, CAIRO_ANTIALIAS_SUBPIXEL);
        cairo_set_source_rgb(cr,cglstBrushes[idxColor].RED,cglstBrushes[idxColor].GREEN,cglstBrushes[idxColor].BLUE);
        // an ticken weiter gehen
        double tick = 0.75;
        cairo_arc(cr, surfaceW/2, surfaceH/2, surfaceH/4*1.5, ((angle-step+90)*(M_PI/180)), ((angle+90+tick)*(M_PI/180)));//AUSSENRING
        cairo_arc_negative(cr, surfaceW/2, surfaceH/2, surfaceH/4*1.5-25, ((angle+90+tick)*(M_PI/180)), ((angle-step+90)*(M_PI/180)));
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
        float x1,x2,x3,y1,y2,y3;
        char Text[15];
        snprintf(Text,15,"%i°C",(26-(i/10)));

        // Textzentrum finden
        cairo_text_extents(cr,Text,&extents);

        //punkt innen; radius=radgauge+1pixel (versuch)
        float radA = surfaceH/4*1.5 + 2; // Innen
        float radB = surfaceH/4*1.5 + 12; // Außen
        float radC = surfaceH/4*1.5 + 6; // Mitte

        float xa=(radA * sinf(deg*(M_PI/180)));
        x1=xa+surfaceW/2;

        float ya=(radA * cosf(deg*(M_PI/180)));
        y1=ya+surfaceH/2;

        float xb=(radB * sinf(deg*(M_PI/180)));
        x2=xb+surfaceW/2;

        float yb=(radB * cosf(deg*(M_PI/180)));
        y2=yb+surfaceH/2;

        float xc=(radC * sinf(deg*(M_PI/180)));
        x3=xc+surfaceW/2;

        float yc=(radC * cosf(deg*(M_PI/180)));
        y3=yc+surfaceH/2;

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
    snprintf(Text1,15,"%.1lf°C",cg_Value);
    cairo_text_extents(cr,Text1,&extents1);
    cairo_move_to(cr, surfaceW/2 - extents1.width/2, surfaceH/2 - extents1.height);
    cairo_show_text(cr,Text1);
    float Temp = priv->temp;
    snprintf(Text1,15,"%.1lf°C", Temp);
    cairo_text_extents(cr,Text1,&extents1);
    cairo_move_to(cr, surfaceW/2 - extents1.width/2, surfaceH/2 + extents1.height);
    cairo_show_text(cr,Text1);

    cairo_set_font_size(cr,10);
    snprintf(Text1,15,"Soll");
    cairo_text_extents(cr,Text1,&extents1);
    cairo_move_to(cr, surfaceW/2 - extents1.width/2, surfaceH/2 - extents1.height*6);
    cairo_show_text(cr,Text1);
    snprintf(Text1,15,"Ist");
    cairo_text_extents(cr,Text1,&extents1);
    cairo_move_to(cr, surfaceW/2 - extents1.width/2, surfaceH/2);// - extents1.height*6);
    cairo_show_text(cr,Text1);
    #pragma endregion
    #pragma region Zeiger
    // °C in Winkel umrechnen
    // 1.: Temp<16°C -> Temp=16°C ; 2.: Temp>26°C -> Temp=26°C
    if(Temp<16) Temp=16;
    if(Temp>26) Temp=26;
    // 315°(Winkel) - (pro 1°C 27°Winkel)
    float deg = (315-((Temp-16)*27)); // paßt

    float radA = surfaceH/4*1.5 + 1; // Innen
    float radB = surfaceH/4*1.5 - 26; // Außen

    float xa=(radA * sinf(deg*(M_PI/180)));
    float x1=xa+surfaceW/2;

    float ya=(radA * cosf(deg*(M_PI/180)));
    float y1=ya+surfaceH/2;

    float xb=(radB * sinf(deg*(M_PI/180)));
    float x2=xb+surfaceW/2;

    float yb=(radB * cosf(deg*(M_PI/180)));
    float y2=yb+surfaceH/2;
    cairo_set_line_width(cr,5);
    cairo_move_to(cr,x1,y1);
    cairo_line_to(cr,x2,y2);
    cairo_stroke(cr);
    #pragma endregion
    cairo_destroy(cr);
    
    return TRUE;
}
gfloat my_colorgauge_get_temp(ColorGauge *colorgauge)
{
    g_return_val_if_fail(MY_IS_COLORGAUGE(colorgauge),0);
    return(colorgauge->priv->temp);
}
void my_colorgauge_set_temp(ColorGauge *colorgauge, gfloat sel)
{
    g_return_if_fail(MY_IS_COLORGAUGE(colorgauge));
    colorgauge->priv->temp = sel;
    gtk_widget_queue_draw(GTK_WIDGET(colorgauge));
}
GtkWidget *my_colorgauge_new(void)
{
    return(g_object_new(MY_TYPE_COLORGAUGE,NULL));
}
static void BuildColorList()
{
    int idx, cnt,sdc;
    cgbrushLenght = ((int)pow(2,cgsmooth+2)+1);
    cglstBrushes = malloc(sizeof(CGBrush)*cgbrushLenght);
    CGBrush b1;
    b1.RED=(128./255.); b1.GREEN=0; b1.BLUE=0;
    CGBrush b5;
    b5.RED=1; b5.GREEN=(40./255.); b5.BLUE=0;
    CGBrush b2;
    b2.RED=(57./255.); b2.GREEN=255./255.; b2.BLUE=(20./255.);
    CGBrush b3;
    b3.RED=8./255.; b3.GREEN=37./255.; b3.BLUE=103./255.;
    CGBrush b4;
    b4.RED=15./255.; b4.GREEN=82./255.; b4.BLUE=186./255.;
    cglstBrushes[0]=b3; cglstBrushes[1] = b4; cglstBrushes[2] = b2; cglstBrushes[3] = b5; cglstBrushes[4]=b1;
    for(sdc=0;sdc<cgsmooth; sdc++)
    {
        idx =0;
        cnt=(pow(2,sdc+2));
        while(idx<cnt)
        {
            CGBrush b =InterpolateColors((CGBrush)cglstBrushes[idx],(CGBrush)cglstBrushes[idx+1]);
            InsertColor(b,idx+1);
            idx +=2;
            cnt++;
        }
    }
}
static gboolean InsertColor(CGBrush neu, int position)
{
    for(int i=cgbrushLenght-1;i>=position+1;i--)
    {
        cglstBrushes[i]=cglstBrushes[i-1];
    }
    cglstBrushes[position]=neu;
    return TRUE;
}
static CGBrush InterpolateColors(CGBrush color1, CGBrush color2)
{
    CGBrush temp;
    double r = (color1.RED + color2.RED) /2;
    double g = (color1.GREEN + color2.GREEN)/2;
    double b = (color1.BLUE + color2.BLUE)/2;
    temp.RED = r;
    temp.GREEN = g;
    temp.BLUE = b;
    return temp;
}