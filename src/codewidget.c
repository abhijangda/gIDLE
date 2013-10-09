#include "codewidget.h"
#include "core_funcs.h"
#include "main.h"
#include "line_iterator.h"
#include "file_modify_box.h"

#include <string.h>

extern GtkWidget *status_bar;
extern gIDLEOptions options;

static void
_codewidget_parse_function (CodeWidget *codewidget, PyClass *klass, PyFunc *func, int curr_line);

static gboolean
display_status_bar_message (gpointer data);

void
_codewidget_link_child_classes_to_their_parents (CodeWidget *codewidget);

static int
_codewidget_get_line_at_multiline_comment_end (GtkTextBuffer *buffer, int line);

static int
_codewidget_start_parse_from_line (CodeWidget *codewidget, GtkTextBuffer *buffer, int line, PyClass **klass, gboolean);

static void
_codewidget_import_module_async (CodeWidget *codewidget, char *line_text);

static void
get_class_funcs_thread_func (GSimpleAsyncResult *res, GObject *object,
                             GCancellable *cancellable);

static void
get_class_func_callback (GObject *object, GAsyncResult *res, gpointer data);

static void
_codewidget_hide_code_list_and_grab_source_view (CodeWidget *codewidget);

static gboolean 
_codewidget_show_code_list_view (CodeWidget *codewidget);

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

static void
codewidget_line_num_widget_draw (GtkWidget *widget, cairo_t *cr,
                                 gpointer data);

static void
codewidget_draw (GtkWidget *widget, cairo_t *cr, gpointer data);

static void
codewidget_code_scroll_win_draw (GtkWidget *widget, cairo_t *cr, gpointer data);

static void
_codewidget_move_cursor_to_screen (CodeWidget *codewidget);

static void
codewidget_mark_set (GtkTextBuffer *, GtkTextIter *, GtkTextMark *, gpointer data);

static gboolean
codewidget_key_press (GtkWidget *, GdkEvent *, gpointer);

static gboolean
codewidget_key_release (GtkWidget *, GdkEvent *, gpointer);

static void
_codewidget_code_list_row_activated (GtkTreeView *view, GtkTreePath *path,
                             GtkTreeViewColumn *column, gpointer data);

static void
_codewidget_fm_box_clicked (GtkWidget *, FileModifyBoxResponse response, gpointer data);

static gboolean can_combo_func_activate = TRUE;
static gboolean can_combo_class_activate = TRUE;

extern GRegex *regex_class, *regex_func, *regex_static_var;
static GRegex  *regex_comments, *regex_str, *regex_mutlistr, *regex_line;
extern GRegex *regex_word, *regex_local_var;
extern GRegex *regex_self_var;

extern GAsyncQueue *async_queue;
static gboolean display_status_bar_message_return_false;

static int code_list_show_pos = -1;

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
    PangoFontDescription *font_desc;
    
    codewidget->code_assist_widget = code_assist_widget_new ();

    g_signal_connect (G_OBJECT (codewidget->code_assist_widget->list_view), "row-activated", 
                     G_CALLBACK (_codewidget_code_list_row_activated), 
                     codewidget);

    codewidget->hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 2);
    codewidget->vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 2);
    codewidget->scrollwin = gtk_scrolled_window_new (NULL, NULL);
    //gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (codewidget->scrollwin), 
                                         //GTK_SHADOW_IN);
    codewidget->class_combobox = gtk_combo_box_text_new ();

    codewidget->combo_class_changed_handler_id = 
    g_signal_connect (G_OBJECT (codewidget->class_combobox), "changed",
                     G_CALLBACK (codewidget_class_combo_changed), codewidget);

    codewidget->func_combobox = gtk_combo_box_text_new ();
    
    codewidget->combo_func_changed_handler_id = 
    g_signal_connect (G_OBJECT (codewidget->func_combobox), "changed",
                     G_CALLBACK (codewidget_func_combo_changed), codewidget);

    codewidget->sourcebuffer = gtk_source_buffer_new (NULL);
    codewidget->invisible_tag = gtk_text_buffer_create_tag (GTK_TEXT_BUFFER (codewidget->sourcebuffer),
                                                           NULL, "invisible", TRUE, NULL);
    codewidget->py_class_array = NULL;

    codewidget->buffer_mark_set_handler_id = 
    g_signal_connect (G_OBJECT (codewidget->sourcebuffer), "mark-set",
                     G_CALLBACK (codewidget_mark_set), codewidget);

    g_signal_connect (G_OBJECT (codewidget->sourcebuffer), "insert-text",
                     G_CALLBACK (codewidget_insert_text), codewidget);
    
    g_signal_connect (G_OBJECT (codewidget->sourcebuffer), "delete-range",
                     G_CALLBACK (codewidget_delete_range), codewidget);
    
    codewidget->sourceview = gtk_source_view_new_with_buffer (codewidget->sourcebuffer);
 
    /*Set default font*/
    font_desc = pango_font_description_from_string (options.font_name);
    gtk_widget_modify_font (codewidget->sourceview, font_desc);
    pango_font_description_free (font_desc);

    g_signal_connect (G_OBJECT (codewidget->sourceview), "key-press-event",
                     G_CALLBACK (codewidget_key_press), codewidget); 

    g_signal_connect (G_OBJECT (codewidget->sourceview), "key-release-event",
                     G_CALLBACK (codewidget_key_release), codewidget); 

    g_signal_connect_after (G_OBJECT (codewidget->sourceview), "draw",
                     G_CALLBACK (codewidget_draw), codewidget);
                    
    GtkSourceLanguageManager *languagemanager = gtk_source_language_manager_new ();
    GtkSourceLanguage *language = gtk_source_language_manager_guess_language (languagemanager, "file.py", NULL);
    gtk_source_buffer_set_language (codewidget->sourcebuffer, language);
    
    codewidget->line_num_widget = gtk_drawing_area_new (); 
    
    g_signal_connect (G_OBJECT (codewidget->line_num_widget), "draw",
                     G_CALLBACK (codewidget_line_num_widget_draw), codewidget);

    gtk_widget_set_size_request (codewidget->line_num_widget,
                                 10, 100);
    
    codewidget->code_folding_widget = code_folding_widget_new ();

    g_signal_connect (G_OBJECT (codewidget->code_folding_widget->drawing_area),
                     "draw", G_CALLBACK (code_folding_widget_draw),
                     codewidget);
    
    g_signal_connect (G_OBJECT (codewidget->code_folding_widget->drawing_area),
                     "button-press-event", G_CALLBACK (code_folding_button_press),
                     codewidget);

    gtk_widget_set_size_request (codewidget->code_folding_widget->drawing_area,
                                 11, 100);

    codewidget->hbox_scroll_line = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);

    gtk_box_pack_start (GTK_BOX (codewidget->vbox), codewidget->hbox,
                                       FALSE, FALSE, 2);
    gtk_box_pack_start (GTK_BOX (codewidget->vbox), codewidget->hbox_scroll_line,
                                       TRUE, TRUE, 2);

    gtk_box_pack_start (GTK_BOX (codewidget->hbox_scroll_line),
                       codewidget->line_num_widget, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (codewidget->hbox_scroll_line),
                       codewidget->code_folding_widget->drawing_area, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (codewidget->hbox_scroll_line), codewidget->scrollwin,
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
    codewidget->py_nested_class_array = NULL;
    codewidget->py_nested_class_array_size = 0;

    /*New class "Global Scope" for global functions*/
    PyClass *py_class = py_class_new ("Global Scope", NULL, NULL, NULL, -1, 0);
    codewidget_add_class (codewidget, py_class);

    gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (codewidget->class_combobox),
                                           ((PyVariable *)py_class)->name);

    codewidget->py_variable_array = NULL;
    codewidget->py_variable_array_size = 0;
    
    codewidget->curr_func_static_var_array = NULL;
    codewidget->curr_func_static_var_array_size = 0;

    codewidget->thread_ended = TRUE;

    regex_comments = g_regex_new ("#.+", 0, 0, NULL);
    regex_str = g_regex_new ("['\"][^'\"]*.+['\"]", 0, 0, NULL);
    regex_str = g_regex_new ("'''.+?'''", G_REGEX_DOTALL, 0, NULL);
    regex_str = g_regex_new ("\"\"\".+?\"\"\"", G_REGEX_DOTALL, 0, NULL);
    regex_class = g_regex_new ("[ \\ \\t]*\\bclass\\b\\s*\\w+\\s*\\(*.*\\)*:", 0, 0, NULL);
    regex_func = g_regex_new ("[ \\ \\t]*def\\s+[\\w\\d_]+\\s*\\(.+\\)\\:", 0, 0, NULL); 
    regex_line = g_regex_new (".+", 0, 0, NULL);
    
    codewidget->file_modify_box = NULL;
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
    g_signal_handler_disconnect (code_widget->sourcebuffer, code_widget->buffer_mark_set_handler_id);
    //code_folding_widget_destroy (code_widget->code_folding_widget);
    //gtk_widget_destroy (code_widget->line_num_widget);
    //gtk_widget_destroy (code_widget->sourceview);
    //gtk_widget_destroy (code_widget->func_combobox);
    //gtk_widget_destroy (code_widget->class_combobox);
    //gtk_widget_destroy (code_widget->scrollwin);
    //gtk_widget_destroy (code_widget->hbox);
    //gtk_widget_destroy (code_widget->vbox);
    int i;
    for (i = 0; i < code_widget->py_variable_array_size; i++)
        code_widget->py_variable_array [i]->destroy (code_widget->py_variable_array [i]);

    for (i = 0; i < code_widget->py_class_array_size; i++)
    {
        PyVariable *py_var = PY_VARIABLE (code_widget->py_class_array [i]);
        py_var->destroy (py_var);
    }
    

    g_free (code_widget->py_nested_class_array);
    g_free (code_widget->file_path);
    g_free (code_widget);    
}

/*Signal Handler for line_num_widget
 * draw signal
 */
static void
codewidget_line_num_widget_draw (GtkWidget *widget, cairo_t *cr,
                             gpointer data)
{
    CodeWidget *codewidget = (CodeWidget *)data;
    
    if (!options.show_line_numbers)
         return;
    
    GtkTextBuffer *buffer = GTK_TEXT_BUFFER (codewidget->sourcebuffer);
    GdkRectangle location;
    GtkTextIter rect_start_iter, rect_end_iter, current_iter;
    int i, last_line;
    
    cairo_set_source_rgb (cr, 1.0, 1.0, 1.0);

    cairo_rectangle (cr, 0, 0, gtk_widget_get_allocated_width (widget),
                     gtk_widget_get_allocated_height (widget));

    cairo_fill (cr);

    cairo_text_extents_t text_extents;
    gchar *line_str;

    gtk_text_buffer_get_iter_at_mark (buffer, &current_iter,
                                      gtk_text_buffer_get_insert (buffer));

    cairo_set_source_rgb (cr, 0, 0, 0);
    cairo_set_line_width (cr, 1);
    cairo_set_font_size (cr, 14);

    gtk_text_view_get_visible_rect (GTK_TEXT_VIEW (codewidget->sourceview),
                                    &location);
    gtk_text_view_get_iter_at_location (GTK_TEXT_VIEW (codewidget->sourceview),
                                        &rect_start_iter, location.x,location.y);
    gtk_text_view_get_iter_at_location (GTK_TEXT_VIEW (codewidget->sourceview),
                                        &rect_end_iter, location.x + location.width, 
                                        location.y + location.height);

    if (gtk_text_iter_get_line (&rect_end_iter) < gtk_text_buffer_get_line_count (buffer))
         last_line = gtk_text_iter_get_line (&rect_end_iter);
    else
         last_line = gtk_text_buffer_get_line_count (buffer);

    for (i = gtk_text_iter_get_line (&rect_start_iter); i <= last_line; i++)
    {        
        GtkTextIter iter;
        GdkRectangle loc2;
        int window_x, window_y;
        
        gtk_text_buffer_get_iter_at_line (buffer, &iter, i);
        gtk_text_view_get_iter_location (GTK_TEXT_VIEW (codewidget->sourceview),
                                        &iter, &loc2);
        gtk_text_view_buffer_to_window_coords (GTK_TEXT_VIEW (codewidget->sourceview),
                                              GTK_TEXT_WINDOW_WIDGET, loc2.x,
                                              loc2.y, &window_x, &window_y);

        if (i == gtk_text_iter_get_line (&current_iter))
            cairo_select_font_face (cr, "monospace", CAIRO_FONT_SLANT_NORMAL,
                                   CAIRO_FONT_WEIGHT_BOLD);
        else
            cairo_select_font_face (cr, "monospace", CAIRO_FONT_SLANT_NORMAL,
                                   CAIRO_FONT_WEIGHT_NORMAL);

        cairo_move_to (cr, 0, window_y + 15);
        /*Find if i lies in any folded text*/
        int j;
        for (j = 0; j < codewidget->code_folding_widget->invisible_text_info_array_size; j++)
        {
            if (codewidget->code_folding_widget->invisible_text_info_array [j]->start_line
               == i)
            {
                /*If yes, then display "..."*/
                line_str = g_strdup_printf ("...");
                i = codewidget->code_folding_widget->invisible_text_info_array [j]->end_line;
                break;
            }
        }
        if (j == codewidget->code_folding_widget->invisible_text_info_array_size)
            line_str = g_strdup_printf ("%d", i+1);

        cairo_show_text (cr, line_str);
        g_free (line_str);        
    }
    line_str = g_strdup_printf ("%d", i);
    cairo_text_extents (cr, line_str, &text_extents);
    g_free (line_str);
    gtk_widget_set_size_request (codewidget->line_num_widget,
                                text_extents.width + 5, 100);
}

/* Signal Handler for
 * emitted "mark-set" signal
 */
static void
codewidget_mark_set (GtkTextBuffer *buffer, GtkTextIter *iter, GtkTextMark *mark, gpointer data)
{
    GtkTextIter current_iter, _iter;
    gtk_text_buffer_get_iter_at_mark (buffer, &current_iter,
                                      gtk_text_buffer_get_insert (buffer));

    int chars = gtk_text_iter_get_offset (&current_iter);
    int col = gtk_text_iter_get_line_offset (&current_iter);
    int line = gtk_text_iter_get_line (&current_iter);

    CodeWidget *codewidget = (CodeWidget *)data;

    gchar *status_bar_msg;
    asprintf (&status_bar_msg, "Chars %d Col %d Line %d",
              chars+1, col+1, line+1);
    gtk_statusbar_push (GTK_STATUSBAR (status_bar), 0, status_bar_msg);
    
    gtk_text_buffer_get_iter_at_offset (buffer, &_iter, code_list_show_pos);
    if (codewidget->code_assist_widget && 
        GTK_IS_WIDGET (codewidget->code_assist_widget->parent) &&
        gtk_widget_get_parent (codewidget->code_assist_widget->parent) != NULL &&
        (chars < code_list_show_pos || line != gtk_text_iter_get_line (&_iter)))
        _codewidget_hide_code_list_and_grab_source_view (codewidget);

    if (line != codewidget->prev_line)
    {
        _codewidget_move_cursor_to_screen (codewidget);
        
        if (gtk_text_buffer_get_has_selection (buffer))
        {
            codewidget->prev_line = -1;
            return;
        }

        gchar *line_text;
        line_text = gtk_text_buffer_get_line_text (buffer,
                                                    codewidget->prev_line,
                                                    TRUE);

        _codewidget_import_module_async (codewidget, line_text);

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
                    func = -1;
                    gtk_combo_box_set_active (GTK_COMBO_BOX (codewidget->func_combobox), -1);
                }

            }

            can_combo_func_activate = TRUE;
            can_combo_class_activate = TRUE;
        }
         if (class != -1 && func != -1 && codewidget->py_class_array [class]->py_func_array)
             _codewidget_parse_function (codewidget, codewidget->py_class_array [class], codewidget->py_class_array [class]->py_func_array [func], line);
    }
    
    codewidget->prev_line = line;
}

static void
_codewidget_move_cursor_to_screen (CodeWidget *codewidget)
{
    if (!codewidget || !GTK_IS_TEXT_VIEW (codewidget->sourceview))
        return;

    GdkRectangle visible_rect;

    GtkTextBuffer *buffer;
    buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (codewidget->sourceview));

    GtkTextIter current_iter, start_bound_iter, end_bound_iter;
    
    gtk_text_buffer_get_iter_at_mark (buffer, &current_iter,
                                      gtk_text_buffer_get_insert (buffer));
    
    int line = gtk_text_iter_get_line (&current_iter);
    
    gtk_text_view_get_visible_rect (GTK_TEXT_VIEW (codewidget->sourceview),
                                    &visible_rect);
    gtk_text_view_get_iter_at_location (GTK_TEXT_VIEW (codewidget->sourceview),
                                        &start_bound_iter, visible_rect.x,
                                        visible_rect.y);
    
    gtk_text_view_get_iter_at_location (GTK_TEXT_VIEW (codewidget->sourceview),
                                        &end_bound_iter, visible_rect.x,
                                        visible_rect.y + visible_rect.height);
    
    if (gtk_text_iter_get_line (&start_bound_iter) > line || 
        line > gtk_text_iter_get_line (&end_bound_iter))
        gtk_text_view_scroll_to_iter (GTK_TEXT_VIEW (codewidget->sourceview),
                                     &current_iter, 0.0, FALSE, 0, 0);
}

/* Handler for SourceView
 * draw signal
 */
static void
codewidget_draw (GtkWidget *widget, cairo_t *cr, gpointer data)
{
    CodeWidget *codewidget = (CodeWidget *)data;
    if (gtk_widget_get_window (codewidget->line_num_widget))
        gdk_window_invalidate_rect (gtk_widget_get_window (codewidget->line_num_widget),
                                    NULL,
                                    FALSE);
    if (gtk_widget_get_window (codewidget->code_folding_widget->drawing_area))
        gdk_window_invalidate_rect (gtk_widget_get_window (codewidget->code_folding_widget->drawing_area),
                                    NULL, FALSE);
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

    if (gtk_widget_get_parent (codewidget->code_assist_widget->parent) != NULL)
    {
        if (gtk_text_iter_get_offset (start) + len < code_list_show_pos)
            _codewidget_hide_code_list_and_grab_source_view (codewidget);

        else
        {
            GtkTextIter code_list_show_iter;
            gtk_text_buffer_get_iter_at_offset (buffer, &code_list_show_iter, 
                                                code_list_show_pos);

            gchar *sel_text = gtk_text_buffer_get_text (buffer,
                                                        &code_list_show_iter,
                                                        start, FALSE);
            g_strstrip (sel_text);
            printf ("sel_text %s\n", sel_text);
            /* If there is nothing to be filled in code_list_view, remove it*/
            if (!code_assist_show_matches (codewidget->code_assist_widget, sel_text))
                _codewidget_hide_code_list_and_grab_source_view (codewidget);
        }
    }
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

    if (gtk_widget_get_parent (codewidget->code_assist_widget->parent) != NULL)
    {
        if (gtk_text_iter_get_offset (location) + len < code_list_show_pos)
            _codewidget_hide_code_list_and_grab_source_view (codewidget);

        else
        {
            GtkTextIter code_list_show_iter;
            gtk_text_buffer_get_iter_at_offset (buffer, &code_list_show_iter, 
                                                code_list_show_pos);

            gchar *sel_text = gtk_text_buffer_get_text (buffer,
                                                        &code_list_show_iter,
                                                        location, FALSE);
            g_strstrip (sel_text);

            if (g_strcmp0 (sel_text, "") != 0)
            {
                gchar *full_text = g_strconcat (sel_text, text, NULL);
                /* If there is nothing to be filled in code_list_view, remove it*/
                if (!code_assist_show_matches (codewidget->code_assist_widget, full_text))
                    _codewidget_hide_code_list_and_grab_source_view (codewidget);
            }
        }
    }

update:
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

    gtk_text_buffer_set_text (gtk_text_view_get_buffer (GTK_TEXT_VIEW (codewidget->sourceview)),
                                text, -1);  
    
    while (gtk_events_pending ())
        gtk_main_iteration_do (FALSE);

    GtkTextIter start_iter;
    gtk_text_buffer_get_start_iter (gtk_text_view_get_buffer (GTK_TEXT_VIEW (codewidget->sourceview)),
                                   &start_iter);
    gtk_text_buffer_place_cursor (gtk_text_view_get_buffer (GTK_TEXT_VIEW (codewidget->sourceview)),
                                 &start_iter);
    
    gtk_combo_box_set_active (GTK_COMBO_BOX (codewidget->class_combobox), 0);
    
    codewidget->buffer_mark_set_handler_id = 
    g_signal_connect (G_OBJECT (codewidget->sourcebuffer), "mark-set",
                     G_CALLBACK (codewidget_mark_set), codewidget);

    codewidget_get_class_funcs (codewidget, text);
}

void
codewidget_update_class_funcs (CodeWidget *codewidget)
{
    gchar *ext = strrchr (codewidget->file_path, '.');
    if (g_strcmp0 (ext, ".py"))
        return;

    if (!codewidget->thread_ended)
        return;

    gchar *code_widget_str;
    GtkTextBuffer *text_buffer;
    text_buffer = gtk_text_view_get_buffer (
        GTK_TEXT_VIEW (codewidget->sourceview));

    int i;
    for (i = 0; i < codewidget->py_class_array_size; i++)
    {
        PyVariable *py_var = PY_VARIABLE (codewidget->py_class_array [i]);
        py_var->destroy (py_var);
    }

    for (i = codewidget->py_class_array_size; i > 0; i--)
        gtk_combo_box_text_remove (GTK_COMBO_BOX_TEXT (codewidget->class_combobox), i);
    
    codewidget->py_class_array_size = 0;
    codewidget->py_nested_class_array_size = 0;
    
    PyClass *py_class = py_class_new ("Global Scope", NULL, NULL, NULL, -1, 0);
    codewidget_add_class (codewidget, py_class);
 
    int line = 0;

    while(line < gtk_text_buffer_get_line_count (text_buffer))
    {
        int _line = _codewidget_get_line_at_multiline_comment_end (text_buffer, line);
        if (_line != -1 && _line != line)
            line = _line + 1;

        line = _codewidget_start_parse_from_line (codewidget, text_buffer, line,
                                                    &(codewidget->py_class_array[0]), FALSE);
    }

    _codewidget_link_child_classes_to_their_parents (codewidget);
    get_class_func_callback (NULL, NULL, codewidget);
    symbols_view_fill_from_codewidget (SYMBOLS_VIEW (symbols_view), codewidget);
}

static gboolean
is_pos_in_array (int start, int end, int **array, int array_size)
{
    int i;
    for (i = 0; i <array_size; i++)
    {
        if (array [i][0] <= start && array[i][1] >= end)
             return TRUE;
    }
    return FALSE;
}

/*Get classes and functions 
 * of current codewidget
 */

static int
_codewidget_get_line_at_multiline_comment_end (GtkTextBuffer *buffer, int line)
{
    char *line_text = NULL;
    
    line--;
    do
    {
        line++;
        line_text = gtk_text_buffer_get_line_text (buffer, line, FALSE);
        /*Remove Comments*/
        char *hash_text = g_strrstr (line_text, "#");
        if (hash_text)
            line_text[hash_text - line_text] = '\0';

        g_strstrip (line_text);
    }
    while (!g_strstr_len (line_text, -1, "") && line < gtk_text_buffer_get_line_count (buffer));

    if (!g_strstr_len (line_text, -1, "'''"))
        return line;

    line++;
    while (line < gtk_text_buffer_get_line_count (buffer))
    {
        line_text = gtk_text_buffer_get_line_text (buffer, line, FALSE);
        if (g_strstr_len (line_text, -1, "'''"))
            return line;

        line++;
    }

    return line - 1;
}

static void
_codewidget_code_list_row_activated (GtkTreeView *view, GtkTreePath *path,
                             GtkTreeViewColumn *column, gpointer data)
{
    /* Remove the text when code_list_view first appear and
     * insert the selected PyFunc there
     */
    CodeWidget *codewidget = (CodeWidget *)data;
    GtkTextBuffer *buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (
                                                      codewidget->sourceview));
    GtkTreeSelection *selection;
    selection = gtk_tree_view_get_selection (
        GTK_TREE_VIEW (codewidget->code_assist_widget->list_view));

    GtkTreeIter tree_iter;
    gtk_tree_selection_get_selected (selection, NULL, &tree_iter);
    gpointer value;
    gtk_tree_model_get (
        GTK_TREE_MODEL (codewidget->code_assist_widget->list_store),
                        &tree_iter, ASSIST_COL_POINTER, &value, -1);
    PyVariable *py_var = (PyVariable *) value;

    GtkTextIter prev_iter, iter;
    gtk_text_buffer_get_iter_at_mark (buffer, &iter,
                                      gtk_text_buffer_get_insert (buffer));
    gtk_text_buffer_get_iter_at_offset (buffer, &prev_iter,
                                        code_list_show_pos);
    gtk_text_buffer_delete (buffer, &prev_iter, &iter);
    gtk_text_buffer_get_iter_at_mark (buffer, &iter,
                                      gtk_text_buffer_get_insert (buffer));
    gtk_text_buffer_insert (buffer, &iter, py_var->name, -1);

    _codewidget_hide_code_list_and_grab_source_view (codewidget);
}

/*To link base classes of a child 
 *class to its parent classes recursively
 */

void
_codewidget_link_class_to_its_parents (CodeWidget *codewidget, PyClass *class)
{
    gchar **name = class->base_class_names;
    while (*name)
    {
        if (!g_strcmp0 (*name, "object"))
        {
            name++;
            continue;
        }

        int i;
        for (i = 0; i < codewidget->py_class_array_size; i++)
        {
            if (!g_strcmp0 (*name,
                            PY_VARIABLE (codewidget->py_class_array [i])->name))
                py_classv_add_py_class (&(class->base_classes),
                                        &(class->base_classes_size),
                                        codewidget->py_class_array [i]);
        }
        
        for (i = 0; i < codewidget->py_variable_array_size; i++)
        {
            if (codewidget->py_variable_array [i]->type == MODULE)
            {
                PyVariable *_class = py_module_search_for_class_name (
                    PY_MODULE (codewidget->py_variable_array [i]), *name);

                if (_class)
                    py_classv_add_py_class (&(class->base_classes),
                                            &(class->base_classes_size),
                                            PY_CLASS (_class));
            }
            else if (codewidget->py_variable_array [i]->type == CLASS &&
                     !g_strcmp0 (codewidget->py_variable_array [i]->name,
                                 *name))
            {
                py_classv_add_py_class (&(class->base_classes),
                                        &(class->base_classes_size),
                                        PY_CLASS (codewidget->py_variable_array[i]));
            }
        }
        name++; 
    }
}

/*This function is the callback function 
 */
static void
check_import_callback (GObject *source, GAsyncResult *res, gpointer user_data)
{
    display_status_bar_message_return_false = TRUE;
    g_object_unref (res);
}

/*This function is called by _codewidget_import_module_async
 *in a separate thread which will import a module
 */
static void
_check_import_module_thread_func (GSimpleAsyncResult *res, GObject *object, GCancellable *cancellable)
{
    CodeWidget *codewidget;

    codewidget = (CodeWidget *) g_async_result_get_user_data (
        G_ASYNC_RESULT (res));
    
    gchar *ext = strrchr (codewidget->file_path, '.');
    if (g_strcmp0 (ext, ".py"))
        return;

    check_for_import_module (codewidget, NULL, (gchar *) codewidget->data);
}

/*This function will import module from line_text
 *if available, asynchronously by calling 
 *GSimpleAsyncResult.
 */
static void
_codewidget_import_module_async (CodeWidget *codewidget, char *line_text)
{
    if (!line_text || !g_strstr_len (line_text, -1, "import"))
        return;

    codewidget->data = line_text;
    GSimpleAsyncResult *check_import_async_res;
    check_import_async_res = g_simple_async_result_new (NULL,
                                                       check_import_callback,
                                                       codewidget,
                                                       _codewidget_import_module_async);

    /*Call get_class_funcs_thread_func in another thread*/
    g_simple_async_result_run_in_thread (check_import_async_res,
                                         _check_import_module_thread_func,
                                         0, NULL);

     g_timeout_add (100, (GSourceFunc)display_status_bar_message, codewidget);
}

/*To link base classes of a child 
 *class to its parent classes.
 *It will internally call the above
 *function.
 */
void
_codewidget_link_child_classes_to_their_parents (CodeWidget *codewidget)
{
    int i;
    for (i = 0; i < codewidget->py_class_array_size; i++)
    {
        if (codewidget->py_class_array [i]->base_class_names)
            _codewidget_link_class_to_its_parents (codewidget, 
                                                   codewidget->py_class_array [i]);
    }
}

static gboolean
display_status_bar_message (gpointer data)
{
    if (display_status_bar_message_return_false)
        return FALSE;

    gchar *message = (gchar *)g_async_queue_try_pop (async_queue);
    if (!message)
        return TRUE;

    gtk_statusbar_push (GTK_STATUSBAR (status_bar), 0, message);
    g_free (message);
    return TRUE;
}

/*This function will parse current file
 *it will import all the modules, detect all 
 *classes and the functions. All this work is
 *done asynchronously. 
 */

static void
codewidget_get_class_funcs (CodeWidget *codewidget, gchar *text)
{
    /*Create a GSimpleAsyncResult. It will call get_class_func_callback in
     *main thread when its ThreadFunc is completed
     */

    gchar *ext = strrchr (codewidget->file_path, '.');
    if (g_strcmp0 (ext, ".py"))
        return;

    GSimpleAsyncResult *get_class_func_thread ;
    get_class_func_thread = g_simple_async_result_new (NULL,
                                                       get_class_func_callback,
                                                       codewidget,
                                                       codewidget_get_class_funcs);
   
    codewidget->thread_ended = FALSE;
    /*Call get_class_funcs_thread_func in another thread*/
    g_simple_async_result_run_in_thread (get_class_func_thread, 
                                         get_class_funcs_thread_func,
                                         0, NULL);

    g_timeout_add (100, (GSourceFunc)display_status_bar_message, codewidget);
}

/*This function is the callback function
 *for above GSimpleAsyncResult. It will 
 *be called in the main thread. It will
 *setup class_combo_box.
 */

static void
get_class_func_callback (GObject *object, GAsyncResult *res, gpointer data)
{
    CodeWidget *codewidget = (CodeWidget *)data;
    codewidget->thread_ended = TRUE;

    GtkTextBuffer *buffer = gtk_text_view_get_buffer (
        GTK_TEXT_VIEW (codewidget->sourceview));

    int line = 0;
    for (line = 1; line < codewidget->py_class_array_size; line++)
    {
        gtk_combo_box_text_append_text (
            GTK_COMBO_BOX_TEXT (codewidget->class_combobox),
            (PY_VARIABLE (codewidget->py_class_array[line])->name));
    }
    
    /*for (line = 0; line < codewidget->py_variable_array_size; line++)
    {
        printf ("VVV %s %d\n", codewidget->py_variable_array [line]->name, codewidget->py_variable_array [line]->type);
        if (codewidget->py_variable_array [line]->type == MODULE)
        {
            int i;
            for (i = 0; i < PY_MODULE (codewidget->py_variable_array[line])->py_variable_array_size; i++)
                printf ("NNN %s\n", PY_MODULE (codewidget->py_variable_array[line])->py_variable_array[i]->name);
        }
    }*/
    
    gtk_statusbar_push (GTK_STATUSBAR (status_bar), 0, 
                         "Parsing files completed");
    
    display_status_bar_message_return_false = TRUE;

    symbols_view_fill_from_codewidget (SYMBOLS_VIEW (symbols_view), codewidget);

    /*Set Class and Function according to current position of cursor*/
    /*GtkTextIter current_iter;
    if (!GTK_IS_TEXT_VIEW (codewidget->sourceview))
         return;

    if (!buffer)
        return;

    gtk_text_buffer_get_iter_at_mark (buffer, &current_iter,
                                       gtk_text_buffer_get_insert (buffer));

    if (gtk_text_iter_get_line (&current_iter) == 0)
        return;

    gtk_text_buffer_get_iter_at_line (buffer, &current_iter,
                                        gtk_text_iter_get_line (&current_iter)-1);
    gtk_text_buffer_place_cursor (buffer, &current_iter);
    codewidget_mark_set (buffer, &current_iter, NULL, codewidget);
    */
    if (G_IS_OBJECT (res))
        g_object_unref (res);
}

/*This is the function called by
 *GSimpleAsyncResult in another thread.
 *It also calls _codewidget_start_parse_from_line 
 *and
 *_codewidget_link_child_classes_to_their_parents 
 */

static void
get_class_funcs_thread_func (GSimpleAsyncResult *res, GObject *object,
                             GCancellable *cancellable)
{
    CodeWidget *codewidget;

    codewidget = (CodeWidget *) g_async_result_get_user_data (
        G_ASYNC_RESULT (res));

    GtkTextBuffer *buffer = gtk_text_view_get_buffer (
        GTK_TEXT_VIEW (codewidget->sourceview));

    int line = 0;

    while(line < gtk_text_buffer_get_line_count (buffer))
    {
        int _line = _codewidget_get_line_at_multiline_comment_end (buffer, line);
        if (_line != -1 && _line != line)
            line = _line + 1;

        line = _codewidget_start_parse_from_line (codewidget, buffer, line,
                                                    &(codewidget->py_class_array[0]), TRUE);
    }
    
    _codewidget_link_child_classes_to_their_parents (codewidget);
}

static char *
get_line_from_buffer (GtkTextBuffer *buffer, int *_line)
{
    int line = *_line;
    GString *g_string = g_string_new (gtk_text_buffer_get_line_text (buffer, line, TRUE));
    while (count_str_str (g_string->str, g_string->len, "(") != count_str_str (g_string->str, g_string->len, ")"))
    {
        //printf ("%d %d %s\n", line, count_str_str (g_string->str, "("), g_string->str);
        line++;
        g_string_append (g_string, gtk_text_buffer_get_line_text (buffer, line, TRUE));
    }
    return g_string->str;
}

/*This function parses each line. It will detect
 *all the classes and their functions with imported
 *modules. It will import the module and will add it
 *to the codewidget.
 */

static int
_codewidget_start_parse_from_line (CodeWidget *codewidget,
                                   GtkTextBuffer *buffer,
                                   int line, PyClass **_klass, gboolean import_module)
{
    PyClass *klass = *_klass;
    GMatchInfo *match_info_class, *match_info_func;
    int indentation=0, start, end, _line;
    char *line_text = gtk_text_buffer_get_line_text (buffer, line, TRUE);
    static int class = 1;
    gboolean can_find_static = TRUE;

    while (line < gtk_text_buffer_get_line_count (buffer))
    {
        line_text = gtk_text_buffer_get_line_text (buffer, line, TRUE);
        if (strcmp(g_strstrip(line_text), "") == 0 &&
            indentation - get_indent_spaces_in_string (line_text) > 0)
            return line;

        if (import_module)
            check_for_import_module (codewidget, NULL, line_text);

        if (g_regex_match (regex_class, line_text, 0, &match_info_class))
        {
            g_match_info_fetch_pos (match_info_class, 0, &start, &end);
            start += get_line_pos (buffer, line);

            gchar *class_def_string = g_match_info_fetch(match_info_class,0);
            class_def_string = remove_char (class_def_string, ':');

            indentation = get_indent_spaces_in_string (class_def_string) / options.indent_width;
            PyClass *new_class = py_class_new_from_def (class_def_string, 
                                                        start, indentation);

            codewidget_add_class (codewidget, new_class);

            g_free (class_def_string);
            
            if (indentation > 0)
            {
                py_classv_add_py_class (&(klass->nested_classes), &(klass->nested_classes_size), new_class);
            }
            
            /*Detecting for class doc strings*/
            line += 1;
            _line = _codewidget_get_line_at_multiline_comment_end (buffer, line);
            if (_line != line)
            {
                gchar *doc_string = get_doc_string_between_lines (buffer, line, _line);

                py_variable_set_doc_string ((PyVariable *)new_class, doc_string);
                g_free (doc_string);
                line = _line;
            }
           else
                line -= 1;

            line = _codewidget_start_parse_from_line (codewidget, buffer, line + 1, &new_class, import_module);
            class++;
            g_match_info_free (match_info_class);
        }
        else if (g_regex_match (regex_func, line_text ,0, &match_info_func))
        {
            can_find_static = FALSE;
            g_match_info_fetch_pos (match_info_func, 0, &start, &end);
            start += get_line_pos (buffer, line)+1;;

            gchar *func_def_string = g_match_info_fetch(match_info_func,0);

            func_def_string = remove_char (func_def_string, ':');
            indentation = get_indent_spaces_in_string (func_def_string) / options.indent_width;
            PyFunc *py_func = py_func_new_from_def (func_def_string, start,
                                                   indentation);

            if (!py_func)
            {
                g_free (func_def_string);
                line++;
                continue;
            }

            py_funcv_append (&(klass->py_func_array), py_func);
            /*************************/

            g_free (func_def_string);
            g_match_info_free (match_info_func);
            
             /*Detecting for function's doc string*/
            line += 1;
            _line = _codewidget_get_line_at_multiline_comment_end (buffer, line);
            if (_line != line)
            {
                gchar *doc_string = get_doc_string_between_lines (buffer, line, _line);
    
                py_variable_set_doc_string (PY_VARIABLE (py_func), doc_string);
                g_free (doc_string);
            }
            line = _line;

            if (klass && py_func && !g_strcmp0 (PY_VARIABLE (py_func)->name, "__init__"))
            {
                /*Detect __init__ and parse it. As __init__ function is always
                 *executed when object is created, so all instance variable 
                 *declared in __init__ will be added
                 */

                line += 1;
                while (line < gtk_text_buffer_get_line_count (buffer))
                {
                    line_text = gtk_text_buffer_get_line_text (buffer, line, TRUE);
                    int indentation2 = get_indent_spaces_in_string (line_text) / options.indent_width;
                    if (indentation2 == indentation)
                    {
                        line--;
                        break;
                    }

                    if (g_regex_match (regex_self_var, line_text ,0, &match_info_class))
                    {
                        gchar *self_dot_pos = strchr (line_text, '.');
                        self_dot_pos++;
                        PyStaticVar *static_var = py_static_var_new_from_def (self_dot_pos);
                        if (static_var)
                        {
                            int i;
                            for (i = 0; i < klass->py_static_var_array_size; i++)
                            {
                                if (!g_strcmp0 (PY_VARIABLE (static_var)->name, 
                                                 PY_VARIABLE (klass->py_static_var_array[i])->name))
                                    break;
                            }
                            
                            if (i == klass->py_static_var_array_size)
                            {
                                py_static_varv_add_py_static_var (&(klass->py_static_var_array),
                                                                &(klass->py_static_var_array_size), static_var);
                                /*Detecting for static_var's doc string. All though Python
                                  *doesn't support it but static variables has their doc string
                                  *set by user
                                  */
                               
                                line += 1;
                                _line = _codewidget_get_line_at_multiline_comment_end (buffer, line);
                                if (_line != line)
                                {
                                    gchar *doc_string = get_doc_string_between_lines (buffer, line, _line);
                                    py_variable_set_doc_string (PY_VARIABLE (static_var), doc_string);
                                    g_free (doc_string);
                                    line = _line;
                                }
                                else
                                    line -= 1;
                            }
                            else
                                py_static_var_destroy (PY_VARIABLE (static_var));
                        }
                        g_match_info_free (match_info_class);
                    }
                    line += 1;
                }
            line -= 1;                
            }
        }
        else if (can_find_static && g_regex_match (regex_static_var, line_text ,0, &match_info_class))
        {
            PyStaticVar *static_var = py_static_var_new_from_def (line_text);
            if (static_var)
            {
                py_static_varv_add_py_static_var (&(klass->py_static_var_array),
                                                &(klass->py_static_var_array_size), static_var);
                
                 /*Detecting for static_var's doc string. All though Python
                  *doesn't support it but static variables has their doc string
                  *set by user
                  */
                line += 1;
                _line = _codewidget_get_line_at_multiline_comment_end (buffer, line);
                if (_line != line)
                {
                    gchar *doc_string = get_doc_string_between_lines (buffer, line, _line);
                    py_variable_set_doc_string (PY_VARIABLE (static_var), doc_string);
                    g_free (doc_string);
                    line = _line;
                }
                else
                    line -= 1;
            }
            g_match_info_free (match_info_class);
        }
        g_free (line_text);
        line++;
    }
    
    return line + 1;
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
    {
        if (can_combo_class_activate)
            go_to_pos_and_select_line (GTK_TEXT_VIEW (codewidget->sourceview),
                                 codewidget->py_class_array [index]->pos);
        return;
    }

    can_combo_func_activate = FALSE;
    int i = 0;
    while (codewidget->py_class_array [index]->py_func_array [i] != NULL)
        gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (codewidget->func_combobox),
                                       ((PyVariable *) (codewidget->py_class_array [index]->py_func_array [i++]))->name);

    while (gtk_events_pending ())
        gtk_main_iteration_do (FALSE);

    if (can_combo_class_activate)
    {
        can_combo_func_activate = TRUE;
        go_to_pos_and_select_line (GTK_TEXT_VIEW (codewidget->sourceview),
                                  codewidget->py_class_array [index]->pos);
    }
}

/* To remove code_list_view
 * and to grab sourceview
 * focus
 */
static void
_codewidget_hide_code_list_and_grab_source_view (CodeWidget *codewidget)
{
    if (gtk_widget_get_parent (codewidget->code_assist_widget->parent) != NULL)
             gtk_container_remove (GTK_CONTAINER (gtk_widget_get_parent (codewidget->code_assist_widget->parent)),
                                     codewidget->code_assist_widget->parent);
    code_assist_clear (codewidget->code_assist_widget);
    //gtk_widget_grab_focus (codewidget->sourceview);
}

/* To show code_list_view at desired position
 * it will adjust the coordinates of list view.
 */
static gboolean 
_codewidget_show_code_list_view (CodeWidget *codewidget)
{
    gchar *ext = strrchr (codewidget->file_path, '.');
    if (g_strcmp0 (ext, ".py"))
        return;
    
    /*if (codewidget->code_assist_widget && 
        GTK_IS_WIDGET (codewidget->code_assist_widget->parent) &&
        gtk_widget_get_parent (codewidget->code_assist_widget->parent) != NULL)
            _codewidget_hide_code_list_and_grab_source_view (codewidget);*/
    
    while (gtk_events_pending ())
        gtk_main_iteration_do (FALSE);

    GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (codewidget->code_assist_widget->list_view));
    GtkTreeIter tree_iter;
    gtk_tree_model_get_iter_first (GTK_TREE_MODEL (codewidget->code_assist_widget->list_store), &tree_iter);

    if (!gtk_list_store_iter_is_valid (codewidget->code_assist_widget->list_store, &tree_iter))
    {
        return FALSE;
    }
    
    //gtk_widget_grab_focus (codewidget->code_assist_widget->list_view);
    gtk_tree_selection_select_iter (selection, &tree_iter);
    
    if (gtk_widget_get_parent (codewidget->code_assist_widget->parent))
       goto end;

    GtkTextIter iter;
    GtkTextBuffer *buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (codewidget->sourceview));
    gtk_text_buffer_get_iter_at_mark (buffer, &iter, gtk_text_buffer_get_insert (buffer));
    int pos = gtk_text_iter_get_offset (&iter);

    GdkRectangle rect;
    int window_x, window_y;
    gtk_text_view_get_iter_location (GTK_TEXT_VIEW (codewidget->sourceview), &iter, &rect);

    gtk_text_view_buffer_to_window_coords (GTK_TEXT_VIEW (codewidget->sourceview), 
                                           GTK_TEXT_WINDOW_TEXT,
                                           rect.x + rect.width, rect.y + rect.height, 
                                           &window_x, &window_y);
    
    /*To check if available height and width is available to show list view
     * if not then reduce code list view's height and width
     */
    
    GdkRectangle visible_rect;
    gtk_text_view_get_visible_rect (GTK_TEXT_VIEW (codewidget->sourceview), &visible_rect);
    int codewidget_width, codewidget_height;
    codewidget_width = gtk_widget_get_allocated_width (codewidget->vbox);
    codewidget_height = gtk_widget_get_allocated_width (codewidget->vbox);

    if (visible_rect.width < window_x + CODE_ASSIST_WIDTH)
        window_x -= CODE_ASSIST_WIDTH;
    
    if (visible_rect.height < window_y + CODE_ASSIST_HEIGHT)
        window_y -= CODE_ASSIST_HEIGHT + 2*rect.height;

    gtk_text_view_add_child_in_window (GTK_TEXT_VIEW (codewidget->sourceview),
                                        codewidget->code_assist_widget->parent,
                                        GTK_TEXT_WINDOW_TEXT,
                                        window_x, window_y);

     /* Record the pos where, code_list_view is shown, 
     * "+ 1" because this is key_press not key_release 
    */

end:
     code_list_show_pos = pos;

     gtk_widget_show_all (codewidget->code_assist_widget->parent);
    return TRUE;
}

void
_codewidget_add_class_funcs_to_code_assist (CodeWidget *codewidget, PyClass *class)
{
    int i;
    for (i = 0; i < class->py_static_var_array_size; i++)
    {
        PyStaticVar *var = class->py_static_var_array [i];
        code_assist_add_py_var (codewidget->code_assist_widget, PY_VARIABLE (var));
    }

    PyFunc **py_func = class->py_func_array;
    if (py_func)
    {
        while (*py_func)
        {
            code_assist_add_py_var (codewidget->code_assist_widget, PY_VARIABLE (*py_func));
            py_func++;
         }
    }

    for (i = 0; i < class->base_classes_size; i++)
        _codewidget_add_class_funcs_to_code_assist (codewidget, class->base_classes[i]);
}

/* This function will show the code_assist on the basis of matches
 */
static void
show_auto_completion (CodeWidget *codewidget, char *line_text, int pos, gboolean dot_entered)
{
    if (!codewidget->file_path)
        return;

    gchar *ext = strrchr (codewidget->file_path, '.');
    if (g_strcmp0 (ext, ".py"))
        return;

    GMatchInfo *match_info;

    gchar *text = line_text + strlen(line_text) - 1;
    for (; text >= line_text; text--)
    {
        if (!isalnum (*text) && *text != '_' && *text != '.')
            break;
    }
    text++;

    if (dot_entered && !g_strcmp0 (text, "self"))
    {
        /*Found "self."*/
        /*Populate list store*/

        int curr_class_index = gtk_combo_box_get_active (GTK_COMBO_BOX (codewidget->class_combobox));
        code_assist_clear (codewidget->code_assist_widget);
        _codewidget_add_class_funcs_to_code_assist (codewidget,
                                                codewidget->py_class_array [curr_class_index]);
        _codewidget_show_code_list_view (codewidget);
    }
    else if (dot_entered)
    {
        /*If dot is entered then there can be two cases, may be module name is 
         *<module>.<module/class/function> or it may be just <module> which
         *contains other children.
         */
        int i;
        PyVariable *parent_py_var = NULL;
        for (i = 0; i < codewidget->py_variable_array_size; i++)
        {
            gchar *name = codewidget->py_variable_array [i]->name;
            if (!g_strcmp0 (name, text))
            {
                /*If name is like <module>.<module/class/function>*/
                parent_py_var = codewidget->py_variable_array [i];
                break;
            }
        }

        if (i == codewidget->py_variable_array_size)
        {          
            /*It may be just <module> which
             *contains other children.
             */
            /*Split text on the basis of "." and navigate through variable array
             *to find which module it is 
             */
    
            gchar **text_split = g_strsplit (text, ".", 0);
            gchar **_split = text_split;
            gboolean found_module = FALSE;
    
            for (i = 0; i < codewidget->py_variable_array_size; i++)
            {
                gchar *name = codewidget->py_variable_array [i]->name;
                if (!g_strcmp0 (name, _split [0]))
                    break;
            }
    
            _split++;
            if (i < codewidget->py_variable_array_size)
            {
                parent_py_var = codewidget->py_variable_array [i];
                code_assist_clear (codewidget->code_assist_widget);
                while (*_split)
                {
                    if (parent_py_var->type != MODULE)
                        break;
    
                    for (i = 0; i < PY_MODULE (parent_py_var)->py_variable_array_size; i++)
                    {
                        if (!g_strcmp0 (PY_MODULE (parent_py_var)->py_variable_array [i]->name, _split [0]))
                            break;
                    }
                    if (i == PY_MODULE (parent_py_var)->py_variable_array_size)
                        break;
    
                    parent_py_var = PY_MODULE (parent_py_var)->py_variable_array [i];
                    _split++;
                }
                g_strfreev (text_split);
            }
        }
        
        if (parent_py_var)
        {
            for (i = 0; i < PY_MODULE (parent_py_var)->py_variable_array_size; i++)
            {
                PyVariable *py_var = PY_MODULE (parent_py_var)->py_variable_array [i];
                code_assist_add_py_var (codewidget->code_assist_widget, py_var);
            }
            _codewidget_show_code_list_view (codewidget);
        }
    }
    else if (g_regex_match (regex_word, line_text, 0, &match_info))
    {
        gchar *word = g_match_info_fetch (match_info, 0);
        if (g_strstr_len (word, -1, "self.") == word)
        {
            gchar *after_self = word + 5; /*strlen ("self.") is 5*/
            if (code_assist_show_matches (codewidget->code_assist_widget,
                                           after_self))
                 goto end;
        }

        if (strlen (word) < 4)
        {
            _codewidget_hide_code_list_and_grab_source_view (codewidget);
            goto end;
        }
        code_assist_clear (codewidget->code_assist_widget);
        int i;
        /*Add all PyVariables*/
        for (i = 0; i <codewidget->py_variable_array_size; i++)
        {
            PyVariable *py_var = codewidget->py_variable_array [i];
            if (g_strstr_len (py_var->name, -1, word))
            {
                 code_assist_add_py_var (codewidget->code_assist_widget, py_var);
            }
        }
        
        /*Add all classes*/
        for (i = 0; i < codewidget->py_class_array_size; i++)
        {
            PyVariable *py_var = PY_VARIABLE (codewidget->py_class_array [i]);
            if (g_strstr_len (py_var->name, -1, word))
            {
                 code_assist_add_py_var (codewidget->code_assist_widget, py_var);
            }
        }

        /*Add all global variables*/
        for (i = 0; i < codewidget->py_class_array [0]->py_static_var_array_size; i++)
        {
            PyVariable *py_var = PY_VARIABLE (codewidget->py_class_array [0]->py_static_var_array[i]);
            if (g_strstr_len (py_var->name, -1, word))
            {
                 code_assist_add_py_var (codewidget->code_assist_widget, py_var);
            }
        }

        /*Add all global functions*/
        PyFunc **py_func = codewidget->py_class_array [0]->py_func_array;
        if (py_func)
        {
            while (*py_func)
            {
                if (g_strstr_len (PY_VARIABLE (py_func)->name, -1, word))
                    code_assist_add_py_var (codewidget->code_assist_widget, PY_VARIABLE (*py_func));
                py_func++;
            }
        }

        /*Add all variables defined in function*/
        for (i = 0; i < codewidget->curr_func_static_var_array_size; i++)
        {
            PyVariable *py_var = PY_VARIABLE (codewidget->curr_func_static_var_array [i]);
            if (g_strstr_len (py_var->name, -1, word))
            {
                 code_assist_add_py_var (codewidget->code_assist_widget, py_var);
            }
        }
        
        /*Add arguments of current function*/
        int class = gtk_combo_box_get_active (GTK_COMBO_BOX (codewidget->class_combobox));
        int func = gtk_combo_box_get_active (GTK_COMBO_BOX (codewidget->func_combobox));
        
        if (class != -1 && func != -1)
        {
            PyStaticVar **varv = codewidget->py_class_array [class]->py_func_array [func]->arg_array;
            int varc = codewidget->py_class_array [class]->py_func_array [func]->arg_array_size;
    
            for (i = 0; i < varc; i++)
            {
                PyVariable *py_var = PY_VARIABLE (varv [i]);
                if (g_strstr_len (py_var->name, -1, word))
                {
                     code_assist_add_py_var (codewidget->code_assist_widget, py_var);
                }
            }
        }

        if (_codewidget_show_code_list_view (codewidget))
            code_list_show_pos = pos - strlen (word) - 1;
end:
        g_free (word);
        g_match_info_free (match_info);
    }
}

static gboolean
codewidget_key_release (GtkWidget *widget, GdkEvent *event, gpointer data)
{
    CodeWidget *codewidget = (CodeWidget *)data;
    GtkTextIter iter, new_iter;
    GtkTextBuffer *buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (widget));
    int indentation, i, col, new_col, line, pos;
    gtk_text_buffer_get_iter_at_mark (buffer, &iter, gtk_text_buffer_get_insert (buffer));
    gchar *line_text = gtk_text_buffer_get_line_text (buffer, 
                                                      gtk_text_iter_get_line (&iter), TRUE);
    col = gtk_text_iter_get_line_offset (&iter);
    line = gtk_text_iter_get_line (&iter);
    pos = gtk_text_iter_get_offset (&iter);
    
    gchar line_text_till_col [col];
    for (i = 0; i <col-1; i++)
        line_text_till_col [i] = line_text[i];
    line_text_till_col [i] = '\0';

    if (event->key.keyval == GDK_KEY_KP_Decimal || event->key.keyval == 46)
    {
        show_auto_completion (codewidget, line_text_till_col, pos, TRUE);
    }
    else if (event->key.keyval != GDK_KEY_Down &&
              event->key.keyval != GDK_KEY_Left &&
              event->key.keyval != GDK_KEY_Return &&
              event->key.keyval != GDK_KEY_Home &&
              event->key.keyval != GDK_KEY_Up &&
              event->key.keyval != GDK_KEY_Escape &&
              event->key.keyval != GDK_KEY_Right)
    {
        show_auto_completion (codewidget, line_text_till_col, pos, FALSE);
    }

    return FALSE;
}

static char *
get_line_text_from_buffer (GtkTextBuffer *buffer, int *_line)
{
    int line = *_line;
    
    GString *g_string = g_string_new (gtk_text_buffer_get_line_text (buffer, line, TRUE));
    while (count_str_str (g_string->str, g_string->len, "(") != count_str_str (g_string->str, g_string->len, ")"))
    {
        line++;
        g_string = g_string_append (g_string, gtk_text_buffer_get_line_text (buffer, line, TRUE));
    }
    //printf ("START%sEND\n", g_string->str);
    return g_string->str;
}

/*This function will parse the lines of 
 *function of this class.
 */
static void
_codewidget_parse_function (CodeWidget *codewidget, PyClass *klass, PyFunc *func, int curr_line)
{
    GtkTextIter iter, new_iter;
    GtkTextBuffer *buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (codewidget->sourceview));
    gtk_text_buffer_get_iter_at_offset (buffer, &iter, func->pos);
    int line = gtk_text_iter_get_line (&iter);
    char *line_text;

    int i;
    for (i = 0; i < codewidget->curr_func_static_var_array_size; i++)
        py_static_var_destroy (PY_VARIABLE (codewidget->curr_func_static_var_array [i]));
    
    g_free (codewidget->curr_func_static_var_array);

    codewidget->curr_func_static_var_array = NULL;
    codewidget->curr_func_static_var_array_size = 0;

    GMatchInfo *match_info = NULL;

    while (line <= curr_line)
    {
        line_text = gtk_text_buffer_get_line_text (buffer, line, TRUE);
        if (g_regex_match (regex_local_var, line_text, 0, &match_info))
        {
            PyStaticVar *static_var = py_static_var_new_from_def (line_text);
            if (static_var)
            {
                int i;
                for (i = 0; i < codewidget->curr_func_static_var_array_size; i++)
                {
                    if (!g_strcmp0 (PY_VARIABLE (static_var)->name, 
                                     PY_VARIABLE (codewidget->curr_func_static_var_array[i])->name))
                        break;
                }
                
                if (i == codewidget->curr_func_static_var_array_size)
                {
                    py_static_varv_add_py_static_var (&(codewidget->curr_func_static_var_array),
                                                    &(codewidget->curr_func_static_var_array_size), static_var);
                }
                else
                     py_static_var_destroy (PY_VARIABLE (static_var));
            }
        }
        g_free (line_text);
        line ++;
    }
    g_match_info_free (match_info);
}

/*Event Handler for
 *"key-press-event"
 */
static gboolean
codewidget_key_press (GtkWidget *widget, GdkEvent *event, gpointer data)
{
    CodeWidget *codewidget = (CodeWidget *)data;
    GtkTextIter iter, new_iter;
    GtkTextBuffer *buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (widget));
    int indentation, i, col, new_col, line, pos;
    gtk_text_buffer_get_iter_at_mark (buffer, &iter, gtk_text_buffer_get_insert (buffer));
    gchar *line_text = gtk_text_buffer_get_line_text (buffer, 
                                                      gtk_text_iter_get_line (&iter), TRUE);
    col = gtk_text_iter_get_line_offset (&iter);
    line = gtk_text_iter_get_line (&iter);
    pos = gtk_text_iter_get_offset (&iter);
    
    gchar line_text_till_col [col+1];
    for (i = 0; i <col; i++)
        line_text_till_col [i] = line_text[i];
    line_text_till_col [i] = '\0';

    switch (event->key.keyval)
    {
        case GDK_KEY_Return:
            {
                //int first_open_pos;
                //first_open_pos = gtk_text_buffer_get_first_open_unmatched_parenthesis_pos (buffer, pos);
                //if (first_open_pos != -1)
                    //indentation = first_open_pos + 1;
               // else
                indentation = get_indent_spaces_in_string (line_text_till_col);
                /*Search for ':' in line and increase indentation if found otherwise not*/
                if (g_strrstr (line_text_till_col, ":"))
                    indentation += options.indent_width;
                
                gchar *line_after_col = &line_text [col+1];
                indentation -= get_indent_spaces_in_string (line_after_col);
                gtk_text_buffer_insert (buffer, &iter, "\n", 1);
                for (i = 1; i <= indentation; i++)
                    gtk_text_buffer_insert (buffer, &iter, " ", 1);

                return TRUE;
            }
        
        case GDK_KEY_Left:
            indentation = get_indent_spaces_in_string (line_text);
            
            if (col > indentation || col == 0)
                return FALSE;
                
            if (col % options.indent_width == 0)
                new_col = col - options.indent_width;
            else
                new_col = col - col % options.indent_width;

            gtk_text_buffer_get_iter_at_line_offset (buffer, &new_iter, line, new_col);
            gtk_text_buffer_place_cursor (buffer, &new_iter);
            
            return TRUE;
        
        case GDK_KEY_BackSpace:
            indentation = get_indent_spaces_in_string (line_text);
            if (indentation == 0)
                return FALSE;

            if (col > indentation || col == 0)
                return FALSE;
                
            if (col % options.indent_width == 0)
                new_col = col - options.indent_width;
            else
                new_col = col - col % options.indent_width;

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

    if (event->key.keyval == GDK_KEY_Escape)
    {
        _codewidget_hide_code_list_and_grab_source_view (codewidget);
    }
    else if (event->key.keyval == GDK_KEY_Up)
    {
        if (gtk_widget_get_parent (codewidget->code_assist_widget->parent) != NULL)
        {
            /* Move current selection of code_list_view up */
            GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (codewidget->code_assist_widget->list_view));
            GtkTreeIter iter;
            
            gtk_tree_selection_get_selected (selection, NULL, &iter);
            if (gtk_list_store_iter_is_valid (codewidget->code_assist_widget->list_store, &iter) &&
                 gtk_tree_model_iter_previous (GTK_TREE_MODEL (codewidget->code_assist_widget->list_store), &iter))
             {
                GtkTreePath *path = gtk_tree_model_get_path (GTK_TREE_MODEL (codewidget->code_assist_widget->list_store), &iter);
                gtk_tree_selection_select_iter (selection, &iter);
                gtk_tree_view_scroll_to_cell (GTK_TREE_VIEW (codewidget->code_assist_widget->list_view), path, NULL, FALSE, 0, 0);
                gtk_tree_path_free (path);
            }

            return TRUE;
        }
    }
    else if (event->key.keyval == GDK_KEY_Down)
    {
        if (gtk_widget_get_parent (codewidget->code_assist_widget->parent) != NULL)
        {
            /* Move current selection of code_list_view down */
            GtkTreeSelection *selection = gtk_tree_view_get_selection (
                GTK_TREE_VIEW (codewidget->code_assist_widget->list_view));

            GtkTreeIter iter;
            gtk_tree_selection_get_selected (selection, NULL, &iter);
            if (gtk_list_store_iter_is_valid (codewidget->code_assist_widget->list_store, &iter) &&
                 gtk_tree_model_iter_next (GTK_TREE_MODEL (codewidget->code_assist_widget->list_store), &iter))
            {
                GtkTreePath *path = gtk_tree_model_get_path (GTK_TREE_MODEL (codewidget->code_assist_widget->list_store), &iter);
                gtk_tree_selection_select_iter (selection, &iter);
                GtkTreePath *start, *end;
                gtk_tree_view_get_visible_range (GTK_TREE_VIEW (codewidget->code_assist_widget->list_view), &start, &end);

                /*Do not scroll to path if it is already visible*/
                if (start && end &&
                    gtk_tree_path_compare (start, path) != -1 ||
                    gtk_tree_path_compare (path, end) != -1);
                    gtk_tree_view_scroll_to_cell (GTK_TREE_VIEW (codewidget->code_assist_widget->list_view), path, NULL, FALSE, 0, 0);
 
                gtk_tree_path_free (path);
                gtk_tree_path_free (start);
                gtk_tree_path_free (end);
            }

            return TRUE;
        }
    }
    else if (event->key.keyval == GDK_KEY_Tab)
    {
        if (gtk_widget_get_parent (codewidget->code_assist_widget->parent) != NULL)
        {
            _codewidget_code_list_row_activated (NULL, NULL, NULL, codewidget);
            return TRUE;
        }
    }

    return FALSE;
}

static void
_codewidget_fm_box_clicked (GtkWidget *widget, FileModifyBoxResponse response, gpointer data)
{
    CodeWidget *codewidget = (CodeWidget *)data;
    gtk_container_remove (GTK_CONTAINER (codewidget->vbox), codewidget->file_modify_box);
    codewidget->file_modify_box = NULL;

    if (response == FILE_MODIFY_BOX_RESPONSE_YES)
    {
        gchar *file_str = get_file_data (codewidget->file_path);
        //codewidget_set_text (codewidget, file_str);
        gtk_text_buffer_set_text (GTK_TEXT_BUFFER (codewidget->sourcebuffer), file_str, -1);
        codewidget_update_class_funcs (codewidget);
    }
}

void
 codewidget_show_modified_dialog (CodeWidget *codewidget)
{
    if (codewidget->file_modify_box)
        return;

    codewidget->file_modify_box = file_modify_box_new ();
    gtk_box_pack_start (GTK_BOX (codewidget->vbox), codewidget->file_modify_box, FALSE, FALSE, 2);
    gtk_box_reorder_child (GTK_BOX (codewidget->vbox), codewidget->file_modify_box, 0);
    g_signal_connect (G_OBJECT (codewidget->file_modify_box), "clicked", G_CALLBACK (_codewidget_fm_box_clicked), codewidget);
    gtk_widget_show_all (codewidget->vbox);
}