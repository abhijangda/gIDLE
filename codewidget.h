/*39 Lines*/ 
#include<gdk/gdkkeysyms.h>
#include<gtksourceview/gtksourceview.h>
#include<gtksourceview/gtksourcelanguage.h>
#include<gtksourceview/gtksourcelanguagemanager.h>
#include"pymodule.h"

#ifndef __CODE_WIDGET_H
#define __CODE_WIDGET_H
//A Code Widget will contain a GtkSourceView, two ComboBox, GtkFixedPut, GtkListView and GtkHBox

struct _codewidget
{
    GtkWidget *txtinput; //done
    GtkWidget *func_combo_box; //done
    GtkWidget *class_combo_box; //done
    GtkWidget *fixed_txtinput; //done
    GtkWidget *list_view;
    GtkWidget *hbox;
    GtkTooltip *tooltip;
    
    pyclass *pyclass_array;
    int pyclass_count;
    
    pymodule *module_array;
    int module_array_size;
       
    char **path_array;
    };

typedef struct _codewidget CodeWidget; 
void codewidget_new(CodeWidget *); 
void codewidget_get_class_and_members(CodeWidget *);
void codewidget_fill_combo_boxes(CodeWidget *);
void codewidget_clear_arrays(CodeWidget *);
void codewidget_get_import_files_class(CodeWidget *,char *filename);
void codewidget_destroy(CodeWidget *cwidget);
void func(CodeWidget *);
#endif