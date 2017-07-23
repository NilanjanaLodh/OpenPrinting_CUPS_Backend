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

#define DIALOG_BUS_NAME "org.openprinting.PrintFrontend"
#define DIALOG_OBJ_PATH "/"
#define DBUS_DIR "/usr/share/print-backends"
#define BACKEND_PREFIX "org.openprinting.Backend"

typedef struct _Settings Settings;
/**********Supportedvalues**************************************/
typedef struct _SupportedValues SupportedValues;
/**********PrinterCapabilities**********************************/
typedef struct _PrinterCapabilities PrinterCapabilities;
/**********PrinterObj definitions*******************************/
// Don't use these functions directly use the Frontend
typedef struct _PrinterObj PrinterObj;

typedef int (*event_callback)(void *);

PrinterObj *get_new_PrinterObj();
void fill_basic_options(PrinterObj *, GVariant *);
void print_basic_options(PrinterObj *);
void update_basic_options(PrinterObj *);
PrinterCapabilities get_capabilities(PrinterObj *);
void print_capabilities(PrinterObj *);
void get_option_default(PrinterObj *, gchar *);
int get_all_options(PrinterObj *, Option **);
void get_supported_values_raw(PrinterObj *, gchar *);
void get_supported_media(PrinterObj *);
void get_supported_color(PrinterObj *);
void get_supported_quality(PrinterObj *);
void get_supported_orientation(PrinterObj *);
void get_supported_resolution(PrinterObj *);
void get_state(PrinterObj *);
void is_accepting_jobs(PrinterObj *); //
char *get_media(PrinterObj *);

void get_resolution(PrinterObj *);
void get_orientation(PrinterObj *);
void get_quality(PrinterObj *);              // to to
char *get_default_color(PrinterObj *);
gboolean _print_file(PrinterObj *p , char *file_path);
int _get_active_jobs_count(PrinterObj *);//


/**********FrontendObj definitions*******************************/
typedef struct _FrontendObj FrontendObj;

FrontendObj *get_new_FrontendObj(char *instance_name, event_callback add_cb , event_callback remove_cb);
void connect_to_dbus(FrontendObj *);//
void disconnect_from_dbus(FrontendObj *);//
void activate_backends(FrontendObj *);//
gboolean add_printer(FrontendObj *, PrinterObj *, gchar *, gchar *); //
gboolean remove_printer(FrontendObj *, char *);//
PrinterObj *update_basic_printer_options(FrontendObj *, gchar *);//X
void refresh_printer_list(FrontendObj *f);//
void hide_remote_cups_printers(FrontendObj *f);//
void unhide_remote_cups_printers(FrontendObj *f);//
void hide_temporary_cups_printers(FrontendObj *f);//
void unhide_temporary_cups_printers(FrontendObj *f);//
Option* get_all_printer_options(FrontendObj *f, gchar *printer_name , int *); //for now: just print the values
char* get_printer_capabilities(FrontendObj *, gchar *);//X
char* get_printer_option_default(FrontendObj *, gchar *, gchar *);
char* get_printer_supported_values_raw(FrontendObj *, gchar *, gchar *);
char* get_printer_supported_media(FrontendObj *, gchar *);
char* get_printer_supported_color(FrontendObj *, gchar *);
char* get_printer_supported_quality(FrontendObj *, gchar *);
char* get_printer_supported_orientation(FrontendObj *, gchar *);
char* get_printer_supported_resolution(FrontendObj *, gchar *);
char *get_printer_state(FrontendObj *, gchar *);//
gboolean printer_is_accepting_jobs(FrontendObj *, gchar *);
char *get_printer_default_resolution(FrontendObj *, gchar *);
char *get_printer_default_orientation(FrontendObj *, gchar *);
void get_printer_quality(FrontendObj *, gchar *);          // to to
char *get_printer_color_mode(FrontendObj *, gchar *);
void apply_printer_settings(FrontendObj *, gchar *);
gboolean print_file(FrontendObj *, gchar *, gchar *);
void pingtest(FrontendObj *, gchar *);
char* get_printer_default_media(FrontendObj *, gchar *);
char *get_default_printer(FrontendObj *, gchar *);
int get_active_jobs_count(FrontendObj *, gchar *);
int get_all_jobs(FrontendObj *, Job **j, gboolean active_only);
/**************Settings*****************************************/
void add_setting(Settings *, char *name , char *val);
/***************************************************************/
PrintBackend *create_backend_from_file( const char *);
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
    char **res;
};

struct _Settings
{
    int num_settings;
    GHashTable *values;
    //planned functions:
    // add
    // clear
    // disable
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
    //Settings defaults;
    //Settings current;
    //add options here
};
struct _FrontendObj
{
    PrintFrontend *skeleton;
    char *bus_name;
    event_callback add_cb;
    event_callback rem_cb;

    int num_backends;
    GHashTable *backend;
    int num_printers;
    GHashTable *printer;
};

#endif

//todo: clean up, normalize all names and types