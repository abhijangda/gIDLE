#include <gtk/gtk.h>

#ifndef CLASS_BROWSER_H
#define CLASS_BROWSER_H
void
load_class_browser_dialog (GtkTextView *text_view);

void
class_browser_cmdCancel_clicked (GtkWidget *widget, gpointer);
#endif /*CLASS_BROWSER_H*/