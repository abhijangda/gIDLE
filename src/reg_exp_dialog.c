#include "reg_exp_dialog.h"
#include "core_funcs.h"

GtkWidget *reg_exp_entry;
GtkTextView *text_view;

/*Load
 * Find Dialog
 */
void
load_reg_exp_dialog (GtkTextView *view)
{
    text_view = view;
    GtkBuilder *reg_exp_builder;
    
    reg_exp_builder = gtk_builder_new ();
    gtk_builder_add_from_file (reg_exp_builder, "./ui/reg_exp_dialog.glade", NULL);
    
    GObject *dialog = gtk_builder_get_object (reg_exp_builder, "reg_exp_dialog");
    gtk_widget_show (GTK_WIDGET (dialog));
    
    reg_exp_entry = GTK_WIDGET (gtk_builder_get_object (reg_exp_builder,
                            "reg_exp_entry"));
    
    g_signal_connect (gtk_builder_get_object (reg_exp_builder, "cmdClose"),
                     "clicked", G_CALLBACK (reg_exp_dialog_cmdClose_clicked), (gpointer)dialog);
    g_signal_connect (gtk_builder_get_object (reg_exp_builder, "cmdSearch"),
                     "clicked", G_CALLBACK (reg_exp_dialog_cmdSearch_clicked), NULL);
}

void
reg_exp_dialog_cmdClose_clicked (GtkWidget *widget, gpointer dialog)
{
    gtk_widget_destroy (GTK_WIDGET (dialog));
}

void
reg_exp_dialog_cmdSearch_clicked (GtkWidget *widget)
{
    static int counter = 0;
    const char *regexp_text = gtk_entry_get_text (GTK_ENTRY (reg_exp_entry));
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
    gchar *full_text;
    GtkTextIter start_iter, end_iter;
    
    gtk_text_buffer_get_start_iter (buffer, &start_iter);
    gtk_text_buffer_get_end_iter (buffer, &end_iter);
    
    full_text = gtk_text_buffer_get_text (buffer, &start_iter,
                                                                &end_iter, TRUE);
    GRegex *regex;
    GMatchInfo *match_info;
    if (!g_regex_match(regex,full_text,0,&match_info))
    {
        //Status bar cannot find
        return;
    }
    //Move to the last matched string
    int i, start = 0, end = 0;
    for(i=0;i<counter;i++)
        g_match_info_next(match_info,NULL);
    
    if (!g_match_info_fetch_pos (match_info,0,&start,&end))
    {
        gtk_text_buffer_get_iter_at_offset(buffer, &start_iter,start);
        gtk_text_buffer_get_iter_at_offset(buffer, &end_iter,end);
        gtk_text_buffer_select_range(buffer,&start_iter,&end_iter);
        counter++;
    }
    else
        counter = 0;
}