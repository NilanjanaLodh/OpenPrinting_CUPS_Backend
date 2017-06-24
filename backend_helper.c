#include "backend_helper.h"

BackendObj *get_new_BackendObj()
{
    BackendObj *b = (BackendObj *)(malloc(sizeof(BackendObj)));
    b->dbus_connection = NULL;
    b->dialog_printers = g_hash_table_new(g_str_hash, g_str_equal);
    b->dialog_cancel = g_hash_table_new(g_str_hash, g_str_equal);
    b->num_frontends = 0;
}

void connect_to_dbus(BackendObj *b, char *obj_path)
{
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
}

int *get_dialog_cancel(BackendObj *b, const char *dialog_name)
{
    int *cancel = (int *)(g_hash_table_lookup(b->dialog_cancel, dialog_name));
    return cancel;
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