#include "menus.h"
#include "core_funcs.h"
#include "find_dialog.h"
#include "find_replace_dialog.h"
#include "reg_exp_dialog.h"
#include "go_to_line_dialog.h"
#include "main.h"
#include "python_shell.h"
#include "run_script_dialog.h"

#include <string.h>

//File Menu
/* To create a
 * new file
 */
extern gchar *search_text;
extern GtkWidget *status_bar;
extern ChildProcessData *python_shell_data;
extern GtkWidget *python_shell_win;

void
file_new_activate (GtkWidget *widget)
{
    int current_index = get_current_index ();
    if (current_index != -1)
    {
        GtkWidget *dialog;
        dialog = gtk_message_dialog_new (GTK_WINDOW (window),
                                        GTK_DIALOG_DESTROY_WITH_PARENT, 
                                        GTK_MESSAGE_QUESTION,
                                        GTK_BUTTONS_YES_NO,
                                        "Do you want to create a new file?");
        if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_NO)
        {
            gtk_widget_destroy (dialog);
            return;
        }
        gtk_widget_destroy (dialog);
    }
    
    if (current_index == -1)
        file_new_tab_activate (widget);

    gtk_text_buffer_set_text (gtk_text_view_get_buffer (
                                                 GTK_TEXT_VIEW (code_widget_array [current_index]->sourceview)),
                                                 "", 0);
    g_free (code_widget_array[current_index]->file_path);
    code_widget_array [current_index]->file_mode = FILE_NEW;
}

/*To create a 
 * new tab
*/

void
file_new_tab_activate (GtkWidget *widget)
{
    CodeWidget *codewidget = add_new_code_widget();
    gtk_notebook_append_page (GTK_NOTEBOOK (notebook), 
                                                     code_widget_array [code_widget_array_size-1]->vbox,
                                                     gtk_label_new ("New File"));    
    gtk_widget_show_all (GTK_WIDGET (notebook));
    gtk_notebook_set_current_page (GTK_NOTEBOOK (notebook),
                                  gtk_notebook_get_n_pages(GTK_NOTEBOOK (notebook)) -1);
}

/*To open a 
 * file
*/

void
file_open_activate (GtkWidget *widget)
{
    int index = get_current_index ();

    if (get_current_index () == -1)
        file_new_tab_activate (NULL);

    /* Check if file has modified since last save */    
    GtkWidget *dialog = gtk_file_chooser_dialog_new ("Open File", GTK_WINDOW (window),
                                                     GTK_FILE_CHOOSER_ACTION_OPEN,
                                                     GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                                     GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
                                                     NULL);
    init_file_filters ();
    gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (dialog), py_filter);
    gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (dialog), all_filter);

    if (code_widget_array [index]->file_path)
        gtk_file_chooser_set_filename (GTK_FILE_CHOOSER (dialog),
                                         code_widget_array [index]->file_path);

    if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
    {
        char *filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
        index = get_current_index ();

        /*If filename is already open then set filename's page as current page*/
        for (index = 0; index < get_total_pages(); index++)
        {
            if (code_widget_array [index]->file_path && 
                strcmp(filename, code_widget_array [index]->file_path) == 0)
                 break;
        }
       
        if (index != get_total_pages())
        {
            gtk_notebook_set_current_page (GTK_NOTEBOOK (notebook), index);
            gtk_widget_destroy (dialog);
            return;
        }

        index = get_current_index ();

        if (code_widget_array [index]->file_path != NULL ||
           strcmp (get_text_at_index (index), "") != 0 )
            file_new_tab_activate (NULL);

        index = get_current_index ();

        if (!open_file_at_index (filename, index))
        {
            //Error Cannot open
            show_error_message_dialog ("Cannot open file!");
            gtk_widget_destroy (dialog);
            return;
        }
        
        while (gtk_events_pending ())
            gtk_main_iteration_do (FALSE);

        gtk_statusbar_push (GTK_STATUSBAR (status_bar), 0, "File Loaded");

        gchar *uri = g_filename_to_uri (filename, NULL, NULL);
        if (!uri)
        {
            gtk_statusbar_push (GTK_STATUSBAR (status_bar), 0, 
                               "File Loaded but cannot get its URI");
            gtk_widget_destroy (dialog);
            return;
        }

        GtkRecentManager *recent_manager = gtk_recent_manager_get_default ();
        if (!gtk_recent_manager_add_item (recent_manager, uri))
            gtk_statusbar_push (GTK_STATUSBAR (status_bar), 0,
                                "File Loaded but cannot add it to recent files");
        
        g_free (uri);
    }
    gtk_widget_destroy (dialog);
}

/* Called, when item is selected
 * from recent chooser menu
 */
void
file_recent_menu_selection_done (GtkMenuShell *menu_shell, GtkWidget *item)
{
    gchar *uri = gtk_recent_chooser_get_current_uri (GTK_RECENT_CHOOSER (menu_shell));
    gchar *filename;
    if (!uri)
        return;
    
    filename = g_filename_from_uri (uri, NULL, NULL);
    if (!filename)
        return;

    int index = get_current_index ();
    if (index == -1)
        file_new_tab_activate (NULL);
    
    index = get_current_index ();
    if (code_widget_array [index]->file_path != NULL ||
       strcmp (get_text_at_index (index), "") != 0 )
       file_new_tab_activate (NULL);

    index = get_current_index ();

    if (!open_file_at_index (filename, index))
    {
        //Error Cannot open
        show_error_message_dialog ("Cannot open file!");
        g_free (filename);
        g_free (uri);
        return;
    }

    g_free (filename);
    g_free (uri);
}

/* To save
 * a file
 */
void
file_save_activate (GtkWidget *widget)
{
    if (get_current_index () == -1)
        return;
    
    gchar *contents = get_text_at_index (get_current_index ());

    if (code_widget_array [get_current_index ()]->file_mode == FILE_NEW)
    {
        file_save_as_activate (widget);
    }
    else
    {
        GtkTextIter end_iter;
        gtk_text_buffer_get_end_iter (GTK_TEXT_BUFFER (code_widget_array[get_current_index ()]->sourcebuffer),
                                     &end_iter);
        if (!set_file_data (code_widget_array[get_current_index ()]->file_path,
                            contents, gtk_text_iter_get_offset (&end_iter)))
        {
            //Error cannot save
            show_error_message_dialog ("Cannot save file!");
            return;
        }
        codewidget_update_class_funcs (code_widget_array[get_current_index ()]);
        gtk_statusbar_push (GTK_STATUSBAR (status_bar), 0, "File Saved");
    }
}

/* To save as
 * a file
 */

void
file_save_as_activate (GtkWidget *widget)
{
    int index = get_current_index();
    if (index == -1)
        return;
        
    GtkWidget *dialog = gtk_file_chooser_dialog_new ("Save File", GTK_WINDOW (window),
                                                     GTK_FILE_CHOOSER_ACTION_SAVE,
                                                     GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                                     GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
                                                     NULL);
    init_file_filters ();
    gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (dialog), py_filter);
    gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (dialog), all_filter);
    
    if (code_widget_array [index]->file_path)
        gtk_file_chooser_set_filename (GTK_FILE_CHOOSER (dialog),
                                         code_widget_array [index]->file_path);

    if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
    {
        char *file_path = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));

        code_widget_array [get_current_index ()]->file_path = g_strdup (file_path);
        
        gchar *contents = get_text_at_index (get_current_index ());
        GtkTextIter end_iter;
        gtk_text_buffer_get_end_iter (GTK_TEXT_BUFFER (code_widget_array[get_current_index ()]->sourcebuffer),
                                     &end_iter);
        if (!set_file_data (code_widget_array[get_current_index ()]->file_path,
                            contents, gtk_text_iter_get_offset (&end_iter)))
        {
            //Error cannot save
            show_error_message_dialog ("Cannot save to file!");
            return;
        }
        gtk_statusbar_push (GTK_STATUSBAR (status_bar), 0, "File Saved");           
        code_widget_array [get_current_index ()]->file_mode = FILE_EXISTS;

        char *file_name = strrchr (file_path, '/');
        file_name++;
        gtk_notebook_set_tab_label_text (GTK_NOTEBOOK (notebook),
                                        code_widget_array [get_current_index ()]->vbox, file_name);
        
        gchar *uri = g_filename_to_uri (file_path, NULL, NULL);
        if (!uri)
        {
            gtk_statusbar_push (GTK_STATUSBAR (status_bar), 0, "File Saved but cannot get its URI");
            gtk_widget_destroy (dialog);
            return;
        }

        GtkRecentManager *recent_manager = gtk_recent_manager_get_default ();
        if (!gtk_recent_manager_add_item (recent_manager, uri))
            gtk_statusbar_push (GTK_STATUSBAR (status_bar), 0, "File Saved but cannot add it to recent files");
        
        codewidget_update_class_funcs (code_widget_array[get_current_index ()]);
        gtk_statusbar_push (GTK_STATUSBAR (status_bar), 0, "File Saved");
        g_free (file_path);
    }    
    gtk_widget_destroy (dialog);
}

/* To save
 * file's copy
 */
void
file_save_copy_as_activate (GtkWidget *widget)
{
    if (get_current_index () == -1)
        return;
        
    GtkWidget *dialog = gtk_file_chooser_dialog_new ("Save File", GTK_WINDOW (window),
                                                     GTK_FILE_CHOOSER_ACTION_SAVE,
                                                     GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                                     GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
                                                     NULL);
    init_file_filters ();
    gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (dialog), py_filter);
    gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (dialog), all_filter);
    
    if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
    {
        char *filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
        
        gchar *contents = get_text_at_index (get_current_index ());
        GtkTextIter end_iter;
        gtk_text_buffer_get_end_iter (GTK_TEXT_BUFFER (code_widget_array[get_current_index ()]->sourcebuffer),
                                     &end_iter);
        if (!set_file_data (code_widget_array[get_current_index ()]->file_path,
                            contents, gtk_text_iter_get_offset (&end_iter)))
        {
            //Message Box Error
        }
        g_free (filename);
    }    
    gtk_widget_destroy (dialog);
}

/* To save 
 * all files
 */
void
file_save_all_activate (GtkWidget *widget)
{
    int current_index = get_current_index ();
    if (current_index == -1)
        return;
        
    int i;
    for (i = 0; i<get_total_pages(); i++)
    {
        gtk_notebook_set_current_page (GTK_NOTEBOOK (notebook), i);
        file_save_activate (widget);
    }
    gtk_notebook_set_current_page (GTK_NOTEBOOK (notebook), current_index);
}

/* To close
 * all tabs
 */
void
file_close_all_tabs_activate (GtkWidget *widget)
{
    if (get_total_pages () == 0)
        return;
    int i;
    GtkWidget *dialog;
    dialog = gtk_message_dialog_new (GTK_WINDOW (window),
                                    GTK_DIALOG_DESTROY_WITH_PARENT, 
                                    GTK_MESSAGE_QUESTION,
                                    GTK_BUTTONS_YES_NO,
                                    "Do you want to close all tabs?");
    if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_NO)
    {
        gtk_widget_destroy (dialog);
        return;
    }
    gtk_widget_destroy (dialog);
    
    for (i = get_total_pages ()-1; i>=0; i--)
    {
        gtk_notebook_remove_page (GTK_NOTEBOOK (notebook), i);
    }
    remove_all_code_widgets ();
    free_code_widget_array ();    
}

/* To close 
 * one tab
 */

void
file_close_tab_activate (GtkWidget *widget)
{
    int current_index = get_current_index ();
    if (current_index == -1)
        return;
        
    if (is_file_modified (current_index))
    {
        GtkWidget *dialog;
        dialog = gtk_message_dialog_new (GTK_WINDOW (window),
                                        GTK_DIALOG_DESTROY_WITH_PARENT, 
                                        GTK_MESSAGE_QUESTION,
                                        GTK_BUTTONS_YES_NO,
                                        "Current file is modified, since last saved. Do you still want to close?");
        if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_NO)
        {
            gtk_widget_destroy (dialog);
            return;
        }
        gtk_widget_destroy (dialog);
    }
    
    gtk_notebook_remove_page (GTK_NOTEBOOK (notebook), current_index);
    remove_code_widget_at (current_index);
}

/* To quit
 * program
 */

void
file_quit_activate (GtkWidget *widget)
{
    gtk_widget_destroy (GTK_WIDGET (window));
}

//Edit Menu
/* To
 * undo
 */
void
edit_undo_activate (GtkWidget *widget)
{
    if (get_current_index () == -1) 
        return;
        
    GtkSourceBuffer *buffer = code_widget_array [get_current_index ()]->sourcebuffer;
    if (gtk_source_buffer_can_undo (buffer))
        gtk_source_buffer_undo (buffer);
}

/* To
 * redo
 */
void
edit_redo_activate (GtkWidget *widget)
{
    if (get_current_index () == -1) 
        return;
        
    GtkSourceBuffer *buffer = code_widget_array [get_current_index ()]->sourcebuffer;
    if (gtk_source_buffer_can_redo (buffer))
        gtk_source_buffer_redo (buffer);
}

/* To 
 * copy
 */
void
edit_copy_activate (GtkWidget *widget)
{
    if (get_current_index () == -1) 
        return;
    
    GtkTextBuffer *buffer = GTK_TEXT_BUFFER (code_widget_array [get_current_index ()]->sourcebuffer);
    GtkClipboard *clipboard = gtk_clipboard_get (GDK_NONE);
    gtk_text_buffer_copy_clipboard (buffer, clipboard);
}

/* To cut
 *
 */
void
edit_cut_activate (GtkWidget *widget)
{
    if (get_current_index () == -1) 
        return;
    
    GtkTextBuffer *buffer = GTK_TEXT_BUFFER (code_widget_array [get_current_index ()]->sourcebuffer);
    GtkClipboard *clipboard = gtk_clipboard_get (GDK_NONE);
    gtk_text_buffer_cut_clipboard (buffer, clipboard, TRUE);
}

/* To 
 * paste
 */
void
edit_paste_activate (GtkWidget *widget)
{
    if (get_current_index () == -1) 
        return;
    
    GtkTextBuffer *buffer = GTK_TEXT_BUFFER (code_widget_array [get_current_index ()]->sourcebuffer);
    GtkClipboard *clipboard = gtk_clipboard_get (GDK_NONE);
    gtk_text_buffer_paste_clipboard (buffer, clipboard, NULL, TRUE);
}

/* To 
 * select all
 */
void
edit_select_all_activate (GtkWidget *widget)
{
    if (get_current_index () == -1) 
        return;
    
    GtkTextBuffer *buffer = GTK_TEXT_BUFFER (code_widget_array [get_current_index ()]->sourcebuffer);
    GtkTextIter start_iter, end_iter;
    gtk_text_buffer_get_start_iter (buffer, &start_iter);
    gtk_text_buffer_get_end_iter (buffer, &end_iter);
    gtk_text_buffer_select_range (buffer, &start_iter, &end_iter);
}

/* To
 * delete selection
 */
void
edit_delete_activate (GtkWidget *widget)
{
    if (get_current_index () == -1) 
        return;
    
    GtkTextBuffer *buffer = GTK_TEXT_BUFFER (code_widget_array [get_current_index ()]->sourcebuffer);
    gtk_text_buffer_delete_selection (buffer, TRUE, TRUE);
}

/* To select the current function
 * in which the cursor is located
 */
void
edit_select_function_activate (GtkWidget *widget) 
{
    GtkTextBuffer *buffer = GTK_TEXT_BUFFER (code_widget_array [get_current_index ()]->sourcebuffer);
    int func = gtk_combo_box_get_active (GTK_COMBO_BOX (code_widget_array [get_current_index ()]->func_combobox));

    if (func == -1)
        return;

    int class = gtk_combo_box_get_active (GTK_COMBO_BOX (code_widget_array [get_current_index ()]->class_combobox));
    gdouble start_pos = code_widget_array [get_current_index ()]->py_class_array [class]->py_func_array [func]->pos;
    gdouble end_pos;
    GtkTextIter start_iter, end_iter;

    if (code_widget_array [get_current_index ()]->py_class_array [class]->py_func_array [func+1] != NULL)
        end_pos = code_widget_array [get_current_index ()]->py_class_array [class]->py_func_array [func+1]->pos;
    else if (class + 1 != code_widget_array [get_current_index ()]->py_class_array_size)
        end_pos = code_widget_array [get_current_index ()]->py_class_array [class+1]->pos;
    else
        end_pos = gtk_text_buffer_get_char_count (buffer);
    
    end_pos--;
    
    gtk_text_buffer_get_iter_at_offset (buffer, &start_iter, start_pos);
    gtk_text_buffer_get_iter_at_offset (buffer, &end_iter, end_pos);
    gtk_text_buffer_select_range (buffer, &start_iter, &end_iter);
}

/*To select the current block
 * in which cursor is located
 */
void
edit_select_block_activate (GtkWidget *widget)
{
    GtkTextBuffer *buffer = GTK_TEXT_BUFFER (code_widget_array [get_current_index ()]->sourcebuffer);
    GtkTextIter current_iter, start_iter, end_iter;
    gtk_text_buffer_get_iter_at_mark (buffer, &current_iter, gtk_text_buffer_get_insert (buffer));
    int line;
    int start_line = gtk_text_iter_get_line (&current_iter);
    
    for (line = start_line; line >=0; line--)
    {
        gchar *text = gtk_text_buffer_get_line_text (buffer, line, TRUE);
        if (strrchr (text, ':'))
            /*Found the block's start*/
            break;
    }
    if (line == -1)
    {
        gtk_statusbar_push (GTK_STATUSBAR (status_bar), 0,
                            "Cannot Block's Start");
        return;
    }
    
    gtk_text_buffer_get_iter_at_line (buffer, &start_iter, line);

    for (line = start_line; line < gtk_text_buffer_get_line_count (buffer);
        line ++)
    {
        gchar *text = gtk_text_buffer_get_line_text (buffer, line, TRUE);
        if (strrchr (text, ':'))
            /*Found the block's end*/
            break;
    }
    if (line == gtk_text_buffer_get_line_count (buffer))
    {
        gtk_statusbar_push (GTK_STATUSBAR (status_bar), 0,
                           "Cannot Block's End");
        return;
    }
    gtk_text_buffer_get_iter_at_line (buffer, &end_iter, line);
    gtk_text_buffer_select_range (buffer, &start_iter, &end_iter);
}

/*To fold all
 * functions
 */
void
edit_fold_all_func_activate (GtkWidget *widget)
{
    int i;
    CodeFoldingWidget *code_folding_widget = code_widget_array [get_current_index ()]->code_folding_widget;
    GtkTextBuffer *buffer = GTK_TEXT_BUFFER (code_widget_array [get_current_index ()]->sourcebuffer);

    for (i = 0; i < code_widget_array [get_current_index ()]->py_class_array_size; i++)
    {
        PyFunc **p = code_widget_array [get_current_index ()]->py_class_array [i]->py_func_array;
        
        if (!p)
             continue;
        
        while (*p)
        {
            GtkTextIter start_iter, end_iter;

            /*Get block starting information*/
            gtk_text_buffer_get_iter_at_offset (buffer, &start_iter, (*p)->pos);
            gtk_text_buffer_get_iter_at_line (buffer, &start_iter,
                                             gtk_text_iter_get_line (&start_iter)+1);

            /*Get block ending information*/
            if (*(p+1))
                gtk_text_buffer_get_iter_at_offset (buffer, &end_iter, (*(p+1))->pos);
             else if (i + 1 != code_widget_array [get_current_index ()]->py_class_array_size)
                gtk_text_buffer_get_iter_at_offset (buffer, &end_iter, 
                                                    code_widget_array [get_current_index ()]->py_class_array [i + 1]->pos);
            else
                gtk_text_buffer_get_end_iter (buffer, &end_iter);

            gtk_text_buffer_get_iter_at_line (buffer, &end_iter, gtk_text_iter_get_line (&end_iter) - 1);
            
            /*Function is already folded do nothing*/
            if (code_folding_widget_invisible_text_info_exists (code_folding_widget,
                                                               gtk_text_iter_get_line (&start_iter),
                                                               gtk_text_iter_get_line (&end_iter))
               == -1)
            {
                code_folding_widget_append_to_invisible_text_info_array (code_folding_widget,
                                                                                              gtk_text_iter_get_line (&start_iter),
                                                                                              gtk_text_iter_get_line (&end_iter));
                gtk_text_buffer_apply_tag (buffer,
                                          code_widget_array [get_current_index ()]->invisible_tag,
                                          &start_iter, &end_iter);
            }
            
            p++;
        }
    }
}

/* To unfold all
 * functions
 */
void
edit_unfold_all_func_activate (GtkWidget *widget)
{
    int i;
    CodeFoldingWidget *code_folding_widget = code_widget_array [get_current_index ()]->code_folding_widget;
    GtkTextBuffer *buffer = GTK_TEXT_BUFFER (code_widget_array [get_current_index ()]->sourcebuffer);

    for (i = 0; i < code_widget_array [get_current_index ()]->py_class_array_size; i++)
    {
        PyFunc **p = code_widget_array [get_current_index ()]->py_class_array [i]->py_func_array;
        
        if (!p)
             continue;
        
        while (*p)
        {
            GtkTextIter start_iter, end_iter;

            /*Get block starting information*/
            gtk_text_buffer_get_iter_at_offset (buffer, &start_iter, (*p)->pos);
            gtk_text_buffer_get_iter_at_line (buffer, &start_iter,
                                             gtk_text_iter_get_line (&start_iter)+1);

            /*Get block ending information*/
            if (*(p+1))
                gtk_text_buffer_get_iter_at_offset (buffer, &end_iter, (*(p+1))->pos);
             else if (i + 1 != code_widget_array [get_current_index ()]->py_class_array_size)
                gtk_text_buffer_get_iter_at_offset (buffer, &end_iter, 
                                                    code_widget_array [get_current_index ()]->py_class_array [i + 1]->pos);
            else
                gtk_text_buffer_get_end_iter (buffer, &end_iter);

            gtk_text_buffer_get_iter_at_line (buffer, &end_iter, gtk_text_iter_get_line (&end_iter) - 1);
            
            /*Function is already unfolded do nothing*/
            int j;
            if (( j = code_folding_widget_invisible_text_info_exists (code_folding_widget,
                                                               gtk_text_iter_get_line (&start_iter),
                                                               gtk_text_iter_get_line (&end_iter)))
               != -1)
            {
                code_folding_widget_invisible_text_info_array_remove (code_folding_widget, j);
                gtk_text_buffer_remove_tag (buffer,
                                          code_widget_array [get_current_index ()]->invisible_tag,
                                          &start_iter, &end_iter);
            }
            
            p++;
        }
    }
}

/*To fold current
 * function
 */
void
edit_fold_current_func_activate (GtkWidget *widget)
{
    GtkTextBuffer *buffer = GTK_TEXT_BUFFER (code_widget_array [get_current_index ()]->sourcebuffer);
    int func = gtk_combo_box_get_active (GTK_COMBO_BOX (code_widget_array [get_current_index ()]->func_combobox));

    if (func == -1)
        return;
    
    CodeFoldingWidget *code_folding_widget = code_widget_array [get_current_index ()]->code_folding_widget;
    GtkTextIter start_iter, end_iter;
    int class = gtk_combo_box_get_active (GTK_COMBO_BOX (code_widget_array [get_current_index ()]->class_combobox));
    gdouble end_pos;

    gtk_text_buffer_get_iter_at_offset (buffer, &start_iter,
                                       code_widget_array [get_current_index ()]->py_class_array [class]->py_func_array [func]->pos);
    gtk_text_buffer_get_iter_at_line (buffer, &start_iter,
                                             gtk_text_iter_get_line (&start_iter)+1);

    if (code_widget_array [get_current_index ()]->py_class_array [class]->py_func_array [func+1] != NULL)
        gtk_text_buffer_get_iter_at_offset (buffer, &end_iter,
                                           code_widget_array [get_current_index ()]->py_class_array [class]->py_func_array [func+1]->pos);
    else if (class + 1 != code_widget_array [get_current_index ()]->py_class_array_size)
        gtk_text_buffer_get_iter_at_offset (buffer, &end_iter, 
                                           code_widget_array [get_current_index ()]->py_class_array [class+1]->pos);
    else
       gtk_text_buffer_get_iter_at_offset (buffer, &end_iter,
                                           gtk_text_buffer_get_char_count (buffer) -1);

    gtk_text_buffer_get_iter_at_line (buffer, &end_iter, gtk_text_iter_get_line (&end_iter) - 1);

     /*Function is already folded do nothing*/
    if (code_folding_widget_invisible_text_info_exists (code_folding_widget,
                                                       gtk_text_iter_get_line (&start_iter),
                                                       gtk_text_iter_get_line (&end_iter))
       == -1)
    {
        code_folding_widget_append_to_invisible_text_info_array (code_folding_widget,
                                                                                      gtk_text_iter_get_line (&start_iter),
                                                                                      gtk_text_iter_get_line (&end_iter));
        gtk_text_buffer_apply_tag (buffer,
                                  code_widget_array [get_current_index ()]->invisible_tag,
                                  &start_iter, &end_iter);
    }
}

void
edit_autocomplete_activate (GtkWidget *widget)
{
    
}

/* To get matching 
 * paranthesis
 */
void
edit_matching_paranthesis_activate (GtkWidget *widget)
{
    gint pos, match_pos;
    GtkTextBuffer *buffer = GTK_TEXT_BUFFER (code_widget_array [get_current_index ()]->sourcebuffer);
    GtkTextIter iter, iter2;
    gchar *line_text;
    gtk_text_buffer_get_iter_at_mark (buffer, &iter, gtk_text_buffer_get_insert (buffer));
    pos = gtk_text_iter_get_offset (&iter);
    line_text = gtk_text_buffer_get_line_text (buffer, gtk_text_iter_get_line (&iter), TRUE);    
    match_pos = gtk_text_buffer_get_matching_parethesis_pos (buffer, pos, line_text [pos]);
    if (match_pos == -1)
    {
        gtk_statusbar_push (GTK_STATUSBAR (status_bar), 0, 
                           "Cannot Find Matching Parenthesis");
        g_free (line_text);
        return;
    }

    gtk_text_buffer_get_iter_at_offset (buffer, &iter2, match_pos);
    if (match_pos > pos)
        gtk_text_buffer_select_range (buffer, &iter, &iter2);
    else
       gtk_text_buffer_select_range (buffer, &iter2, &iter);
    
    g_free (line_text);
}

//Format Menu
/* To increase
 * indent of selection
 */
void
format_inc_indent_activate (GtkWidget *widget)
{
    if (get_current_index () == -1) 
        return;
        
    GtkTextIter start_iter, end_iter, iter;
    GtkTextBuffer *buffer = GTK_TEXT_BUFFER (code_widget_array [get_current_index ()]->sourcebuffer);
    if (!gtk_text_buffer_get_selection_bounds (buffer, &start_iter, &end_iter))
        return;
        
    int line;
    int first_line = gtk_text_iter_get_line (&start_iter);
    int last_line = gtk_text_iter_get_line (&end_iter);
    
    for (line =  first_line; line <= last_line; line++)
    {
        gtk_text_buffer_get_iter_at_line (buffer, &iter, line);
        gtk_text_buffer_insert (buffer, &iter, indent_width_str, -1);
    }
}

/* To decrease
 * the indentation of selection
 */
void
format_dec_indent_activate (GtkWidget *widget)
{
    if (get_current_index () == -1) 
        return;
        
    GtkTextIter start_iter, end_iter, iter, iter1;
    GtkTextBuffer *buffer = GTK_TEXT_BUFFER (code_widget_array [get_current_index ()]->sourcebuffer);
    if (!gtk_text_buffer_get_selection_bounds (buffer, &start_iter, &end_iter))
        return;
        
    int line;
    int first_line = gtk_text_iter_get_line (&start_iter);
    int last_line = gtk_text_iter_get_line (&end_iter);
    
    for (line = first_line; line <= last_line; line ++)
    {
        gtk_text_buffer_get_iter_at_line (buffer, &iter, line);
        gchar *text = gtk_text_buffer_get_line_text (buffer, line, TRUE);
        if (get_indent_spaces_in_string (text) < indent_width)
            continue;

        gtk_text_buffer_get_iter_at_line_offset (buffer, &iter1, line, indent_width);
        text = gtk_text_buffer_get_text (buffer, &iter, &iter1, TRUE);
        if (g_strcmp0 (text, indent_width_str) != 0)
             continue;
        gtk_text_buffer_delete (buffer, &iter, &iter1);
    }
}

/* To comment out
 * the selection
 */
void
format_comment_out_activate (GtkWidget *widget)
{
    if (get_current_index () == -1) 
        return;
        
    GtkTextIter start_iter, end_iter, iter;
    GtkTextBuffer *buffer = GTK_TEXT_BUFFER (code_widget_array [get_current_index ()]->sourcebuffer);
    if (!gtk_text_buffer_get_selection_bounds (buffer, &start_iter, &end_iter))
        return;
        
    int line;
    int first_line = gtk_text_iter_get_line (&start_iter);
    int last_line = gtk_text_iter_get_line (&end_iter);
    
    for (line =  first_line; line <= last_line; line++)
    {
        gtk_text_buffer_get_iter_at_line (buffer, &iter, line);
        gtk_text_buffer_insert (buffer, &iter, comment_out_str, -1);
    }
}

/*To uncomment out
 * the selection
*/
void
format_uncomment_out_activate (GtkWidget *widget)
{
    if (get_current_index () == -1) 
        return;
        
    GtkTextIter start_iter, end_iter, iter, iter1;
    GtkTextBuffer *buffer = GTK_TEXT_BUFFER (code_widget_array [get_current_index ()]->sourcebuffer);
    if (!gtk_text_buffer_get_selection_bounds (buffer, &start_iter, &end_iter))
        return;
        
    int line;
    int first_line = gtk_text_iter_get_line (&start_iter);
    int last_line = gtk_text_iter_get_line (&end_iter);
    
    for (line = first_line; line <= last_line; line ++)
    {
        gtk_text_buffer_get_iter_at_line (buffer, &iter, line);
        gtk_text_buffer_get_line_end_iter (buffer, &iter1, line);
        if (gtk_text_iter_get_line_offset (&iter1) < strlen (comment_out_str))
             continue;

        gtk_text_buffer_get_iter_at_line_offset (buffer, &iter1, line, strlen (comment_out_str));
        gchar *text = gtk_text_buffer_get_text (buffer, &iter, &iter1, TRUE);
        if (g_strcmp0 (text, comment_out_str) != 0)
             continue;
        gtk_text_buffer_delete (buffer, &iter, &iter1);
    }
}

/* To convert
 * spaces to tabs
 */
void
format_tabify_region_activate (GtkWidget *widget)
{
    if (get_current_index () == -1) 
        return;
        
    GtkTextIter start_iter, end_iter, iter, iter1;
    GtkTextBuffer *buffer = GTK_TEXT_BUFFER (code_widget_array [get_current_index ()]->sourcebuffer);
    if (!gtk_text_buffer_get_selection_bounds (buffer, &start_iter, &end_iter))
        return;
        
    int line;
    int first_line = gtk_text_iter_get_line (&start_iter);
    int last_line = gtk_text_iter_get_line (&end_iter);
    for (line = first_line; line <= last_line; line ++)
    {
        gchar* line_text = gtk_text_buffer_get_line_text (buffer, line, TRUE);
        int count_spaces = get_indent_spaces_in_string (line_text);

        gtk_text_buffer_get_iter_at_line (buffer, &iter, line);
        gtk_text_buffer_get_iter_at_line_offset (buffer, &iter1, line, count_spaces);
        gtk_text_buffer_select_range (buffer, &iter, &iter1);
        gtk_text_buffer_delete_selection (buffer, TRUE, TRUE);
        gtk_text_buffer_get_iter_at_line (buffer, &iter, line);
        
        int i;
        for (i = 0; i <count_spaces/tab_width; i++)
            gtk_text_buffer_insert (buffer, &iter, "\t", -1);
    }
}

/* To convert
 * tabs to spaces
 */
void
format_untabify_region_activate (GtkWidget *widget)
{
    if (get_current_index () == -1) 
        return;
        
    GtkTextIter start_iter, end_iter, iter, iter1;
    GtkTextBuffer *buffer = GTK_TEXT_BUFFER (code_widget_array [get_current_index ()]->sourcebuffer);
    if (!gtk_text_buffer_get_selection_bounds (buffer, &start_iter, &end_iter))
        return;
        
    int line;
    int first_line = gtk_text_iter_get_line (&start_iter);
    int last_line = gtk_text_iter_get_line (&end_iter);
    for (line = first_line; line <= last_line; line ++)
    {
        int i, count_tabs = 0;
        gchar* line_text = gtk_text_buffer_get_line_text (buffer, line, TRUE);
        
        for (i = 0; i< strlen (line_text); i++)
        {
            if (line_text[i] == '\t')
                count_tabs ++;
            else
                break;
        }

        gtk_text_buffer_get_iter_at_line (buffer, &iter, line);
        gtk_text_buffer_get_iter_at_line_offset (buffer, &iter1, line, count_tabs);
        gtk_text_buffer_select_range (buffer, &iter, &iter1);
        gtk_text_buffer_delete_selection (buffer, TRUE, TRUE);
        gtk_text_buffer_get_iter_at_line (buffer, &iter, line);
       
        for (i = 0; i <count_tabs; i++)
            gtk_text_buffer_insert (buffer, &iter, tab_width_str, -1);
    }
}

/* To uppercase
 * all chars in selection
 */
void
format_uppercase_activate (GtkWidget *widget)
{
    if (get_current_index () == -1) 
        return;
        
    GtkTextBuffer *buffer = GTK_TEXT_BUFFER (code_widget_array [get_current_index ()]->sourcebuffer);
    gchar *selected_text = gtk_text_buffer_get_selected_text (buffer);
    gtk_text_buffer_delete_selection (buffer, TRUE, TRUE);
    gchar *up_selected_text = g_utf8_strup (selected_text, -1);
    gtk_text_buffer_insert_at_cursor (buffer, up_selected_text, -1);
}

/* To lowercase
 * all chars in selection
 */
void
format_lowercase_activate (GtkWidget *widget)
{
    if (get_current_index () == -1) 
        return;
        
    GtkTextBuffer *buffer = GTK_TEXT_BUFFER (code_widget_array [get_current_index ()]->sourcebuffer);
    
    gchar *selected_text = gtk_text_buffer_get_selected_text (buffer);
    gtk_text_buffer_delete_selection (buffer, TRUE, TRUE);
    gchar *up_selected_text = g_utf8_strdown (selected_text, -1);
    gtk_text_buffer_insert_at_cursor (buffer, up_selected_text, -1);
}

/* To strip all the 
 * trailing spaces
 */
void
format_strip_spaces_activate (GtkWidget *widget)
{
    
}


//Search Menu
/* Open Find 
 * Dialog
 */
void
search_find_activate (GtkWidget *widget)
{    
    load_find_dialog (GTK_TEXT_VIEW (code_widget_array [get_current_index ()]->sourceview));    
}

/* To Find Next
 * word
 */
void
search_find_next_activate (GtkWidget *widget)
{
    if (!find_next (GTK_TEXT_VIEW (code_widget_array [get_current_index ()]->sourceview),
              search_text, GTK_TEXT_SEARCH_TEXT_ONLY))
    {
        gtk_statusbar_push (GTK_STATUSBAR (status_bar), 0, "Cannot Find Text");
    }
}

/*To Find Previous
 * word
 */
void
search_find_prev_activate (GtkWidget *widget)
{
    if (!find_previous (GTK_TEXT_VIEW (code_widget_array [get_current_index ()]->sourceview),
              search_text, GTK_TEXT_SEARCH_TEXT_ONLY))
    {
        gtk_statusbar_push (GTK_STATUSBAR (status_bar), 0, "Cannot Find Text");
    }    
}

/* To Replace
 * Text
 */
void
search_find_replace_activate  (GtkWidget *widget)
{
    load_find_replace (GTK_TEXT_VIEW (code_widget_array [get_current_index ()]->sourceview));
}

/* To Find
 * Selected Text
 */
void
search_find_selected_activate  (GtkWidget *widget)
{
    GtkTextBuffer *buffer = GTK_TEXT_BUFFER (code_widget_array [get_current_index ()]->sourcebuffer);
    g_free (search_text);
    search_text = gtk_text_buffer_get_selected_text (buffer);
    find_next (GTK_TEXT_VIEW (code_widget_array [get_current_index ()]->sourceview),
              search_text, GTK_TEXT_SEARCH_TEXT_ONLY);
    
}

void
search_find_in_text_activate  (GtkWidget *widget)
{
    //Later on
}

void
search_find_file_activate  (GtkWidget *widget)
{
    //Later On
}

void
search_replace_file_activate  (GtkWidget *widget)
{
    //Later On
}

/* To search with
 * regular expression
 */
void
search_reg_exp_activate  (GtkWidget *widget)
{
    load_reg_exp_dialog (GTK_TEXT_VIEW (code_widget_array [get_current_index ()]->sourceview));
}


//Navigation Menu
/* Go to previous line
 * in line_history
 */
void
navigate_back_activate (GtkWidget *widget)
{
    GtkTextIter iter;
    CodeWidget *codewidget = code_widget_array [get_current_index ()];
    GtkTextBuffer *buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (codewidget->sourceview));
    gtk_text_buffer_get_iter_at_mark (buffer, &iter, gtk_text_buffer_get_insert (buffer));
    int line = gtk_text_iter_get_line (&iter);
    
    /*Search for the line in line_history*/
    int i = -1;
    
    while (++i < 5 && codewidget->line_history [i] != line);
    
    if (i == 5)
        return;
    
    if (i==0)
        i=4;
    else
       i--;
    
    go_to_line (GTK_TEXT_VIEW (codewidget->sourceview), codewidget->line_history [i]);
}

/* Go to next line
 * in line_history
 */
void
navigate_forward_activate (GtkWidget *widget)
{
    GtkTextIter iter;
    CodeWidget *codewidget = code_widget_array [get_current_index ()];
    GtkTextBuffer *buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (codewidget->sourceview));
    gtk_text_buffer_get_iter_at_mark (buffer, &iter, gtk_text_buffer_get_insert (buffer));
    int line = gtk_text_iter_get_line (&iter);
    
    /*Search for the line in line_history*/
    int i = -1;
    
    while (++i < 5 && codewidget->line_history [i] != line);
    
    if (i == 5)
        return;
    
    if (i==4)
        i=0;
    else
       i++;
    
    go_to_line (GTK_TEXT_VIEW (codewidget->sourceview), codewidget->line_history [i]);
}

/* Go to first line
 * in file
 */
void
navigate_first_line_activate  (GtkWidget *widget)
{
    go_to_line (GTK_TEXT_VIEW (code_widget_array [get_current_index ()]->sourceview), 0);
}

/* Go to last line
 * in file
 */
void
navigate_last_line_activate  (GtkWidget *widget)
{
    GtkTextBuffer *buffer = GTK_TEXT_BUFFER (code_widget_array [get_current_index ()]->sourcebuffer);
    go_to_line (GTK_TEXT_VIEW (code_widget_array [get_current_index ()]->sourceview),
               gtk_text_buffer_get_line_count (buffer) - 1);
}

/*Called when navigate_line_history
 * is selected
 */
void
navigate_line_history_select  (GtkWidget *widget)
{
    if (get_current_index () == -1)
        return;

    int i; 
    /*Clean line_history_menu if it has childs*/
    if (line_history_item_array)
    {
        for (i = 0; i <5; i++)
        if (line_history_item_array [i])
           gtk_container_remove (GTK_CONTAINER (line_history_menu),
                                line_history_item_array [i]->menu_item);
    }

    clean_line_history_array ();
    init_line_history_array ();
   
    /*Populate line_history_menu*/
    for (i = 0; i < 5; i++)
    {
        if (line_history_item_array [i])        
            gtk_menu_shell_append (GTK_MENU_SHELL (line_history_menu),
                                  line_history_item_array [i]->menu_item);
    }
    gtk_widget_show_all (line_history_menu);
}

/* To add
 * a bookmark
 */
void
naviagate_add_bookmark_activate  (GtkWidget *widget)
{
    GtkTextIter iter;
    GtkTextBuffer *buffer = GTK_TEXT_BUFFER (code_widget_array [get_current_index ()]->sourcebuffer);
    
    gtk_text_buffer_get_iter_at_mark (buffer, &iter,
                                     gtk_text_buffer_get_insert (buffer));
    char *file_name = strrchr (code_widget_array [get_current_index ()]->file_path, '/');
    file_name ++;
    add_bookmark (get_current_index (), gtk_text_iter_get_line (&iter),
                 file_name);
}

/*When Bookmark Menu Item
 * is activated
*/
void
bookmark_item_activate (GtkWidget *widget, gpointer data)
{
    Bookmark *bookmark = (Bookmark *)data;
    gtk_notebook_set_current_page (GTK_NOTEBOOK (notebook), bookmark->index);
    
    go_to_line (GTK_TEXT_VIEW (code_widget_array [get_current_index ()]->sourceview), bookmark->line);
    int i;
    for (i = 0; i<bookmark_array_size; i++)
    {
        if (bookmark_array [i] == bookmark)
        {
            current_bookmark_index = i;
            break;
        }
    }        
}

/*To clear all 
 * bookmarks
*/
void
naviagate_clear_bookmarks_activate  (GtkWidget *widget)
{
    remove_all_bookmarks ();
}

/* To move to
 * previous bookmark
 */
void
navigate_prev_bookmarks_activate  (GtkWidget *widget)
{
    if (current_bookmark_index == 0)
        current_bookmark_index = bookmark_array_size -1;
    else
        current_bookmark_index -=1;
    
    gtk_menu_item_activate (GTK_MENU_ITEM (bookmark_array [current_bookmark_index]));
}

/* To move to
 * next bookmark
 */
void
navigate_next_bookmark_activate  (GtkWidget *widget)
{
    if (current_bookmark_index == bookmark_array_size -1)
        current_bookmark_index =  0;
    else
        current_bookmark_index +=1;
    
    gtk_menu_item_activate (GTK_MENU_ITEM (bookmark_array [current_bookmark_index]));
}

void
navigate_bookmarks_activate  (GtkWidget *widget)
{
}
/*To navigate to the begginning
 * of current block
 */
void
navigate_go_to_block_start_activate  (GtkWidget *widget)
{
    GtkTextBuffer *buffer = GTK_TEXT_BUFFER (code_widget_array [get_current_index ()]->sourcebuffer);
    GtkTextIter current_iter;
    gtk_text_buffer_get_iter_at_mark (buffer, &current_iter, gtk_text_buffer_get_insert (buffer));
    int line;
    int start_line = gtk_text_iter_get_line (&current_iter);
    
    for (line = start_line; line >=0; line--)
    {
        gchar *text = gtk_text_buffer_get_line_text (buffer, line, TRUE);
        if (strrchr (text, ':'))
        {
            //Found the block now select the line
            go_to_line (GTK_TEXT_VIEW (code_widget_array [get_current_index ()]->sourceview), line);
            return;
        }
    }
    gtk_statusbar_push (GTK_STATUSBAR (status_bar), 0, "Cannot Block's Start");
}

/*To navigate to the definition
 * of selected function
 */
void
navigate_go_to_func_def_activate  (GtkWidget *widget)
{
    GtkTextBuffer *buffer = GTK_TEXT_BUFFER (code_widget_array [get_current_index ()]->sourcebuffer);
    gchar *selected_text = gtk_text_buffer_get_selected_text (buffer);
    int i;
    for (i = 0; i < code_widget_array [get_current_index ()]->py_class_array_size; i++)
    {
        PyFunc **p = code_widget_array [get_current_index ()]->py_class_array [i]->py_func_array;

        if (!p)
             continue;

        while (*p)
        {
            if (g_strcmp0 (selected_text, ((PyVariable *)(*p))->name) == 0)
            {
                GtkTextIter iter;
                gtk_text_buffer_get_iter_at_offset (buffer, &iter, (*p)->pos);
                go_to_pos_and_select_line (GTK_TEXT_VIEW (code_widget_array [get_current_index ()]->sourceview),
                                          (*p)->pos);
                return;
            }
            p++;
        }
    }
    gtk_statusbar_push (GTK_STATUSBAR (status_bar), 0, "Cannot Find Function");
}

/*To go to entered
 * line
 */
void
navigate_go_to_line_activate  (GtkWidget *widget)
{
    load_go_to_line_dialog (GTK_TEXT_VIEW(code_widget_array [get_current_index ()]->sourceview));
}

//Project Menu
void
project_new_activate (GtkWidget *widget)
{
    
}

void
project_open_activate (GtkWidget *widget)
{
}

void
project_save_activate (GtkWidget *widget)
{
}

void
project_save_as_activate (GtkWidget *widget)
{
}

void
project_save_copy_as_activate (GtkWidget *widget)
{
}

void
project_empty_activate (GtkWidget *widget)
{
}

void
project_close_activate (GtkWidget *widget)
{
}

void
project_preferences_activate (GtkWidget *widget)
{
}

void
project_recent_activate (GtkWidget *widget)
{
}


//Python Shell Menu
void
python_shell_open_activate (GtkWidget *widget)
{
    load_python_shell ();
}

void
python_shell_restart_activate (GtkWidget *widget)
{
    shell_restart_shell_activate (widget);
}

void
python_shell_close_activate (GtkWidget *widget)
{
    extern GtkWidget *python_shell_win;
    gtk_widget_destroy (python_shell_win);
}


//Run Menu
void
run_run_script_activate (GtkWidget *widget)
{
    load_run_script_dialog (code_widget_array [get_current_index ()]->file_path);
}

void
run_debug_script_activate (GtkWidget *widget)
{
}

void
run_run_project_activate (GtkWidget *widget)
{
}

void
run_debug_project_activate (GtkWidget *widget)
{
}


//Debug Menu
void
debug_open_pdb_shell_activate (GtkWidget *widget)
{
}


//Tools Menu
void
tools_options_activate (GtkWidget *widget)
{
}

void
tools_auto_indent_activate (GtkWidget *widget)
{
}

void
tools_class_browser_activate (GtkWidget *widget)
{
}