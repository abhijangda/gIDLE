#include "py_class.h"
#include "code_folding_widget.h"

#include <gtk/gtk.h>
#include <gtksourceview/gtksourceview.h>
#include <gtksourceview/gtksourcelanguagemanager.h>

#ifndef CODEWIDGET_H
#define CODEWIDGET_H

#define FILE_NEW 0
#define FILE_EXISTS 1

struct _codewidget
{
    GtkWidget *vbox;
    GtkWidget *hbox;
    GtkWidget *scrollwin;
    GtkWidget *sourceview;
    GtkWidget *class_combobox;
    GtkWidget *func_combobox;
    GtkWidget *line_num_widget; //For Line Numbers
    GtkWidget *hbox_scroll_line; //HBox for line_num_widget, code_folding_widget and scrollwin
    GtkTextTag *invisible_tag;
    GtkSourceBuffer *sourcebuffer;
    CodeFoldingWidget *code_folding_widget;
    char *file_path;
    int file_mode;
    int line_history [5];
    int line_history_current_index;
    int prev_line;
    PyClass **py_class_array;
    int py_class_array_size;
    int drawing_area_width;
    gulong combo_class_changed_handler_id;
    gulong combo_func_changed_handler_id;
    gulong buffer_mark_set_handler_id;
};

typedef struct _codewidget CodeWidget;

CodeWidget *
codewidget_new ();

void
codewidget_destroy (CodeWidget *);

void
codewidget_mark_set (GtkTextBuffer *, GtkTextIter *, GtkTextMark *, gpointer data);

gboolean
codewidget_key_press (GtkWidget *, GdkEvent *, gpointer);

void
codewidget_set_text (CodeWidget *, gchar *text);

void
codewidget_add_class (CodeWidget *, PyClass *);
#endif /*CODEWIDGET_H*/