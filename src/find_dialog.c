#include "find_dialog.h"
#include "core_funcs.h"

gchar *search_text;
GtkWidget *entry_text, *chkCase;
GtkTextView *text_view;

/*Load
 * Find Dialog
 */
void
load_find_dialog (GtkTextView *view)
{
    text_view = view;
    GtkBuilder *find_builder;
    
    find_builder = gtk_builder_new ();
    gtk_builder_add_from_file (find_builder, "./ui/find_dialog.glade", NULL);
    
    GObject *dialog = gtk_builder_get_object (find_builder, "find_dialog");
    gtk_widget_show (GTK_WIDGET (dialog));
    
    entry_text = GTK_WIDGET (gtk_builder_get_object (find_builder,
                            "find_dialog_entry_text"));
    if (search_text != NULL)
        gtk_entry_set_text (GTK_ENTRY (entry_text), search_text);
    g_free (search_text);
    
    chkCase = GTK_WIDGET (gtk_builder_get_object (find_builder,
                         "find_dialog_chkCase"));
    
    g_signal_connect (gtk_builder_get_object (find_builder, "find_dialog_cmdClose"),
                     "clicked", G_CALLBACK (cmdclose_clicked), (gpointer)dialog);
    g_signal_connect (gtk_builder_get_object (find_builder, "find_dialog_cmdFind"),
                     "clicked", G_CALLBACK (cmdfind_clicked), NULL);
    g_signal_connect (gtk_builder_get_object (find_builder, "find_dialog_cmdFindNext"),
                     "clicked", G_CALLBACK (cmdfindnext_clicked), NULL);
    g_signal_connect (gtk_builder_get_object (find_builder, "find_dialog_cmdFindPrev"),
                     "clicked", G_CALLBACK (cmdfindprev_clicked), NULL);
}

void
cmdclose_clicked (GtkWidget *widget, gpointer dialog)
{
    gtk_widget_destroy (GTK_WIDGET (dialog));
}

void
cmdfind_clicked (GtkWidget *widget)
{
    GtkTextIter start_iter, end_iter;
    GtkTextBuffer *buffer = gtk_text_view_get_buffer (text_view);
    const gchar *text = gtk_entry_get_text (GTK_ENTRY (entry_text));
    GtkTextSearchFlags flags = GTK_TEXT_SEARCH_TEXT_ONLY;
    
    if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (chkCase)))
        flags = flags | GTK_TEXT_SEARCH_CASE_INSENSITIVE;

    gtk_text_buffer_get_iter_at_offset (buffer, &start_iter, 0);
    
    if (gtk_text_iter_forward_search (&start_iter, text, flags, 
                                       &start_iter, &end_iter, NULL))
    {
        gtk_text_buffer_select_range (buffer, &start_iter, &end_iter);
        gtk_text_view_scroll_to_iter (text_view,  &start_iter,
                                     0.1, FALSE, 0.5, 0.5);
    }
    else
    {
        //Status Bar show cannot find text
    }
    search_text = g_strdup (text);
}

void
cmdfindnext_clicked (GtkWidget *widget)
{
    const gchar *text = gtk_entry_get_text (GTK_ENTRY (entry_text));
    GtkTextSearchFlags flags = GTK_TEXT_SEARCH_TEXT_ONLY;
    
    if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (chkCase)))
        flags = flags | GTK_TEXT_SEARCH_CASE_INSENSITIVE;

    if (!find_next (text_view, text, flags))
    {
        //Status Bar show cannot find text
    }
    search_text = g_strdup (text);
}

void
cmdfindprev_clicked (GtkWidget *widget)
{
    const gchar *text = gtk_entry_get_text (GTK_ENTRY (entry_text));
    GtkTextSearchFlags flags = GTK_TEXT_SEARCH_TEXT_ONLY;
    
    if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (chkCase)))
        flags = flags | GTK_TEXT_SEARCH_CASE_INSENSITIVE;

    if (!find_previous (text_view, text, flags))
    {
        //Status Bar show cannot find text
    }
    search_text = g_strdup (text);
}