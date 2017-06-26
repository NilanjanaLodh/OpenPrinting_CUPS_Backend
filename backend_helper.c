#include "backend_helper.h"

BackendObj *get_new_BackendObj()
{
    BackendObj *b = (BackendObj *)(malloc(sizeof(BackendObj)));
    b->dbus_connection = NULL;
    b->dialog_printers = g_hash_table_new(g_str_hash, g_str_equal);
    b->dialog_cancel = g_hash_table_new(g_str_hash, g_str_equal);
    b->num_frontends = 0;
    b->obj_path = NULL;
}

void connect_to_dbus(BackendObj *b, char *obj_path)
{
    b->obj_path = obj_path;
    GError *error = NULL;
    g_dbus_interface_skeleton_export(G_DBUS_INTERFACE_SKELETON(b->skeleton),
                                     b->dbus_connection,
                                     obj_path,
                                     &error);
    g_assert_no_error(error);
}

void add_frontend(BackendObj *b, const char *_dialog_name)
{
    char *dialog_name = get_string_copy(_dialog_name);
    int *cancel = malloc(sizeof(int));
    *cancel = 0;
    g_hash_table_insert(b->dialog_cancel, dialog_name, cancel); //memory issues

    GHashTable *printers = g_hash_table_new(g_str_hash, g_str_equal);
    g_hash_table_insert(b->dialog_printers, dialog_name, printers);
    b->num_frontends ++;
}
void remove_frontend(BackendObj *b, const char *dialog_name)
{
    //mem management
    int *cancel = get_dialog_cancel(b, dialog_name);
    g_hash_table_remove(b->dialog_cancel, dialog_name);
    free(cancel);

    GHashTable *p = (GHashTable *)(g_hash_table_lookup(b->dialog_printers, dialog_name));
    g_hash_table_remove(b->dialog_printers, dialog_name);
    g_hash_table_destroy(p);
    b->num_frontends --;
    g_message("removed Frontend entry for %s", dialog_name);
}
gboolean no_frontends(BackendObj *b)
{
    if ((b->num_frontends) == 0)
        return TRUE;
    return FALSE;
}
int *get_dialog_cancel(BackendObj *b, const char *dialog_name)
{
    int *cancel = (int *)(g_hash_table_lookup(b->dialog_cancel, dialog_name));
    return cancel;
}
void set_dialog_cancel(BackendObj *b, const char *dialog_name)
{
    int *x = get_dialog_cancel(b, dialog_name);
    *x = 1;
}
void reset_dialog_cancel(BackendObj *b, const char *dialog_name)
{
    int *x = get_dialog_cancel(b, dialog_name);
    *x = 0;
}
gboolean dialog_contains_printer(BackendObj *b, const char *dialog_name, const char *printer_name)
{
    GHashTable *printers = g_hash_table_lookup(b->dialog_printers, dialog_name);
    if (printers == NULL)
        return FALSE;
    if (g_hash_table_contains(printers, printer_name))
        return TRUE;
    return FALSE;
}

void add_printer_to_dialog(BackendObj *b, const char *dialog_name, const char *printer_name)
{
    GHashTable *printers = g_hash_table_lookup(b->dialog_printers, dialog_name);
    if (printers == NULL)
    {
        g_message("Invalid dialog name");
        exit(0);
    }

    char *pname = get_string_copy(printer_name); ///mem
    g_hash_table_add(printers, pname);
}
void send_printer_added_signal(BackendObj *b, const char *dialog_name, cups_dest_t *dest)
{
    ///see if this works well for remote printers too
    char *printer_name = get_string_copy(dest->name);
    GVariant *gv = g_variant_new(PRINTER_ADDED_ARGS,
                                 printer_name,
                                 cupsGetOption("printer-info", dest->num_options, dest->options),
                                 cupsGetOption("printer-location", dest->num_options, dest->options),
                                 cupsGetOption("printer-make-and-model", dest->num_options, dest->options),
                                 cups_is_accepting_jobs(dest),
                                 cups_printer_state(dest));

    GError *error = NULL;
    g_dbus_connection_emit_signal(b->dbus_connection,
                                  dialog_name,
                                  b->obj_path,
                                  "org.openprinting.PrintBackend",
                                  PRINTER_ADDED_SIGNAL,
                                  gv,
                                  &error);
    g_assert_no_error(error);
}
/***************************PrinterObj********************************/
PrinterObj *get_new_PrinterObj(cups_dest_t *dest)
{
    PrinterObj *p = (PrinterObj *)(malloc(sizeof(PrinterObj)));
    p->dest = dest;
    p->name = dest->name;
}
/*********Mappings********/
Mappings *get_new_Mappings()
{
    return NULL;
}
char *cups_printer_state(cups_dest_t *dest)
{
    //cups_dest_t *dest = cupsGetNamedDest(CUPS_HTTP_DEFAULT, printer_name, NULL);
    g_assert_nonnull(dest);
    const char *state = cupsGetOption("printer-state", dest->num_options,
                                      dest->options);
    if (state == NULL)
        return "NA";

    switch (state[0])
    {
    case '3':
        return STATE_IDLE;
    case '4':
        return STATE_PRINTING;
    case '5':
        return STATE_STOPPED;

    default:
        return "NA";
    }
}

gboolean cups_is_accepting_jobs(cups_dest_t *dest)
{

    g_assert_nonnull(dest);
    const char *val = cupsGetOption("printer-is-accepting-jobs", dest->num_options,
                                    dest->options);

    return get_boolean(val);
}

void cups_get_Resolution(cups_dest_t *dest, int *xres, int *yres)
{
    http_t *http = cupsConnectDest(dest, CUPS_DEST_FLAGS_NONE, 500, NULL, NULL, 0, NULL, NULL);
    g_assert_nonnull(http);
    cups_dinfo_t *dinfo = cupsCopyDestInfo(http, dest);
    g_assert_nonnull(dinfo);
    ipp_attribute_t *attr = cupsFindDestDefault(http, dest, dinfo, "printer-resolution");
    ipp_res_t *units;
    *xres = ippGetResolution(attr, 0, yres, units);
}