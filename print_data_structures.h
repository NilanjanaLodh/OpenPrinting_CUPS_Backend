#ifndef _PRINT_DATA_STRUCTURES_H_
#define _PRINT_DATA_STRUCTURES_H_

#define STOP_BACKEND_SIGNAL "StopListing"
#define ACTIVATE_BACKEND_SIGNAL "GetBackend"
#define REFRESH_BACKEND_SIGNAL "RefreshBackend"
#define PRINTER_ADDED_SIGNAL "PrinterAdded"
#define PRINTER_REMOVED_SIGNAL "PrinterRemoved"
#include <glib.h>

gboolean get_boolean(gchar *);

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
/***************************************************************/

/*********LISTING OF ALL POSSIBLE OPTIONS*****/
//Rename these to something better if needed
/// replace these with #defines maybe ?

#define MEDIA_A4 "A4"
#define MEDIA_A3 "A3"
#define MEDIA_A5 "A5"
#define MEDIA_A6 "A6"
#define MEDIA_LEGAL "LEGAL"
#define MEDIA_LETTER "LETTER"
#define MEDIA_PHOTO "PHOTO"
#define MEDIA_TABLOID "TABLOID"
#define MEDIA_ENV "ENV10"

#define COLOR_MODE_COLOR "color"
#define COLOR_MODE_BW "monochrome"
#define COLOR_MODE_AUTO "auto"


#define QUALITY_DRAFT "draft"
#define QUALITY_NORMAL "normal"
#define QUALITY_HIGH "high"

#define SIDES_ONE_SIDED "one-sided"
#define SIDES_TWO_SIDED_SHORT "two-sided-short"
#define SIDES_TWO_SIDED_LONG "two-sided-long"

#define ORIENTATION_PORTRAIT "portrait"
#define ORIENTATION_LANDSCAPE "landscape"

#define PRIOIRITY_URGENT "urgent"
#define PRIOIRITY_HIGH "high"
#define PRIOIRITY_MEDIUM "medium"
#define PRIOIRITY_LOW "low"


#endif