#include "backend_helper.h"

BackendObj *get_new_BackendObj()
{
    BackendObj *b = (BackendObj *)(malloc(sizeof(BackendObj)));
    b->dbus_connection = NULL;
    b->dialog_printers = g_hash_table_new(g_str_hash, g_str_equal);
    b->dialog_cancel = g_hash_table_new(g_str_hash, g_str_equal);
    b->dialog_hide_remote = g_hash_table_new(g_str_hash, g_str_equal);
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
    char *dialog_name = strdup(_dialog_name);
    int *cancel = malloc(sizeof(int));
    *cancel = 0;
    g_hash_table_insert(b->dialog_cancel, dialog_name, cancel); //memory issues

    int *hide_rem = malloc(sizeof(gboolean));
    *hide_rem = FALSE;
    g_hash_table_insert(b->dialog_hide_remote, dialog_name, hide_rem); //memory issues

    GHashTable *printers = g_hash_table_new(g_str_hash, g_str_equal);
    g_hash_table_insert(b->dialog_printers, dialog_name, printers);
    b->num_frontends++;
}
void remove_frontend(BackendObj *b, const char *dialog_name)
{
    //mem management
    int *cancel = get_dialog_cancel(b, dialog_name);
    g_hash_table_remove(b->dialog_cancel, dialog_name);
    free(cancel);

    gboolean *hide_rem =  (gboolean *)(g_hash_table_lookup(b->dialog_hide_remote, dialog_name));
    g_hash_table_remove(b->dialog_hide_remote, dialog_name);
    free(hide_rem);

    GHashTable *p = (GHashTable *)(g_hash_table_lookup(b->dialog_printers, dialog_name));
    g_hash_table_remove(b->dialog_printers, dialog_name);
    g_hash_table_destroy(p);
    b->num_frontends--;

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
void set_hide_remote_printers(BackendObj *b, const char *dialog_name)
{
    gboolean *hide_rem = (gboolean *)(g_hash_table_lookup(b->dialog_hide_remote, dialog_name));
    *hide_rem = TRUE;
}
void unset_hide_remote_printers(BackendObj *b, const char *dialog_name)
{
    gboolean *hide_rem = (gboolean *)(g_hash_table_lookup(b->dialog_hide_remote, dialog_name));
    *hide_rem = FALSE;
}
gboolean dialog_contains_printer(BackendObj *b, const char *dialog_name, const char *printer_name)
{
    GHashTable *printers = g_hash_table_lookup(b->dialog_printers, dialog_name);

    if (printers == NULL)
    {
        printf("printers is NULL");
        return FALSE;
    }
    if (g_hash_table_contains(printers, printer_name))
        return TRUE;
    return FALSE;
}

void add_printer_to_dialog(BackendObj *b, const char *dialog_name, cups_dest_t *dest)
{
    char *printer_name = strdup(dest->name);
    GHashTable *printers = g_hash_table_lookup(b->dialog_printers, dialog_name);
    if (printers == NULL)
    {
        g_message("Invalid dialog name : add %s to %s\n", printer_name, dialog_name);
        exit(0);
    }
    cups_dest_t *dest_copy = NULL;
    cupsCopyDest(dest, 0, &dest_copy);
    g_hash_table_insert(printers, printer_name, dest_copy); ///mem
}
void remove_printer_from_dialog(BackendObj *b, const char *dialog_name, const char *printer_name)
{
    GHashTable *printers = g_hash_table_lookup(b->dialog_printers, dialog_name);
    if (printers == NULL)
    {
        g_message("Invalid dialog name: remove %s from %s\n", printer_name, dialog_name);
        exit(0);
    }
    g_hash_table_remove(printers, printer_name);
}
void send_printer_added_signal(BackendObj *b, const char *dialog_name, cups_dest_t *dest)
{
    ///see if this works well for remote printers too

    if (dest == NULL)
    {
        printf("dest is NULL, can't send signal\n");
        exit(EXIT_FAILURE);
    }
    char *printer_name = strdup(dest->name);
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

void send_printer_removed_signal(BackendObj *b, const char *dialog_name, const char *printer_name)
{
    GError *error = NULL;
    g_dbus_connection_emit_signal(b->dbus_connection,
                                  dialog_name,
                                  b->obj_path,
                                  "org.openprinting.PrintBackend",
                                  PRINTER_REMOVED_SIGNAL,
                                  g_variant_new("(s)", printer_name),
                                  &error);
    g_assert_no_error(error);
}

void    notify_removed_printers(BackendObj *b, const char *dialog_name, GHashTable *new_table)
{
    GHashTableIter iter;
    GHashTable *prev = g_hash_table_lookup(b->dialog_printers, dialog_name);
    printf("Notifying removed printers.\n");
    gpointer printer_name;
    g_hash_table_iter_init(&iter, prev);
    while (g_hash_table_iter_next(&iter, &printer_name, NULL))
    {
        //g_message("                                             .. %s ..\n", (gchar *)printer_name);
        if (!g_hash_table_contains(new_table, (gchar *)printer_name))
        {
            g_message("Printer %s removed\n", (char *)printer_name);
            send_printer_removed_signal(b, dialog_name, (char *)printer_name);
        }
    }
}

void notify_added_printers(BackendObj *b, const char *dialog_name, GHashTable *new_table)
{
    GHashTableIter iter;
    GHashTable *prev = g_hash_table_lookup(b->dialog_printers, dialog_name);
    printf("Notifying added printers.\n");
    gpointer printer_name;
    gpointer dest_struct;
    g_hash_table_iter_init(&iter, new_table);
    while (g_hash_table_iter_next(&iter, &printer_name, &dest_struct))
    {
        //g_message("                                             .. %s ..\n", (gchar *)printer_name);
        if (!g_hash_table_contains(prev, (gchar *)printer_name))
        {
            g_message("Printer %s added\n", (char *)printer_name);
            send_printer_added_signal(b, dialog_name, (cups_dest_t *)dest_struct);
        }
    }
}

void replace_printers(BackendObj *b, const char *dialog_name, GHashTable *new_table)
{
    g_hash_table_replace(b->dialog_printers, (gpointer)dialog_name, new_table);
}

gboolean get_hide_remote(BackendObj *b, char *dialog_name)
{
    gboolean *hide_rem = (gboolean *)(g_hash_table_lookup(b->dialog_hide_remote, dialog_name));
    return (*hide_rem);
}
void refresh_printer_list(BackendObj *b, char *dialog_name)
{
    GHashTable *new_printers;
    if (get_hide_remote(b, dialog_name))
        new_printers = cups_get_local_printers();
    else
        new_printers = cups_get_all_printers();
    notify_removed_printers(b, dialog_name, new_printers);
    notify_added_printers(b, dialog_name, new_printers);
    replace_printers(b, dialog_name, new_printers);
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

int add_printer_to_ht(void *user_data, unsigned flags, cups_dest_t *dest)
{
    GHashTable *h = (GHashTable *)user_data;
    char *printername = strdup(dest->name);
    cups_dest_t *dest_copy = NULL;
    cupsCopyDest(dest, 0, &dest_copy);
    g_hash_table_insert(h, printername, dest_copy);
}
GHashTable *cups_get_all_printers()
{
    printf("all printers\n");
    GHashTable *printers_ht = g_hash_table_new(g_str_hash, g_str_equal);
    cupsEnumDests(CUPS_DEST_FLAGS_NONE,
                  3000,               //timeout
                  NULL,              //cancel
                  0,                 //TYPE
                  0,                 //MASK
                  add_printer_to_ht, //function
                  printers_ht);      //user_data

    return printers_ht;
}
GHashTable *cups_get_local_printers()
{
    printf("local printers\n");
    GHashTable *printers_ht = g_hash_table_new(g_str_hash, g_str_equal);
    cupsEnumDests(CUPS_DEST_FLAGS_NONE,
                  1200,                 //timeout
                  NULL,                //cancel
                  CUPS_PRINTER_LOCAL,  //TYPE
                  CUPS_PRINTER_REMOTE, //MASK
                  add_printer_to_ht,   //function
                  printers_ht);        //user_data

    return printers_ht;
}