#ifndef _FRONTEND_HELPER_H_
#define _FRONTEND_HELPER_H_

#include <glib.h>



/**********Supportedvalues**************************************/
typedef struct _SupportedValues SupportedValues;
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
void get_option_default(PrinterObj *, gchar *);
void get_supported_values_raw(PrinterObj *, gchar *);
void get_supported_media(PrinterObj *);
void get_supported_color(PrinterObj *);
void get_supported_quality(PrinterObj *);
void get_supported_orientation(PrinterObj *);
void get_state(PrinterObj *);
void is_accepting_jobs(PrinterObj *);

/**********FrontendObj definitions*******************************/
typedef struct _FrontendObj FrontendObj;

FrontendObj *get_new_FrontendObj();
gboolean add_printer(FrontendObj *, PrinterObj *, gchar *, gchar *); ///think about this definition a little more
void update_basic_printer_options(FrontendObj *, gchar *);
void get_printer_capabilities(FrontendObj *, gchar *);
void get_printer_option_default(FrontendObj *, gchar *, gchar *);
void get_printer_supported_values_raw(FrontendObj *, gchar *, gchar *);
void get_printer_supported_media(FrontendObj *, gchar *);
void get_printer_supported_color(FrontendObj *, gchar *);
void get_printer_supported_quality(FrontendObj *, gchar *);
void get_printer_supported_orientation(FrontendObj *, gchar *);
void get_printer_state(FrontendObj *, gchar *);
void printer_is_accepting_jobs(FrontendObj *, gchar *);

/***************************************************************/



#endif