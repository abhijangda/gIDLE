#include "codewidget.h"
#include "core_funcs.h"
#include "main.h"
#include <string.h>

extern GtkWidget *status_bar;
extern char *font_name;

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
_codewidget_move_cursor_to_screen (CodeWidget *codewidget);

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
    PangoFontDescription *font_desc;

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
    font_desc = pango_font_description_from_string (font_name);
    gtk_widget_modify_font (codewidget->sourceview, font_desc);
    pango_font_description_free (font_desc);

    g_signal_connect (G_OBJECT (codewidget->sourceview), "key-press-event",
                     G_CALLBACK (codewidget_key_press), NULL); 
    
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
                                 10, 100);

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
    
    if (!show_line_numbers)
         return;
    
    GtkTextBuffer *buffer = GTK_TEXT_BUFFER (codewidget->sourcebuffer);
    GdkRectangle location;
    GtkTextIter rect_start_iter, rect_end_iter, current_iter;
    int i, last_line;
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
            cairo_select_font_face (cr, "sans-serif", CAIRO_FONT_SLANT_NORMAL,
                                   CAIRO_FONT_WEIGHT_BOLD);
        else
            cairo_select_font_face (cr, "sans-serif", CAIRO_FONT_SLANT_NORMAL,
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
void
codewidget_mark_set (GtkTextBuffer *buffer, GtkTextIter *iter, GtkTextMark *mark, gpointer data)
{
    GtkTextIter current_iter, start_bound_iter, end_bound_iter;
    gtk_text_buffer_get_iter_at_mark (buffer, &current_iter, gtk_text_buffer_get_insert (buffer));

    int chars = gtk_text_iter_get_offset (&current_iter);
    int col = gtk_text_iter_get_line_offset (&current_iter);
    int line = gtk_text_iter_get_line (&current_iter);
    int first_line = -1, last_line = -1;
    
    GdkRectangle visible_rect;
    CodeWidget *codewidget = (CodeWidget *)data;

    gchar *status_bar_msg;
    asprintf (&status_bar_msg, "Chars %d Col %d Line %d", chars+1, col+1, line+1);
    gtk_statusbar_push (GTK_STATUSBAR (status_bar), 0, status_bar_msg);

    if (line != codewidget->prev_line)
    {
        _codewidget_move_cursor_to_screen (codewidget);
        
        if (gtk_text_buffer_get_has_selection (buffer))
        {
            codewidget->prev_line = -1;
            return;
        }

        //printf ("line %d\n", line);
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
                    gtk_combo_box_set_active (GTK_COMBO_BOX (codewidget->func_combobox), -1);

            }

            can_combo_func_activate = TRUE;
            can_combo_class_activate = TRUE;
        }
    }
    
    codewidget->prev_line = line;
}

static void
_codewidget_move_cursor_to_screen (CodeWidget *codewidget)
{
    if (!GTK_IS_TEXT_VIEW (codewidget->sourceview))
        return;
    
    GdkRectangle visible_rect;
    GtkTextBuffer *buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (codewidget->sourceview));
    GtkTextIter current_iter, start_bound_iter, end_bound_iter;
    
    gtk_text_buffer_get_iter_at_mark (buffer, &current_iter, gtk_text_buffer_get_insert (buffer));
    
    int line = gtk_text_iter_get_line (&current_iter);
    
    gtk_text_view_get_visible_rect (GTK_TEXT_VIEW (codewidget->sourceview),
                                    &visible_rect);
     gtk_text_view_get_iter_at_location (GTK_TEXT_VIEW (codewidget->sourceview),
                                       &start_bound_iter, visible_rect.x, visible_rect.y);
    
    gtk_text_view_get_iter_at_location (GTK_TEXT_VIEW (codewidget->sourceview),
                                       &end_bound_iter, visible_rect.x, visible_rect.y + visible_rect.height);
    
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
        gdk_window_invalidate_rect (gtk_widget_get_window (codewidget->line_num_widget), NULL,
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
}

void
print_nested_array (PyClass **py_classv, int index, int size)
{
    if (index == size)
        return;

    int i;
    printf ("Inside class %s\n", py_classv [index]->name);
    for (i = 0; i < py_classv [index]->nested_classes_size; i++)
    {
        printf ("Class %s size %d\n", py_classv [index]->nested_classes [i]->name, py_classv [index]->nested_classes [i]->nested_classes_size);
        print_nested_array (py_classv [index]->nested_classes [i]->nested_classes, 0, py_classv [index]->nested_classes [i]->nested_classes_size);
    }
    printf ("Outside class %s\n", py_classv[index]->name);
    print_nested_array (py_classv, ++index, size);
}


void
codewidget_update_class_funcs (CodeWidget *codewidget)
{
    gchar *code_widget_str;
    GtkTextBuffer *text_buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (
                                                                    codewidget->sourceview));
    GtkTextIter start_iter, end_iter;
    
    gtk_text_buffer_get_start_iter (text_buffer, &start_iter);
    gtk_text_buffer_get_end_iter (text_buffer, &end_iter);
    
    code_widget_str = gtk_text_buffer_get_text (text_buffer, &start_iter,
                                                                                  &end_iter, TRUE);
    int i;
    for (i = 1; i < codewidget->py_class_array_size; i++)
        py_class_destroy (codewidget->py_class_array [i]);
    
    //g_free (codewidget->py_class_array);
    //g_free (codewidget->py_nested_class_array);
    
    for (i = codewidget->py_class_array_size; i > 0; i--)
        gtk_combo_box_text_remove (GTK_COMBO_BOX_TEXT (codewidget->class_combobox), i);

    codewidget->py_class_array_size = 1;
    codewidget->py_nested_class_array_size = 1;
    
    codewidget_get_class_funcs (codewidget, code_widget_str);
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

static void
codewidget_get_class_funcs (CodeWidget *codewidget, gchar *text)
{
    GRegex *regex_class, *regex_comments, *regex_func;
    GRegex *regex_str, *regex_mutlistr;
    GMatchInfo *match_info_class,*match_info_func;
    GMatchInfo *match_info_comments, *match_info_str, *match_info_mutlistr;
    int indentation=0, start, end;
    int **str_comments_pos = NULL, str_comments_pos_size = 0;
    
    /*Find all the comments and add to str_comments_pos*/
    regex_comments = g_regex_new ("#+.+", 0, 0, NULL);
    
    if (g_regex_match (regex_comments, text, 0, &match_info_comments))
    {
        do
        {
            str_comments_pos = g_realloc (str_comments_pos, 
                                          sizeof (int *) * (str_comments_pos_size + 1));
            str_comments_pos [str_comments_pos_size] = g_malloc (sizeof (int) *2);

            g_match_info_fetch_pos (match_info_comments, 0, 
                                   &(str_comments_pos [str_comments_pos_size][0]) , 
                                   &(str_comments_pos [str_comments_pos_size][1]));

            g_match_info_next (match_info_comments, NULL);
            str_comments_pos_size++;
        }
        while (g_match_info_matches (match_info_comments));
    }
    
    g_match_info_free (match_info_comments);
    g_regex_unref (regex_comments);
    
    /* Find all the strings*/
    regex_str = g_regex_new ("['\"][^'\"]*.+['\"]", 0, 0, NULL);
    
    if (g_regex_match (regex_str, text, 0, &match_info_str))
    {
        do
        {
            str_comments_pos = g_realloc (str_comments_pos, 
                                          sizeof (int *) * (str_comments_pos_size + 1));
            str_comments_pos [str_comments_pos_size] = g_malloc (sizeof (int) *2);

            g_match_info_fetch_pos (match_info_str, 0, 
                                   &(str_comments_pos [str_comments_pos_size][0]) , 
                                   &(str_comments_pos [str_comments_pos_size][1]));

            g_match_info_next (match_info_str, NULL);
            str_comments_pos_size++;
        }
        while (g_match_info_matches (match_info_str));
    }
    int i;
    for (i = 0; i <str_comments_pos_size; i++)
    {
        printf ("%d %d\n", str_comments_pos [i][0], str_comments_pos[i][1]);
    }
    
    g_match_info_free (match_info_str);
    g_regex_unref (regex_str);
    
    /* Find all the multiline strings with single quotes*/
    regex_str = g_regex_new ("'''.+?'''", G_REGEX_DOTALL, 0, NULL);
    
    if (g_regex_match (regex_str, text, 0, &match_info_str))
    {
        do
        {
            str_comments_pos = g_realloc (str_comments_pos, 
                                          sizeof (int *) * (str_comments_pos_size + 1));
            str_comments_pos [str_comments_pos_size] = g_malloc (sizeof (int) *2);

            g_match_info_fetch_pos (match_info_str, 0, 
                                   &(str_comments_pos [str_comments_pos_size][0]) , 
                                   &(str_comments_pos [str_comments_pos_size][1]));

            g_match_info_next (match_info_str, NULL);
            str_comments_pos_size++;
        }
        while (g_match_info_matches (match_info_str));
    }
    
    g_match_info_free (match_info_str);
    g_regex_unref (regex_str);
    
    /* Find all the multiline strings with double quotes*/
    regex_str = g_regex_new ("\"\"\".+?\"\"\"", G_REGEX_DOTALL, 0, NULL);
    
    if (g_regex_match (regex_str, text, 0, &match_info_str))
    {
        do
        {
            str_comments_pos = g_realloc (str_comments_pos, 
                                          sizeof (int *) * (str_comments_pos_size + 1));
            str_comments_pos [str_comments_pos_size] = g_malloc (sizeof (int) *2);

            g_match_info_fetch_pos (match_info_str, 0, 
                                   &(str_comments_pos [str_comments_pos_size][0]) , 
                                   &(str_comments_pos [str_comments_pos_size][1]));

            g_match_info_next (match_info_str, NULL);
            str_comments_pos_size++;
        }
        while (g_match_info_matches (match_info_str));
    }
    
    g_match_info_free (match_info_str);
    g_regex_unref (regex_str);

    /*int i;
    for (i = 0; i <str_comments_pos_size; i++)
    {
        printf ("%d %d\n", str_comments_pos [i][0], str_comments_pos[i][1]);
    }*/

    /*getting class names and their positions*/
    regex_class = g_regex_new ("[ \\ \\t]*\\bclass\\b\\s*\\w+\\s*\\(*.*\\)*:", 0, 0, NULL);
    
    if (g_regex_match (regex_class, text, 0, &match_info_class))
    {
        int class = 1;
        do
        {
            g_match_info_fetch_pos (match_info_class, 0, &start, &end);
            
            /* If found class lies in between comments or strings then continue*/
            if (is_pos_in_array (start, end, str_comments_pos, 
                                str_comments_pos_size))
            {
                g_match_info_next (match_info_class, NULL);
                continue;
            }

            gchar *class_def_string = g_match_info_fetch(match_info_class,0);

            class_def_string = remove_char (class_def_string, ':');
            indentation = get_indent_spaces_in_string (class_def_string) / indent_width;
            codewidget_add_class (codewidget,
                                             py_class_new_from_def (class_def_string,
                                             start, indentation));
            g_free (class_def_string);
            g_match_info_next (match_info_class, NULL);
            //printf ("class %d\n",class);// codewidget->py_class_array [class]->name);
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
    
    regex_func = g_regex_new ("[ \\ \\t]*def\\s+[\\w\\d_]+\\s*\\(.+\\)\\:", 0, 0, NULL); 
    
    if (g_regex_match (regex_func, text ,0, &match_info_func))
    {
        do
        {
            g_match_info_fetch_pos (match_info_func, 0, &start, &end);
            gchar *func_def_string = g_match_info_fetch(match_info_func,0);

            /* If found function lies in between comments or strings then continue*/
            if (is_pos_in_array (start, end, str_comments_pos, 
                                str_comments_pos_size))
            {
                g_match_info_next (match_info_func, NULL);
                continue;
            }

            func_def_string = remove_char (func_def_string, ':');
            indentation = get_indent_spaces_in_string (func_def_string) / indent_width;
            PyFunc *py_func = py_func_new_from_def (func_def_string, start,
                                                   indentation);

            if (!py_func)
            {
                g_free (func_def_string);
                g_match_info_next (match_info_func, NULL);
                if (g_match_info_matches (match_info_func))
                    continue;
                else
                   break;
            }
            
            /*Add py_func to desired py_class*/
            int i = -1;
            int class = codewidget->py_class_array_size - 1;
            for (; class >= 0; class --)
            {
                if (codewidget->py_class_array [class]->pos < start &&
                   codewidget->py_class_array [class]->indentation + 1 == indentation)
                    break;
            }
            if (class == -1)
                class = 0;
            
            py_funcv_append (&(codewidget->py_class_array [class]->py_func_array),
                            py_func);
            /*************************/

            g_free (func_def_string);
            g_match_info_next (match_info_func, NULL);
        }
        while (g_match_info_matches (match_info_func));
    }
    /*Now, add nested classes in to their parents*/     
    convert_py_class_array_to_nested_class_array (&(codewidget->py_nested_class_array), 
                                                  &(codewidget->py_nested_class_array_size),
                                                  codewidget->py_class_array,
                                                  codewidget->py_class_array_size,
                                                  0);
    
    /*Set Class and Function according to current position of cursor*/
    GtkTextIter current_iter;
    if (!GTK_IS_TEXT_VIEW (codewidget->sourceview))
         return;

    GtkTextBuffer *buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (codewidget->sourceview));
    if (!buffer)
        return;

    gtk_text_buffer_get_iter_at_mark (buffer, &current_iter, gtk_text_buffer_get_insert (buffer));
    if (gtk_text_iter_get_line (&current_iter) == 0)
        return;

    gtk_text_buffer_get_iter_at_line (buffer, &current_iter,
                                        gtk_text_iter_get_line (&current_iter)-1);
    gtk_text_buffer_place_cursor (buffer, &current_iter);
    codewidget_mark_set (buffer, &current_iter, NULL, codewidget);
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
    int indentation, i, col, new_col, line, pos;
    gtk_text_buffer_get_iter_at_mark (buffer, &iter, gtk_text_buffer_get_insert (buffer));
    gchar *line_text = gtk_text_buffer_get_line_text (buffer, 
                                                       gtk_text_iter_get_line (&iter), TRUE);
    col = gtk_text_iter_get_line_offset (&iter);
    line = gtk_text_iter_get_line (&iter);
    pos = gtk_text_iter_get_offset (&iter);

    switch (event->key.keyval)
    {
        case GDK_KEY_Return:
            {
                //int first_open_pos;
                //first_open_pos = gtk_text_buffer_get_first_open_unmatched_parenthesis_pos (buffer, pos);
                //if (first_open_pos != -1)
                    //indentation = first_open_pos + 1;
               // else
                indentation = get_indent_spaces_in_string (line_text);
                /*Search for ':' in line and increase indentation if found otherwise not*/
                if (g_strrstr (line_text, ":"))
                    indentation += indent_width;
                
                gtk_text_buffer_insert (buffer, &iter, "\n", 1);
                for (i = 1; i <= indentation; i++)
                    gtk_text_buffer_insert (buffer, &iter, " ", 1);
    
                return TRUE;
            }
        
        case GDK_KEY_Left:
            indentation = get_indent_spaces_in_string (line_text);
            
            if (col > indentation || col == 0)
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

            if (col > indentation || col == 0)
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
    
    if (event->key.keyval == GDK_KEY_KP_Decimal)
    {
        
    }
    return FALSE;
}