#ifndef _BACKEND_HELPER_H
#define _BACKEND_HELPER_H

#include "backend_interface.h"
#include "common_helper.h"
#include <glib.h>



typedef struct _PrinterObj
{
    gchar *name;
    cups_dest_t *dest;
    // add rest later
} PrinterObj;

typedef struct _BackendObj
{
    GDBusConnection *dbus_connection;
    PrintBackend *skeleton;
    GHashTable *dialog_printers; /**the hash table to map from dialog to the set of printers **/
    int num_frontends;
} BackendObj;

typedef struct _Mappings
{
    GHashTable *media;
    GHashTable *color;
    GHashTable *print_quality;
    GHashTable *orientation;
} Mappings;

/********Backend related functions*******************/
BackendObj *get_new_BackendObj();
void connect_to_dbus(BackendObj * , char* obj_path);
/**********Mapping related functions*****************/
Mappings *get_new_Mappings();
/*************CUPS RELATED FUNCTIONS******************/
char *cups_printer_state(cups_dest_t *dest);
gboolean cups_is_accepting_jobs(cups_dest_t *dest);
void cups_get_Resolution(cups_dest_t *dest, int *xres, int *yres);

#endif