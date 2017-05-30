#include <stdio.h>
#include <stdlib.h>
#include <glib.h>
#include "mydbus.h"

static void on_name_acquired(GDBusConnection *connection, const gchar *name, gpointer user_data);

static gboolean on_handle_add(MyDBusCalculator *interface,
                          GDBusMethodInvocation *invocation,
                          const gint num1, const gint num2,
                          gpointer user_data);

static gboolean on_handle_sub(MyDBusCalculator *interface,
                          GDBusMethodInvocation *invocation,
                          const gint num1, const gint num2,
                          gpointer user_data);
int main()
{
    GMainLoop *loop;
    loop = g_main_loop_new(NULL, FALSE);

    g_bus_own_name(G_BUS_TYPE_SESSION, "com.Nilanjana", 0, NULL, on_name_acquired, NULL, NULL, NULL);

    g_main_loop_run(loop);
}

static void
on_name_acquired(GDBusConnection *connection,
                 const gchar *name,
                 gpointer user_data)
{
    MyDBusCalculator *interface;
    GError *error;

    interface = my_dbus_calculator_skeleton_new();
    g_signal_connect(interface, "handle-add", G_CALLBACK(on_handle_add), NULL);
    g_signal_connect(interface, "handle-sub", G_CALLBACK(on_handle_sub), NULL);
    error = NULL;
    g_dbus_interface_skeleton_export(G_DBUS_INTERFACE_SKELETON(interface),connection,"/com/Nilanjana/Calculator",&error);

}

static gboolean
on_handle_add(MyDBusCalculator *interface,
              GDBusMethodInvocation *invocation,
              const gint num1, const gint num2,
              gpointer user_data)
{
    gint ans = num1 + num2;
    my_dbus_calculator_complete_add(interface,invocation,ans);
    g_print("Added %d and %d.\n", num1 , num2);

    return TRUE;
}

static gboolean
on_handle_sub(MyDBusCalculator *interface,
              GDBusMethodInvocation *invocation,
              const gint num1, const gint num2,
              gpointer user_data)
{
    gint ans = num1 - num2;
    my_dbus_calculator_complete_sub(interface,invocation,ans);
    g_print("Subtracted %d from %d.\n", num2 , num1);

    return TRUE;
}
