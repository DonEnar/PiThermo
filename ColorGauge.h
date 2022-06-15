#ifndef _color_gauge_h
#define _color_gauge_h

#include <gtk/gtk.h>
#include <cairo.h>

G_BEGIN_DECLS
#pragma region neue macros
//#define MY_NEW_TYPE_COLORGAUGE(my_new_colorgauge_type())
#pragma endregion

#pragma region GObject macros
#define MY_TYPE_COLORGAUGE (my_colorgauge_get_type())
#define MY_COLORGAUGE(obj) (G_TYPE_CHECK_CLASS_CAST((obj), MY_TYPE_COLORGAUGE, ColorGauge))
#define MY_COLORGAUGE_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass),MY_TYPE_COLORGAUGE, ColorGaugeClass))
#define MY_IS_COLORGAUGE(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj),MY_TYPE_COLORGAUGE))
#define MY_IS_COLORGAUGE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), MY_TYPE_COLORGAUGE))
#define MY_COLORGAUGE_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS((obj), MY_TYPE_COLORGAUGE, ColorGaugeClass))
#pragma endregion

#pragma region Struct
//typedef struct MyBrush Brush;
typedef struct MyColorGauge ColorGauge;
typedef struct MyColorGaugeClass ColorGaugeClass;
typedef struct MyColorGaugePrivate ColorGaugePrivate;
//struct MyBrush
//{
//    double RED;
//    double GREEN;
//    double BLUE;
//};
struct MyColorGauge
{
    GtkWidget parent;
    ColorGaugePrivate *priv;
};
struct MyColorGaugeClass ;
struct MyColorGaugeClass
{
    GtkWidgetClass parent_class;
};
#pragma endregion
#pragma region Public API

GType      my_colorgauge_get_type(void) G_GNUC_CONST;
GtkWidget *my_colorgauge_new(void);
gfloat     my_colorgauge_get_temp(ColorGauge *colorgauge);
void       my_colorgauge_set_temp(ColorGauge *colorgauge, gfloat val);
#pragma endregion
G_END_DECLS
#endif