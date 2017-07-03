#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <glib.h>
#include "common_helper.h"
#include "backend_interface.h"
#include "frontend_interface.h"

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
    if(str==NULL)
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

    *val = (char **)malloc(sizeof(char *) * num_val);
    char **array = *val;
    g_variant_get(variant, "a(s)", &iter);
    int i = 0;
    char *str;
    for (i = 0; i < num_val; i++)
    {
        g_variant_iter_loop(iter, "(s)", &str);
        array[i] = get_string_copy(str);
        printf(" %s\n", str);
    }
}
GVariant* pack_string_array(int num_val, char **val)
{
    GVariantBuilder *builder;
    GVariant *values;
    builder = g_variant_builder_new(G_VARIANT_TYPE("a(s)"));
    for (int i = 0; i < num_val; i++)
    {
        g_message("%s", val[i]);
        g_variant_builder_add(builder, "(s)", val[i]);
    }

    if (num_val == 0)
        g_variant_builder_add(builder, "(s)", "NA");

    values = g_variant_new("a(s)", builder);
    return values;
}