#include<gtk/gtk.h>

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
};

typedef struct _code_assist CodeAssistWidget;

CodeAssistWidget *
code_assist_widget_new();

void
code_assist_destroy (CodeAssistWidget *);

#endif /*CODE_ASSIST_H*/