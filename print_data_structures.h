#ifndef _PRINT_DATA_STRUCTURES_H_
#define _PRINT_DATA_STRUCTURES_H_

#define STOP_BACKEND_SIGNAL "StopListing"
#define ACTIVATE_BACKEND_SIGNAL "GetBackend"
#define REFRESH_BACKEND_SIGNAL "RefreshBackend"
#define PRINTER_ADDED_SIGNAL "PrinterAdded"
#define PRINTER_REMOVED_SIGNAL "PrinterRemoved"
#include <glib.h>

gboolean get_boolean(gchar *);


/**********PrinterCapabilities**********************************/
typedef struct _PrinterCapabilities PrinterCapabilities;
/**********PrinterObj definitions*******************************/

typedef struct _PrinterObj PrinterObj;
PrinterObj *get_new_PrinterObj();
void fill_basic_options(PrinterObj *, GVariant *);
void print_basic_options(PrinterObj *);
void update_basic_options(PrinterObj *);
void get_capabilities(PrinterObj *);
void print_capabilities(PrinterObj *);
void get_option_default(PrinterObj * , gchar *);
void get_supported_values(PrinterObj * , gchar *);
/**********FrontendObj definitions*******************************/
typedef struct _FrontendObj FrontendObj;

FrontendObj *get_new_FrontendObj();
gboolean add_printer(FrontendObj *, PrinterObj * ,gchar *, gchar *); ///think about this definition a little more
void update_basic_printer_options(FrontendObj * , gchar *);
void get_printer_capabilities(FrontendObj * , gchar *);
void get_printer_option_default(FrontendObj *,gchar * , gchar *);
void get_printer_supported_values(FrontendObj *,gchar * , gchar *);
/***************************************************************/
#endif