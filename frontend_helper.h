#ifndef _FRONTEND_HELPER_H_
#define _FRONTEND_HELPER_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <glib.h>
#include <dirent.h>
#include "common_helper.h"
#include "backend_interface.h"
#include "frontend_interface.h"

#define DBUS_DIR "/usr/share/print-backends"
#define BACKEND_PREFIX "org.openprinting.Backend"

typedef struct _CurrentValues CurrentValues;
/**********Supportedvalues**************************************/
typedef struct _SupportedValues SupportedValues;
/**********PrinterCapabilities**********************************/
typedef struct _PrinterCapabilities PrinterCapabilities;
/**********PrinterObj definitions*******************************/
// Don't use these functions directly use the Fr
typedef struct _PrinterObj PrinterObj;
PrinterObj *get_new_PrinterObj();
void fill_basic_options(PrinterObj *, GVariant *);
void print_basic_options(PrinterObj *);
void update_basic_options(PrinterObj *);
void get_capabilities(PrinterObj *);
void print_capabilities(PrinterObj *);
void get_option_default(PrinterObj *, gchar *);
void get_supported_values_raw(PrinterObj *, gchar *);
void get_supported_media(PrinterObj *);
void get_supported_color(PrinterObj *);
void get_supported_quality(PrinterObj *);
void get_supported_orientation(PrinterObj *);
void get_state(PrinterObj *);
void is_accepting_jobs(PrinterObj *);
void get_media(PrinterObj *);          //to do
void set_media(PrinterObj *, gchar *); /// research about custom media size // to do
void get_resolution(PrinterObj *);
void set_resolution(PrinterObj *, int, int);
void get_orientation(PrinterObj *);
void set_orientation(PrinterObj *, gchar *); //to do
void get_quality(PrinterObj *);              // to to
void set_quality(PrinterObj *, gchar *);     //todo
void get_color_mode(PrinterObj *);           //todo
void set_color_mode(PrinterObj *, gchar *);
void apply_settings(PrinterObj *);

/**********FrontendObj definitions*******************************/
typedef struct _FrontendObj FrontendObj;

FrontendObj *get_new_FrontendObj();
void activate_backends(FrontendObj *);
PrintBackend *add_backend(FrontendObj *, const char *);
gboolean add_printer(FrontendObj *, PrinterObj *, gchar *, gchar *); ///think about this definition a little more
PrinterObj* update_basic_printer_options(FrontendObj *, gchar *);
void get_printer_capabilities(FrontendObj *, gchar *);
void get_printer_option_default(FrontendObj *, gchar *, gchar *);
void get_printer_supported_values_raw(FrontendObj *, gchar *, gchar *);
void get_printer_supported_media(FrontendObj *, gchar *);
void get_printer_supported_color(FrontendObj *, gchar *);
void get_printer_supported_quality(FrontendObj *, gchar *);
void get_printer_supported_orientation(FrontendObj *, gchar *);
void get_printer_state(FrontendObj *, gchar *);
void printer_is_accepting_jobs(FrontendObj *, gchar *);
void get_printer_resolution(FrontendObj *, gchar *);
void set_printer_resolution(FrontendObj *, gchar *, int, int); //to do
void get_printer_orientation(FrontendObj *, gchar *);
void set_printer_orientation(FrontendObj *, gchar *, gchar *); //to do
void get_printer_quality(FrontendObj *, gchar *);              // to to
void set_printer_quality(FrontendObj *, gchar *, gchar *);     //todo
void get_printer_color_mode(FrontendObj *, gchar *);           //todo
void set_printer_color_mode(FrontendObj *, gchar *, gchar *);  //to do
void apply_printer_settings(FrontendObj *, gchar *);
void print_job(FrontendObj *, gchar *);
/***************************************************************/




/************Struvture definitions********************************/
typedef struct _Resolution
{
    int xres;
    int yres;
} Resolution;

struct _SupportedValues
{
    int num_media;
    char **media;

    int num_color;
    char **color;

    int num_quality;
    char **quality;

    int num_orientation;
    char **orientation;

    int num_res;
    Resolution *res;
};

struct _CurrentValues
{
    char *media;
    char *color;
    char *quaity;
    char *orientation;
    Resolution res;
};

struct _PrinterCapabilities
{
    gboolean copies; // if multiple copies can be set
    gboolean media;  // if the media size option is available
    gboolean number_up;
    gboolean orientation;
    gboolean color_mode;
    gboolean print_quality;
    gboolean sides;      // one sided or both sided
    gboolean resolution; ////////////// to do .. i.e. add this also in getCapabilities
};
struct _PrinterObj
{
    /**The basic options first**/
    PrintBackend *backend_proxy;
    char *name;
    char *uri;
    char *location;
    char *info;
    char *make_and_model;
    char *state; //to do : change the boolean state variables too when you inp
    gboolean is_printing;
    gboolean is_accepting_jobs;
    PrinterCapabilities capabilities;
    SupportedValues supported;
    CurrentValues current;
    //add options here
};
#endif