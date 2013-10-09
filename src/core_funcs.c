#include "core_funcs.h"
#include "menus.h"
#include "main.h"
#include "proj_notebook.h"

#include <gtk/gtk.h>
#include <string.h>
#include <ctype.h>

extern GtkWidget *status_bar;
extern FileMonitor *file_monitor;

int
count_str_str (char *str1, int len, char *str2)
{
    char *p = str1;
    int count = 0;
    while ((p = g_strstr_len (p + 1, len, str2)))
    {
        count++;}
    
    return count;
}

gboolean
open_project_from_file (gchar *proj_file)
{
    if (current_project)
    {
        GtkWidget *dialog;
        dialog = gtk_message_dialog_new (GTK_WINDOW (window),
                                        GTK_DIALOG_DESTROY_WITH_PARENT, 
                                        GTK_MESSAGE_QUESTION,
                                        GTK_BUTTONS_YES_NO,
                                        "Do you want to open a new project?");
        if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_NO)
        {
            gtk_widget_destroy (dialog);
            return;
        }
        gtk_widget_destroy (dialog);
        project_destroy (current_project);
        current_project = NULL;

        project_notebook_clear (PROJECT_NOTEBOOK (proj_notebook));
    }

    gchar *file_data = get_file_data (proj_file);
    if (!file_data)
    {
        gtk_statusbar_push (GTK_STATUSBAR (status_bar), 0, "Cannot Open Project.");
        return;
    }
    
    current_project = project_new_from_string (file_data, proj_file);

    if (!current_project)
    {
        g_free (file_data);
        gtk_statusbar_push (GTK_STATUSBAR (status_bar), 0, "Cannot Open Project.");
        return;
    }

    g_free (file_data);

    project_notebook_open_project (PROJECT_NOTEBOOK (proj_notebook), current_project);

    set_mode (GIDLE_MODE_PROJECT);
}

gchar *
g_file_input_stream_read_line (GFileInputStream *istream)
{
    GString *line_str = g_string_new ("");
    const gsize count = 100;
    gchar buffer [count+1];
    gssize readed = -1;

    while ((readed = g_input_stream_read (G_INPUT_STREAM (istream), buffer,
                                                count, NULL, NULL)) == count)
    {
        gchar *new_line_pos = NULL;
        gchar *line_cont_pos = NULL;

        buffer [readed] = '\0';

        if ((new_line_pos = strchr (buffer, '\n')))
        {
            if ((line_cont_pos = strchr (buffer, '\\')) && new_line_pos - line_cont_pos > 0)
            {
                goffset seek_pos = g_seekable_tell (G_SEEKABLE (istream));
                g_seekable_seek (G_SEEKABLE (istream),
                                 seek_pos - strlen (new_line_pos) + 1, G_SEEK_SET,
                                 NULL, NULL);
                buffer [new_line_pos - buffer] = '\0';
                line_str = g_string_append (line_str, buffer);
            }
            else
            {
                goffset seek_pos = g_seekable_tell (G_SEEKABLE (istream));
                g_seekable_seek (G_SEEKABLE (istream),
                                 seek_pos - strlen (new_line_pos) + 1, G_SEEK_SET,
                                 NULL, NULL);

                buffer [new_line_pos - buffer] = '\0';
                line_str = g_string_append (line_str, buffer);
                gchar *line = line_str->str;
                g_string_free (line_str, FALSE);
                return line;
            }
        }
        else
            line_str = g_string_append (line_str, buffer);
    }
    if (readed == 0 || readed == -1)
       return NULL;

    buffer [readed] = '\0';
    line_str = g_string_append (line_str, buffer);
    gchar *line = line_str->str;
    g_string_free (line_str, FALSE);
    return line;
}

/* To get a newly allocated string containing doc string of 
 * Python Functions, Classes or Modules between lines
 * start and end
 */
char *
get_doc_string_between_lines (GtkTextBuffer *buffer, int start, int end)
{
    GString *string = g_string_new ("");
    GtkTextIter start_iter, end_iter;
    gtk_text_buffer_get_iter_at_line (buffer, &start_iter, start);
    gtk_text_buffer_get_line_end_iter (buffer, &end_iter, start);

    gchar *first_line_str = gtk_text_buffer_get_text (buffer, &start_iter, 
                                                      &end_iter, TRUE);
    g_strstrip (first_line_str);

    gchar *pos_triple_quotes = g_strstr_len (first_line_str, -1, "'''");
    if (!pos_triple_quotes)
        pos_triple_quotes = g_strstr_len (first_line_str, -1, "\"\"\"");
    
    if (!pos_triple_quotes)
        return NULL;
    
    pos_triple_quotes += 3;

    g_string_append (string, pos_triple_quotes);
    
    int line;
    
    for (line = start + 1; line <end; line++)
    {
        gtk_text_buffer_get_iter_at_line (buffer, &start_iter, line);
        gtk_text_buffer_get_line_end_iter (buffer, &end_iter, line);

        gchar *line_str = gtk_text_buffer_get_text (buffer, &start_iter, 
                                                    &end_iter, TRUE);
        g_strstrip (line_str);
        g_string_append_c (string, '\n');
        g_string_append (string, line_str);
    }
    
    gtk_text_buffer_get_iter_at_line (buffer, &start_iter, end);
    gtk_text_buffer_get_line_end_iter (buffer, &end_iter, end);

    gchar *end_line_str = gtk_text_buffer_get_text (buffer, &start_iter, 
                                                      &end_iter, TRUE);
    g_strstrip (end_line_str);

    pos_triple_quotes = g_strstr_len (first_line_str, -1, "'''");
    if (!pos_triple_quotes)
        pos_triple_quotes = g_strstr_len (first_line_str, -1, "\"\"\"");
    
    if (!pos_triple_quotes)
        return NULL;

    end_line_str [pos_triple_quotes - end_line_str] = '\0';

    g_string_append (string, end_line_str);

    gchar *doc_string = string->str;
    g_string_free (string, FALSE);
    
    return doc_string;
}

/* Get position of a line 
 * in buffer
 */
int
get_line_pos (GtkTextBuffer *buffer, int line)
{
    GtkTextIter iter;
    gtk_text_buffer_get_iter_at_line (buffer, &iter, line);
    return gtk_text_iter_get_offset (&iter);
}

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
    int _index;
    /*If filename is already open then set filename's page as current page*/
    for (_index = 0; _index < get_total_pages(); _index++)
    {
        if (code_widget_array [_index]->file_path && 
            strcmp(file_path, code_widget_array [_index]->file_path) == 0)
             break;
    }
   
    if (_index != get_total_pages())
    {
        gtk_notebook_set_current_page (GTK_NOTEBOOK (notebook), _index);
        return TRUE;
    }

    gchar *file_str = get_file_data (file_path);
    if (!file_str)
        return FALSE;

    if (index != -1 && (code_widget_array [index]->file_path != NULL ||
           strcmp (get_text_at_index (index), "")) != 0 )
        file_new_tab_activate (NULL);

    else if (index == -1)
         file_new_tab_activate (NULL);

    index = get_current_index ();

    code_widget_array [index]->file_path = g_strdup (file_path);
    codewidget_set_text (code_widget_array [index], file_str);
    code_widget_array [index]->file_mode = FILE_EXISTS;
    char *file_name = strrchr (file_path, '/');
    file_name++;
    gtk_notebook_set_tab_label_text (GTK_NOTEBOOK (notebook),
                                    code_widget_array [index]->vbox, file_name);
    file_monitor_file (file_monitor, file_path);
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

/*To move cursor to position
 * and select that line
*/
void
go_to_pos_and_select_line (GtkTextView *view, int pos)
{
    GtkTextIter iter;
    GtkTextBuffer *buffer = gtk_text_view_get_buffer (view);
    gtk_text_buffer_get_iter_at_offset (buffer, &iter, pos);
    go_to_line (view, gtk_text_iter_get_line (&iter));
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

/*To count number of times str2
 * appears in str1
 */
int
str_count_str (gchar *str1, gchar *str2)
{
    int count = 0;

    while ((str1 = g_strstr_len (str1, -1, str2)) != NULL)
    {
        str1 += strlen (str2);
        count++;
    }
    return count;
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

/* Get selected text
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

/* To append text to GtkTextBuffer 
 */
void
gtk_text_buffer_append (GtkTextBuffer *buffer, gchar *text, int len)
{
    GtkTextIter iter;
    gtk_text_buffer_get_end_iter (buffer, &iter);
    gtk_text_buffer_insert (buffer, &iter, text, len);
}

/* To get first open unmatched 
 * parenthesis position from reverse
 */
gint
gtk_text_buffer_get_first_open_unmatched_parenthesis_pos (GtkTextBuffer *buffer, gint pos)
{
    GtkTextIter iter;
    gint line;
    gchar bracket1 = 0, bracket2 = 0;
    int count = 0, indentation, le_indent_count = 0;
    gchar *line_text;
    gtk_text_buffer_get_iter_at_offset (buffer, &iter, pos);
    line = gtk_text_iter_get_line (&iter);
    pos++;
    line_text = gtk_text_buffer_get_line_text (buffer, line, TRUE);
    indentation = get_indent_spaces_in_string (line_text);
    /* Traverse each line upward, untill difference of numbers of open and closed
    *  brackets is equal to 1.
    * Break the loop when second line which is less than or equal to of the same
    *  indentation as of starting line is found
    */
    do
    {
        pos --;
        if (pos < 0)
        {
            g_free (line_text);
            line_text = gtk_text_buffer_get_line_text (buffer, --line, TRUE);
            if (!line_text)
                return -1;
            if (get_indent_spaces_in_string (line_text) < indentation)
                le_indent_count++;
            if (le_indent_count == 2)
            {
                printf("same indent count\n");
                return -1;
            }
            pos = strlen (line_text) - 1;
        }
        if (line_text [pos] == '(' || line_text [pos] == '[')
            count ++;
        if (line_text [pos] == ')' || line_text [pos] == ']')
            count --; 
    }
    while (count != 1 && line >= 0);
    g_free (line_text);
    printf ("pos%d\n", pos);
    return pos;
}

void
go_to_file_at_line (gchar *file_path, gint line)
{
    int i;

    for (i = 0; i<get_total_pages (); i++)
    {
        if (strcmp (code_widget_array [i]->file_path, file_path) == 0)
            break;
    }

    if (i < get_total_pages ())
    {
        gtk_notebook_set_current_page (GTK_NOTEBOOK (notebook), i);
        go_to_line (GTK_TEXT_VIEW (code_widget_array [i]->sourceview), line - 1);
    }
    else
    {
        file_new_tab_activate (NULL);
        if (!open_file_at_index (file_path, get_total_pages () - 1))
        {
            gchar *message = g_strdup_printf ("Cannot open '%s'", file_path);
            show_error_message_dialog (message);
            g_free (message);
            return;
        }

        gtk_notebook_set_current_page (GTK_NOTEBOOK (notebook),
                                       get_total_pages () - 1);
        go_to_line (GTK_TEXT_VIEW (code_widget_array [get_total_pages () - 1]->sourceview), line - 1);
    }

    gtk_window_present (GTK_WINDOW (window));
}

/* To get matching paranthesis pos 
 * for char bracket
 */
gdouble
gtk_text_buffer_get_matching_parethesis_pos (GtkTextBuffer *buffer, gint pos, gchar bracket)
{
    GtkTextIter iter;
    gint line;
    gchar bracket1, bracket2;
    gboolean increase_pos;
    int count;
    gchar *line_text;
    gtk_text_buffer_get_iter_at_offset (buffer, &iter, pos);
    line = gtk_text_iter_get_line (&iter);

    if (bracket == '(')
    {
        bracket1 = '(';
        bracket2 = ')';
        increase_pos = TRUE;
    }
    else if (bracket == ')')
    {
        bracket1 = ')';
        bracket2 = '(';
        increase_pos = FALSE;
    }
    else if (bracket == '[')
    {
        bracket1 = '[';
        bracket2 = ']';
        increase_pos = TRUE;
    }
    else if (bracket == ']')
    {
        bracket1 = ']';
        bracket2 = '[';
        increase_pos = FALSE;
    }
    else if (bracket == '{')
    {
        bracket1 = '{';
        bracket2 = '}';
        increase_pos = TRUE;
    }
    else if (bracket == '}')
    {
        bracket1 = '}';
        bracket2 = '{';
        increase_pos = FALSE;
    }
    else
        return -1;
        
    line_text = gtk_text_buffer_get_line_text (buffer, line, TRUE);
    
    while ((increase_pos && pos < strlen (line_text) && line_text [pos++] != bracket1)
           || (!increase_pos && pos >= 0 && line_text [pos--] != bracket1));
   
    if (pos >= 0 || pos < strlen (line_text))
        count ++;

    while (count != 0)
    {
        if (increase_pos)
        {
            pos ++;
            if (pos >= strlen (line_text))
            {
                g_free (line_text);
                line_text = gtk_text_buffer_get_line_text (buffer, ++line, TRUE);
                pos = 0;
            }
        }
        else
        {
            pos --;
            if (pos < 0)
            {
                g_free (line_text);
                line_text = gtk_text_buffer_get_line_text (buffer, --line, TRUE);
                pos = strlen (line_text);
            }
        }
        
        if (line_text [pos] == bracket1)
            count ++;
        if (line_text [pos] == bracket2)
            count --;
    }
    return pos;
}

/* Returns number of
 * indent spaces in string
 */
int
get_indent_spaces_in_string (char *string)
{
    if (!string)
        return 0;

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
is_file_modified (int current_page)
{
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
 * with comments and strings removed
 */

gchar *
gtk_text_buffer_get_line_text (GtkTextBuffer *buffer, int line_index, gboolean strip_strings)
{
    GtkTextIter start_iter, end_iter;
    if (line_index < 0)
        return NULL;

    gtk_text_buffer_get_iter_at_line (buffer, &start_iter, line_index);
    gtk_text_buffer_get_line_end_iter (buffer, &end_iter, line_index);
    gchar *text = gtk_text_buffer_get_text (buffer, &start_iter, 
                                           &end_iter, TRUE);

    char *hash_text = g_strrstr (text, "#");    
    if (hash_text)
        text[hash_text - text] = '\0';
    
    if (strip_strings)
    {
        text = remove_text_between_strings (text, "'", "'");
        gchar *new_text = remove_text_between_strings (text, "\"", "\"");
        g_free (text);
        return new_text;
    }

    return text;
}

/*To return string between
 *start and end, including start
 *and including end 
 */
gchar *
get_text_between_strings (gchar *text, gchar *start, gchar *end)
{
    if (!start || !end)
        return NULL;

    gchar *new_str = g_malloc ((end - start+2)*sizeof (gchar));
    gchar *p = start;
    gchar *q = new_str;

    while (p != end + 1)
    {
        *q = *p;
        p++;
        q++;
    }
    new_str [end - start + 1] = '\0';
    return new_str;
}

/* To remove text between
 * two characters
 * Returns a newly allocated text
 */
gchar *
remove_text_between_strings (gchar *text, gchar *start, gchar *end)
{
    gchar *p_start = g_strstr_len (text, -1, start);
    gchar *p_end;
    if (g_strcmp0 (start, end) == 0 && p_start - text > 0)
        p_end = g_strstr_len (p_start+1, -1, end);
    else
        p_end = g_strstr_len (text, -1, end);
    
    if (!p_start || !p_end)
        return g_strdup (text);
    
    GString *g_string = g_string_new (text);
    g_string = g_string_erase (g_string, p_start - text, p_end - p_start);
    gchar *ret_str = g_string->str;
    g_string_free (g_string, FALSE);
    return ret_str;
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