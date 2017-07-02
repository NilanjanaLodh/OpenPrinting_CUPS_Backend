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
static gboolean on_handle_activate_backend(PrintBackend *interface,
                                           GDBusMethodInvocation *invocation,
                                           gpointer not_used);
gpointer list_printers(gpointer _dialog_name);
int send_printer_added(void *_dialog_name, unsigned flags, cups_dest_t *dest);
static void on_stop_backend(GDBusConnection *connection,
                            const gchar *sender_name,
                            const gchar *object_path,
                            const gchar *interface_name,
                            const gchar *signal_name,
                            GVariant *parameters,
                            gpointer not_used);
static void on_refresh_backend(GDBusConnection *connection,
                               const gchar *sender_name,
                               const gchar *object_path,
                               const gchar *interface_name,
                               const gchar *signal_name,
                               GVariant *parameters,
                               gpointer not_used);
static void on_hide_remote_printers(GDBusConnection *connection,
                                    const gchar *sender_name,
                                    const gchar *object_path,
                                    const gchar *interface_name,
                                    const gchar *signal_name,
                                    GVariant *parameters,
                                    gpointer not_used);
static gboolean on_handle_list_basic_options(PrintBackend *interface,
                                             GDBusMethodInvocation *invocation,
                                             const gchar *printer_name,
                                             gpointer user_data);

void connect_to_signals();

BackendObj *b;

int main()
{
    b = get_new_BackendObj();
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
    connect_to_dbus(b, OBJECT_PATH);
}
static gboolean on_handle_activate_backend(PrintBackend *interface,
                                           GDBusMethodInvocation *invocation,
                                           gpointer not_used)
{
    /**
    This function starts the backend and starts sending the printers
    **/
    const char *dialog_name = g_dbus_method_invocation_get_sender(invocation); /// potential risk
    add_frontend(b, dialog_name);
    g_thread_new(NULL, list_printers, (gpointer)dialog_name);
    return TRUE;
}

gpointer list_printers(gpointer _dialog_name)
{
    char *dialog_name = (char *)_dialog_name;
    g_message("New thread for dialog at %s\n", dialog_name);
    int *cancel = get_dialog_cancel(b, dialog_name);

    cupsEnumDests(CUPS_DEST_FLAGS_NONE,
                  -1, //NO timeout
                  cancel,
                  0, //TYPE
                  0, //MASK
                  send_printer_added,
                  _dialog_name);

    g_message("Exiting thread for dialog at %s\n", dialog_name);
}
int send_printer_added(void *_dialog_name, unsigned flags, cups_dest_t *dest)
{

    char *dialog_name = (char *)_dialog_name;
    char *printer_name = dest->name;

    if (dialog_contains_printer(b, dialog_name, printer_name))
    {
        g_message("%s already sent.\n", printer_name);
        return 1;
    }

    add_printer_to_dialog(b, dialog_name, dest);
    send_printer_added_signal(b, dialog_name, dest);
    g_message("     Sent notification for printer %s\n", printer_name);

    ///fix memory leaks
    return 1; //continue enumeration
}
static void on_stop_backend(GDBusConnection *connection,
                            const gchar *sender_name,
                            const gchar *object_path,
                            const gchar *interface_name,
                            const gchar *signal_name,
                            GVariant *parameters,
                            gpointer not_used)
{
    g_message("Stop backend signal from %s\n", sender_name);
    set_dialog_cancel(b, sender_name);
    remove_frontend(b, sender_name);
    if (no_frontends(b))
    {
        g_message("No frontends connected .. exiting backend.\n");
        exit(EXIT_SUCCESS);
    }
}
static void on_refresh_backend(GDBusConnection *connection,
                               const gchar *sender_name,
                               const gchar *object_path,
                               const gchar *interface_name,
                               const gchar *signal_name,
                               GVariant *parameters,
                               gpointer not_used)
{
    char *dialog_name = get_string_copy(sender_name);
    g_message("Refresh backend signal from %s\n", dialog_name);
    set_dialog_cancel(b, dialog_name); /// this stops the enumeration of printers
    refresh_printer_list(b, dialog_name);
}
static void on_hide_remote_printers(GDBusConnection *connection,
                                    const gchar *sender_name,
                                    const gchar *object_path,
                                    const gchar *interface_name,
                                    const gchar *signal_name,
                                    GVariant *parameters,
                                    gpointer not_used)
{
    char *dialog_name = get_string_copy(sender_name);
    g_message("%s signal from %s\n", HIDE_REMOTE_CUPS_SIGNAL, dialog_name);
    if (!get_hide_remote(b, dialog_name))
    {
        set_dialog_cancel(b, dialog_name);
        set_hide_remote_printers(b, dialog_name);
        refresh_printer_list(b, dialog_name);
    }
}
static void on_unhide_remote_printers(GDBusConnection *connection,
                                      const gchar *sender_name,
                                      const gchar *object_path,
                                      const gchar *interface_name,
                                      const gchar *signal_name,
                                      GVariant *parameters,
                                      gpointer not_used)
{
    char *dialog_name = get_string_copy(sender_name);
    g_message("%s signal from %s\n", UNHIDE_REMOTE_CUPS_SIGNAL, dialog_name);
    if (get_hide_remote(b, dialog_name))
    {
        set_dialog_cancel(b, dialog_name);
        unset_hide_remote_printers(b, dialog_name);
        refresh_printer_list(b, dialog_name);
    }
}
static void on_hide_temp_printers(GDBusConnection *connection,
                                  const gchar *sender_name,
                                  const gchar *object_path,
                                  const gchar *interface_name,
                                  const gchar *signal_name,
                                  GVariant *parameters,
                                  gpointer not_used)
{
    char *dialog_name = get_string_copy(sender_name);
    g_message("%s signal from %s\n", HIDE_TEMP_CUPS_SIGNAL, dialog_name);
    if (!get_hide_temp(b, dialog_name))
    {
        set_dialog_cancel(b, dialog_name);
        set_hide_temp_printers(b, dialog_name);
        refresh_printer_list(b, dialog_name);
    }
}
static void on_unhide_temp_printers(GDBusConnection *connection,
                                    const gchar *sender_name,
                                    const gchar *object_path,
                                    const gchar *interface_name,
                                    const gchar *signal_name,
                                    GVariant *parameters,
                                    gpointer not_used)
{
    char *dialog_name = get_string_copy(sender_name);
    g_message("%s signal from %s\n", UNHIDE_TEMP_CUPS_SIGNAL, dialog_name);
    if (get_hide_temp(b, dialog_name))
    {
        set_dialog_cancel(b, dialog_name);
        unset_hide_temp_printers(b, dialog_name);
        refresh_printer_list(b, dialog_name);
    }
}
gboolean on_handle_list_basic_options(PrintBackend *interface,
                                      GDBusMethodInvocation *invocation,
                                      const gchar *printer_name,
                                      gpointer user_data)
{
    g_message("Listing basic options for %s", printer_name);
    const char *dialog_name = g_dbus_method_invocation_get_sender(invocation); /// potential risk
    PrinterCUPS *p = get_printer_by_name(b, dialog_name, printer_name);
    cups_dest_t *dest = p->dest;
    g_assert_nonnull(dest);
    print_backend_complete_list_basic_options(interface, invocation,
                                              cups_retrieve_string(dest, "printer-info"),
                                              cups_retrieve_string(dest, "printer-location"),
                                              cups_retrieve_string(dest, "printer-make-and-model"),
                                              cups_is_accepting_jobs(dest),
                                              cups_printer_state(dest)); //change
    return TRUE;
}
static gboolean on_handle_get_printer_capabilities(PrintBackend *interface,
                                                   GDBusMethodInvocation *invocation,
                                                   const gchar *printer_name,
                                                   gpointer user_data)
{
    g_message("Listing basic options for %s", printer_name);
    const char *dialog_name = g_dbus_method_invocation_get_sender(invocation); /// potential risk
    PrinterCUPS *p = get_printer_by_name(b, dialog_name, printer_name);
    int capabilities = get_printer_capabilities(p);
    print_backend_complete_get_printer_capabilities(interface, invocation,
                                                    (capabilities & CAPABILITY_COPIES),
                                                    (capabilities & CAPABILITY_MEDIA),
                                                    (capabilities & CAPABILITY_NUMBER_UP),
                                                    (capabilities & CAPABILITY_ORIENTATION),
                                                    (capabilities & CAPABILITY_COLOR_MODE),
                                                    (capabilities & CAPABILITY_QUALITY),
                                                    (capabilities & CAPABILITY_SIDES),
                                                    (capabilities & CAPABILITY_RESOLUTION));
}

static gboolean on_handle_is_accepting_jobs(PrintBackend *interface,
                                            GDBusMethodInvocation *invocation,
                                            const gchar *printer_name,
                                            gpointer user_data)
{
    const char *dialog_name = g_dbus_method_invocation_get_sender(invocation); /// potential risk
    cups_dest_t *dest = get_dest_by_name(b, dialog_name, printer_name);
    g_assert_nonnull(dest);
    print_backend_complete_is_accepting_jobs(interface, invocation, cups_is_accepting_jobs(dest));
    return TRUE;
}

static gboolean on_handle_get_printer_state(PrintBackend *interface,
                                            GDBusMethodInvocation *invocation,
                                            const gchar *printer_name,
                                            gpointer user_data)
{
    const char *dialog_name = g_dbus_method_invocation_get_sender(invocation); /// potential risk
    PrinterCUPS *p = get_printer_by_name(b, dialog_name, printer_name);
    const char *state = get_printer_state(p);
    printf("%s is %s\n", printer_name, state);
    print_backend_complete_get_printer_state(interface, invocation, state);
    return TRUE;
}

static gboolean on_handle_ping(PrintBackend *interface,
                               GDBusMethodInvocation *invocation,
                               const gchar *printer_name,
                               gpointer user_data)
{
    const char *dialog_name = g_dbus_method_invocation_get_sender(invocation); /// potential risk
    PrinterCUPS *p = get_printer_by_name(b, dialog_name, printer_name);
    const char *orientation = get_orientation_default(p);
    printf("The default orientation is %s\n", orientation);
    print_backend_complete_ping(interface, invocation);
    return TRUE;
}
static gboolean on_handle_get_default_media(PrintBackend *interface,
                                            GDBusMethodInvocation *invocation,
                                            const gchar *printer_name,
                                            gpointer user_data)
{
    const char *dialog_name = g_dbus_method_invocation_get_sender(invocation); /// potential risk
    PrinterCUPS *p = get_printer_by_name(b, dialog_name, printer_name);
    const char *media = get_media_default(p);
    g_message("Default media for %s is %s\n", printer_name, media);
    print_backend_complete_get_default_media(interface, invocation, media);
    return TRUE;
}

static gboolean on_handle_get_supported_media(PrintBackend *interface,
                                              GDBusMethodInvocation *invocation,
                                              const gchar *printer_name,
                                              gpointer user_data)
{
    const char *dialog_name = g_dbus_method_invocation_get_sender(invocation); /// potential risk
    PrinterCUPS *p = get_printer_by_name(b, dialog_name, printer_name);
    char **supported_media = NULL;
    int count = get_media_supported(p, &supported_media);
    GVariantBuilder *builder;
    GVariant *values;
    builder = g_variant_builder_new(G_VARIANT_TYPE("a(s)"));
    for (int i = 0; i < count; i++)
    {
        g_message("%s", supported_media[i]);
        g_variant_builder_add(builder, "(s)", supported_media[i]);
    }

    if (count == 0)
        g_variant_builder_add(builder, "(s)", "NA");

    values = g_variant_new("a(s)", builder);
    print_backend_complete_get_supported_media(interface, invocation, count, values);

    return TRUE;
}
static gboolean on_handle_get_default_printer(PrintBackend *interface,
                                              GDBusMethodInvocation *invocation,
                                              gpointer user_data)
{
    char *def = get_default_printer(b);
    printf("%s\n", def);
    print_backend_complete_get_default_printer(interface, invocation, def);
    return TRUE;
}
static gboolean on_handle_get_default_orientation(PrintBackend *interface,
                                                  GDBusMethodInvocation *invocation,
                                                  const gchar *printer_name,
                                                  gpointer user_data)
{
    const char *dialog_name = g_dbus_method_invocation_get_sender(invocation); /// potential risk
    PrinterCUPS *p = get_printer_by_name(b, dialog_name, printer_name);
    const char *orientation = get_orientation_default(p);
    printf("The default orientation is %s\n", orientation);
    print_backend_complete_get_default_orientation(interface, invocation, orientation);
    return TRUE;
}
void connect_to_signals()
{
    PrintBackend *skeleton = b->skeleton;
    g_signal_connect(skeleton,                               //instance
                     "handle-activate-backend",              //signal name
                     G_CALLBACK(on_handle_activate_backend), //callback
                     NULL);

    g_signal_connect(skeleton,                                 //instance
                     "handle-list-basic-options",              //signal name
                     G_CALLBACK(on_handle_list_basic_options), //callback
                     NULL);                                    //user_data

    g_signal_connect(skeleton,                                       //instance
                     "handle-get-printer-capabilities",              //signal name
                     G_CALLBACK(on_handle_get_printer_capabilities), //callback
                     NULL);
    g_signal_connect(skeleton,                   //instance
                     "handle-ping",              //signal name
                     G_CALLBACK(on_handle_ping), //callback
                     NULL);
    g_signal_connect(skeleton,                                  //instance
                     "handle-get-default-printer",              //signal name
                     G_CALLBACK(on_handle_get_default_printer), //callback
                     NULL);
    // g_signal_connect(skeleton,                                //instance
    //                  "handle-get-default-value",              //signal name
    //                  G_CALLBACK(on_handle_get_default_value), //callback
    //                  NULL);
    // g_signal_connect(skeleton,                                   //instance
    //                  "handle-get-supported-values-raw-string",   //signal name
    //                  G_CALLBACK(on_handle_get_supported_values), //callback
    //                  NULL);
    g_signal_connect(skeleton,                                //instance
                     "handle-get-default-media",              //signal name
                     G_CALLBACK(on_handle_get_default_media), //callback
                     NULL);
    g_signal_connect(skeleton,                                  //instance
                     "handle-get-supported-media",              //signal name
                     G_CALLBACK(on_handle_get_supported_media), //callback
                     NULL);
    g_signal_connect(skeleton,                                      //instance
                     "handle-get-default-orientation",              //signal name
                     G_CALLBACK(on_handle_get_default_orientation), //callback
                     NULL);
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
    g_signal_connect(skeleton,                                //instance
                     "handle-get-printer-state",              //signal name
                     G_CALLBACK(on_handle_get_printer_state), //callback
                     NULL);
    g_signal_connect(skeleton,                                //instance
                     "handle-is-accepting-jobs",              //signal name
                     G_CALLBACK(on_handle_is_accepting_jobs), //callback
                     NULL);
    // g_signal_connect(skeleton,                             //instance
    //                  "handle-get-resolution",              //signal name
    //                  G_CALLBACK(on_handle_get_resolution), //callback
    //                  NULL);
    // /**subscribe to signals **/

    g_dbus_connection_signal_subscribe(b->dbus_connection,
                                       NULL,                             //Sender name
                                       "org.openprinting.PrintFrontend", //Sender interface
                                       STOP_BACKEND_SIGNAL,              //Signal name
                                       NULL,                             /**match on all object paths**/
                                       NULL,                             /**match on all arguments**/
                                       0,                                //Flags
                                       on_stop_backend,                  //callback
                                       NULL,                             //user_data
                                       NULL);
    g_dbus_connection_signal_subscribe(b->dbus_connection,
                                       NULL,                             //Sender name
                                       "org.openprinting.PrintFrontend", //Sender interface
                                       REFRESH_BACKEND_SIGNAL,           //Signal name
                                       NULL,                             /**match on all object paths**/
                                       NULL,                             /**match on all arguments**/
                                       0,                                //Flags
                                       on_refresh_backend,               //callback
                                       NULL,                             //user_data
                                       NULL);
    g_dbus_connection_signal_subscribe(b->dbus_connection,
                                       NULL,                             //Sender name
                                       "org.openprinting.PrintFrontend", //Sender interface
                                       HIDE_REMOTE_CUPS_SIGNAL,          //Signal name
                                       NULL,                             /**match on all object paths**/
                                       NULL,                             /**match on all arguments**/
                                       0,                                //Flags
                                       on_hide_remote_printers,          //callback
                                       NULL,                             //user_data
                                       NULL);
    g_dbus_connection_signal_subscribe(b->dbus_connection,
                                       NULL,                             //Sender name
                                       "org.openprinting.PrintFrontend", //Sender interface
                                       UNHIDE_REMOTE_CUPS_SIGNAL,        //Signal name
                                       NULL,                             /**match on all object paths**/
                                       NULL,                             /**match on all arguments**/
                                       0,                                //Flags
                                       on_unhide_remote_printers,        //callback
                                       NULL,                             //user_data
                                       NULL);
    g_dbus_connection_signal_subscribe(b->dbus_connection,
                                       NULL,                             //Sender name
                                       "org.openprinting.PrintFrontend", //Sender interface
                                       HIDE_TEMP_CUPS_SIGNAL,            //Signal name
                                       NULL,                             /**match on all object paths**/
                                       NULL,                             /**match on all arguments**/
                                       0,                                //Flags
                                       on_hide_temp_printers,            //callback
                                       NULL,                             //user_data
                                       NULL);
    g_dbus_connection_signal_subscribe(b->dbus_connection,
                                       NULL,                             //Sender name
                                       "org.openprinting.PrintFrontend", //Sender interface
                                       UNHIDE_TEMP_CUPS_SIGNAL,          //Signal name
                                       NULL,                             /**match on all object paths**/
                                       NULL,                             /**match on all arguments**/
                                       0,                                //Flags
                                       on_unhide_temp_printers,          //callback
                                       NULL,                             //user_data
                                       NULL);
}