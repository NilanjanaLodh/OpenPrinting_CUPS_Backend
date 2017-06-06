#include <stdio.h>
#include <stdlib.h>
#include <glib.h>
#include "print_data_structures.h"
#include "backend_interface.h"
#include "frontend_interface.h"


struct _PrinterObj
{
    gchar *name;
    PrintBackend *backend_proxy;
    gchar *uri;
    gchar *location;
    gchar *description;
    gboolean is_printing;
    //add options here
};

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