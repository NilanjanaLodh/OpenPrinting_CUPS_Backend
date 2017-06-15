#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <glib.h>
#include "common_helper.h"
#include "backend_interface.h"
#include "frontend_interface.h"

gboolean get_boolean(gchar *g)
{
    if (g_str_equal(g, "true"))
        return TRUE;

    return FALSE;
}