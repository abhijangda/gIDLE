#include "py_class.h"
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
    GtkSourceBuffer *sourcebuffer;
    char *file_path;
    int file_mode;
    int line_history [5];
    int line_history_current_index;
    int prev_line;
    PyClass *py_class_array;
    int py_class_array_size;
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