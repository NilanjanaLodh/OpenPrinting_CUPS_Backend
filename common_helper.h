#ifndef _COMMON_HELPER_H_
#define _COMMON_HELPER_H_

#define PRINTER_ADDED_ARGS "(sssssbs)"
/*********LISTING OF ALL POSSIBLE OPTIONS*****/
//Rename these to something better if needed

#define new_cstring_array(x) ((char**)(malloc(sizeof(char*) * x))) 
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

#define STATE_IDLE "idle"
#define STATE_PRINTING "printing"
#define STATE_STOPPED "stopped"

#define STOP_BACKEND_SIGNAL "StopListing"
#define REFRESH_BACKEND_SIGNAL "RefreshBackend"
#define PRINTER_ADDED_SIGNAL "PrinterAdded"
#define PRINTER_REMOVED_SIGNAL "PrinterRemoved"
#define HIDE_REMOTE_CUPS_SIGNAL "HideRemotePrintersCUPS"
#define UNHIDE_REMOTE_CUPS_SIGNAL "UnhideRemotePrintersCUPS"
#define HIDE_TEMP_CUPS_SIGNAL "HideTemporaryPrintersCUPS"
#define UNHIDE_TEMP_CUPS_SIGNAL "UnhideTemporaryPrintersCUPS"
gboolean get_boolean(const char *);
char *get_string_copy(const char *);
void unpack_string_array(GVariant *variant, int num_val, char ***val);
GVariant *pack_string_array(int num_val, char **val);

#endif