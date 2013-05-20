#include <gtk/gtk.h>

#ifndef CODE_FOLDING_WIDGET_H
#define CODE_FOLDING_WIDGET_H

struct _block_info
{
    int start_line;
    int start_line_pos;
    int end_line;
    int end_line_pos;
    GdkRectangle start_line_rect;
    GdkRectangle end_line_rect;
};

typedef struct _block_info BlockInfo;

struct invisible_text_info
{
    int start_line;
    int end_line;
};

typedef struct invisible_text_info InvisibleTextInfo;

struct code_folding_widget
{
    GtkWidget *drawing_area;
    BlockInfo **block_info_array;
    int block_info_array_size;
    InvisibleTextInfo **invisible_text_info_array;
    int invisible_text_info_array_size;
};

typedef struct code_folding_widget CodeFoldingWidget;

CodeFoldingWidget*
code_folding_widget_new ();

void
code_folding_widget_destroy (CodeFoldingWidget *);

void
code_folding_widget_append_to_block_info_array (CodeFoldingWidget *,
                                               int start_line, int start_line_pos, GdkRectangle start_line_rect,
                                               int end_line, int end_line_pos, GdkRectangle end_line_rect);

void
code_folding_widget_draw (GtkWidget *widget, cairo_t *cr, gpointer data);

void
code_folding_widget_free_block_info_array (CodeFoldingWidget *);

void
code_folding_widget_free_invisible_text_info_array (CodeFoldingWidget *);

gboolean
code_folding_button_press (GtkWidget *widget, GdkEvent *event, gpointer data);

void
code_folding_widget_append_to_invisible_text_info_array (CodeFoldingWidget *,
                                               int start_pos, int end_pos);

int
code_folding_widget_invisible_text_info_exists (CodeFoldingWidget *widget,
                                               int start_line, int end_line);
void
code_folding_widget_invisible_text_info_array_remove (CodeFoldingWidget *widget,
                                                     int index);
#endif