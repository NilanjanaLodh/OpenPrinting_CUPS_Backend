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
    char *s = malloc(sizeof(char) * (strlen(str) + 1));
    strcpy(s, str);
    return s;
}