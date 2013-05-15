#include <gtk/gtk.h>

#ifndef FIND_REPLACE_DIALOG_H
#define FIND_REPLACE_DIALOG_H
void
load_find_replace (GtkTextView *text_view);

void
find_replace_cmdFind_clicked (GtkWidget *widget);

void
find_replace_cmdReplaceAll_clicked (GtkWidget *widget);

void
find_replace_cmdReplace_clicked (GtkWidget *widget);

void
find_replace_cmdClose_clicked (GtkWidget *widget, gpointer dialog);
#endif //FIND_REPLACE_DIALOG_H