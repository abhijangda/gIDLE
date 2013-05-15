#include "codewidget.h"
#include "core_funcs.h"
#include "main.h"
#include <string.h>

extern GtkWidget *status_bar;

/* Initializes
 * line_history stack
 */
static void
init_line_history (int *line_history)
{
    int i;
    for (i = 0; i < 5; i++)
        line_history [i] = -1;
}

/* Append Line 
 * to line_history stack
 */
static void
line_history_push (int *line_history, int line)
{
    /*search if line is already in line_history*/
    int i = -1;
    
    while (++i < 5 && line_history [i] != line);

    /*if found, remove it from that index and insert at 0 */
    if (i < 5)
    {
        int j = i+1;
        while ( --j > 0)
            line_history [j] = line_history [j-1];
        line_history [0] = line;
        return;
    }
    
    int index = -1;
    while (++index < 5 && line_history [index] != -1);
    if (index == 5)
        index -=1;

    i = index+1;
    while (--i > 0)
        line_history [i] = line_history [i-1];
    line_history [0] = line;
}

/* Create new
 * CodeWidget
*/

CodeWidget *
codewidget_new ()
{
    CodeWidget *codewidget = g_malloc0 (sizeof (CodeWidget));
    
    codewidget->hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 2);
    codewidget->vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 2);
    codewidget->scrollwin = gtk_scrolled_window_new (NULL, NULL);
    codewidget->class_combobox = gtk_combo_box_new ();
    codewidget->func_combobox = gtk_combo_box_new ();
    codewidget->sourcebuffer = gtk_source_buffer_new (NULL);
    codewidget->py_class_array = NULL;

    g_signal_connect (G_OBJECT (codewidget->sourcebuffer), "mark-set",
                     G_CALLBACK (codewidget_mark_set), codewidget);

    codewidget->sourceview = gtk_source_view_new_with_buffer (codewidget->sourcebuffer);

    g_signal_connect (G_OBJECT (codewidget->sourceview), "key-press-event",
                     G_CALLBACK (codewidget_key_press), NULL);
                    
    GtkSourceLanguageManager *languagemanager = gtk_source_language_manager_new();
    GtkSourceLanguage *language = gtk_source_language_manager_guess_language(languagemanager,"file.py",NULL);
    gtk_source_buffer_set_language(codewidget->sourcebuffer,language);
    
    gtk_box_pack_start (GTK_BOX (codewidget->vbox), codewidget->hbox,
                                       FALSE, FALSE, 2);
    gtk_box_pack_start (GTK_BOX (codewidget->vbox), codewidget->scrollwin,
                                       TRUE, TRUE, 2);
    gtk_container_add (GTK_CONTAINER (codewidget->scrollwin),
                                     codewidget->sourceview);
    gtk_box_pack_start (GTK_BOX (codewidget->hbox), codewidget->class_combobox,
                                       FALSE, FALSE, 2);
    gtk_box_pack_start (GTK_BOX (codewidget->hbox), codewidget->func_combobox,
                                       FALSE, FALSE, 2);
    
    init_line_history (codewidget->line_history);
    codewidget->line_history_current_index = -1;
    codewidget->prev_line = 0;
    codewidget->file_path = NULL;
    codewidget->file_mode = FILE_NEW;
    codewidget->py_class_array = NULL;
    codewidget->py_class_array_size = 0;

    PyClass *py_class = py_class_new ("Global Scope", "", NULL, NULL, 0, 0);
    codewidget_add_class (codewidget, py_class);
    return codewidget;
}

/*Add class to PyClass array
 * of codewidget
 */
void
codewidget_add_class (CodeWidget *codewidget, PyClass *py_class)
{
    codewidget->py_class_array = g_realloc (codewidget->py_class_array,
                                           codewidget->py_class_array_size * sizeof (PyClass *));
    codewidget->py_class_array [codewidget->py_class_array_size] = py_class;
    codewidget->py_class_array_size++;
}

/*Destroy 
 *CodeWidget
 */
void
codewidget_destroy (CodeWidget * code_widget)
{
    gtk_widget_destroy (code_widget->sourceview);
    gtk_widget_destroy (code_widget->func_combobox);
    gtk_widget_destroy (code_widget->class_combobox);
    gtk_widget_destroy (code_widget->scrollwin);
    gtk_widget_destroy (code_widget->hbox);
    gtk_widget_destroy (code_widget->vbox);
    
    g_free (code_widget->file_path);
    g_free (code_widget);    
}

/* Signal Handler for
 * emitted "mark-set" signal
 */
void
codewidget_mark_set (GtkTextBuffer *buffer, GtkTextIter *iter, GtkTextMark *mark, gpointer data)
{
    int chars = gtk_text_iter_get_offset (iter);
    int col = gtk_text_iter_get_line_offset (iter);
    int line = gtk_text_iter_get_line (iter);
    
    CodeWidget *codewidget = (CodeWidget *)data;
    
    if (line != codewidget->prev_line)
    {
        line_history_push (codewidget->line_history, line);
    }
    codewidget->prev_line = line;

    gchar *status_bar_msg;
    asprintf (&status_bar_msg, "Chars %d Col %d Line %d", chars, col, line);
    gtk_statusbar_push (GTK_STATUSBAR (status_bar), 0, status_bar_msg);
}

/*Set CodeWidget
 * text
 */
void
codewidget_set_text (CodeWidget *codewidget, gchar *text)
{
    
    gtk_text_buffer_set_text (gtk_text_view_get_buffer (GTK_TEXT_VIEW (codewidget->sourceview)),
                                text, -1);  
}

/*Event Handler for
 * "key-press-event"
 */
gboolean
codewidget_key_press (GtkWidget *widget, GdkEvent *event, gpointer data)
{
    GtkTextIter iter, new_iter;
    GtkTextBuffer *buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (widget));
    int indentation, i, col, new_col, line;
    gtk_text_buffer_get_iter_at_mark (buffer, &iter, gtk_text_buffer_get_insert (buffer));
    gchar *line_text = gtk_text_buffer_get_line_text (buffer, gtk_text_iter_get_line (&iter));
    col = gtk_text_iter_get_line_offset (&iter);
    line = gtk_text_iter_get_line (&iter);

    switch (event->key.keyval)
    {
        case GDK_KEY_Return:
            indentation = get_indent_spaces_in_string (line_text);
            //Search for ':' in line and increase indentation if found otherwise not
            if (g_strrstr (line_text, ":"))
                indentation += indent_width;
            
            gtk_text_buffer_insert (buffer, &iter, "\n", 1);
            for (i = 1; i <= indentation/indent_width; i++)
                gtk_text_buffer_insert (buffer, &iter, indent_width_str, indent_width);

            return TRUE;
        
        case GDK_KEY_Left:
            indentation = get_indent_spaces_in_string (line_text);
            
            if (col > indentation)
                return FALSE;
                
            if (col % indent_width == 0)
                new_col = col - indent_width;
            else
                new_col = col - col % indent_width;

            gtk_text_buffer_get_iter_at_line_offset (buffer, &new_iter, line, new_col);
            gtk_text_buffer_place_cursor (buffer, &new_iter);
            
            return TRUE;
        
        case GDK_KEY_BackSpace:
            indentation = get_indent_spaces_in_string (line_text);
            if (indentation == 0)
                return FALSE;

            if (col > indentation)
                return FALSE;
                
            if (col % indent_width == 0)
                new_col = col - indent_width;
            else
                new_col = col - col % indent_width;

            gtk_text_buffer_get_iter_at_line_offset (buffer, &new_iter, line, new_col);
            gtk_text_buffer_delete (buffer, &new_iter, &iter);
            
            return TRUE;
        
        case GDK_KEY_Home:
            indentation = get_indent_spaces_in_string (line_text);
            
            if (col == indentation)
                new_col = 0;
            else
                new_col = indentation;
                
           gtk_text_buffer_get_iter_at_line_offset (buffer, &new_iter, line, new_col);
           gtk_text_buffer_place_cursor (buffer, &new_iter);
            
            return TRUE;     
                
    }
    return FALSE;
}