#include <stdio.h>
#include <stdlib.h>
#include <glib.h>
#include <string.h>
#include <cups/cups.h>
#include "backend_interface.h"
#include "backend_helper.h"
#include <glib.h>

char* cups_printer_state(cups_dest_t *dest)
{
    //cups_dest_t *dest = cupsGetNamedDest(CUPS_HTTP_DEFAULT, printer_name, NULL);
    g_assert_nonnull(dest);
    char *state = cupsGetOption("printer-state", dest->num_options,
                                    dest->options);
    if(state==NULL)
        return "NA";
    
    switch(state[0])
    {
        case '3': return STATE_IDLE;
        case '4': return STATE_PRINTING;
        case '5': return STATE_STOPPED;

        default: return "NA";
    }
}

gboolean cups_is_accepting_jobs(cups_dest_t *dest)
{
    g_assert_nonnull(dest);
    char *val = cupsGetOption("printer-is-accepting-jobs", dest->num_options,
                                    dest->options);
    return get_boolean(val);
}