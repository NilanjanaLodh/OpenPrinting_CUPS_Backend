#ifndef _PRINT_DATA_STRUCTURES_H_
#define _PRINT_DATA_STRUCTURES_H_

#define ACTIVATE_BACKEND_SIGNAL "GetBackend"
#define PRINTER_ADDED_SIGNAL "PrinterAdded"
#define PRINTER_REMOVED_SIGNAL "PrinterRemoved"

typedef struct _PrinterObj PrinterObj;



/**********FrontendObj definitions*******************************/
typedef struct _FrontendObj FrontendObj;

FrontendObj *get_new_FrontendObj();
gboolean add_printer(FrontendObj* , PrinterObj*); ///think about this definition a little more
/***************************************************************/
#endif