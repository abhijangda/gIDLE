#include "core_funcs.h"
#include <gtk/gtk.h>
#include <string.h>
#include <ctype.h>

/* To get
 * current index
*/

int
get_current_index ()
{
    return gtk_notebook_get_current_page (GTK_NOTEBOOK (notebook));
}

/* To get
 * total pages
*/

int
get_total_pages ()
{
    return gtk_notebook_get_n_pages (GTK_NOTEBOOK (notebook));
}

/* To get
 * file data
 */

gchar *
get_file_data (char *file_path)
{
    GFile *file = g_file_new_for_path (file_path);
    GError *error = NULL;
    gchar *buffer;
    gsize readed = -1;
    if (!g_file_load_contents (file, NULL, &buffer, &readed, NULL, NULL))
    {
        g_object_unref (file);
        return NULL;
    }
    g_object_unref (file);
    return buffer;
}

/* Open file
 * at given index
 */
gboolean
open_file_at_index (char *file_path, int index)
{
    gchar *file_str = get_file_data (file_path);
    code_widget_array [index]->file_path = g_strdup (file_path);
    if (!file_str)
        return FALSE;

    codewidget_set_text (code_widget_array [index], file_str);
    code_widget_array [index]->file_mode = FILE_EXISTS;
    char *file_name = strrchr (file_path, '/');
    file_name++;
    gtk_notebook_set_tab_label_text (GTK_NOTEBOOK (notebook),
                                    code_widget_array [index]->vbox, file_name);
    g_free (file_str);
    return TRUE;
}

/*To move cursor
 * and select required line
 */
void
go_to_line (GtkTextView *view, int line)
{
    GtkTextBuffer *buffer = gtk_text_view_get_buffer (view);
    GtkTextIter start_iter, end_iter;
    gtk_text_buffer_get_iter_at_line (buffer, &start_iter, line);
    gtk_text_buffer_get_line_end_iter (buffer, &end_iter, line);
    gtk_text_buffer_select_range (buffer, &start_iter, &end_iter);
    gtk_text_view_scroll_to_iter (view, &start_iter, 0.1, FALSE, 0.5, 0.5);
}

/* To find
 * next
 */
gboolean
find_next (GtkTextView *text_view, const gchar *text, GtkTextSearchFlags flags)
{
    GtkTextIter start_iter, end_iter;
    GtkTextBuffer *buffer = gtk_text_view_get_buffer (text_view);

    gtk_text_buffer_get_iter_at_mark (buffer, &start_iter, 
                                     gtk_text_buffer_get_insert (buffer));
    gtk_text_buffer_get_iter_at_offset (buffer, &start_iter, 
                                       gtk_text_iter_get_offset (&start_iter) + 1);

    if (gtk_text_iter_forward_search (&start_iter, text, flags, 
                                       &start_iter, &end_iter, NULL))
    {
        gtk_text_buffer_select_range (buffer, &start_iter, &end_iter);
        gtk_text_view_scroll_to_iter (text_view,  &start_iter,
                                     0.1, FALSE, 0.5, 0.5);
        return TRUE;
    }
    return FALSE;
}

/* To find 
 * previous
 */
gboolean
find_previous (GtkTextView *text_view, const gchar *text, GtkTextSearchFlags flags)
{
    GtkTextIter start_iter, end_iter;
    GtkTextBuffer *buffer = gtk_text_view_get_buffer (text_view);

    gtk_text_buffer_get_iter_at_mark (buffer, &start_iter, 
                                     gtk_text_buffer_get_insert (buffer));
    gtk_text_buffer_get_iter_at_offset (buffer, &start_iter, 
                                       gtk_text_iter_get_offset (&start_iter) - 1);

    if (gtk_text_iter_backward_search (&start_iter, text, flags, 
                                       &start_iter, &end_iter, NULL))
    {
        gtk_text_buffer_select_range (buffer, &start_iter, &end_iter);
        gtk_text_view_scroll_to_iter (text_view,  &start_iter,
                                     0.1, FALSE, 0.5, 0.5);
        return TRUE;
    }
    return FALSE;
}

/* To add a 
 * bookmark
 */
void
add_bookmark (int index, int line, char *file_name)
{
    Bookmark *bookmark = bookmark_new (index, line, file_name);
    bookmark_array = g_realloc (bookmark_array,
                               (bookmark_array_size+1)* sizeof (Bookmark *));
    
    g_signal_connect (G_OBJECT (bookmark->menu_item), "activate",
                     G_CALLBACK (bookmark_item_activate), bookmark);

    gtk_menu_shell_append (GTK_MENU_SHELL (bookmarks_menu), bookmark->menu_item);
    gtk_widget_show_all (bookmarks_menu);
    bookmark_array [bookmark_array_size] = bookmark;
    bookmark_array_size ++;
}

/* To remove
 * all bookmarks
 */
void
remove_all_bookmarks ()
{
    int i;
    for (i = 0; i < bookmark_array_size; i++)
    {
        gtk_container_remove (GTK_CONTAINER (bookmark_menu),
                             bookmark_array[i]->menu_item);
        bookmark_destroy (bookmark_array [i]);
    }
    g_free (bookmark_array);
}

/* To set 
 * file data
*/

gboolean
set_file_data (char *file_path, gchar *file_data, gsize length)
{
    GFile *file = g_file_new_for_path (file_path);
    GError *error = NULL;
    
    if (!g_file_replace_contents (file, file_data, length, NULL, FALSE,
                               G_FILE_CREATE_NONE, NULL, NULL, &error))
    {
        g_object_unref (file);
        g_error_free (error);
        return FALSE;
     }
     g_object_unref (file);
    return TRUE;
}

/*Get selected text
 * in buffer
 */
gchar *
gtk_text_buffer_get_selected_text (GtkTextBuffer *buffer)
{
    GtkTextIter start, end;
    if (!gtk_text_buffer_get_selection_bounds (buffer, &start, &end))
        return NULL;
    return gtk_text_buffer_get_text (buffer, &start, &end, TRUE);
}

/*Returns number of
 *indent spaces in string
 */
int
get_indent_spaces_in_string (char *string)
{
    int i, count = 0;
    for (i = 0; i<strlen (string); i++)
    {
        if (string[i]==' ')
            count ++;
        else
           return count;
    }
}

/*Initializes
 *line_history_array
 */
void
init_line_history_array ()
{
    int *line_history_array = code_widget_array [get_current_index ()]->line_history;
    int i;
    line_history_item_array = (LineHistoryItem **)g_malloc (5 * sizeof (LineHistoryItem *));

    for (i = 0; i <5; i++)
    {
        if (line_history_array [i] != -1){printf ("creating%d\n", i);
            line_history_item_array [i] = line_history_item_new (line_history_array [i]);}
        else
            line_history_item_array [i] = NULL;
    }
}

/*Clean
 * line_history_array
 */
void
clean_line_history_array ()
{
    if (!line_history_item_array)
        return;
    printf ("clean_line_history\n");
    int i;
    for (i = 0; i <5; i++)
    {
        printf ("clean %i\n");
        if (line_history_item_array [i])
            line_history_item_destroy (line_history_item_array [i]);
    }

    g_free (line_history_item_array);
}

/* To get sourceview 
 * contents at index
 */
gchar *
get_text_at_index (int index)
{
    gchar *code_widget_str;
    GtkTextBuffer *text_buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (
                                                                    code_widget_array [index]->sourceview));
    GtkTextIter start_iter, end_iter;
    
    gtk_text_buffer_get_start_iter (text_buffer, &start_iter);
    gtk_text_buffer_get_end_iter (text_buffer, &end_iter);
    
    code_widget_str = gtk_text_buffer_get_text (text_buffer, &start_iter,
                                                                                  &end_iter, TRUE);
    return code_widget_str;
}

/* To check if
 * file has been modified
 */

gboolean
is_file_modified ()
{
    int current_page = get_current_index ();    
    gchar *code_widget_str = get_text_at_index (current_page);
    
    if (code_widget_array [current_page]->file_mode == FILE_NEW)
    {
        if (strcmp (code_widget_str, "") != 0)
            return TRUE;
        return FALSE;
    }
        
    gchar *file_data = get_file_data (code_widget_array [current_page]->file_path);

    if (g_strcmp0 (file_data, code_widget_str)!=0)
    {
        g_free (code_widget_str);
        g_free (file_data);
        return TRUE;
    }
    g_free (code_widget_str);
    g_free (file_data);    
    return FALSE;
}

/* To get GtkTextIter
 * at the end of line
 */

void
gtk_text_buffer_get_line_end_iter (GtkTextBuffer *buffer,  GtkTextIter *iter, int line_index)
{
    if (line_index +1 == gtk_text_buffer_get_line_count (buffer))
        gtk_text_buffer_get_end_iter (buffer, iter);
        
    else
    {
        gtk_text_buffer_get_iter_at_line (buffer, iter, line_index + 1);
        gtk_text_buffer_get_iter_at_offset (buffer, iter, gtk_text_iter_get_offset (iter) - 1);
    }
}

/* To remove c from string
 * returns a newly allocated string
*/
gchar *
remove_char (gchar *string, gchar c)
{
    gchar *p = string;
    while (*p != c && *++p);
    
    --p;
    while (*++p)
         *p = *(p+1);
    
     *--p = '\0';
    return string;
}

/* To get line text of line_index
 * with comments removed
 */

gchar *
gtk_text_buffer_get_line_text (GtkTextBuffer *buffer, int line_index)
{
    GtkTextIter start_iter, end_iter;
    gtk_text_buffer_get_iter_at_line (buffer, &start_iter, line_index);
    gtk_text_buffer_get_line_end_iter (buffer, &end_iter, line_index);
    gchar *text = gtk_text_buffer_get_text (buffer, &start_iter, &end_iter, TRUE);

    char *hash_text = g_strrstr (text, "#");    
    if (!hash_text)
        return text;
    text[hash_text - text] = '\0';
    return text;
}


/* Add new 
 * CodeWidget
 */
CodeWidget *
add_new_code_widget ()
{
    CodeWidget *code_widget = codewidget_new ();
    code_widget_array = g_realloc ((gpointer)code_widget_array, 
                                        (code_widget_array_size+1) * sizeof (CodeWidget *));
    code_widget_array [code_widget_array_size] = code_widget;
    code_widget_array_size++;
    return code_widget;
}

/* To initialize
 * GtkFileFilter
*/

void
init_file_filters ()
{
    py_filter = gtk_file_filter_new ();
    gtk_file_filter_set_name (py_filter, "Python Files");
    gtk_file_filter_add_pattern (py_filter, "*.py");
    
    all_filter = gtk_file_filter_new ();
    gtk_file_filter_set_name (all_filter, "All Files");
    gtk_file_filter_add_pattern (all_filter, "*.*");
    
    pyproj_file = gtk_file_filter_new ();
    gtk_file_filter_set_name (pyproj_file, "Project Files");
    gtk_file_filter_add_pattern (pyproj_file, "*.pyproj");
}

/* Show Error
 * MessageDialog
 */

void
show_error_message_dialog (gchar *message)
{
    GtkWidget *dialog;
    dialog = gtk_message_dialog_new (GTK_WINDOW (window),
                                    GTK_DIALOG_DESTROY_WITH_PARENT, 
                                    GTK_MESSAGE_ERROR,
                                    GTK_BUTTONS_OK,
                                    message);
                                    
    gtk_dialog_run (GTK_DIALOG (dialog));
    gtk_widget_destroy (dialog);
}

/* Remove CodeWidget
 * at index
*/

void
remove_code_widget_at (int index)
{
    int i;
    codewidget_destroy (code_widget_array [index]);

    for (i = index; i < code_widget_array_size-1; i++)
        code_widget_array [i] = code_widget_array [i+1];
    
    code_widget_array = g_realloc (code_widget_array,
                                        (code_widget_array_size-1) * sizeof (CodeWidget *));
    code_widget_array_size--;
}

/* Remove all
 * CodeWidget
*/

void
remove_all_code_widgets ()
{
    int i;
    for (i = 0; i < code_widget_array_size; i++)
        codewidget_destroy (code_widget_array [i]);
    
    free_code_widget_array();
}

/* Free 
 * code_widget_array
*/

void
free_code_widget_array ()
{
    g_free (code_widget_array);
    code_widget_array_size = 0;
}