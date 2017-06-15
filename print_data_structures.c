#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <glib.h>
#include "print_data_structures.h"
#include "backend_interface.h"
#include "frontend_interface.h"

gboolean get_boolean(gchar *g)
{
    if (g_str_equal(g, "true"))
        return TRUE;

    return FALSE;
}
struct _SupportedValues
{
    int num_media;
    char **media;

    int num_color;
    char **color;

    int num_quality;
    char **quality;

    int num_orientation;
    char **orientation;
};

struct _PrinterCapabilities
{
    gboolean copies; // if multiple copies can be set
    gboolean media;  // if the media size option is available
    gboolean number_up;
    gboolean orientation;
    gboolean color_mode;
    gboolean print_quality;
    gboolean sides; // one sided or both sided
};
struct _PrinterObj
{
    /**The basic options first**/
    char *name;
    PrintBackend *backend_proxy;
    char *uri;
    char *location;
    char *info;
    char *make_and_model;
    char *state;
    gboolean is_printing;
    gboolean is_accepting_jobs;

    PrinterCapabilities capabilities;
    SupportedValues supported;
    //add options here
};
PrinterObj *get_new_PrinterObj()
{
    PrinterObj *p = malloc(sizeof(PrinterObj));
}
void fill_basic_options(PrinterObj *p, GVariant *gv)
{
    char *is_accepting_jobs_str;
    g_variant_get(gv, "(ssssss)",
                  &(p->name),
                  &(p->info),
                  &(p->location),
                  &(p->make_and_model),
                  &is_accepting_jobs_str,
                  &(p->state));

    p->is_accepting_jobs = get_boolean(is_accepting_jobs_str);
    if (p->state)
    {
        switch (p->state[0])
        {
        case '3':
            free(p->state);
            p->state = "idle";
            break;
        case '4':
            free(p->state);
            p->state = "printing";
            break;
        case '5':
            free(p->state);
            p->state = "stopped";
            break;
        default:
            free(p->state);
            p->state = "NA";
        }
    }
}
void print_basic_options(PrinterObj *p)
{
    g_message(" Printer %s\n\
                location : %s\n\
                info : %s\n\
                make and model : %s\n\
                accepting_jobs : %d\n\
                state : %s\n",
              p->name,
              p->location, p->info, p->make_and_model, p->is_accepting_jobs, p->state);
}

void update_basic_options(PrinterObj *p)
{
    PrintBackend *proxy = p->backend_proxy;
    print_backend_call_list_basic_options_sync(p->backend_proxy,
                                               p->name,
                                               &p->info,
                                               &p->location,
                                               &p->make_and_model,
                                               &p->state,
                                               NULL, NULL, NULL);
    //actual updation
    print_basic_options(p);
}
void get_capabilities(PrinterObj *p)
{
    GError *error = NULL;
    print_backend_call_get_printer_capabilities_sync(p->backend_proxy, p->name,
                                                     &(p->capabilities.copies),
                                                     &(p->capabilities.media),
                                                     &(p->capabilities.number_up),
                                                     &(p->capabilities.orientation),
                                                     &(p->capabilities.color_mode),
                                                     &(p->capabilities.print_quality),
                                                     &(p->capabilities.sides),
                                                     NULL, &error);

    print_capabilities(p);
    g_assert_no_error(error);
}
void print_capabilities(PrinterObj *p)
{
    g_message("1 means supported, 0 means that option is not supported.");
    printf("copies : %d \nmedia : %d\nnumber_up : %d\norientation : %d\ncolor_mode : %d\n(many other capabilites not printed!)\n",
           p->capabilities.copies,
           p->capabilities.media,
           p->capabilities.number_up,
           p->capabilities.orientation,
           p->capabilities.color_mode);
}
void get_option_default(PrinterObj *p, gchar *option_name)
{
    char *value = NULL;
    GError *error = NULL;
    print_backend_call_get_default_value_sync(p->backend_proxy, p->name,
                                              option_name, &value,
                                              NULL, &error);
    g_assert_no_error(error);
    g_message("%s", value);
    // this only prints the value to screen.. edit it so that it sets the actual value
}
void get_supported_values(PrinterObj *p, gchar *option_name)
{
    GError *error = NULL;
    int num_values;
    GVariant *values;
    print_backend_call_get_supported_values_sync(p->backend_proxy, p->name,
                                                 option_name, &num_values, &values,
                                                 NULL, &error);
    g_assert_no_error(error);
    g_message("%d supported values", num_values);
    int i;
    gchar *str;
    GVariantIter *iter;

    g_variant_get(values, "a(s)", &iter);
    while (g_variant_iter_loop(iter, "(s)", &str))
        g_print("%s\n", str);

    g_variant_iter_free(iter);

    // this only prints the value to screen.. edit it so that it sets the actual value
}
void get_supported_media(PrinterObj *p)
{
    GError *error = NULL;
    GVariant *values;
    gchar *str;
    GVariantIter *iter;
    print_backend_call_get_supported_media_sync(p->backend_proxy, p->name,
                                                &p->supported.num_media, &values, NULL, &error);
    p->supported.media = (char **)malloc(sizeof(char *) * p->supported.num_media);

    g_variant_get(values, "a(s)", &iter);
    int i = 0;
    while (g_variant_iter_loop(iter, "(s)", &str))
    {
        p->supported.media[i] = malloc(sizeof(char) * (strlen(str) + 1));
        strcpy(p->supported.media[i], str);
        i++;
    }

    for (i = 0; i < p->supported.num_media; i++)
    {
        g_print(" %s\n", p->supported.media[i]);
    }
}
void get_supported_color(PrinterObj *p)
{
    GError *error = NULL;
    GVariant *values;
    gchar *str;
    GVariantIter *iter;
    print_backend_call_get_supported_color_sync(p->backend_proxy, p->name,
                                                &p->supported.num_color, &values, NULL, &error);
    p->supported.color = (char **)malloc(sizeof(char *) * p->supported.num_color);

    g_variant_get(values, "a(s)", &iter);
    int i = 0;
    while (g_variant_iter_loop(iter, "(s)", &str))
    {
        p->supported.color[i] = malloc(sizeof(char) * (strlen(str) + 1));
        strcpy(p->supported.color[i], str);
        i++;
    }

    for (i = 0; i < p->supported.num_color; i++)
    {
        g_print(" %s\n", p->supported.color[i]);
    }
}
void get_supported_quality(PrinterObj *p)
{
    GError *error = NULL;
    GVariant *values;
    gchar *str;
    GVariantIter *iter;
    print_backend_call_get_supported_quality_sync(p->backend_proxy, p->name,
                                                  &p->supported.num_quality, &values, NULL, &error);
    p->supported.quality = (char **)malloc(sizeof(char *) * p->supported.num_quality);

    g_variant_get(values, "a(s)", &iter);
    int i = 0;
    while (g_variant_iter_loop(iter, "(s)", &str))
    {
        p->supported.quality[i] = malloc(sizeof(char) * (strlen(str) + 1));
        strcpy(p->supported.quality[i], str);
        i++;
    }

    for (i = 0; i < p->supported.num_quality; i++)
    {
        g_print(" %s\n", p->supported.quality[i]);
    }
}
void get_supported_orientation(PrinterObj *p)
{
    GError *error = NULL;
    GVariant *values;
    gchar *str;
    GVariantIter *iter;
    print_backend_call_get_supported_orientation_sync(p->backend_proxy, p->name,
                                                      &p->supported.num_orientation, &values, NULL, &error);
    p->supported.orientation = (char **)malloc(sizeof(char *) * p->supported.num_orientation);

    g_variant_get(values, "a(s)", &iter);
    int i = 0;
    while (g_variant_iter_loop(iter, "(s)", &str))
    {
        p->supported.orientation[i] = malloc(sizeof(char) * (strlen(str) + 1));
        strcpy(p->supported.orientation[i], str);
        i++;
    }

    for (i = 0; i < p->supported.num_orientation; i++)
    {
        g_print(" %s\n", p->supported.orientation[i]);
    }
}
/************************************************* FrontendObj********************************************/
struct _FrontendObj
{
    int num_backends;
    GHashTable *backend;
    int num_printers;
    GHashTable *printer;
};

FrontendObj *get_new_FrontendObj()
{
    FrontendObj *f = malloc(sizeof(FrontendObj));
    f->num_backends = 0;
    f->backend = g_hash_table_new(g_str_hash, g_str_equal);
    f->num_printers = 0;
    f->printer = g_hash_table_new(g_str_hash, g_str_equal);
    return f;
}
gboolean add_printer(FrontendObj *f, PrinterObj *p, gchar *backend_name, gchar *obj_path)
{
    //g_message("backend name and object path : %s %s", backend_name, obj_path);
    PrintBackend *proxy;
    if (g_hash_table_contains(f->backend, backend_name))
    {
        // g_message("Backend already exists");
        proxy = g_hash_table_lookup(f->backend, backend_name);
    }
    else
    {
        proxy = print_backend_proxy_new_for_bus_sync(G_BUS_TYPE_SESSION, 0,
                                                     backend_name, obj_path, NULL, NULL);
        char *t = malloc(strlen(backend_name) + 1); //fix memory leak
        strcpy(t, backend_name);
        g_hash_table_insert(f->backend, t, proxy);
    }
    p->backend_proxy = proxy;
    g_hash_table_insert(f->printer, p->name, p);
}
void update_basic_printer_options(FrontendObj *f, gchar *printer_name)
{
    PrinterObj *p = g_hash_table_lookup(f->printer, printer_name);
    g_assert_nonnull(p);

    update_basic_options(p);
}
void get_printer_capabilities(FrontendObj *f, gchar *printer_name)
{
    PrinterObj *p = g_hash_table_lookup(f->printer, printer_name);
    g_assert_nonnull(p);

    get_capabilities(p);
}
void get_printer_option_default(FrontendObj *f, gchar *printer_name, gchar *option_name)
{
    PrinterObj *p = g_hash_table_lookup(f->printer, printer_name);
    g_assert_nonnull(p);

    get_option_default(p, option_name);
}

void get_printer_supported_values(FrontendObj *f, gchar *printer_name, gchar *option_name)
{
    PrinterObj *p = g_hash_table_lookup(f->printer, printer_name);
    g_assert_nonnull(p);

    get_supported_values(p, option_name);
}
void get_printer_supported_media(FrontendObj *f, gchar *printer_name)
{
    PrinterObj *p = g_hash_table_lookup(f->printer, printer_name);
    g_assert_nonnull(p);

    get_supported_media(p);
}
void get_printer_supported_color(FrontendObj *f, gchar *printer_name)
{
    PrinterObj *p = g_hash_table_lookup(f->printer, printer_name);
    g_assert_nonnull(p);

    get_supported_color(p);
}
void get_printer_supported_quality(FrontendObj *f, gchar *printer_name)
{
    PrinterObj *p = g_hash_table_lookup(f->printer, printer_name);
    g_assert_nonnull(p);

    get_supported_quality(p);
}
void get_printer_supported_orientation(FrontendObj *f, gchar *printer_name)
{
    PrinterObj *p = g_hash_table_lookup(f->printer, printer_name);
    g_assert_nonnull(p);

    get_supported_orientation(p);
}