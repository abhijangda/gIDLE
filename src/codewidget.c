#include "codewidget.h"
#include "core_funcs.h"
#include "main.h"
#include <string.h>

extern GtkWidget *status_bar;

static void
codewidget_get_class_funcs (CodeWidget *codewidget, gchar *text);

static void
codewidget_class_combo_changed (GtkComboBox *combobox, gpointer data);

static void
codewidget_func_combo_changed (GtkComboBox *combobox, gpointer data);

static void
codewidget_insert_text (GtkTextBuffer *buffer, GtkTextIter *location,
                       gchar *text, gint len, gpointer data);

static void
codewidget_delete_range (GtkTextBuffer *buffer, GtkTextIter *start,
                       GtkTextIter *end, gpointer data);

static gboolean can_combo_func_activate = TRUE;
static gboolean can_combo_class_activate = TRUE;

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
    codewidget->class_combobox = gtk_combo_box_text_new ();
    
    codewidget->combo_class_changed_handler_id = 
    g_signal_connect (G_OBJECT (codewidget->class_combobox), "changed",
                     G_CALLBACK (codewidget_class_combo_changed), codewidget);

    codewidget->func_combobox = gtk_combo_box_text_new ();
    
    codewidget->combo_func_changed_handler_id = 
    g_signal_connect (G_OBJECT (codewidget->func_combobox), "changed",
                     G_CALLBACK (codewidget_func_combo_changed), codewidget);

    codewidget->sourcebuffer = gtk_source_buffer_new (NULL);
    codewidget->py_class_array = NULL;

    codewidget->buffer_mark_set_handler_id = 
    g_signal_connect (G_OBJECT (codewidget->sourcebuffer), "mark-set",
                     G_CALLBACK (codewidget_mark_set), codewidget);

    g_signal_connect (G_OBJECT (codewidget->sourcebuffer), "insert-text",
                     G_CALLBACK (codewidget_insert_text), codewidget);
    
    g_signal_connect (G_OBJECT (codewidget->sourcebuffer), "delete-range",
                     G_CALLBACK (codewidget_delete_range), codewidget);
    
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
    
    /*New class "Global Scope" for global functions*/
    PyClass *py_class = py_class_new ("Global Scope", NULL, NULL, NULL, -1, -1);
    codewidget_add_class (codewidget, py_class);
    
    gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (codewidget->class_combobox),
                                           py_class->name);
    
    return codewidget;
}

/*Add class to PyClass array
 * of codewidget
 */
void
codewidget_add_class (CodeWidget *codewidget, PyClass *py_class)
{
    codewidget->py_class_array = g_realloc (codewidget->py_class_array,
                                           (codewidget->py_class_array_size + 1) * sizeof (PyClass *));
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
    GtkTextIter current_iter;
    gtk_text_buffer_get_iter_at_mark (buffer, &current_iter, gtk_text_buffer_get_insert (buffer));

    int chars = gtk_text_iter_get_offset (&current_iter);
    int col = gtk_text_iter_get_line_offset (&current_iter);;
    int line = gtk_text_iter_get_line (&current_iter);;
    int first_line = -1, last_line = -1;

    gchar *status_bar_msg;
    asprintf (&status_bar_msg, "Chars %d Col %d Line %d", chars, col, line);
    gtk_statusbar_push (GTK_STATUSBAR (status_bar), 0, status_bar_msg);

    CodeWidget *codewidget = (CodeWidget *)data;

    if (line != codewidget->prev_line)
    {
        int func, class;
        line_history_push (codewidget->line_history, line);
        for (class =  codewidget->py_class_array_size -1; class >= 0 &&
            codewidget->py_class_array [class]->pos > chars; class--);
        
        if (class == -1)
            class ++;

        if (class != -1)
        {
            can_combo_class_activate = FALSE;
            can_combo_func_activate = FALSE;

            gtk_combo_box_set_active (GTK_COMBO_BOX (codewidget->class_combobox), class);

            if (codewidget->py_class_array [class]->py_func_array != NULL)
            {
                /*check if cursor is between class <class_name> and def <func_name*/
                if (codewidget->py_class_array [class]->py_func_array [0]->pos <= chars)
                {                    
                    func = -1;
    
                    while (codewidget->py_class_array [class]->py_func_array [++func]);
    
                    func --;
                    for (; func >= 0 &&
                        codewidget->py_class_array [class]->py_func_array [func]->pos > chars;
                        func--);
                    if (func == -1)
                        func = 0;
                    
                    if (func != -1)
                        gtk_combo_box_set_active (GTK_COMBO_BOX (codewidget->func_combobox),
                                                 func);
                }
                else
                {
                    gtk_combo_box_set_active (GTK_COMBO_BOX (codewidget->func_combobox), -1);
                }
            }

           while (gtk_events_pending ())
                gtk_main_iteration_do (FALSE);

            can_combo_func_activate = TRUE;
            can_combo_class_activate = TRUE;
        }
    }
    codewidget->prev_line = line;
}

/*Handler for TextBuffer
 *delete-range signal
 */
static void
codewidget_delete_range (GtkTextBuffer *buffer, GtkTextIter *start,
                       GtkTextIter *end, gpointer data)
{
    int len = gtk_text_iter_get_offset (end) - gtk_text_iter_get_offset (start);
    
    CodeWidget *codewidget = (CodeWidget *)data;
    int class = gtk_combo_box_get_active (GTK_COMBO_BOX (codewidget->class_combobox));
    int func = gtk_combo_box_get_active (GTK_COMBO_BOX (codewidget->func_combobox));
    if (class == -1)
        return;

    /*Updating Functions position when text is deleted*/
    do
    {
        if (codewidget->py_class_array [class]-> py_func_array != NULL)
        {
            while (codewidget->py_class_array [class]-> py_func_array [++func])
                codewidget->py_class_array [class]->py_func_array [func]->pos -= len;
        }
        func = -1;
    }
    while (++class < codewidget->py_class_array_size);
    
    /*Updating Class position when text is deleted*/
    class = gtk_combo_box_get_active (GTK_COMBO_BOX (codewidget->class_combobox));
    while (++class < codewidget->py_class_array_size)
            codewidget->py_class_array [class]->pos -= len;
}
                        
/*Handler for TextBuffer
 *insert-text signal
 */
static void
codewidget_insert_text (GtkTextBuffer *buffer, GtkTextIter *location,
                       gchar *text, gint len, gpointer data)
{
    CodeWidget *codewidget = (CodeWidget *)data;
    int class = gtk_combo_box_get_active (GTK_COMBO_BOX (codewidget->class_combobox));
    int func = gtk_combo_box_get_active (GTK_COMBO_BOX (codewidget->func_combobox));
    if (class == -1)
        return;

    /*Updating Functions position when text is inserted*/
    do
    {
        if (codewidget->py_class_array [class]-> py_func_array != NULL)
        {
            while (codewidget->py_class_array [class]-> py_func_array [++func])
                codewidget->py_class_array [class]->py_func_array [func]->pos += len;
        }
        func = -1;
    }
    while (++class < codewidget->py_class_array_size);
    
    /*Updating Class position when text is inserted*/
    class = gtk_combo_box_get_active (GTK_COMBO_BOX (codewidget->class_combobox));
    while (++class < codewidget->py_class_array_size)
            codewidget->py_class_array [class]->pos += len;
}

/*Set CodeWidget
 * text
 */
void
codewidget_set_text (CodeWidget *codewidget, gchar *text)
{
    g_signal_handler_disconnect (codewidget->sourcebuffer,
                                codewidget->buffer_mark_set_handler_id);

    codewidget_get_class_funcs (codewidget, text);
    gtk_text_buffer_set_text (gtk_text_view_get_buffer (GTK_TEXT_VIEW (codewidget->sourceview)),
                                text, -1);  
    
    GtkTextIter start_iter;
    gtk_text_buffer_get_start_iter (gtk_text_view_get_buffer (GTK_TEXT_VIEW (codewidget->sourceview)),
                                   &start_iter);
    gtk_text_buffer_place_cursor (gtk_text_view_get_buffer (GTK_TEXT_VIEW (codewidget->sourceview)),
                                 &start_iter);
    
    gtk_combo_box_set_active (GTK_COMBO_BOX (codewidget->class_combobox), 0);

    codewidget->buffer_mark_set_handler_id = 
    g_signal_connect (G_OBJECT (codewidget->sourcebuffer), "mark-set",
                     G_CALLBACK (codewidget_mark_set), codewidget);
}

/*Get classes and functions 
 * of current codewidget
 */

static void
codewidget_get_class_funcs (CodeWidget *codewidget, gchar *text)
{
    GRegex *regex_class,*regex_func;
    GMatchInfo *match_info_class,*match_info_func;
    int indentation=0, start, end;
    
    /*getting class names and their positions*/
    regex_class = g_regex_new ("[ \\ \\t]*\\bclass\\b\\s*\\w+\\s*\\(*.*\\)*:", 0, 0, NULL);
    
    if (g_regex_match (regex_class, text, 0, &match_info_class))
    {
        int class = 1;
        do
        {
            gchar *class_def_string = g_match_info_fetch(match_info_class,0);
            
            class_def_string = remove_char (class_def_string, ':');
            indentation = get_indent_spaces_in_string (class_def_string) / indent_width;
            g_match_info_fetch_pos (match_info_class, 0, &start, &end);
            codewidget_add_class (codewidget,
                                             py_class_new_from_def (class_def_string,
                                             start, indentation));
            g_free (class_def_string);
            g_match_info_next (match_info_class, NULL);

            /*Adding to combo_class*/
            gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (codewidget->class_combobox),
                                           codewidget->py_class_array [class]->name);
            /******************/
            class++;
        }
        while (g_match_info_matches (match_info_class));        
    }
    g_match_info_free (match_info_class);
    g_regex_unref (regex_class);
    
    regex_func = g_regex_new ("([ \\ \\t]*\\bdef\\b\\s*.+)\\:", 0, 0, NULL); 
    
    if (g_regex_match (regex_func, text ,0, &match_info_func))
    {
        do
        {
            gchar *func_def_string = g_match_info_fetch(match_info_func,0);
            func_def_string = remove_char (func_def_string, ':');
            indentation = get_indent_spaces_in_string (func_def_string) / indent_width;
            g_match_info_fetch_pos (match_info_func, 0, &start, &end);
            PyFunc *py_func = py_func_new_from_def (func_def_string, start,
                                                   indentation);
            
            /*Add py_func to desired py_class*/
            int i = -1;
            int class = codewidget->py_class_array_size - 1;
            for (; class >= 0; class --)
            {
                if (codewidget->py_class_array [class]->pos < start &&
                   codewidget->py_class_array [class]->indentation + 1 == indentation)
                    break;
            }

            py_funcv_append (&(codewidget->py_class_array [class]->py_func_array),
                            py_func);
            /*************************/

            g_free (func_def_string);
            g_match_info_next (match_info_func, NULL);
        }
        while (g_match_info_matches (match_info_func));        
    }
}

/*Called when func_combobox
 * emits changed signal
 */
static void
codewidget_func_combo_changed (GtkComboBox *combobox, gpointer data)
{
    CodeWidget *codewidget = (CodeWidget*)data;
    int index = gtk_combo_box_get_active (combobox);
    int class = gtk_combo_box_get_active (GTK_COMBO_BOX (codewidget->class_combobox));

    if (index == -1 || class == -1)
        return;
    
    if (codewidget->py_class_array [class]->py_func_array == NULL)
        return;
    
    if (!can_combo_func_activate)
        return;

    go_to_pos_and_select_line (GTK_TEXT_VIEW (codewidget->sourceview),
                              codewidget->py_class_array [class]->py_func_array [index]->pos);
}

/*Called when class_combobox
 * emits changed signal
 */
static void
codewidget_class_combo_changed (GtkComboBox *combobox, gpointer data)
{
    CodeWidget *codewidget = (CodeWidget*)data;
    int index = gtk_combo_box_get_active (combobox);
    
    if (index == -1)
        return;

    gtk_combo_box_text_remove_all (GTK_COMBO_BOX_TEXT (codewidget->func_combobox));
    
    if (codewidget->py_class_array [index]->py_func_array == NULL)
        return;

    can_combo_func_activate = FALSE;
    int i = 0;
    while (codewidget->py_class_array [index]->py_func_array [i] != NULL)
        gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (codewidget->func_combobox),
                                       codewidget->py_class_array [index]->py_func_array [i++]->name);

    while (gtk_events_pending ())
        gtk_main_iteration_do (FALSE);

    if (can_combo_class_activate)
    {
        can_combo_func_activate = TRUE;
        go_to_pos_and_select_line (GTK_TEXT_VIEW (codewidget->sourceview),
                                  codewidget->py_class_array [index]->pos);
    }
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