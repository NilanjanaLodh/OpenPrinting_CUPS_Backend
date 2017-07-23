#include "frontend_helper.h"

/**
________________________________________________ FrontendObj __________________________________________

**/
/**
 * These static functions are callbacks used by the actual FrontendObj functions.
 */
static void on_printer_added(GDBusConnection *connection,
                             const gchar *sender_name,
                             const gchar *object_path,
                             const gchar *interface_name,
                             const gchar *signal_name,
                             GVariant *parameters,
                             gpointer user_data)
{
    FrontendObj *f = (FrontendObj *)user_data;
    PrinterObj *p = get_new_PrinterObj();
    fill_basic_options(p, parameters);
    add_printer(f, p);
    print_basic_options(p);
    f->add_cb(p);
}
static void on_printer_removed(GDBusConnection *connection,
                               const gchar *sender_name,
                               const gchar *object_path,
                               const gchar *interface_name,
                               const gchar *signal_name,
                               GVariant *parameters,
                               gpointer user_data)
{
    FrontendObj *f = (FrontendObj *)user_data;
    char *printer_name;
    g_variant_get(parameters, "(s)", &printer_name);
    remove_printer(f, printer_name);
    g_message("Removed Printer %s!\n", printer_name);
    f->rem_cb(printer_name);
}
static void
on_name_acquired(GDBusConnection *connection,
                 const gchar *name,
                 gpointer user_data)
{
    printf("on name acquired.\n");
    fflush(stdout);
    FrontendObj *f = (FrontendObj *)user_data;
    GError *error = NULL;

    g_dbus_connection_signal_subscribe(connection,
                                       NULL,                            //Sender name
                                       "org.openprinting.PrintBackend", //Sender interface
                                       PRINTER_ADDED_SIGNAL,            //Signal name
                                       NULL,                            /**match on all object paths**/
                                       NULL,                            /**match on all arguments**/
                                       0,                               //Flags
                                       on_printer_added,                //callback
                                       user_data,                       //user_data
                                       NULL);

    g_dbus_connection_signal_subscribe(connection,
                                       NULL,                            //Sender name
                                       "org.openprinting.PrintBackend", //Sender interface
                                       PRINTER_REMOVED_SIGNAL,          //Signal name
                                       NULL,                            /**match on all object paths**/
                                       NULL,                            /**match on all arguments**/
                                       0,                               //Flags
                                       on_printer_removed,              //callback
                                       user_data,                       //user_data
                                       NULL);

    g_dbus_interface_skeleton_export(G_DBUS_INTERFACE_SKELETON(f->skeleton), connection, DIALOG_OBJ_PATH, &error);
    g_assert_no_error(error);
    activate_backends(f);
}

FrontendObj *get_new_FrontendObj(char *instance_name, event_callback add_cb, event_callback rem_cb)
{
    FrontendObj *f = malloc(sizeof(FrontendObj));
    f->skeleton = print_frontend_skeleton_new();
    if (!instance_name)
        f->bus_name = DIALOG_BUS_NAME;
    else
    {
        char tmp[300];
        sprintf(tmp, "%s%s", DIALOG_BUS_NAME, instance_name);
        f->bus_name = get_string_copy(tmp);
    }
    f->add_cb = add_cb;
    f->rem_cb = rem_cb;
    f->num_backends = 0;
    f->backend = g_hash_table_new(g_str_hash, g_str_equal);
    f->num_printers = 0;
    f->printer = g_hash_table_new(g_str_hash, g_str_equal);
    return f;
}

void connect_to_dbus(FrontendObj *f)
{
    g_bus_own_name(G_BUS_TYPE_SESSION,
                   f->bus_name,
                   0,                //flags
                   NULL,             //bus_acquired_handler
                   on_name_acquired, //name acquired handler
                   NULL,             //name_lost handler
                   f,                //user_data
                   NULL);            //user_data free function
}

void disconnect_from_dbus(FrontendObj *f)
{
    print_frontend_emit_stop_listing(f->skeleton);
}

void activate_backends(FrontendObj *f)
{
    DIR *d;
    struct dirent *dir;
    d = opendir(DBUS_DIR);
    int len = strlen(BACKEND_PREFIX);
    PrintBackend *proxy;

    char *backend_suffix;
    if (d)
    {
        while ((dir = readdir(d)) != NULL)
        {
            if (strncmp(BACKEND_PREFIX, dir->d_name, len) == 0)
            {
                backend_suffix = get_string_copy((dir->d_name) + len);
                printf("Found backend %s\n", backend_suffix);
                proxy = create_backend_from_file(dir->d_name);

                g_hash_table_insert(f->backend, backend_suffix, proxy);
                f->num_backends++;

                print_backend_call_activate_backend(proxy, NULL, NULL, NULL);
            }
        }

        closedir(d);
    }
}

PrintBackend *create_backend_from_file(const char *backend_file_name)
{
    PrintBackend *proxy;
    char *backend_name = get_string_copy(backend_file_name);
    char path[1024];
    sprintf(path, "%s/%s", DBUS_DIR, backend_file_name);
    FILE *file = fopen(path, "r");
    char obj_path[200];
    fscanf(file, "%s", obj_path);
    fclose(file);
    GError *error = NULL;
    proxy = print_backend_proxy_new_for_bus_sync(G_BUS_TYPE_SESSION, 0,
                                                 backend_name, obj_path, NULL, &error);
    g_assert_no_error(error);
    return proxy;
}

gboolean add_printer(FrontendObj *f, PrinterObj *p)
{
    /**
     * todo : add error checking when p->backend_name is not there in the backend hash table 
     */
    p->backend_proxy = g_hash_table_lookup(f->backend, p->backend_name);

    g_hash_table_insert(f->printer, concat(p->name, p->backend_name), p);
    f->num_printers++;
    return TRUE;
}

gboolean remove_printer(FrontendObj *f, char *printer_name)
{
    if (g_hash_table_contains(f->printer, printer_name))
    {
        g_hash_table_remove(f->printer, printer_name);
        f->num_printers--;
        return TRUE;
    }
    return FALSE;
}

void refresh_printer_list(FrontendObj *f)
{
    print_frontend_emit_refresh_backend(f->skeleton);
}

void hide_remote_cups_printers(FrontendObj *f)
{
    print_frontend_emit_hide_remote_printers_cups(f->skeleton);
}
void unhide_remote_cups_printers(FrontendObj *f)
{
    print_frontend_emit_unhide_remote_printers_cups(f->skeleton);
}

void hide_temporary_cups_printers(FrontendObj *f)
{
    print_frontend_emit_hide_temporary_printers_cups(f->skeleton);
}
void unhide_temporary_cups_printers(FrontendObj *f)
{
    print_frontend_emit_unhide_temporary_printers_cups(f->skeleton);
}

PrinterObj *find_PrinterObj(FrontendObj *f, char *printer_name, char *backend_name)
{
    char hashtable_key[512];
    sprintf(hashtable_key, "%s#%s", printer_name, backend_name);

    PrinterObj *p = g_hash_table_lookup(f->printer, hashtable_key);
    g_assert_nonnull(p);
    return p;
}

/** 
 * The following functions are wrappers to the corresponding PrinterObj functions
*/
gboolean printer_is_accepting_jobs(FrontendObj *f, char *printer_name, char *backend_name)
{
    PrinterObj *p = find_PrinterObj(f, printer_name, backend_name);
    return is_accepting_jobs(p);
}
char *get_printer_state(FrontendObj *f, char *printer_name, char *backend_name)
{
    PrinterObj *p = find_PrinterObj(f, printer_name, backend_name);
    return get_state(p);
}
Options *get_all_printer_options(FrontendObj *f, char *printer_name, char *backend_name)
{
    PrinterObj *p = find_PrinterObj(f, printer_name, backend_name);
    return get_all_options(p);
}
int get_active_jobs_count(FrontendObj *f, char *printer_name, char *backend_name)
{
    PrinterObj *p = find_PrinterObj(f, printer_name, backend_name);
    return _get_active_jobs_count(p);
}

int get_all_jobs(FrontendObj *f, Job **j, gboolean active_only)
{
    GHashTableIter iter;
    gpointer key, value;

    int *num_jobs = g_new(int, f->num_backends);
    GVariant **var = g_new(GVariant *, f->num_backends);
    g_hash_table_iter_init(&iter, f->backend);

    int i = 0;
    int total_jobs = 0;
    while (g_hash_table_iter_next(&iter, &key, &value))
    {
        /** Polling all the backends for their active jobs**/
        PrintBackend *proxy = (PrintBackend *)value;
        if (active_only)
            print_backend_call_get_all_active_jobs_sync(proxy, &(num_jobs[i]), &(var[i]), NULL, NULL);
        else
            print_backend_call_get_all_jobs_sync(proxy, &(num_jobs[i]), &(var[i]), NULL, NULL);

        printf("%d jobs\n", num_jobs[i]);
        total_jobs += num_jobs[i];
    }
    Job *jobs = g_new(Job, total_jobs);
    int n = 0;
    for (i = 0; i < f->num_backends; i++)
    {
        unpack_job_array(var[i], num_jobs[i], jobs + n);
        n += num_jobs[i];
    }

    *j = jobs;
    return total_jobs;
}

/**
________________________________________________ PrinterObj __________________________________________
**/

PrinterObj *get_new_PrinterObj()
{
    PrinterObj *p = malloc(sizeof(PrinterObj));
    p->options = NULL;
    //p->settings= ;
    return p;
}

void fill_basic_options(PrinterObj *p, GVariant *gv)
{
    g_variant_get(gv, PRINTER_ADDED_ARGS,
                  &(p->name),
                  &(p->info),
                  &(p->location),
                  &(p->make_and_model),
                  &(p->uri),
                  &(p->is_accepting_jobs),
                  &(p->state),
                  &(p->backend_name));
}

void print_basic_options(PrinterObj *p)
{
    g_message(" Printer %s\n\
                location : %s\n\
                info : %s\n\
                make and model : %s\n\
                uri : %s\n\
                accepting_jobs : %d\n\
                state : %s\n\
                backend: %s\n ",
              p->name,
              p->location,
              p->info,
              p->make_and_model,
              p->uri,
              p->is_accepting_jobs,
              p->state,
              p->backend_name);
}

gboolean is_accepting_jobs(PrinterObj *p)
{
    GError *error = NULL;
    print_backend_call_is_accepting_jobs_sync(p->backend_proxy, p->name,
                                              &p->is_accepting_jobs, NULL, &error);
    g_assert_no_error(error);

    g_message("%d", p->is_accepting_jobs);
    return p->is_accepting_jobs;
}

char *get_state(PrinterObj *p)
{
    GError *error = NULL;
    print_backend_call_get_printer_state_sync(p->backend_proxy, p->name, &p->state, NULL, &error);
    g_assert_no_error(error);

    g_message("%s", p->state);
    return p->state;
}

Options *get_all_options(PrinterObj *p)
{
    /** 
     * If the options were previously queried, 
     * return them, instead of querying again
    */
    if (p->options)
        return p->options;

    p->options = get_new_Options();
    GError *error = NULL;
    int num_options;
    GVariant *var;
    print_backend_call_get_all_attributes_sync(p->backend_proxy, p->name,
                                               &num_options, &var, NULL, &error);
    printf("Num_options is %d\n", num_options);
    unpack_options(var, num_options, p->options);

    return p->options;
}

int _get_active_jobs_count(PrinterObj *p)
{
    int count;
    print_backend_call_get_active_jobs_count_sync(p->backend_proxy, p->name, &count, NULL, NULL);
    printf("%d jobs currently active.\n", count);
    return count;
}

/**
________________________________________________ Settings __________________________________________
**/
Settings *get_new_Settings()
{
    Settings *s = g_new0(Settings, 1);
    s->count = 0;
    s->table = g_hash_table_new(g_str_hash, g_str_equal);
    return s;
}

void add_setting(Settings *s, char *name, char *val)
{
    char *prev = g_hash_table_lookup(s->table, name);
    if (prev)
    {
        /**
         * The value is already there, so replace it instead
         */
        g_hash_table_replace(s->table, name, val);
        free(prev);
    }
    else
    {
        g_hash_table_insert(s->table, name, val);
        s->count++;
    }
}

gboolean clear_setting(Settings *s, char *name)
{
    if (g_hash_table_contains(s->table, name))
    {
        g_hash_table_remove(s->table, name);
        s->count--;
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}
/**
________________________________________________ Options __________________________________________
**/
Options *get_new_Options()
{
    Options *o = g_new0(Options, 1);
    o->count = 0;
    o->table = g_hash_table_new(g_str_hash, g_str_equal);
    return o;
}

/**
 * ________________________________utility functions__________________________
 */
char *concat(char *a, char *b)
{
    char str[512];
    sprintf(str, "%s#%s", a, b);
    return get_string_copy(str);
}

void unpack_options(GVariant *var, int num_options, Options *options)
{
    options->count = num_options;
    int i, j;
    char *str;
    GVariantIter *iter;
    GVariantIter *array_iter;
    char *name, *default_val;
    int num_sup;
    g_variant_get(var, "a(ssia(s))", &iter);
    Option *opt;
    for (i = 0; i < num_options; i++)
    {
        //printf("i = %d\n", i);
        opt = g_new0(Option, 1);
        g_variant_iter_loop(iter, "(ssia(s))", &name, &default_val,
                            &num_sup, &array_iter);
        opt->option_name = get_string_copy(name);
        opt->default_value = get_string_copy(default_val);
        opt->num_supported = num_sup;
        opt->supported_values = new_cstring_array(num_sup);
        for (j = 0; j < num_sup; j++)
        {
            g_variant_iter_loop(array_iter, "(s)", &str);
            opt->supported_values[j] = get_string_copy(str);
        }
        g_hash_table_insert(options->table, (gpointer)opt->option_name, (gpointer)opt);
        print_option(opt);
    }
}
/************************************************************************************************/
