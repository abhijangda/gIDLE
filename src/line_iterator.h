#include <glib.h>
#include <glib/gprintf.h>

#ifndef LINE_ITERATOR_H
#define LINE_ITERATOR_H

struct line_iter
{
    gchar *text;
    GRegex *regex;
    GMatchInfo *match_info;
};

typedef struct line_iter LineIterator;

LineIterator *
line_iterator_for_text (gchar* text);

gchar *
line_iterator_get_line (LineIterator *line);

void
line_iterator_destroy (LineIterator *line);

#endif /*LINE_ITERATOR_H*/