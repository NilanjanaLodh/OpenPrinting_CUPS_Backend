#include "common_helper.h"

gboolean get_boolean(const char *g)
{
    if (!g)
        return FALSE;

    if (g_str_equal(g, "true"))
        return TRUE;

    return FALSE;
}

char *get_string_copy(const char *str)
{
    if (str == NULL)
        return NULL;
    char *s = malloc(sizeof(char) * (strlen(str) + 1));
    strcpy(s, str);
    return s;
}

void unpack_string_array(GVariant *variant, int num_val, char ***val)
{
    GVariantIter *iter;

    if (!num_val)
    {
        printf("No supported values found.\n");
        *val = NULL;
        return;
    }
    char **array = (char **)malloc(sizeof(char *) * num_val);

    g_variant_get(variant, "a(s)", &iter);

    int i = 0;
    char *str;
    for (i = 0; i < num_val; i++)
    {
        g_variant_iter_loop(iter, "(s)", &str);
        array[i] = get_string_copy(str);
        printf(" %s\n", str);
    }
    *val = array;
}

GVariant *pack_string_array(int num_val, char **val)
{
    GVariantBuilder *builder;
    GVariant *values;
    builder = g_variant_builder_new(G_VARIANT_TYPE("a(s)"));
    for (int i = 0; i < num_val; i++)
    {
        // g_message("%s", val[i]);
        g_variant_builder_add(builder, "(s)", val[i]);
    }

    if (num_val == 0)
        g_variant_builder_add(builder, "(s)", "NA");

    values = g_variant_new("a(s)", builder);
    return values;
}

char *get_absolute_path(const char *file_path)
{
    if (!file_path)
        return NULL;

    if (file_path[0] == '/')
        return (char *)file_path;

    if (file_path[0] == '~')
    {
        struct passwd *passwdEnt = getpwuid(getuid());
        char *home = passwdEnt->pw_dir;
        char fp[1024], suffix[512];
        strcpy(suffix, file_path + 2);
        sprintf(fp, "%s/%s", home, suffix);
        return get_string_copy(fp);
    }
    char fp[1024], cwd[512];
    getcwd(cwd, sizeof(cwd));
    sprintf(fp, "%s/%s", cwd, file_path);
    printf("%s\n", fp);
    return get_string_copy(fp);
}
char *extract_file_name(const char *file_path)
{
    if (!file_path)
        return NULL;

    char *file_name_ptr = (char *)file_path;

    char *x = (char *)file_path;
    char c = *x;
    while (c)
    {
        if (c == '/')
            file_name_ptr = x;

        x++;
        c = *x;
    }
    return file_name_ptr;
}