#include <stdio.h>
#include <stdlib.h>
#include <glib.h>
#include <string.h>
#include <cups/cups.h>
#include "backend_interface.h"
#include "common_helper.h"
#include "backend_helper.h"

#define _CUPS_NO_DEPRECATED 1
#define BUS_NAME "org.openprinting.Backend.CUPS"
#define OBJECT_PATH "/"

static void acquire_session_bus_name();
static void on_name_acquired(GDBusConnection *connection,
                             const gchar *name,
                             gpointer not_used);
void connect_to_signals();

BackendObj *b;
Mappings *m;
int main()
{
    b = get_new_BackendObj();
    m = get_new_Mappings();
    
    acquire_session_bus_name(BUS_NAME);
    GMainLoop *loop = g_main_loop_new(NULL, FALSE);
    g_main_loop_run(loop);
}

static void acquire_session_bus_name(char *bus_name)
{
    g_bus_own_name(G_BUS_TYPE_SESSION,
                   bus_name,
                   0,                //flags
                   NULL,             //bus_acquired_handler
                   on_name_acquired, //name acquired handler
                   NULL,             //name_lost handler
                   NULL,             //user_data
                   NULL);            //user_data free function
}

static void
on_name_acquired(GDBusConnection *connection,
                 const gchar *name,
                 gpointer not_used)
{
    b->dbus_connection = connection;
    b->skeleton = print_backend_skeleton_new();
    connect_to_signals();
    connect_to_dbus(b,OBJECT_PATH);
    GError *error = NULL;
    g_assert_no_error(error);
}

void connect_to_signals()
{
    PrintBackend *skeleton = b->skeleton;
    // g_signal_connect(skeleton,                                 //instance
    //                  "handle-list-basic-options",              //signal name
    //                  G_CALLBACK(on_handle_list_basic_options), //callback
    //                  NULL);                                    //user_data

    // g_signal_connect(skeleton,                                       //instance
    //                  "handle-get-printer-capabilities",              //signal name
    //                  G_CALLBACK(on_handle_get_printer_capabilities), //callback
    //                  NULL);
    // g_signal_connect(skeleton,                                //instance
    //                  "handle-get-default-value",              //signal name
    //                  G_CALLBACK(on_handle_get_default_value), //callback
    //                  NULL);
    // g_signal_connect(skeleton,                                   //instance
    //                  "handle-get-supported-values-raw-string",   //signal name
    //                  G_CALLBACK(on_handle_get_supported_values), //callback
    //                  NULL);
    // g_signal_connect(skeleton,                                  //instance
    //                  "handle-get-supported-media",              //signal name
    //                  G_CALLBACK(on_handle_get_supported_media), //callback
    //                  NULL);
    // g_signal_connect(skeleton,                                  //instance
    //                  "handle-get-supported-color",              //signal name
    //                  G_CALLBACK(on_handle_get_supported_color), //callback
    //                  NULL);
    // g_signal_connect(skeleton,                                    //instance
    //                  "handle-get-supported-quality",              //signal name
    //                  G_CALLBACK(on_handle_get_supported_quality), //callback
    //                  NULL);
    // g_signal_connect(skeleton,                                        //instance
    //                  "handle-get-supported-orientation",              //signal name
    //                  G_CALLBACK(on_handle_get_supported_orientation), //callback
    //                  NULL);
    // g_signal_connect(skeleton,                                //instance
    //                  "handle-get-printer-state",              //signal name
    //                  G_CALLBACK(on_handle_get_printer_state), //callback
    //                  NULL);
    // g_signal_connect(skeleton,                                //instance
    //                  "handle-is-accepting-jobs",              //signal name
    //                  G_CALLBACK(on_handle_is_accepting_jobs), //callback
    //                  NULL);
    // g_signal_connect(skeleton,                             //instance
    //                  "handle-get-resolution",              //signal name
    //                  G_CALLBACK(on_handle_get_resolution), //callback
    //                  NULL);
    // /**subscribe to signals **/
    // g_dbus_connection_signal_subscribe(dbus_connection,
    //                                    NULL,                             //Sender name
    //                                    "org.openprinting.PrintFrontend", //Sender interface
    //                                    ACTIVATE_BACKEND_SIGNAL,          //Signal name
    //                                    NULL,                             /**match on all object paths**/
    //                                    NULL,                             /**match on all arguments**/
    //                                    0,                                //Flags
    //                                    on_activate_backend,              //callback
    //                                    NULL,                             //user_data
    //                                    NULL);
    // g_dbus_connection_signal_subscribe(dbus_connection,
    //                                    NULL,                             //Sender name
    //                                    "org.openprinting.PrintFrontend", //Sender interface
    //                                    STOP_BACKEND_SIGNAL,              //Signal name
    //                                    NULL,                             /**match on all object paths**/
    //                                    NULL,                             /**match on all arguments**/
    //                                    0,                                //Flags
    //                                    on_stop_backend,                  //callback
    //                                    NULL,                             //user_data
    //                                    NULL);
    // g_dbus_connection_signal_subscribe(dbus_connection,
    //                                    NULL,                             //Sender name
    //                                    "org.openprinting.PrintFrontend", //Sender interface
    //                                    REFRESH_BACKEND_SIGNAL,           //Signal name
    //                                    NULL,                             /**match on all object paths**/
    //                                    NULL,                             /**match on all arguments**/
    //                                    0,                                //Flags
    //                                    on_refresh_backend,               //callback
    //                                    NULL,                             //user_data
    //                                    NULL);
}