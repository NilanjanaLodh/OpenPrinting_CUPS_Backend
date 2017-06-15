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

GDBusConnection *dbus_connection;
PrintBackend *skeleton;
GHashTable *dialog_printers;
GHashTable *dialog_cancel;

///mapping of the cups values to the values expected by the frontend
GHashTable *media_mappings;
GHashTable *color_mappings;
GHashTable *print_quality_mappings;
GHashTable *orientation_mappings;

// GHashTable *job_priority_mappings;
// GHashTable *sides_mappings;
int num_frontends; // the number of frontends that are currently connected

static void on_name_acquired(GDBusConnection *connection, const gchar *name, gpointer not_used);
static void on_activate_backend(GDBusConnection *connection,
                                const gchar *sender_name,
                                const gchar *object_path,
                                const gchar *interface_name,
                                const gchar *signal_name,
                                GVariant *parameters,
                                gpointer not_used);
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
gpointer list_printers(gpointer _dialog_name);
int send_printer_added(void *_dialog_name, unsigned flags, cups_dest_t *dest);
//gpointer find_removed_printers(gpointer not_used);
int add_printer_to_list(void *user_data, unsigned flags, cups_dest_t *dest);
void set_up_mappings();
void connect_to_signals();

static gboolean on_handle_list_basic_options(PrintBackend *interface,
                                             GDBusMethodInvocation *invocation,
                                             const gchar *printer_name,
                                             gpointer user_data);
static gboolean on_handle_get_printer_capabilities(PrintBackend *interface,
                                                   GDBusMethodInvocation *invocation,
                                                   const gchar *printer_name,
                                                   gpointer user_data);

static gboolean on_handle_get_default_value(PrintBackend *interface,
                                            GDBusMethodInvocation *invocation,
                                            const gchar *printer_name,
                                            const gchar *option_name,
                                            gpointer user_data);
static gboolean on_handle_get_supported_values(PrintBackend *interface,
                                               GDBusMethodInvocation *invocation,
                                               const gchar *printer_name,
                                               const gchar *option_name,
                                               gpointer user_data);
static gboolean on_handle_get_supported_media(PrintBackend *interface,
                                              GDBusMethodInvocation *invocation,
                                              const gchar *printer_name,
                                              gpointer user_data);
static gboolean on_handle_get_supported_color(PrintBackend *interface,
                                              GDBusMethodInvocation *invocation,
                                              const gchar *printer_name,
                                              gpointer user_data);
static gboolean on_handle_get_supported_quality(PrintBackend *interface,
                                                GDBusMethodInvocation *invocation,
                                                const gchar *printer_name,
                                                gpointer user_data);
static gboolean on_handle_get_supported_orientation(PrintBackend *interface,
                                                    GDBusMethodInvocation *invocation,
                                                    const gchar *printer_name,
                                                    gpointer user_data);
static gboolean on_handle_get_printer_state(PrintBackend *interface,
                                            GDBusMethodInvocation *invocation,
                                            const gchar *printer_name,
                                            gpointer user_data);
static gboolean on_handle_is_accepting_jobs(PrintBackend *interface,
                                            GDBusMethodInvocation *invocation,
                                            const gchar *printer_name,
                                            gpointer user_data);
static gboolean on_handle_get_resolution(PrintBackend *interface,
                                         GDBusMethodInvocation *invocation,
                                         const gchar *printer_name,
                                         gpointer user_data);
int main()
{
    set_up_mappings();
    dbus_connection = NULL;
    skeleton = NULL;
    dialog_cancel = g_hash_table_new(g_str_hash, g_str_equal); /// to do : add destroy functions
    dialog_printers = g_hash_table_new(g_str_hash, g_str_equal);
    num_frontends = 0;

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
                 gpointer not_used)
{
    dbus_connection = connection;

    GError *error = NULL;

    skeleton = print_backend_skeleton_new();

    connect_to_signals();
    g_dbus_interface_skeleton_export(G_DBUS_INTERFACE_SKELETON(skeleton), connection, OBJECT_PATH, &error);
    g_assert_no_error(error);
}

static void on_activate_backend(GDBusConnection *connection,
                                const gchar *sender_name,
                                const gchar *object_path,
                                const gchar *interface_name,
                                const gchar *signal_name,
                                GVariant *parameters,
                                gpointer not_used)
{

    //g_message("Enumerating printers for %s\n", sender_name);

    char *t = malloc(sizeof(gchar) * (strlen(sender_name) + 1));
    strcpy(t, sender_name);

    int *cancel = malloc(sizeof(int));
    *cancel = 0;
    g_hash_table_insert(dialog_cancel, t, cancel);

    GHashTable *g = g_hash_table_new(g_str_hash, g_str_equal);
    g_hash_table_insert(dialog_printers, t, g);
    num_frontends++;
    g_thread_new(NULL, list_printers, t);
    // g_message("Leaving this function: the threads will take care.\n");
}

gpointer list_printers(gpointer _dialog_name)
{
    gchar *dialog_name = (gchar *)_dialog_name;
    g_message("New thread for dialog at %s\n", dialog_name);
    int *cancel = (int *)(g_hash_table_lookup(dialog_cancel, dialog_name));
    *cancel = 0;

    cupsEnumDests(CUPS_DEST_FLAGS_NONE,
                  -1, //NO timeout
                  cancel,
                  0, //TYPE
                  0, //MASK
                  send_printer_added,
                  _dialog_name);

    //g_hash_table_remove(dialog_cancel, dialog_name);
    //g_hash_table_destroy(g_hash_table_lookup(dialog_printers, dialog_name));
    //g_hash_table_remove(dialog_printers, dialog_name); //huge memory leak
    g_message("Exiting thread for dialog at %s\n", dialog_name);
}

int send_printer_added(void *_dialog_name, unsigned flags, cups_dest_t *dest)
{

    char *dialog_name = (char *)_dialog_name;

    GHashTable *g = g_hash_table_lookup(dialog_printers, dialog_name);
    g_assert_nonnull(g);

    if (g_hash_table_contains(g, dest->name))
    {
        g_message("%s already sent.\n", dest->name);
        return 1;
    }
    cups_ptype_t pt = *cupsGetOption("printer-type", dest->num_options, dest->options);

    char *t = malloc(sizeof(gchar) * (strlen(dest->name) + 1));
    strcpy(t, dest->name);
    g_hash_table_add(g, t);
    GVariant *gv = g_variant_new("(ssssbs)",
                                 t,
                                 cupsGetOption("printer-info", dest->num_options, dest->options),
                                 cupsGetOption("printer-location", dest->num_options, dest->options),
                                 cupsGetOption("printer-make-and-model", dest->num_options, dest->options),
                                 cups_is_accepting_jobs(dest),
                                 cups_printer_state(dest));

    GError *error = NULL;
    g_dbus_connection_emit_signal(dbus_connection,
                                  dialog_name,
                                  OBJECT_PATH,
                                  "org.openprinting.PrintBackend",
                                  PRINTER_ADDED_SIGNAL,
                                  gv,
                                  &error);
    g_assert_no_error(error);
    g_message("     Sent notification for printer %s\n", dest->name);

    ///fix memory leaks
    return 1; //continue enumeration
}
static void on_refresh_backend(GDBusConnection *connection,
                               const gchar *sender_name,
                               const gchar *object_path,
                               const gchar *interface_name,
                               const gchar *signal_name,
                               GVariant *parameters,
                               gpointer not_used)
{
    g_message("Refresh backend signal from %s\n", sender_name);
    int *cancel = (int *)(g_hash_table_lookup(dialog_cancel, sender_name));
    *cancel = 1;
    //fix memory leak here
    GHashTable *g = g_hash_table_new(g_str_hash, g_str_equal);
    GHashTableIter iter;
    gpointer key;
    GHashTable *prev = g_hash_table_lookup(dialog_printers, sender_name);
    cupsEnumDests(CUPS_DEST_FLAGS_NONE,
                  1000,                //timeout
                  NULL,                //cancel
                  0,                   //TYPE
                  0,                   //MASK
                  add_printer_to_list, //function
                  g);                  //user_data
    g_hash_table_iter_init(&iter, prev);
    while (g_hash_table_iter_next(&iter, &key, NULL))
    {
        //g_message("                                             .. %s ..\n", (gchar *)key);
        if (!g_hash_table_contains(g, (gchar *)key))
        {
            g_message("Printer %s removed\n", (char *)key);
            //print_backend_emit_printer_removed(skeleton, (char *)key);
            g_dbus_connection_emit_signal(dbus_connection,
                                          sender_name,
                                          OBJECT_PATH,
                                          "org.openprinting.PrintBackend",
                                          PRINTER_REMOVED_SIGNAL,
                                          g_variant_new("(s)", (char *)key),
                                          NULL);
        }
    }

    char *t = malloc(sizeof(gchar) * (strlen(sender_name) + 1));
    strcpy(t, sender_name);
    g_hash_table_replace(dialog_printers, t, g);
    g_thread_new(NULL, list_printers, t);
    //g_message("Hi\n");
    ///call cupsEnumDests once .. check with current hash table.
    //send printer added/removed signals accordingly
}
int add_printer_to_list(void *user_data, unsigned flags, cups_dest_t *dest)
{
    GHashTable *h = (GHashTable *)user_data;
    char *printername = strdup(dest->name);
    g_hash_table_add(h, printername);
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
    int *cancel = (int *)(g_hash_table_lookup(dialog_cancel, sender_name));
    *cancel = 1;
    num_frontends--;
}
gboolean on_handle_list_basic_options(PrintBackend *interface,
                                      GDBusMethodInvocation *invocation,
                                      const gchar *printer_name,
                                      gpointer user_data)
{
    g_message("Listing basic options");
    cups_dest_t *dest = cupsGetNamedDest(CUPS_HTTP_DEFAULT, printer_name, NULL);
    g_assert_nonnull(dest);
    print_backend_complete_list_basic_options(interface, invocation,
                                              cupsGetOption("printer-info", dest->num_options, dest->options),
                                              cupsGetOption("printer-location", dest->num_options, dest->options),
                                              cupsGetOption("printer-make-and-model", dest->num_options, dest->options),
                                              cups_is_accepting_jobs(dest),
                                              cups_printer_state(dest));
    return TRUE;
}

static gboolean on_handle_get_printer_capabilities(PrintBackend *interface,
                                                   GDBusMethodInvocation *invocation,
                                                   const gchar *printer_name,
                                                   gpointer user_data)
{
    cups_dest_t *dest = cupsGetNamedDest(CUPS_HTTP_DEFAULT, printer_name, NULL);
    g_assert_nonnull(dest);
    http_t *http = cupsConnectDest(dest, CUPS_DEST_FLAGS_NONE, 500, NULL, NULL, 0, NULL, NULL);
    g_assert_nonnull(http);
    cups_dinfo_t *dinfo = cupsCopyDestInfo(http, dest);
    g_assert_nonnull(dinfo);

    gboolean copies, media, n_up, orientation, color_mode, quality, sides , res;

    copies = media = n_up = orientation = color_mode = quality = sides = res = FALSE;

    /**
    Actually, it should be checked this way according to the cups programming manual: 

    copies = cupsCheckDestSupported(http, dest, dinfo, CUPS_MEDIA, NULL);
    media = cupsCheckDestSupported(http, dest, dinfo, CUPS_MEDIA, NULL);
    n_up = cupsCheckDestSupported(http, dest, dinfo, CUPS_NUMBER_UP, NULL);
    orientation = cupsCheckDestSupported(http, dest, dinfo, CUPS_ORIENTATION, NULL);
    color_mode = cupsCheckDestSupported(http, dest, dinfo, CUPS_PRINT_COLOR_MODE, NULL);
    quality = cupsCheckDestSupported(http, dest, dinfo, CUPS_PRINT_QUALITY, NULL);
    sides = cupsCheckDestSupported(http, dest, dinfo, CUPS_SIDES, NULL);

    However, this doesn't work ! Probably due to some bug in the cups API
    Filed an issue here: 
    https://github.com/apple/cups/issues/5031 

    Till then , I'm using a workaround to get things done.

**/
    ipp_attribute_t *attrs = cupsFindDestSupported(http, dest, dinfo,
                                                   "job-creation-attributes");
    int num_options = ippGetCount(attrs);
    char *str;
    for (int i = 0; i < num_options; i++)
    {
        str = ippGetString(attrs, i, NULL);
        if (strcmp(str, CUPS_COPIES) == 0)
        {
            copies = TRUE;
        }
        else if (strcmp(str, CUPS_MEDIA) == 0)
        {
            media = TRUE;
        }
        else if (strcmp(str, CUPS_NUMBER_UP) == 0)
        {
            n_up = TRUE;
        }
        else if (strcmp(str, CUPS_ORIENTATION) == 0)
        {
            orientation = TRUE;
        }
        else if (strcmp(str, CUPS_PRINT_COLOR_MODE) == 0)
        {
            color_mode = TRUE;
        }
        else if (strcmp(str, CUPS_PRINT_QUALITY) == 0)
        {
            quality = TRUE;
        }
        else if (strcmp(str, CUPS_SIDES) == 0)
        {
            sides = TRUE;
        }
        else if(strcmp(str,"printer-resolution")==0)
        {
            res = TRUE;
        }
    }
    print_backend_complete_get_printer_capabilities(interface, invocation,
                                                    copies,
                                                    media,
                                                    n_up,
                                                    orientation,
                                                    color_mode,
                                                    quality,
                                                    sides,
                                                    res);
    //fix memory leaks
    return TRUE;
}
static gboolean on_handle_get_default_value(PrintBackend *interface,
                                            GDBusMethodInvocation *invocation,
                                            const gchar *printer_name,
                                            const gchar *option_name,
                                            gpointer user_data)
{

    cups_dest_t *dest = cupsGetNamedDest(CUPS_HTTP_DEFAULT, printer_name, NULL);
    g_assert_nonnull(dest);
    http_t *http = cupsConnectDest(dest, CUPS_DEST_FLAGS_NONE, 500, NULL, NULL, 0, NULL, NULL);
    g_assert_nonnull(http);
    cups_dinfo_t *dinfo = cupsCopyDestInfo(http, dest);
    g_assert_nonnull(dinfo);

    g_message("retrieving default %s for printer %s..", option_name, printer_name);
    char *def_value = cupsGetOption(option_name, dest->num_options,
                                    dest->options);

    ipp_attribute_t *def_attr =
        cupsFindDestDefault(http, dest, dinfo,
                            option_name);
    gchar *value = "NA";
    if (def_value != NULL)
    {

        value = def_value;
        printf("%s\n", def_value);
    }
    else
    {

        if (ippGetCount(def_attr))
        {
            if (ippGetValueTag(def_attr) == IPP_TAG_STRING)
                value = ippGetString(def_attr, 0, NULL);

            else if (ippGetValueTag(def_attr) == IPP_TAG_INTEGER)
            {
                value = malloc(sizeof(char) * 10);
                sprintf(value, "%d", ippGetInteger(def_attr, 0));
            }
            else if (ippGetValueTag(def_attr) == IPP_TAG_INTEGER)
            {
                value = malloc(sizeof(char) * 10);
                sprintf(value, "%d", ippGetInteger(def_attr, 0));
            }
            else if (ippGetValueTag(def_attr) == IPP_TAG_BOOLEAN)
            {
                value = malloc(sizeof(char) * 5);
                sprintf(value, "%d", ippGetBoolean(def_attr, 0));
            }
        }

        printf("%s\n", value);
    }
    print_backend_complete_get_default_value(interface, invocation,
                                             value);

    //free memory leaks
    return TRUE;
}

static gboolean on_handle_get_supported_values(PrintBackend *interface,
                                               GDBusMethodInvocation *invocation,
                                               const gchar *printer_name,
                                               const gchar *option_name,
                                               gpointer user_data)
{
    cups_dest_t *dest = cupsGetNamedDest(CUPS_HTTP_DEFAULT, printer_name, NULL);
    g_assert_nonnull(dest);
    http_t *http = cupsConnectDest(dest, CUPS_DEST_FLAGS_NONE, 500, NULL, NULL, 0, NULL, NULL);
    g_assert_nonnull(http);
    cups_dinfo_t *dinfo = cupsCopyDestInfo(http, dest);
    g_assert_nonnull(dinfo);
    ipp_attribute_t *attrs =
        cupsFindDestSupported(http, dest, dinfo,
                              option_name);
    int i, count = ippGetCount(attrs);
    GVariantBuilder *builder;
    GVariant *values;

    builder = g_variant_builder_new(G_VARIANT_TYPE("a(s)"));

    for (i = 0; i < count; i++)
    {
        g_variant_builder_add(builder, "(s)", ippGetString(attrs, i, NULL));
    }

    values = g_variant_new("a(s)", builder);
    //unref this later
    print_backend_complete_get_supported_values_raw_string(interface, invocation,
                                                           count, values);

    return TRUE;
}
//get_capabilities

static gboolean on_handle_get_supported_media(PrintBackend *interface,
                                              GDBusMethodInvocation *invocation,
                                              const gchar *printer_name,
                                              gpointer user_data)
{
    cups_dest_t *dest = cupsGetNamedDest(CUPS_HTTP_DEFAULT, printer_name, NULL);
    g_assert_nonnull(dest);
    http_t *http = cupsConnectDest(dest, CUPS_DEST_FLAGS_NONE, 500, NULL, NULL, 0, NULL, NULL);
    g_assert_nonnull(http);
    cups_dinfo_t *dinfo = cupsCopyDestInfo(http, dest);
    g_assert_nonnull(dinfo);
    ipp_attribute_t *attrs =
        cupsFindDestSupported(http, dest, dinfo, CUPS_MEDIA);
    int i, count = ippGetCount(attrs);
    GVariantBuilder *builder;
    GVariant *values;

    builder = g_variant_builder_new(G_VARIANT_TYPE("a(s)"));

    char *str;
    for (i = 0; i < count; i++)
    {
        char *str = ippGetString(attrs, i, NULL);
        if (g_hash_table_contains(media_mappings, str))
        {
            str = (char *)g_hash_table_lookup(media_mappings, str);
        }
        g_message("%s", str);
        g_variant_builder_add(builder, "(s)", str);
    }

    values = g_variant_new("a(s)", builder);
    //unref this later
    print_backend_complete_get_supported_media(interface, invocation, count, values);
}

///kind of duplicated code.. refactor  this later keeping the API same
static gboolean on_handle_get_supported_color(PrintBackend *interface,
                                              GDBusMethodInvocation *invocation,
                                              const gchar *printer_name,
                                              gpointer user_data)
{
    cups_dest_t *dest = cupsGetNamedDest(CUPS_HTTP_DEFAULT, printer_name, NULL);
    g_assert_nonnull(dest);
    http_t *http = cupsConnectDest(dest, CUPS_DEST_FLAGS_NONE, 500, NULL, NULL, 0, NULL, NULL);
    g_assert_nonnull(http);
    cups_dinfo_t *dinfo = cupsCopyDestInfo(http, dest);
    g_assert_nonnull(dinfo);
    ipp_attribute_t *attrs =
        cupsFindDestSupported(http, dest, dinfo, CUPS_PRINT_COLOR_MODE);
    int i, count = ippGetCount(attrs);
    GVariantBuilder *builder;
    GVariant *values;

    builder = g_variant_builder_new(G_VARIANT_TYPE("a(s)"));

    char *str;
    for (i = 0; i < count; i++)
    {
        char *str = ippGetString(attrs, i, NULL);
        if (g_hash_table_contains(color_mappings, str))
        {
            str = (char *)g_hash_table_lookup(color_mappings, str);
        }
        g_message("%s", str);
        g_variant_builder_add(builder, "(s)", str);
    }

    values = g_variant_new("a(s)", builder);
    //unref this later
    print_backend_complete_get_supported_color(interface, invocation, count, values);
}
static gboolean on_handle_get_supported_quality(PrintBackend *interface,
                                                GDBusMethodInvocation *invocation,
                                                const gchar *printer_name,
                                                gpointer user_data)
{
    cups_dest_t *dest = cupsGetNamedDest(CUPS_HTTP_DEFAULT, printer_name, NULL);
    g_assert_nonnull(dest);
    http_t *http = cupsConnectDest(dest, CUPS_DEST_FLAGS_NONE, 500, NULL, NULL, 0, NULL, NULL);
    g_assert_nonnull(http);
    cups_dinfo_t *dinfo = cupsCopyDestInfo(http, dest);
    g_assert_nonnull(dinfo);
    ipp_attribute_t *attrs =
        cupsFindDestSupported(http, dest, dinfo, CUPS_PRINT_QUALITY);
    int i, count = ippGetCount(attrs);
    GVariantBuilder *builder;
    GVariant *values;

    builder = g_variant_builder_new(G_VARIANT_TYPE("a(s)"));

    int x;
    char *str;
    for (i = 0; i < count; i++)
    {
        x = ippGetInteger(attrs, i);
        if (g_hash_table_contains(print_quality_mappings, GINT_TO_POINTER(x)))
        {
            str = (char *)g_hash_table_lookup(print_quality_mappings, GINT_TO_POINTER(x));

            g_message("%s", str);
            g_variant_builder_add(builder, "(s)", str);
        }
    }

    values = g_variant_new("a(s)", builder);
    //unref this later
    print_backend_complete_get_supported_quality(interface, invocation, count, values);
}
static gboolean on_handle_get_supported_orientation(PrintBackend *interface,
                                                    GDBusMethodInvocation *invocation,
                                                    const gchar *printer_name,
                                                    gpointer user_data)
{
    cups_dest_t *dest = cupsGetNamedDest(CUPS_HTTP_DEFAULT, printer_name, NULL);
    g_assert_nonnull(dest);
    http_t *http = cupsConnectDest(dest, CUPS_DEST_FLAGS_NONE, 500, NULL, NULL, 0, NULL, NULL);
    g_assert_nonnull(http);
    cups_dinfo_t *dinfo = cupsCopyDestInfo(http, dest);
    g_assert_nonnull(dinfo);
    ipp_attribute_t *attrs =
        cupsFindDestSupported(http, dest, dinfo, CUPS_ORIENTATION);
    int i, count = ippGetCount(attrs);
    GVariantBuilder *builder;
    GVariant *values;

    builder = g_variant_builder_new(G_VARIANT_TYPE("a(s)"));

    int x;
    char *str;
    int c = 0;
    for (i = 0; i < count; i++)
    {
        x = ippGetInteger(attrs, i);
        if (g_hash_table_contains(orientation_mappings, GINT_TO_POINTER(x)))
        {
            c++;
            str = (char *)g_hash_table_lookup(orientation_mappings, GINT_TO_POINTER(x));
            g_variant_builder_add(builder, "(s)", str);
        }
    }

    values = g_variant_new("a(s)", builder);
    //unref this later
    print_backend_complete_get_supported_orientation(interface, invocation, c, values);
}
static gboolean on_handle_get_printer_state(PrintBackend *interface,
                                            GDBusMethodInvocation *invocation,
                                            const gchar *printer_name,
                                            gpointer user_data)
{
    cups_dest_t *dest = cupsGetNamedDest(CUPS_HTTP_DEFAULT, printer_name, NULL);
    g_assert_nonnull(dest);

    print_backend_complete_get_printer_state(interface, invocation, cups_printer_state(dest));
    return TRUE;
}
static gboolean on_handle_is_accepting_jobs(PrintBackend *interface,
                                            GDBusMethodInvocation *invocation,
                                            const gchar *printer_name,
                                            gpointer user_data)
{
    cups_dest_t *dest = cupsGetNamedDest(CUPS_HTTP_DEFAULT, printer_name, NULL);
    g_assert_nonnull(dest);

    print_backend_complete_is_accepting_jobs(interface, invocation, cups_is_accepting_jobs(dest));
    return TRUE;
}
static gboolean on_handle_get_resolution(PrintBackend *interface,
                                         GDBusMethodInvocation *invocation,
                                         const gchar *printer_name,
                                         gpointer user_data)
{
    cups_dest_t *dest = cupsGetNamedDest(CUPS_HTTP_DEFAULT, printer_name, NULL);
    g_assert_nonnull(dest);
    int xres, yres;
    getResolution(dest, &xres, &yres);
    print_backend_complete_get_resolution(interface, invocation, xres, yres);
    return TRUE;
}
void set_up_mappings()
{
    media_mappings = g_hash_table_new(g_str_hash, g_str_equal);
    g_hash_table_insert(media_mappings, CUPS_MEDIA_A3, MEDIA_A3);
    g_hash_table_insert(media_mappings, CUPS_MEDIA_A4, MEDIA_A4);
    g_hash_table_insert(media_mappings, CUPS_MEDIA_A5, MEDIA_A5);
    g_hash_table_insert(media_mappings, CUPS_MEDIA_A6, MEDIA_A6);
    g_hash_table_insert(media_mappings, CUPS_MEDIA_LEGAL, MEDIA_LEGAL);
    g_hash_table_insert(media_mappings, CUPS_MEDIA_LETTER, MEDIA_LETTER);
    g_hash_table_insert(media_mappings, CUPS_MEDIA_TABLOID, MEDIA_TABLOID);
    g_hash_table_insert(media_mappings, CUPS_MEDIA_ENV10, MEDIA_ENV);
    g_hash_table_insert(media_mappings, CUPS_MEDIA_PHOTO_L, MEDIA_PHOTO);

    color_mappings = g_hash_table_new(g_str_hash, g_str_equal);
    g_hash_table_insert(color_mappings, CUPS_PRINT_COLOR_MODE_COLOR, COLOR_MODE_COLOR);
    g_hash_table_insert(color_mappings, CUPS_PRINT_COLOR_MODE_MONOCHROME, COLOR_MODE_BW);
    g_hash_table_insert(color_mappings, CUPS_PRINT_COLOR_MODE_AUTO, COLOR_MODE_AUTO);

    print_quality_mappings = g_hash_table_new(g_direct_hash, g_direct_equal);
    g_hash_table_insert(print_quality_mappings, GINT_TO_POINTER(atoi(CUPS_PRINT_QUALITY_DRAFT)), QUALITY_DRAFT);
    g_hash_table_insert(print_quality_mappings, GINT_TO_POINTER(atoi(CUPS_PRINT_QUALITY_HIGH)), QUALITY_HIGH);
    g_hash_table_insert(print_quality_mappings, GINT_TO_POINTER(atoi(CUPS_PRINT_QUALITY_NORMAL)), QUALITY_NORMAL);

    orientation_mappings = g_hash_table_new(g_direct_hash, g_direct_equal);
    g_hash_table_insert(orientation_mappings, GINT_TO_POINTER(atoi(CUPS_ORIENTATION_LANDSCAPE)), ORIENTATION_LANDSCAPE);
    g_hash_table_insert(orientation_mappings, GINT_TO_POINTER(atoi(CUPS_ORIENTATION_PORTRAIT)), ORIENTATION_PORTRAIT);
}

/*****************************************************************/
void connect_to_signals()
{
    g_signal_connect(skeleton,                                 //instance
                     "handle-list-basic-options",              //signal name
                     G_CALLBACK(on_handle_list_basic_options), //callback
                     NULL);                                    //user_data

    g_signal_connect(skeleton,                                       //instance
                     "handle-get-printer-capabilities",              //signal name
                     G_CALLBACK(on_handle_get_printer_capabilities), //callback
                     NULL);
    g_signal_connect(skeleton,                                //instance
                     "handle-get-default-value",              //signal name
                     G_CALLBACK(on_handle_get_default_value), //callback
                     NULL);
    g_signal_connect(skeleton,                                   //instance
                     "handle-get-supported-values-raw-string",   //signal name
                     G_CALLBACK(on_handle_get_supported_values), //callback
                     NULL);
    g_signal_connect(skeleton,                                  //instance
                     "handle-get-supported-media",              //signal name
                     G_CALLBACK(on_handle_get_supported_media), //callback
                     NULL);
    g_signal_connect(skeleton,                                  //instance
                     "handle-get-supported-color",              //signal name
                     G_CALLBACK(on_handle_get_supported_color), //callback
                     NULL);
    g_signal_connect(skeleton,                                    //instance
                     "handle-get-supported-quality",              //signal name
                     G_CALLBACK(on_handle_get_supported_quality), //callback
                     NULL);
    g_signal_connect(skeleton,                                        //instance
                     "handle-get-supported-orientation",              //signal name
                     G_CALLBACK(on_handle_get_supported_orientation), //callback
                     NULL);
    g_signal_connect(skeleton,                                //instance
                     "handle-get-printer-state",              //signal name
                     G_CALLBACK(on_handle_get_printer_state), //callback
                     NULL);
    g_signal_connect(skeleton,                                //instance
                     "handle-is-accepting-jobs",              //signal name
                     G_CALLBACK(on_handle_is_accepting_jobs), //callback
                     NULL);
    g_signal_connect(skeleton,                             //instance
                     "handle-get-resolution",              //signal name
                     G_CALLBACK(on_handle_get_resolution), //callback
                     NULL);
    /**subscribe to signals **/
    g_dbus_connection_signal_subscribe(dbus_connection,
                                       NULL,                             //Sender name
                                       "org.openprinting.PrintFrontend", //Sender interface
                                       ACTIVATE_BACKEND_SIGNAL,          //Signal name
                                       NULL,                             /**match on all object paths**/
                                       NULL,                             /**match on all arguments**/
                                       0,                                //Flags
                                       on_activate_backend,              //callback
                                       NULL,                             //user_data
                                       NULL);
    g_dbus_connection_signal_subscribe(dbus_connection,
                                       NULL,                             //Sender name
                                       "org.openprinting.PrintFrontend", //Sender interface
                                       STOP_BACKEND_SIGNAL,              //Signal name
                                       NULL,                             /**match on all object paths**/
                                       NULL,                             /**match on all arguments**/
                                       0,                                //Flags
                                       on_stop_backend,                  //callback
                                       NULL,                             //user_data
                                       NULL);
    g_dbus_connection_signal_subscribe(dbus_connection,
                                       NULL,                             //Sender name
                                       "org.openprinting.PrintFrontend", //Sender interface
                                       REFRESH_BACKEND_SIGNAL,           //Signal name
                                       NULL,                             /**match on all object paths**/
                                       NULL,                             /**match on all arguments**/
                                       0,                                //Flags
                                       on_refresh_backend,               //callback
                                       NULL,                             //user_data
                                       NULL);
}