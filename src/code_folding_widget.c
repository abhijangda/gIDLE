#include "code_folding_widget.h"
#include "codewidget.h"
#include "main.h"

extern gIDLEOptions options;

/*To create new 
 * code_folding_widget
 */
CodeFoldingWidget*
code_folding_widget_new ()
{
    CodeFoldingWidget *widget = g_malloc (sizeof (CodeFoldingWidget));
    widget->drawing_area = gtk_drawing_area_new ();
    gtk_widget_add_events (widget->drawing_area, GDK_BUTTON_PRESS_MASK);
    widget->block_info_array =  NULL;
    widget->block_info_array_size = 0;
    widget->invisible_text_info_array = NULL;
    widget->invisible_text_info_array_size = 0;
    return widget;
}

/*To destroy
 * code_folding_widget
 */
void
code_folding_widget_destroy (CodeFoldingWidget *widget)
{
    int i;
    gtk_widget_destroy (widget->drawing_area);
    code_folding_widget_free_block_info_array (widget);
    g_free (widget);
}

/*To free code_folding_widget
 * block_info_array
 */
void
code_folding_widget_free_block_info_array (CodeFoldingWidget *widget)
{
    int i;
    if (widget->block_info_array_size == 0)
        return;

    for (i = 0; i < widget->block_info_array_size; i++)
        g_free (widget->block_info_array [i]);
    
    widget->block_info_array_size = 0;
    g_free (widget->block_info_array);
    widget->block_info_array = NULL;
}

/*To free code_folding_widget
 * invisible_text_info_array
 */
void
code_folding_widget_free_invisible_text_info_array (CodeFoldingWidget *widget)
{
    int i;
    if (widget->invisible_text_info_array == 0)
        return;

    for (i = 0; i < widget->invisible_text_info_array_size; i++)
        g_free (widget->invisible_text_info_array [i]);
    
    widget->block_info_array_size = 0;
    g_free (widget->invisible_text_info_array);
    widget->invisible_text_info_array = NULL;
}

/*To remove InvisibleTextInfo from CodeFoldingWidget's
 * invisible_text_info_array at index
 */
void
code_folding_widget_invisible_text_info_array_remove (CodeFoldingWidget *widget,
                                                     int index)
{
    if (index == -1 || index >= widget->invisible_text_info_array_size)
        return;
    
    g_free (widget->invisible_text_info_array [index]);
    for (; index < widget->invisible_text_info_array_size; index++)
        widget->invisible_text_info_array [index] = 
            widget->invisible_text_info_array [index +1];
            
    widget->invisible_text_info_array_size --;
    widget->invisible_text_info_array = g_realloc (widget->invisible_text_info_array,
                                                  sizeof (InvisibleTextInfo *) * widget->invisible_text_info_array_size);
}

/*Check if InvisibleTextInfo with start_line and end_line
 * already exists in invisible_text_info_array
 */
int
code_folding_widget_invisible_text_info_exists (CodeFoldingWidget *widget,
                                               int start_line, int end_line)
{
    int i;
    for (i=0; i < widget->invisible_text_info_array_size; i++)
    {
        if (widget->invisible_text_info_array [i]->start_line == start_line
           && widget->invisible_text_info_array [i]->end_line == end_line)
            /*If yes then return*/
            return i;
    }
    return -1;
}

/*Append to code_folding_widget's
 * invisible_text_info_array
 */
void
code_folding_widget_append_to_invisible_text_info_array (CodeFoldingWidget *widget,
                                               int start_line, int end_line)
{
    /*Check if InvisibleTextInfo with start_line and end_line already exists*/
    if (code_folding_widget_invisible_text_info_exists (widget, start_line, end_line) != -1)
         return;

    /*Else append*/
    InvisibleTextInfo *invisible_text_info = g_malloc (sizeof (InvisibleTextInfo));

    invisible_text_info->start_line = start_line;
    invisible_text_info->end_line = end_line;

    widget->invisible_text_info_array = g_realloc (widget->invisible_text_info_array,
                                        (++widget->invisible_text_info_array_size) * sizeof (InvisibleTextInfo *));

    widget->invisible_text_info_array [widget->invisible_text_info_array_size -1] = invisible_text_info;
}

/* Append to
 * code_folding_widget's block_info_array
 */
void
code_folding_widget_append_to_block_info_array (CodeFoldingWidget *widget,
                                               int start_line, int start_line_pos, GdkRectangle start_line_rect,
                                               int end_line, int end_line_pos, GdkRectangle end_line_rect)
{
    BlockInfo *block_info = g_malloc (sizeof (BlockInfo));

    block_info->start_line = start_line;
    block_info->start_line_rect = start_line_rect;
    block_info->start_line_pos = start_line_pos;
    block_info->end_line = end_line;
    block_info->end_line_pos = end_line_pos;
    block_info->end_line_rect = end_line_rect;

    widget->block_info_array = g_realloc (widget->block_info_array,
                                        (++widget->block_info_array_size) * sizeof (BlockInfo *));

    widget->block_info_array [widget->block_info_array_size -1] = block_info;
}

/*CodeFoldingWidget's button press
 * event handler
 */
gboolean
code_folding_button_press (GtkWidget *widget, GdkEvent *event, gpointer data)
{
    CodeWidget *codewidget = (CodeWidget *)data;
    CodeFoldingWidget *code_folding_widget = codewidget->code_folding_widget;
    GtkTextBuffer *buffer = GTK_TEXT_BUFFER (codewidget->sourcebuffer);
    GdkEventButton *event_button = (GdkEventButton *)event;
    GtkTextIter start_iter, end_iter;
    int i;
    
    /*Check the block within which mouse pressed*/
    for (i = 0; i < code_folding_widget->block_info_array_size; i++)
    {
        int window_start_y, window_end_y;
        
        gtk_text_view_buffer_to_window_coords (GTK_TEXT_VIEW (codewidget->sourceview),
                                              GTK_TEXT_WINDOW_WIDGET,
                                              0,
                                              code_folding_widget->block_info_array [i]->end_line_rect.y,
                                              NULL, &window_end_y);
        
        gtk_text_view_buffer_to_window_coords (GTK_TEXT_VIEW (codewidget->sourceview),
                                              GTK_TEXT_WINDOW_WIDGET,
                                              0,
                                              code_folding_widget->block_info_array [i]->start_line_rect.y,
                                              NULL, &window_start_y);
 
        if (event_button->y >= window_start_y 
           && event_button->y <= window_end_y)
        {
            break;
        }
    }
    
    if (i == code_folding_widget->block_info_array_size)
        return FALSE;

    gtk_text_buffer_get_iter_at_line (buffer, &start_iter, 
                                     code_folding_widget->block_info_array [i]->start_line+1);
    gtk_text_buffer_get_iter_at_line (buffer, &end_iter, 
                                     code_folding_widget->block_info_array [i]->end_line);
    
    /*Check if the text in area clicked is already hidden*/
    int j;
    for (j = 0; j < code_folding_widget->invisible_text_info_array_size; j++)
    {
        if (code_folding_widget->invisible_text_info_array [j]->start_line
           == code_folding_widget->block_info_array [i]->start_line+1
           && code_folding_widget->invisible_text_info_array [j]->end_line
           == code_folding_widget->block_info_array [i]->end_line)
        {
            /*If yes, then remove tag to show it*/
            gtk_text_buffer_remove_tag (buffer, codewidget->invisible_tag, &start_iter, &end_iter);
            code_folding_widget_invisible_text_info_array_remove (code_folding_widget, j);
            return FALSE;
        }
    }
    /*If no, then apply tag to hide it*/
    gtk_text_buffer_apply_tag (buffer, codewidget->invisible_tag, &start_iter, &end_iter);
    code_folding_widget_append_to_invisible_text_info_array (code_folding_widget,
                                                            code_folding_widget->block_info_array [i]->start_line+1,
                                                            code_folding_widget->block_info_array [i]->end_line);
    return TRUE;
}

/*CodeFoldingWidget's drawing_area
 * "draw" signal handler
 */
void
code_folding_widget_draw (GtkWidget *widget, cairo_t *cr, gpointer data)
{
    if (!options.enable_folding)
        return;
    
    CodeWidget *codewidget = (CodeWidget *)data;
    CodeFoldingWidget *code_folding_widget = codewidget->code_folding_widget;
    GtkTextBuffer *buffer = GTK_TEXT_BUFFER (codewidget->sourcebuffer);
    GdkRectangle location;
    GtkTextIter rect_start_iter, rect_end_iter, current_iter;
    int i, first_line, last_line, last_line_pos, first_line_pos;
    float start_x = 1, end_x = gtk_widget_get_allocated_width (widget) - 2;
    cairo_set_source_rgb (cr, 1.0, 1.0, 1.0);

    cairo_rectangle (cr, 0, 0, gtk_widget_get_allocated_width (widget),
                    gtk_widget_get_allocated_height (widget));

    cairo_fill (cr);
    //cairo_set_line_width (cr, 1.0);
    cairo_set_source_rgb (cr, 0.5, 0.5, 0.5);
    cairo_move_to (cr, gtk_widget_get_allocated_width (widget), 0);
    cairo_line_to (cr, gtk_widget_get_allocated_width (widget), gtk_widget_get_allocated_height (widget));
    cairo_stroke (cr);
    cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);

    gchar *line_str;

    gtk_text_buffer_get_iter_at_mark (buffer, &current_iter,
                                     gtk_text_buffer_get_insert (buffer));

    cairo_set_source_rgb (cr, 0, 0, 0);

    gtk_text_view_get_visible_rect (GTK_TEXT_VIEW (codewidget->sourceview),
                                   &location);
    gtk_text_view_get_iter_at_location (GTK_TEXT_VIEW (codewidget->sourceview),
                                       &rect_start_iter, location.x,location.y);
    gtk_text_view_get_iter_at_location (GTK_TEXT_VIEW (codewidget->sourceview),
                                       &rect_end_iter, location.x + location.width, 
                                       location.y + location.height);

    if (gtk_text_iter_get_line (&rect_end_iter) < gtk_text_buffer_get_line_count (buffer))
    {
        last_line = gtk_text_iter_get_line (&rect_end_iter);
        last_line_pos = gtk_text_iter_get_offset (&rect_end_iter);
    }
    else
    {
        GtkTextIter end_iter;
        last_line = gtk_text_buffer_get_line_count (buffer);
        gtk_text_buffer_get_end_iter (buffer, &end_iter);
        last_line_pos = gtk_text_iter_get_offset (&end_iter);
    }
    
    first_line = gtk_text_iter_get_line (&rect_start_iter);
    first_line_pos = gtk_text_iter_get_offset (&rect_start_iter);
    
    code_folding_widget_free_block_info_array (code_folding_widget);

    for (i = 0; i < codewidget->py_class_array_size; i++)
    {        
        PyFunc **p = codewidget->py_class_array [i]->py_func_array;
        if (!p)
            continue;
        
        while (*p)
        {
            if ((*p)->pos >= first_line_pos && (*p)->pos <= last_line_pos)
            {
                GtkTextIter start_line_iter, end_line_iter;
                GdkRectangle start_line_rect, end_line_rect;
                
                /*Get block starting information*/
                gtk_text_buffer_get_iter_at_offset (buffer, &start_line_iter, (*p)->pos);
                gtk_text_view_get_iter_location (GTK_TEXT_VIEW (codewidget->sourceview),
                                                &start_line_iter, &start_line_rect);
                
                /*Get block ending information*/
                if (*(p+1))
                    gtk_text_buffer_get_iter_at_offset (buffer, &end_line_iter, (*(p+1))->pos);

                else if (i + 1 != codewidget->py_class_array_size)
                    gtk_text_buffer_get_iter_at_offset (buffer, &end_line_iter, 
                                                       codewidget->py_class_array [i + 1]->pos);
                else
                    gtk_text_buffer_get_end_iter (buffer, &end_line_iter);
                
                gtk_text_buffer_get_iter_at_line (buffer, &end_line_iter, gtk_text_iter_get_line (&end_line_iter) - 1);
                gtk_text_view_get_iter_location (GTK_TEXT_VIEW (codewidget->sourceview),
                                                &end_line_iter, &end_line_rect);

                code_folding_widget_append_to_block_info_array (code_folding_widget,
                                                              gtk_text_iter_get_line (&start_line_iter),
                                                              (*p)->pos, 
                                                              start_line_rect,
                                                              gtk_text_iter_get_line (&end_line_iter),
                                                              gtk_text_iter_get_offset (&end_line_iter),
                                                              end_line_rect);
            }
            //else if ((*p)->pos > last_line_pos)
            //    break;

            ++p;
        }
        //if (*p)
           // break;
    }

    for (i = 0; i < code_folding_widget->block_info_array_size; i++)
    {
        int window_x, window_y;
        
        gtk_text_view_buffer_to_window_coords (GTK_TEXT_VIEW (codewidget->sourceview),
                                              GTK_TEXT_WINDOW_WIDGET,
                                              code_folding_widget->block_info_array [i]->start_line_rect.x,
                                              code_folding_widget->block_info_array [i]->start_line_rect.y,
                                              &window_x, &window_y);
        
        /*Check if the text in area clicked is already hidden*/
        gboolean display_plus = TRUE;
        int j;
        for (j = 0; j < code_folding_widget->invisible_text_info_array_size; j++)
        {
            if (code_folding_widget->invisible_text_info_array [j]->start_line
               == code_folding_widget->block_info_array [i]->start_line+1
               && code_folding_widget->invisible_text_info_array [j]->end_line
               == code_folding_widget->block_info_array [i]->end_line)
            {
                /*If yes, then "-" should be display instead of "+" to show it*/
                display_plus = FALSE;
                break;
            }
        }
        /* The following lines till stars, displays GTK+ Creator 
         * type Lines for Code Folding Widget. To display "+" instead
         * Uncomment the commented lines and comment the uncommented
         * lines, till stars 
         */

        /*Drawing line at block start*/
        cairo_set_line_width (cr, 1);
        /*cairo_rectangle (cr, start_x,
                        window_y + code_folding_widget->block_info_array [i]->start_line_rect.height/4.0, 
                        end_x - start_x, 
                        code_folding_widget->block_info_array [i]->start_line_rect.height/2.0);*/
        /*if (display_plus)
        {
            cairo_move_to (cr, start_x + (end_x - start_x)/2, 
                          window_y + code_folding_widget->block_info_array [i]->start_line_rect.height/4.0);
            cairo_line_to (cr, start_x + (end_x - start_x)/2, 
                          window_y +3 * code_folding_widget->block_info_array [i]->start_line_rect.height/4.0); 
        }*/

       /* cairo_move_to (cr, start_x, 
                          window_y +code_folding_widget->block_info_array [i]->start_line_rect.height/2.0);
        cairo_line_to (cr, end_x - start_x,
                          window_y +code_folding_widget->block_info_array [i]->start_line_rect.height/2.0);
        
        cairo_stroke (cr);*/
        cairo_move_to (cr, start_x + (end_x - start_x)/2, 
                          window_y + code_folding_widget->block_info_array [i]->start_line_rect.height/2.0);
        cairo_line_to (cr, end_x,
                          window_y + code_folding_widget->block_info_array [i]->start_line_rect.height/2.0);

        cairo_stroke (cr);
        
        cairo_move_to (cr, start_x + (end_x - start_x)/2,
                        window_y + code_folding_widget->block_info_array [i]->start_line_rect.height/2.0);
        /*Drawing line till block end*/
        /*Move to half of the width of drawing_area at the start of block*/
        /*cairo_move_to (cr, start_x + (end_x - start_x)/2,
                      window_y + 3* code_folding_widget->block_info_array [i]->end_line_rect.height/4.0);*/
        
        /********************************************************************/
        gtk_text_view_buffer_to_window_coords (GTK_TEXT_VIEW (codewidget->sourceview),
                                              GTK_TEXT_WINDOW_WIDGET,
                                              code_folding_widget->block_info_array [i]->end_line_rect.x,
                                              code_folding_widget->block_info_array [i]->end_line_rect.y,
                                              &window_x, &window_y);

        /*Create horizontal line*/
        cairo_line_to (cr, start_x + (end_x - start_x)/2,
                      window_y + code_folding_widget->block_info_array [i]->end_line_rect.height/2.0);
        cairo_line_to (cr, end_x,
                      window_y + code_folding_widget->block_info_array [i]->end_line_rect.height/2.0);
        cairo_stroke (cr);
    }
    //printf ("\n");
}