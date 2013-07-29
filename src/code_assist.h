#include <gtk/gtk.h>

#include "py_variable.h"

#ifndef CODE_ASSIST_H
#define CODE_ASSIST_H

#define CODE_ASSIST_WIDTH 300
#define CODE_ASSIST_HEIGHT 200

struct _code_assist
{
    GtkWidget *list_view;
    GtkWidget *scroll_win;
    GtkWidget *parent;
    GtkListStore *list_store;
    GtkWidget *text_view;
    GtkWidget *text_scroll_win;
    PyVariable **py_var_array;
    int py_var_array_size;
};

typedef struct _code_assist CodeAssistWidget;

CodeAssistWidget *
code_assist_widget_new();

void
code_assist_add_py_var (CodeAssistWidget *, PyVariable *);

void
code_assist_clear (CodeAssistWidget *);

gboolean
code_assist_show_matches (CodeAssistWidget *, gchar *string);

void
code_assist_destroy (CodeAssistWidget *);

#endif /*CODE_ASSIST_H*/