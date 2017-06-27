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
    char *obj_path;
    GHashTable *dialog_printers; /**the hash table to map from dialog name (char*) to the set of printer names(char *) **/
    GHashTable *dialog_cancel;   /**hash table to map from dialog name (char*) to the variable controlling when the enumeration is cancelled**/
    GHashTable *dialog_hide_remote;
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
void remove_frontend(BackendObj *, const char *dialog_name);
gboolean no_frontends(BackendObj *);
int *get_dialog_cancel(BackendObj *, const char *dialog_name);
void set_dialog_cancel(BackendObj *, const char *dialog_name);   //make cancel = 0
void reset_dialog_cancel(BackendObj *, const char *dialog_name); //make cancel = 1
gboolean get_hide_remote(BackendObj *b, char *dialog_name);
void set_hide_remote_printers(BackendObj *, const char *dialog_name);   
void unset_hide_remote_printers(BackendObj *, const char *dialog_name);   
gboolean dialog_contains_printer(BackendObj *, const char *dialog_name, const char *printer_name);
void add_printer_to_dialog(BackendObj *, const char *dialog_name, cups_dest_t *dest);
void remove_printer_from_dialog(BackendObj *, const char *dialog_name, const char *printer_name);
void send_printer_added_signal(BackendObj *b, const char *dialog_name, cups_dest_t *dest);
void send_printer_removed_signal(BackendObj *b, const char *dialog_name, const char *printer_name);
void notify_removed_printers(BackendObj *b, const char *dialog_name , GHashTable *new_table);
void notify_added_printers(BackendObj *b, const char *dialog_name , GHashTable *new_table);
void replace_printers(BackendObj *b, const char *dialog_name , GHashTable *new_table);
void refresh_printer_list(BackendObj *b, char *dialog_name);

/*********Printer related functions******************/
PrinterObj *get_new_PrinterObj(cups_dest_t *dest);

/**********Mapping related functions*****************/
Mappings *get_new_Mappings();
/*************CUPS RELATED FUNCTIONS******************/
char *cups_printer_state(cups_dest_t *dest);
gboolean cups_is_accepting_jobs(cups_dest_t *dest);
void cups_get_Resolution(cups_dest_t *dest, int *xres, int *yres);
GHashTable *cups_get_all_printers();
GHashTable *cups_get_local_printers();
#endif