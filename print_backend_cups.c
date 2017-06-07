#include <stdio.h>
#include <stdlib.h>
#include <glib.h>
#include <string.h>
#include <cups/cups.h>
#include "backend_interface.h"
#include "print_data_structures.h"

#define _CUPS_NO_DEPRECATED 1
#define BUS_NAME "org.openprinting.Backend.CUPS"
#define OBJECT_PATH "/"

static void on_name_acquired(GDBusConnection *connection, const gchar *name, gpointer user_data);
static void on_activate_backend(GDBusConnection *connection,
                                const gchar *sender_name,
                                const gchar *object_path,
                                const gchar *interface_name,
                                const gchar *signal_name,
                                GVariant *parameters,
                                gpointer user_data);
static void on_stop_backend(GDBusConnection *connection,
                            const gchar *sender_name,
                            const gchar *object_path,
                            const gchar *interface_name,
                            const gchar *signal_name,
                            GVariant *parameters,
                            gpointer user_data);
gpointer list_printers(gpointer sender_name);
int send_printer_added(void *user_data, unsigned flags, cups_dest_t *dest);

GDBusConnection *dbus_connection;
GHashTable *dialog_table;
int main()
{
    dbus_connection = NULL;
    dialog_table = g_hash_table_new(g_str_hash, g_str_equal); /// to do : add destroy functions
    GMainLoop *loop = g_main_loop_new(NULL, FALSE);
    g_bus_own_name(G_BUS_TYPE_SESSION,
                   BUS_NAME,
                   0,                //flags
                   NULL,             //bus_acquired_handler
                   on_name_acquired, //name acquired handler
                   NULL,             //name_lost handler
                   NULL,             //user_data
                   NULL);            //user_data free function
    g_main_loop_run(loop);
}

static void
on_name_acquired(GDBusConnection *connection,
                 const gchar *name,
                 gpointer user_data)
{
    dbus_connection = connection;
    PrintBackend *skeleton;
    GError *error = NULL;

    skeleton = print_backend_skeleton_new();

    /** handle remote method calls **/
    /**
    g_signal_connect(skeleton,        //instance
                     "handle-",       //signal name
                     G_CALLBACK(on_), //callback
                     NULL);           //user_data
    **/

    /**subscribe to signals **/
    g_dbus_connection_signal_subscribe(connection,
                                       NULL,                             //Sender name
                                       "org.openprinting.PrintFrontend", //Sender interface
                                       ACTIVATE_BACKEND_SIGNAL,          //Signal name
                                       NULL,                             /**match on all object paths**/
                                       NULL,                             /**match on all arguments**/
                                       0,                                //Flags
                                       on_activate_backend,              //callback
                                       user_data,                        //user_data
                                       NULL);
    g_dbus_connection_signal_subscribe(connection,
                                       NULL,                             //Sender name
                                       "org.openprinting.PrintFrontend", //Sender interface
                                       STOP_BACKEND_SIGNAL,              //Signal name
                                       NULL,                             /**match on all object paths**/
                                       NULL,                             /**match on all arguments**/
                                       0,                                //Flags
                                       on_stop_backend,                  //callback
                                       user_data,                        //user_data
                                       NULL);

    g_dbus_interface_skeleton_export(G_DBUS_INTERFACE_SKELETON(skeleton), connection, OBJECT_PATH, &error);
    g_assert_no_error(error);
}

static void on_activate_backend(GDBusConnection *connection,
                                const gchar *sender_name,
                                const gchar *object_path,
                                const gchar *interface_name,
                                const gchar *signal_name,
                                GVariant *parameters,
                                gpointer user_data)
{

    g_message("Enumerating printers for %s\n", sender_name);

    char *t = malloc(sizeof(gchar) * (strlen(sender_name) + 1));
    int *cancel = malloc(sizeof(int));
    *cancel = 0;
    strcpy(t, sender_name);
    g_hash_table_insert(dialog_table, t, cancel);

    g_thread_new("list_printers_thread", list_printers, t);
}

gpointer list_printers(gpointer sender_name)
{
    g_message("New thread for dialog at %s\n", (gchar *)sender_name);
    int *cancel = (int *)(g_hash_table_lookup(dialog_table, (gchar *)sender_name));
    cupsEnumDests(CUPS_DEST_FLAGS_NONE,
                  -1, //NO timeout
                  cancel,
                  0, //TYPE
                  0, //MASK
                  send_printer_added,
                  sender_name);
    
    g_hash_table_remove(dialog_table,(gchar *)sender_name);
    g_message("Exiting thread for dialog at %s\n", (gchar *)sender_name);
}

int send_printer_added(void *user_data, unsigned flags, cups_dest_t *dest)
{

    char *sender_name = (char *)user_data;
    //g_message("Dialog  is  %s\n",sender_name);
    GVariant *gv = g_variant_new("(s)", dest->name);
    GError *error = NULL;
    g_dbus_connection_emit_signal(dbus_connection,
                                  sender_name,
                                  OBJECT_PATH,
                                  "org.openprinting.PrintBackend",
                                  PRINTER_ADDED_SIGNAL,
                                  gv,
                                  &error);
    g_assert_no_error(error);
    g_message("     Sent notification for printer %s\n", dest->name);

    return 1; //continue enumeration
}
static void on_stop_backend(GDBusConnection *connection,
                            const gchar *sender_name,
                            const gchar *object_path,
                            const gchar *interface_name,
                            const gchar *signal_name,
                            GVariant *parameters,
                            gpointer user_data)
{
    g_message("Stop backend signal from %s\n", sender_name);
    int *cancel = (int *)(g_hash_table_lookup(dialog_table, sender_name));
    *cancel = 1;

}