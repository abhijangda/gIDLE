#include <gtk/gtk.h>

#ifndef GO_TOLINE_DIALOG_H
#define GO_TOLINE_DIALOG_H
void
load_go_to_line_dialog (GtkTextView *text_view);

void
go_to_cmdclose_clicked (GtkWidget *widget, gpointer dialog);

void
go_to_cmdGo_clicked (GtkWidget *widget, gpointer);
#endif /*GO_TOLINE_DIALOG_H*/