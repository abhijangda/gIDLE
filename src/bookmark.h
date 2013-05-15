#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>

#ifndef BOOKMARK_H
#define BOOKMARK_H

struct _bookmark
{
    int index;
    int line;
    char *file_name;
    GtkWidget *menu_item;
};
typedef struct _bookmark Bookmark;

Bookmark *
bookmark_new (int index, int line, char *file_name);

void
bookmark_destroy (Bookmark *bookmark);
#endif //BOOKMARK_H