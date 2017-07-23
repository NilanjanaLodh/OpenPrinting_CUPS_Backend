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
#define BACKEND_PREFIX "org.openprinting.Backend."

typedef int (*event_callback)(void *);
typedef struct _FrontendObj FrontendObj;
typedef struct _PrinterObj PrinterObj;
typedef struct _Settings Settings;
typedef struct _Options Options;

/*********************definitions ***************************/

/**
______________________________________ FrontendObj __________________________________________

**/

struct _FrontendObj
{
    PrintFrontend *skeleton;
    char *bus_name;
    event_callback add_cb;
    event_callback rem_cb;

    int num_backends;
    GHashTable *backend; /**[backend name(like "CUPS" or "GCP")] ---> [BackendObj]**/

    int num_printers;
    GHashTable *printer; /**[printer name] --> [PrinterObj] **/
};
FrontendObj *get_new_FrontendObj(char *instance_name, event_callback add_cb, event_callback remove_cb);
void connect_to_dbus(FrontendObj *);
void disconnect_from_dbus(FrontendObj *);
void activate_backends(FrontendObj *);
gboolean add_printer(FrontendObj *f, PrinterObj *p);
gboolean remove_printer(FrontendObj *f, char *printer_name);

PrintBackend *create_backend_from_file(const char *);

/*******************************************************************************************/

/**
______________________________________ PrinterObj __________________________________________

**/
struct _PrinterObj
{
    PrintBackend *backend_proxy; /** The proxy object of the backend the printer is associated with **/
    char *backend_name;          /** Backend name ,("CUPS"/ "GCP") also used as suffix */

    /**The basic options first**/
    char *name;
    char *uri;
    char *location;
    char *info;
    char *make_and_model;
    char *state;
    gboolean is_accepting_jobs;

    /** The more advanced options we get from the backend **/
    Options *options;

    /**The settings the user selects, and which will be used for printing the job**/
    Settings *settings;
};
PrinterObj *get_new_PrinterObj();
void fill_basic_options(PrinterObj *, GVariant *);
void print_basic_options(PrinterObj *);

/************************************************************************************************/
struct _Settings
{
    int count;
    GHashTable *table; /** [name] --> [value] **/
    //planned functions:
    // add
    // clear
    // disable
};

struct _Options
{
    int count;
    GHashTable *table; /**[name] --> Option **/
};

/**
 * ________________________________utility functions__________________________
 */

char *concat(char *, char *);
#endif