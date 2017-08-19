#ifndef _BACKEND_HELPER_H
#define _BACKEND_HELPER_H

#include <stdio.h>
#include <stdlib.h>
#include <glib.h>
#include <string.h>
#include <cups/cups.h>
#include <cups/ppd.h>
#include <CPDBackend.h>

#define INFO 3
#define WARN 2
#define ERR 1

#define MSG_LOG_LEVEL INFO

typedef struct _PrinterCUPS
{
    gchar *name;
    cups_dest_t *dest;
    http_t *http;
    cups_dinfo_t *dinfo;
} PrinterCUPS;

typedef struct _Mappings
{
    GHashTable *media;
    GHashTable *color;
    GHashTable *print_quality;
    const char *orientation[10];
    const char *state[6];
} Mappings;

typedef struct _BackendObj
{
    GDBusConnection *dbus_connection;
    PrintBackend *skeleton;
    char *obj_path;

    /**the hash table to map from dialog name (char*) to the set of printers (PrinterObj*) **/
    GHashTable *dialog_printers;

    /**hash table to map from dialog name (char*) to the variable controlling
     *  when the enumeration is cancelled**/
    GHashTable *dialog_cancel;

    GHashTable *dialog_hide_remote;
    GHashTable *dialog_hide_temp;

    int num_frontends;
    char *default_printer;
} BackendObj;

typedef struct _Res
{
    int x, y;
    const char *unit;
    char *string;
} Res;

typedef struct _Option
{
    const char *option_name;
    int num_supported;
    char **supported_values;
    const char *default_value;
} Option;

typedef char *(*extract_func)(ipp_attribute_t *, int index);
/********Backend related functions*******************/

/** Get a new BackendObj **/
BackendObj *get_new_BackendObj();

/** Get the printer-id of the default printer of the CUPS Backend**/
char *get_default_printer(BackendObj *b);

/** Connect the BackendObj to the dbus **/
void connect_to_dbus(BackendObj *, char *obj_path);

/** Add the dialog to the list of dialogs of the particular backend**/
void add_frontend(BackendObj *, const char *dialog_name);

/** Remove the dialog from the list of frontends that this backend is 
 * associated with. 
 */
void remove_frontend(BackendObj *, const char *dialog_name);

/** Checks if the backend isn't associated with any frontend **/
gboolean no_frontends(BackendObj *);

/** Get the variable which controls the cancellation of the enumeration thread for
 * that particular dialog 
 */
int *get_dialog_cancel(BackendObj *, const char *dialog_name);

void set_dialog_cancel(BackendObj *, const char *dialog_name);   //make cancel = 0
void reset_dialog_cancel(BackendObj *, const char *dialog_name); //make cancel = 1

gboolean get_hide_remote(BackendObj *b, char *dialog_name);
void set_hide_remote_printers(BackendObj *, const char *dialog_name);
void unset_hide_remote_printers(BackendObj *, const char *dialog_name);
gboolean get_hide_temp(BackendObj *b, char *dialog_name);
void set_hide_temp_printers(BackendObj *, const char *dialog_name);
void unset_hide_temp_printers(BackendObj *, const char *dialog_name);
gboolean dialog_contains_printer(BackendObj *, const char *dialog_name, const char *printer_name);
PrinterCUPS *add_printer_to_dialog(BackendObj *, const char *dialog_name, const cups_dest_t *dest);
void remove_printer_from_dialog(BackendObj *, const char *dialog_name, const char *printer_name);
void send_printer_added_signal(BackendObj *b, const char *dialog_name, cups_dest_t *dest);
void send_printer_removed_signal(BackendObj *b, const char *dialog_name, const char *printer_name);
void notify_removed_printers(BackendObj *b, const char *dialog_name, GHashTable *new_table);
void notify_added_printers(BackendObj *b, const char *dialog_name, GHashTable *new_table);
void replace_printers(BackendObj *b, const char *dialog_name, GHashTable *new_table);
void refresh_printer_list(BackendObj *b, char *dialog_name);
GHashTable *get_dialog_printers(BackendObj *b, const char *dialog_name);
cups_dest_t *get_dest_by_name(BackendObj *b, const char *dialog_name, const char *printer_name);
PrinterCUPS *get_printer_by_name(BackendObj *b, const char *dialog_name, const char *printer_name);
GVariant *get_all_jobs(BackendObj *b, const char *dialog_name, int *num_jobs, gboolean active_only);

/*********Printer related functions******************/

/** Get a new PrinterCUPS struct associated with the cups destination**/
PrinterCUPS *get_new_PrinterCUPS(cups_dest_t *dest);

/** Ensure that we have a connection the server**/
gboolean ensure_printer_connection(PrinterCUPS *p);

/**
 * Get state of the printer
 * state is one of the following {"idle" , "processing" , "stopped"}
 */
const char *get_printer_state(PrinterCUPS *p);

const char *get_media_default(PrinterCUPS *p);
char *get_orientation_default(PrinterCUPS *p);
const char *get_color_default(PrinterCUPS *p);

const char *get_default(PrinterCUPS *p, char *option_name);
int get_supported(PrinterCUPS *p, char ***supported_values, const char *option_name);

int get_all_options(PrinterCUPS *p, Option **options);

int print_file(PrinterCUPS *p, const char *file_path, int num_settings, GVariant *settings);

int get_active_jobs_count(PrinterCUPS *p);
gboolean cancel_job(PrinterCUPS *p, int jobid);

void tryPPD(PrinterCUPS *p);
/*********Option related functions*****************/
void print_option(const Option *opt);
void unpack_option_array(GVariant *var, int num_options, Option **options);
GVariant *pack_option(const Option *opt);
/**********Mapping related functions*****************/
Mappings *get_new_Mappings();

/*************CUPS/IPP RELATED FUNCTIONS******************/
const char *cups_printer_state(cups_dest_t *dest);
gboolean cups_is_accepting_jobs(cups_dest_t *dest);
void cups_get_Resolution(cups_dest_t *dest, int *xres, int *yres);
GHashTable *cups_get_all_printers();
GHashTable *cups_get_local_printers();
char *cups_retrieve_string(cups_dest_t *dest, const char *option_name);
gboolean cups_is_temporary(cups_dest_t *dest);
GHashTable *cups_get_printers(gboolean notemp, gboolean noremote);
char *extract_ipp_attribute(ipp_attribute_t *, int index, const char *option_name, PrinterCUPS *p);
char *extract_res_from_ipp(ipp_attribute_t *, int index);
char *extract_string_from_ipp(ipp_attribute_t *attr, int index);
char *extract_orientation_from_ipp(ipp_attribute_t *attr, int index);
char *extract_quality_from_ipp(ipp_attribute_t *attr, int index);
char *extract_media_from_ipp(ipp_attribute_t *attr, int index, PrinterCUPS *p);
void print_job(cups_job_t *j);
GVariant *pack_cups_job(cups_job_t job);
char *translate_job_state(ipp_jstate_t);

/***************Misc.** **********************/

/**error logging */
void MSG_LOG(const char *msg, int msg_level);

#endif