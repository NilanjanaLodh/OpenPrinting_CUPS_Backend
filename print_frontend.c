#include <stdio.h>
#include <stdlib.h>
#include <glib.h>
#include <string.h>
#include "frontend_interface.h"
#include "backend_interface.h"
#include "frontend_helper.h"
#include "common_helper.h"

#define DIALOG_BUS_NAME "org.openprinting.PrintFrontend"
#define DIALOG_OBJ_PATH "/"
static void on_name_acquired(GDBusConnection *connection, const gchar *name, gpointer user_data);
static void on_printer_added(GDBusConnection *connection,
                             const gchar *sender_name,
                             const gchar *object_path,
                             const gchar *interface_name,
                             const gchar *signal_name,
                             GVariant *parameters,
                             gpointer user_data);
static void on_printer_removed(GDBusConnection *connection,
                               const gchar *sender_name,
                               const gchar *object_path,
                               const gchar *interface_name,
                               const gchar *signal_name,
                               GVariant *parameters,
                               gpointer user_data);
void display_help();
gpointer parse_commands(gpointer user_data);
FrontendObj *f;
int main()
{
    // printers = g_hash_table_new(g_str_hash, g_str_equal);
    //backends = g_hash_table_new(g_str_hash, g_str_equal);

    f = get_new_FrontendObj();
    g_bus_own_name(G_BUS_TYPE_SESSION,
                   DIALOG_BUS_NAME,
                   0,                //flags
                   NULL,             //bus_acquired_handler
                   on_name_acquired, //name acquired handler
                   NULL,             //name_lost handler
                   f,                //user_data
                   NULL);            //user_data free function
    GMainLoop *loop = g_main_loop_new(NULL, FALSE);
    g_main_loop_run(loop);
}

static void
on_name_acquired(GDBusConnection *connection,
                 const gchar *name,
                 gpointer user_data)
{
    PrintFrontend *skeleton;
    skeleton = print_frontend_skeleton_new();
    GError *error = NULL;

    g_dbus_connection_signal_subscribe(connection,
                                       NULL,                            //Sender name
                                       "org.openprinting.PrintBackend", //Sender interface
                                       PRINTER_ADDED_SIGNAL,            //Signal name
                                       NULL,                            /**match on all object paths**/
                                       NULL,                            /**match on all arguments**/
                                       0,                               //Flags
                                       on_printer_added,                //callback
                                       user_data,                       //user_data
                                       NULL);

    g_dbus_connection_signal_subscribe(connection,
                                       NULL,                            //Sender name
                                       "org.openprinting.PrintBackend", //Sender interface
                                       PRINTER_REMOVED_SIGNAL,          //Signal name
                                       NULL,                            /**match on all object paths**/
                                       NULL,                            /**match on all arguments**/
                                       0,                               //Flags
                                       on_printer_removed,              //callback
                                       user_data,                       //user_data
                                       NULL);

    g_dbus_interface_skeleton_export(G_DBUS_INTERFACE_SKELETON(skeleton), connection, DIALOG_OBJ_PATH, &error);
    g_assert_no_error(error);

    //print_frontend_emit_get_backend(skeleton);
    activate_backends(f);
    /**
    I have created the following thread just for testing purpose.
    In reality you don't need a separate thread to parse commands because you already have a GUI. 
    **/
    g_thread_new("parse_commands_thread", parse_commands, skeleton);
}
static void on_printer_added(GDBusConnection *connection,
                             const gchar *sender_name,
                             const gchar *object_path,
                             const gchar *interface_name,
                             const gchar *signal_name,
                             GVariant *parameters,
                             gpointer user_data)
{

    PrinterObj *p = get_new_PrinterObj();
    fill_basic_options(p, parameters);
    // g_variant_get(parameters, "(sssss)", &printer_name, &info, &location, &make_and_model, &is_accepting_jobs);
    add_printer(f, p, sender_name, object_path);
    print_basic_options(p);
}
static void on_printer_removed(GDBusConnection *connection,
                               const gchar *sender_name,
                               const gchar *object_path,
                               const gchar *interface_name,
                               const gchar *signal_name,
                               GVariant *parameters,
                               gpointer user_data)
{
    char *printer_name;
    g_variant_get(parameters, "(s)", &printer_name);
    g_message("Removed Printer %s!\n", printer_name);
}

gpointer parse_commands(gpointer user_data)
{
    PrintFrontend *skeleton = (PrintFrontend *)user_data;
    char buf[100];
    while (1)
    {
        printf("> ");
        fflush(stdout);
        scanf("%s", buf);
        if (strcmp(buf, "stop") == 0)
        {
            print_frontend_emit_stop_listing(skeleton);
            g_message("Stopping front end..\n");
            exit(0);
        }
        else if (strcmp(buf, "refresh") == 0)
        {
            print_frontend_emit_refresh_backend(skeleton);
            g_message("Getting changes in printer list..\n");
        }
        else if (strcmp(buf, "hide-remote-cups") == 0)
        {
            print_frontend_emit_hide_remote_printers_cups(skeleton);
            g_message("Hiding remote printers discovered by the cups backend..\n");
        }
        else if (strcmp(buf, "update-basic") == 0)
        {
            char printer_name[100];
            scanf("%s", printer_name);
            g_message("Updating basic options ..\n");
            update_basic_printer_options(f, printer_name);
        }
        else if (strcmp(buf, "get-capabilities") == 0)
        {
            char printer_name[100];
            scanf("%s", printer_name);
            g_message("Getting basic capabilities ..\n");
            get_printer_capabilities(f, printer_name);
        }
        else if (strcmp(buf, "get-option-default") == 0)
        {
            char printer_name[100], option_name[100];
            scanf("%s%s", printer_name, option_name);
            get_printer_option_default(f, printer_name, option_name);
        }
        else if (strcmp(buf, "get-supported-raw") == 0)
        {
            char printer_name[100], option_name[100];
            scanf("%s%s", printer_name, option_name);
            get_printer_supported_values_raw(f, printer_name, option_name);
        }
        else if (strcmp(buf, "get-supported-media") == 0)
        {
            char printer_name[100];
            scanf("%s", printer_name);
            get_printer_supported_media(f, printer_name);
        }
        else if (strcmp(buf, "get-supported-color") == 0)
        {
            char printer_name[100];
            scanf("%s", printer_name);
            get_printer_supported_color(f, printer_name);
        }
        else if (strcmp(buf, "get-supported-quality") == 0)
        {
            char printer_name[100];
            scanf("%s", printer_name);
            get_printer_supported_quality(f, printer_name);
        }
        else if (strcmp(buf, "get-supported-orientation") == 0)
        {
            char printer_name[100];
            scanf("%s", printer_name);
            get_printer_supported_orientation(f, printer_name);
        }
        else if (strcmp(buf, "get-state") == 0)
        {
            char printer_name[100];
            scanf("%s", printer_name);
            get_printer_state(f, printer_name);
        }
        else if (strcmp(buf, "is-accepting-jobs") == 0)
        {
            char printer_name[100];
            scanf("%s", printer_name);
            printer_is_accepting_jobs(f, printer_name);
        }
        else if (strcmp(buf, "get-resolution") == 0)
        {
            char printer_name[100];
            scanf("%s", printer_name);
            get_printer_resolution(f, printer_name);
        }
        else if (strcmp(buf, "help")==0)
        {
            display_help();
        }
    }
}
void display_help()
{
    g_message("Available commands .. ");
    printf("%s\n","stop");
    printf("%s\n","refresh");
    printf("%s\n","hide-remote-cups");    
    // printf("%s\n","update-basic <printer name>");
    // printf("%s\n","get-capabilities <printer name>");
    // printf("%s\n","get-option-default <printer name> <option name>");
    // printf("%s\n","get-supported-raw <printer name> <option name>");
    // printf("%s\n","get-supported-media <printer name>");
    // printf("%s\n","get-supported-color <printer name>");
    // printf("%s\n","get-supported-quality <printer name>");
    // printf("%s\n","get-supported-orientation <printer name>");
    // printf("%s\n","get-state <printer name>");
    // printf("%s\n","get-resolution <printer name>");    
    // printf("%s\n","is-accepting-jobs <printer name>");
 
    
}
