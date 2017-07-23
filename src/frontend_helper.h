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

/**
 * Get a new FrontendObj instance
 * 
 * @params
 * 
 * instance name: The suffix to be used for the dbus name for Frontend
 *              supply NULL for no suffix
 * 
 * add_cb : The callback function to call when a new printer is added
 * rem_cb : The callback function to call when a printer is removed
 * 
 */
FrontendObj *get_new_FrontendObj(char *instance_name, event_callback add_cb, event_callback remove_cb);

/**
 * Start the frontend D-Bus Service
 */
void connect_to_dbus(FrontendObj *);

/**
 * Notify Backend services before stopping Frontend
 */
void disconnect_from_dbus(FrontendObj *);

/**
 * Discover the currently installed backends and activate them
 * 
 * 
 * 
 * Reads the DBUS_DIR folder to find the files installed by 
 * the respective backends , 
 * For eg:  org.openprinting.Backend.XYZ
 * 
 * XYZ = Backend suffix, using which it will be identified henceforth 
 */
void activate_backends(FrontendObj *);

/**
 * Add the printer to the FrontendObj instance
 */
gboolean add_printer(FrontendObj *f, PrinterObj *p);

/**
 * Remove the printer from FrontendObj
 */
gboolean remove_printer(FrontendObj *f, char *printer_name);
void refresh_printer_list(FrontendObj *f);

/**
 * Hide the remote printers of the CUPS backend
 */
void hide_remote_cups_printers(FrontendObj *f);
void unhide_remote_cups_printers(FrontendObj *f);

/**
 * Hide those (temporary) printers which have been discovered by the CUPS backend,
 * but haven't been yet set up locally
 */
void hide_temporary_cups_printers(FrontendObj *f);
void unhide_temporary_cups_printers(FrontendObj *f);

/**
 * Read the file installed by the backend and create a proxy object 
 * using the backend service name and object path.
 */
PrintBackend *create_backend_from_file(const char *);

PrinterObj *find_PrinterObj(FrontendObj *, char *printer_name, char *backend_name);

/** 
 * The following functions are just wrappers for the respective functions of PrinterObj
 * 
 * You could use these,
 * or call the PrinterObj functions directly instead.
 * 
 * For instance,
 * 
 * Approach 1
 * gboolean b = printer_is_accepting_jobs(f,"HP-Printer", "CUPS");
 * 
 * Approach 2
 * PrinterObj *p= find_PrinterObj(f,"HP-Printer","CUPS");
 * gboolean b = is_accepting_jobs(p);
 * 
 * Note
 * If you have are going to repeatedly going to call functions for the same printer,
 * it may be more efficient to use approach 2 ,
 * to avoid repeated hash table lookup everytime.
 */

gboolean printer_is_accepting_jobs(FrontendObj *, char *printer_name, char *backend_name);
char *get_printer_state(FrontendObj *, char *printer_name, char *backend_name);
Options *get_all_printer_options(FrontendObj *, char *printer_name, char *backend_name);
int get_active_jobs_count(FrontendObj *, char *printer_name, char *backend_name);

/**
 * Get the list of (all/active) jobs
 * 
 * @param j : pointer to a Job array; the retrieved Job list array is stored at this location
 * @param active_only : when set to true , retrieves only the active jobs; 
 *                      otherwise fetches all(active + completed + stopped) jobs
 *                      Retrieves jobs for all users.
 * 
 * returns : number of jobs(i.e. length of the Job array)
 * 
 */
int get_all_jobs(FrontendObj *, Job **j, gboolean active_only);

/**
 * Wrapper to _print_file(PrinterObj* , char *file_path)
 * Submits a single file for printing,
 * using the settings stored in the corresponding PrinterObj
 * 
 * returns: job id
 */
int print_file(FrontendObj *, char *file_name, char *printer_name , char*backend_name);
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

/**
 * Fill the basic options of PrinterObj from the GVariant returned with the printerAdded signal
 */
void fill_basic_options(PrinterObj *, GVariant *);

/**
 * Print the basic options of PrinterObj
 */
void print_basic_options(PrinterObj *);
gboolean is_accepting_jobs(PrinterObj *);
char *get_state(PrinterObj *);

/**
 * Get all advanced supported options for the printer.
 * This function populates the 'options' variable of the PrinterObj structure, 
 * and returns the same.
 * Each option has 
 *  option name,
 *  default value,
 *  number of supported values, 
 *  array of supported values
 */
Options *get_all_options(PrinterObj *);

/**
 * Get number of active jobs(pending + paused + printing)
 * for the printer
 */
int _get_active_jobs_count(PrinterObj *);

/**
 * Submits a single file for printing, using the settings stored in 
 * p->settings
 */
int _print_file(PrinterObj *p, char* file_path);




/************************************************************************************************/
/**
______________________________________ Settings __________________________________________

**/

/**
 * Takes care of the settings the user sets with the help of the dialog.
 * These settings will be used when sending a print job
 */
struct _Settings
{
    int count;
    GHashTable *table; /** [name] --> [value] **/
    //planned functions:
    // disable
    // serialize settings into a GVariant of type a(ss)
};

/**
 * Get an empty Settings struct with no 'settings' in it
 */
Settings *get_new_Settings();

/**
 * Add the particular 'setting' to the Settings struct
 * If the setting already exists, its value is updated instead.
 * 
 * Eg. add_setting(s,"copies","1") 
 */
void add_setting(Settings *, char* name , char *val);

/**
 * Clear the setting specified by @name
 * 
 * @returns
 * TRUE , if the setting was cleared
 * FALSE , if the setting wasn't there and thus couldn't be cleared
 */
gboolean clear_setting(Settings * , char* name);

/************************************************************************************************/
/**
______________________________________ Options __________________________________________

**/
struct _Options
{
    int count;
    GHashTable *table; /**[name] --> Option struct**/
};

/**
 * Get an empty Options struct with no 'options' in it
 */
Options *get_new_Options();

/**
 * ________________________________utility functions__________________________
 */

char *concat(char *printer_name, char *backend_name);

/**
 * 'Unpack' (Deserialize) the GVariant returned in get_all_options
 * and fill the Options structure approriately
 */
void unpack_options(GVariant *var, int num_options, Options *options);
#endif