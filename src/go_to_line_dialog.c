#include "go_to_line_dialog.h"
#include "core_funcs.h"

GtkTextView *text_view;

void
load_go_to_line_dialog (GtkTextView *view)
{
    text_view = view;
    GtkBuilder *go_to_line_builder;
    
    go_to_line_builder = gtk_builder_new ();
    gtk_builder_add_from_file (go_to_line_builder, "./ui/go_to_line_dialog.glade", NULL);
    
    GObject *dialog = gtk_builder_get_object (go_to_line_builder, "go_to_line_dialog");
    gtk_widget_show (GTK_WIDGET (dialog));
    
    GtkWidget *entry = GTK_WIDGET (gtk_builder_get_object (go_to_line_builder,
                            "entry_line"));
    
    g_signal_connect (gtk_builder_get_object (go_to_line_builder, "go_to_cmdClose"),
                     "clicked", G_CALLBACK (go_to_cmdclose_clicked), (gpointer)dialog);
    g_signal_connect (gtk_builder_get_object (go_to_line_builder, "go_to_cmdGo"),
                     "clicked", G_CALLBACK (go_to_cmdGo_clicked), entry);
}

void
go_to_cmdclose_clicked (GtkWidget *widget, gpointer dialog)
{
    gtk_widget_destroy (GTK_WIDGET (dialog));
}

void
go_to_cmdGo_clicked (GtkWidget *widget, gpointer entry)
{
    int line = g_strtod (gtk_entry_get_text (GTK_ENTRY (entry)), NULL);
    go_to_line (text_view, line);
}