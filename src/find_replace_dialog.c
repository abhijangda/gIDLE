#include "find_replace_dialog.h"
#include "core_funcs.h"

GtkTextView *text_view;
GtkWidget *find_entry, *replace_entry, *chkCase;

/* Load Find and Replace
 * Dialog
 */
void
load_find_replace (GtkTextView *view)
{
    text_view = view;
    GtkBuilder *find_replace_builder;
    
    find_replace_builder = gtk_builder_new ();
    gtk_builder_add_from_file (find_replace_builder, "./ui/find_and_replace_dialog.glade", NULL);
    
    GObject *dialog = gtk_builder_get_object (find_replace_builder, "find_replace_dialog");
    gtk_widget_show (GTK_WIDGET (dialog));
    
    find_entry = GTK_WIDGET (gtk_builder_get_object (find_replace_builder,
                            "find_entry"));
    replace_entry = GTK_WIDGET (gtk_builder_get_object (find_replace_builder,
                            "replace_entry"));
    
    chkCase = GTK_WIDGET (gtk_builder_get_object (find_replace_builder,
                         "chkCase"));
    
    g_signal_connect (gtk_builder_get_object (find_replace_builder, "cmdClose"),
                     "clicked", G_CALLBACK (find_replace_cmdClose_clicked), (gpointer)dialog);
    g_signal_connect (gtk_builder_get_object (find_replace_builder, "cmdFind"),
                     "clicked", G_CALLBACK (find_replace_cmdFind_clicked), NULL);
    g_signal_connect (gtk_builder_get_object (find_replace_builder, "cmdReplace"),
                     "clicked", G_CALLBACK (find_replace_cmdReplace_clicked), NULL);
    g_signal_connect (gtk_builder_get_object (find_replace_builder, "cmdReplaceAll"),
                     "clicked", G_CALLBACK (find_replace_cmdReplaceAll_clicked), NULL);
}

/* Find and Replace
 * cmdFind clicked handler
 */
void
find_replace_cmdFind_clicked (GtkWidget *widget)
{
    const gchar *text = gtk_entry_get_text (GTK_ENTRY (find_entry));
    GtkTextSearchFlags flags = GTK_TEXT_SEARCH_TEXT_ONLY;
    
    if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (chkCase)))
        flags = flags | GTK_TEXT_SEARCH_CASE_INSENSITIVE;

    if (!find_next (text_view, text, flags))
    {
        //Status Bar Report
        return;
    }
}

/* Find and Replace
 * cmdReplaceAll clicked handler
 */
void
find_replace_cmdReplaceAll_clicked (GtkWidget *widget)
{
    GtkTextIter start_iter, end_iter;
    GtkTextBuffer *buffer = gtk_text_view_get_buffer (text_view);
    const gchar *text = gtk_entry_get_text (GTK_ENTRY (find_entry));
    
    GtkTextSearchFlags flags = GTK_TEXT_SEARCH_TEXT_ONLY;
    
    if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (chkCase)))
        flags = flags | GTK_TEXT_SEARCH_CASE_INSENSITIVE;
    
    //Find all occurrences 
    while (find_next (text_view, text, flags))
    {
         if (!gtk_text_buffer_get_selection_bounds (buffer,
                                                   &start_iter, &end_iter))
              return;
    
        gtk_text_buffer_delete_selection (buffer, TRUE, TRUE);
        gtk_text_buffer_insert_at_cursor (buffer, //&start_iter, 
                               gtk_entry_get_text (GTK_ENTRY (replace_entry)),
                               -1);
    }
    //Status Bar Replaced All
}

/* Find and Replace
 * cmdReplace clicked handler
 */
void
find_replace_cmdReplace_clicked (GtkWidget *widget)
{
    GtkTextIter start_iter, end_iter;
    GtkTextBuffer *buffer = gtk_text_view_get_buffer (text_view);
    
    //If buffer has selection then replace it otherwise find
    if (!gtk_text_buffer_get_has_selection (buffer))
    {
        const gchar *text = gtk_entry_get_text (GTK_ENTRY (find_entry));
        GtkTextSearchFlags flags = GTK_TEXT_SEARCH_TEXT_ONLY;
        
        if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (chkCase)))
            flags = flags | GTK_TEXT_SEARCH_CASE_INSENSITIVE;
    
        if (!find_next (text_view, text, flags))
        {
            //Status Bar Report
            return;
        }
    }
    if (!gtk_text_buffer_get_selection_bounds (buffer, &start_iter, &end_iter))
         return;
    
    gtk_text_buffer_delete_selection (buffer, TRUE, TRUE);
    gtk_text_buffer_insert_at_cursor (buffer, //&start_iter, 
                           gtk_entry_get_text (GTK_ENTRY (replace_entry)), -1);
}


/* Find and Replace
 * cmdClose clicked handler
 */
void
find_replace_cmdClose_clicked (GtkWidget *widget, gpointer dialog)
{
    gtk_widget_destroy (GTK_WIDGET (dialog));
}