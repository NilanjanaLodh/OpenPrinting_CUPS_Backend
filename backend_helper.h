#ifndef _BACKEND_HELPER_H
#define _BACKEND_HELPER_H

#include <stdio.h>
#include <stdlib.h>
#include <glib.h>
#include <string.h>
#include <cups/cups.h>
#include "backend_interface.h"
#include "common_helper.h"
#include <glib.h>

typedef struct _PrinterObj
{
    gchar *name;
    cups_dest_t *dest;
    // add rest of the members soon .. like http connection , dinfo , ipp_attributes etc
    // Infact, you can use these members to store the aettings the frontend supplies to you
    // and later use these to send print jobs
} PrinterObj;

typedef struct _BackendObj
{
    GDBusConnection *dbus_connection;
    PrintBackend *skeleton;
    GHashTable *dialog_printers; /**the hash table to map from dialog name (char*) to the set of printers(PrinterObjs) **/
    GHashTable *dialog_cancel;   /**hash table to map from dialog name (char*) to the variable controlling when the enumeration is cancelled**/
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
void connect_to_dbus(BackendObj *, char *obj_path);
void add_frontend(BackendObj *, const char *dialog_name);
int *get_dialog_cancel(BackendObj *, const char *dialog_name);
/**********Mapping related functions*****************/
Mappings *get_new_Mappings();
/*************CUPS RELATED FUNCTIONS******************/
char *cups_printer_state(cups_dest_t *dest);
gboolean cups_is_accepting_jobs(cups_dest_t *dest);
void cups_get_Resolution(cups_dest_t *dest, int *xres, int *yres);

#endif