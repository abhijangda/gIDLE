#include "class_browser.h"
#include "codewidget.h"
#include "main.h"

GtkWidget **tree_view_array;

void
load_class_browser_dialog ()
{
    GtkBuilder *class_browser_builder;
    
    class_browser_builder = gtk_builder_new ();
    gtk_builder_add_from_file (class_browser_builder, "./ui/class_browser_dlg.glade", NULL);
    
    GObject *dialog = gtk_builder_get_object (class_browser_builder, "class_browser_dialog");
    gtk_widget_show (GTK_WIDGET (dialog));
    
    g_signal_connect (gtk_builder_get_object (class_browser_builder, "go_to_cmdClose"),
                     "clicked", G_CALLBACK (go_to_cmdclose_clicked), (gpointer)dialog);

    
}

void
insert_rows_in_tree_view (){}
void
class_browser_cmdCancel_clicked (GtkWidget *widget, gpointer dialog)
{
    gtk_widget_destroy (GTK_WIDGET (dialog));
}