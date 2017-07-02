#include "frontend_helper.h"

PrinterObj *get_new_PrinterObj()
{
    PrinterObj *p = malloc(sizeof(PrinterObj));
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
                  &(p->state));
}
void print_basic_options(PrinterObj *p)
{
    g_message(" Printer %s\n\
                location : %s\n\
                info : %s\n\
                make and model : %s\n\
                uri : %s\n\
                accepting_jobs : %d\n\
                state : %s\n",
              p->name,
              p->location, p->info, p->make_and_model, p->uri, p->is_accepting_jobs, p->state);
}

void update_basic_options(PrinterObj *p)
{
    GError *error = NULL;
    print_backend_call_list_basic_options_sync(p->backend_proxy,
                                               p->name,
                                               &p->info,
                                               &p->location,
                                               &p->make_and_model,
                                               &p->is_accepting_jobs,
                                               &p->state,
                                               NULL, &error);

    print_basic_options(p);
}
PrinterCapabilities get_capabilities(PrinterObj *p)
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
                                                     &(p->capabilities.resolution),
                                                     NULL, &error);

    g_assert_no_error(error);
    print_capabilities(p);
    return p->capabilities;
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
void get_supported_values_raw(PrinterObj *p, gchar *option_name)
{
    GError *error = NULL;
    int num_values;
    GVariant *values;
    print_backend_call_get_supported_values_raw_string_sync(p->backend_proxy, p->name,
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
    unpack_string_array(values, p->supported.num_media, &p->supported.media);
}
void get_supported_color(PrinterObj *p)
{
    GError *error = NULL;
    GVariant *values;
    gchar *str;
    GVariantIter *iter;
    print_backend_call_get_supported_color_sync(p->backend_proxy, p->name,
                                                &p->supported.num_color, &values, NULL, &error);
    unpack_string_array(values, p->supported.num_color, &p->supported.color);
}
void get_supported_quality(PrinterObj *p)
{
    GError *error = NULL;
    GVariant *values;
    gchar *str;
    GVariantIter *iter;
    print_backend_call_get_supported_quality_sync(p->backend_proxy, p->name,
                                                  &p->supported.num_quality, &values, NULL, &error);
    unpack_string_array(values, p->supported.num_quality, &p->supported.quality);
}
void get_supported_orientation(PrinterObj *p)
{
    GError *error = NULL;
    GVariant *values;
    gchar *str;
    GVariantIter *iter;
    print_backend_call_get_supported_orientation_sync(p->backend_proxy, p->name,
                                                      &p->supported.num_orientation, &values, NULL, &error);
    unpack_string_array(values, p->supported.num_orientation, &p->supported.orientation);
}
void get_supported_resolution(PrinterObj *p)
{
    GError *error = NULL;
    GVariant *values;
    gchar *str;
    GVariantIter *iter;
    print_backend_call_get_supported_resolution_sync(p->backend_proxy, p->name,
                                                     &p->supported.num_res, &values, NULL, &error);
    unpack_string_array(values, p->supported.num_res, &p->supported.res);
}
void get_state(PrinterObj *p)
{
    GError *error = NULL;
    print_backend_call_get_printer_state_sync(p->backend_proxy, p->name, &p->state, NULL, &error);
    g_assert_no_error(error);

    /// To do : set the other boolean flags too
    g_message("%s", p->state);
}
void get_resolution(PrinterObj *p)
{
    GError *error = NULL;
    print_backend_call_get_default_resolution_sync(p->backend_proxy, p->name,
                                                   &p->defaults.res,
                                                   NULL, &error);
    g_assert_no_error(error);

    g_message("%s", p->defaults.res);
}
char *get_media(PrinterObj *p)
{
    GError *error = NULL;
    print_backend_call_get_default_media_sync(p->backend_proxy, p->name,
                                              &p->defaults.media, NULL, NULL);
    printf("Default media is %s\n", p->defaults.media);

    return p->defaults.media;
    g_assert_no_error(error);
}
void is_accepting_jobs(PrinterObj *p)
{
    GError *error = NULL;
    print_backend_call_is_accepting_jobs_sync(p->backend_proxy, p->name, &p->is_accepting_jobs, NULL, &error);
    g_assert_no_error(error);

    g_message("%d", p->is_accepting_jobs);
}
void set_resolution(PrinterObj *p, int xres, int yres)
{
    // gboolean possible;
    // print_backend_call_check_resolution_sync(p->backend_proxy, p->name,
    //                                          xres, yres, &possible,
    //                                          NULL, NULL);
    // if (possible)
    // {
    //     p->defaults.res.xres = xres;
    //     p->defaults.res.yres = yres;
    // }
}
void get_orientation(PrinterObj *p)
{
    GError *error = NULL;
    print_backend_call_get_default_orientation_sync(p->backend_proxy, p->name,
                                                    &p->defaults.orientation,
                                                    NULL, &error);
    g_assert_no_error(error);

    g_message("default orientation: %s", p->defaults.orientation);
}
/************************************************* FrontendObj********************************************/

FrontendObj *get_new_FrontendObj()
{
    FrontendObj *f = malloc(sizeof(FrontendObj));
    f->num_backends = 0;
    f->backend = g_hash_table_new(g_str_hash, g_str_equal);
    f->num_printers = 0;
    f->printer = g_hash_table_new(g_str_hash, g_str_equal);
    return f;
}
void activate_backends(FrontendObj *f)
{
    DIR *d;
    struct dirent *dir;
    d = opendir(DBUS_DIR);
    int len = strlen(BACKEND_PREFIX);
    PrintBackend *proxy;
    if (d)
    {
        while ((dir = readdir(d)) != NULL)
        {
            if (strncmp(BACKEND_PREFIX, dir->d_name, len) == 0)
            {
                printf("Found backend %s\n", dir->d_name);
                proxy = add_backend(f, dir->d_name);
                print_backend_call_activate_backend(proxy, NULL, NULL, NULL);
            }
        }

        closedir(d);
    }
}

PrintBackend *add_backend(FrontendObj *f, const char *backend_file_name)
{
    PrintBackend *proxy;
    if (g_hash_table_contains(f->backend, backend_file_name))
        return proxy;

    char *backend_name = get_string_copy(backend_file_name);
    ///what about this arbitrary limit?
    char path[1000];
    sprintf(path, "%s/%s", DBUS_DIR, backend_file_name);
    FILE *file = fopen(path, "r");
    char obj_path[200]; // arbit :/
    fscanf(file, "%s", obj_path);

    GError *error = NULL;
    proxy = print_backend_proxy_new_for_bus_sync(G_BUS_TYPE_SESSION, 0,
                                                 backend_name, obj_path, NULL, &error);
    g_assert_no_error(error);
    g_hash_table_insert(f->backend, backend_name, proxy);
    return proxy;
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
PrinterObj *update_basic_printer_options(FrontendObj *f, gchar *printer_name)
{
    PrinterObj *p = g_hash_table_lookup(f->printer, printer_name);
    g_assert_nonnull(p);

    update_basic_options(p);
    return p;
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

void get_printer_supported_values_raw(FrontendObj *f, gchar *printer_name, gchar *option_name)
{
    PrinterObj *p = g_hash_table_lookup(f->printer, printer_name);
    g_assert_nonnull(p);

    get_supported_values_raw(p, option_name);
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
void get_printer_supported_resolution(FrontendObj *f, gchar *printer_name)
{
    PrinterObj *p = g_hash_table_lookup(f->printer, printer_name);
    g_assert_nonnull(p);

    get_supported_resolution(p);
}
char *get_printer_state(FrontendObj *f, gchar *printer_name)
{
    PrinterObj *p = g_hash_table_lookup(f->printer, printer_name);
    g_assert_nonnull(p);

    get_state(p);
    return p->state;
}
gboolean printer_is_accepting_jobs(FrontendObj *f, gchar *printer_name)
{
    PrinterObj *p = g_hash_table_lookup(f->printer, printer_name);
    g_assert_nonnull(p);

    is_accepting_jobs(p);
    return p->is_accepting_jobs;
}
char *get_printer_default_resolution(FrontendObj *f, gchar *printer_name)
{
    PrinterObj *p = g_hash_table_lookup(f->printer, printer_name);
    g_assert_nonnull(p);

    get_resolution(p);
    return p->defaults.res;
}
void set_printer_resolution(FrontendObj *f, gchar *printer_name, int xres, int yres)
{
    PrinterObj *p = g_hash_table_lookup(f->printer, printer_name);
    g_assert_nonnull(p);

    set_resolution(p, xres, yres);
}
char *get_printer_default_orientation(FrontendObj *f, gchar *printer_name)
{
    PrinterObj *p = g_hash_table_lookup(f->printer, printer_name);
    g_assert_nonnull(p);

    get_orientation(p);
    p->defaults.orientation;
}
void get_printer_default_media(FrontendObj *f, gchar *printer_name)
{
    PrinterObj *p = g_hash_table_lookup(f->printer, printer_name);
    g_assert_nonnull(p);

    get_media(p);
}
void pingtest(FrontendObj *f, gchar *printer_name)
{
    /**This function is just for debugging and will be removed soon**/
    PrinterObj *p = g_hash_table_lookup(f->printer, printer_name);
    g_assert_nonnull(p);
    print_backend_call_ping_sync(p->backend_proxy, p->name, NULL, NULL);
}
char *get_default_printer(FrontendObj *f, gchar *backend_name)
{
    PrintBackend *proxy = g_hash_table_lookup(f->backend, backend_name);
    g_assert_nonnull(proxy);
    char *def;
    print_backend_call_get_default_printer_sync(proxy, &def, NULL, NULL);
    printf("%s\n", def);
    return def;
}
