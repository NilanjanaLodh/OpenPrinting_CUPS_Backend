#include <stdio.h>
#include <stdlib.h>
#include <glib.h>
#include "backend_interface.h"

static void on_name_acquired(GDBusConnection *connection, const gchar *name, gpointer user_data);

static gboolean on_handle_hello_world(PrintBackend *interface,
                                      GDBusMethodInvocation *invocation,
                                      gpointer user_data);
int main()
{
    GMainLoop *loop;
    loop = g_main_loop_new(NULL, FALSE);

    g_bus_own_name(G_BUS_TYPE_SESSION, "com.DUMMY", 0, NULL, on_name_acquired, NULL, NULL, NULL);

    g_main_loop_run(loop);
}

static void
on_name_acquired(GDBusConnection *connection,
                 const gchar *name,
                 gpointer user_data)
{
    PrintBackend *interface;
    GError *error;

    interface = print_backend_skeleton_new();
    g_signal_connect(interface, "handle-hello-world", G_CALLBACK(on_handle_hello_world), NULL);
    error = NULL;
    g_dbus_interface_skeleton_export(G_DBUS_INTERFACE_SKELETON(interface), connection, "/", &error);
}


static gboolean on_handle_hello_world(PrintBackend *interface,
                                      GDBusMethodInvocation *invocation,
                                      gpointer user_data)
{
    g_print("Hello there, from the DUMMY backend!\n");
    print_backend_complete_hello_world(interface, invocation);

    return TRUE;
}