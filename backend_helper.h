#ifndef _BACKEND_HELPER_H
#define _BACKEND_HELPER_H

#include "backend_interface.h"
#include "common_helper.h"

char* cups_printer_state(cups_dest_t *dest);
gboolean cups_is_accepting_jobs(cups_dest_t *dest);
void getResolution(cups_dest_t *dest , int *xres , int *yres);

#endif