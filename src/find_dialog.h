#include <gtk/gtk.h>

#ifndef FIND_DIALOG_H
#define FIND_DIALOG_H
void
load_find_dialog (GtkTextView *text_view);

void
cmdclose_clicked (GtkWidget *widget, gpointer dialog);

void
cmdfind_clicked (GtkWidget *widget);

void
cmdfindnext_clicked (GtkWidget *widget);

void
cmdfindprev_clicked (GtkWidget *widget);
#endif /*FIND_DIALOG_H*/