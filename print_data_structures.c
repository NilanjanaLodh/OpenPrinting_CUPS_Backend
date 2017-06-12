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
    //add options here
};
PrinterObj *get_new_PrinterObj()
{
    PrinterObj *p = malloc(sizeof(PrinterObj));
}
void fill_basic_options(PrinterObj *p, GVariant *gv)
{
    char *is_accepting_jobs_str;
    g_variant_get(gv, "(sssss)",
                  &(p->name),
                  &(p->info),
                  &(p->location),
                  &(p->make_and_model),
                  &is_accepting_jobs_str);
    p->is_accepting_jobs = get_boolean(is_accepting_jobs_str);
}
void print_basic_options(PrinterObj *p)
{
    g_message(" Printer %s\n\
                location : %s\n\
                info : %s\n\
                make and model : %s\n\
                accepting_jobs : %d\n",
              p->name, p->location, p->info, p->make_and_model, p->is_accepting_jobs);
}

void update_basic_options(PrinterObj *p)
{
    PrintBackend *proxy = p->backend_proxy;
    print_backend_call_list_basic_options_sync(p->backend_proxy,
                                               p->name,
                                               NULL, NULL, NULL, NULL, NULL, NULL);
    print_basic_options(p);
}

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