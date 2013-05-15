#include "line_history_item.h"
#include "core_funcs.h"
#include "main.h"

LineHistoryItem *
line_history_item_new (int line)
{
    LineHistoryItem *item = g_malloc (sizeof (LineHistoryItem));
    item->line = line;
    gchar *label;
    asprintf (&label, "Label:%d", line);
    item->menu_item  = gtk_menu_item_new_with_label (label);
    g_free (label);
    g_signal_connect (G_OBJECT (item->menu_item), "activate",
                     G_CALLBACK (line_history_item_activated), item);
    return item;
}

void
line_history_item_destroy (LineHistoryItem *item)
{
    g_free (item);
}

void
line_history_item_activated (GtkWidget *widget, gpointer data)
{
    LineHistoryItem * line_history_item = (LineHistoryItem *)data;
    go_to_line (GTK_TEXT_VIEW (code_widget_array [get_current_index ()]->sourceview), line_history_item->line);
}