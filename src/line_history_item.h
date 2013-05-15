#include <gtk/gtk.h>

#ifndef LINE_HISTORY_ITEM_H
#define LINE_HISTORY_ITEM_H

struct _line_history_item
{
    int line;
    GtkWidget *menu_item;
};

typedef struct _line_history_item LineHistoryItem;

LineHistoryItem *
line_history_item_new (int line);

void
line_history_item_destroy (LineHistoryItem *);

void
line_history_item_activated (GtkWidget *widget, gpointer data);
#endif /*LINE_HISTORY_ITEM_H*/