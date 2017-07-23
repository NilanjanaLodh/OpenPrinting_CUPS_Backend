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

/**
 * Get a new FrontendObj instance
 * 
 * @params
 * 
 * instance name: The suffix to be used for the dbus name for Frontend
 *              supply NULL for no suffix
 * 
 * add_cb : The callback function to call when a new printer is added
 * rem_cb : The callback function to call when a printer is removed
 * 
 */
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

/**
 * Start the frontend D-Bus Service
 */
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

/**
 * Notify Backend services before stopping Frontend
 */
void disconnect_from_dbus(FrontendObj *f)
{
    print_frontend_emit_stop_listing(f->skeleton);
}

/**
 * Discover the currently installed backends and activate them
 * 
 * 
 * 
 * Reads the DBUS_DIR folder to find the files installed by 
 * the respective backends , 
 * For eg:  org.openprinting.Backend.XYZ
 * 
 * XYZ = Backend suffix, using which it will be identified henceforth 
 */
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

/**
 * Read the file installed by the backend and create a proxy object 
 * using the backend service name and object path.
 */
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

/**
 * Add the printer to the FrontendObj instance
 */
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

/**
 * Remove the printer from FrontendObj
 */
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

/**
________________________________________________ PrinterObj __________________________________________
**/

PrinterObj *get_new_PrinterObj()
{
    PrinterObj *p = malloc(sizeof(PrinterObj));
    return p;
}

/**
 * Fill the basic options of PrinterObj from the GVariant returned with the printerAdded signal
 */
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

/**
 * Print the basic options of PrinterObj
 */
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

/**
 * ________________________________utility functions__________________________
 */
char *concat(char *a, char *b)
{
    char str[512];
    sprintf(str, "%s#%s", a, b);
    return get_string_copy(str);
}