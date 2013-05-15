#include "bookmark.h"

Bookmark *
bookmark_new (int index, int line, char *file_name)
{
    Bookmark *bookmark = g_malloc (sizeof (Bookmark));
    bookmark->index = index;
    bookmark->line = line;
    bookmark->file_name = g_strdup (file_name);
    char *label;
    asprintf (&label, "%s: %d", file_name, line);
    bookmark->menu_item = gtk_menu_item_new_with_label (label);
    free (label);
    return bookmark;
}

void
bookmark_destroy (Bookmark *bookmark)
{
    gtk_widget_destroy (bookmark->menu_item);
    g_free (bookmark->file_name);
    g_free (bookmark);
}