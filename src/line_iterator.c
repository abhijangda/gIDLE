#include <stdlib.h>
#include "line_iterator.h"

LineIterator *
line_iterator_for_text (gchar* text)
{
    LineIterator *line_iter = g_malloc (sizeof (LineIterator));
    //line_iter->text = g_strdup (text);
    line_iter->regex = g_regex_new (".+", 0, 0, NULL);
    g_regex_match_full (line_iter->regex, text, -1, 0, 0, &(line_iter->match_info), NULL);
    return line_iter;
}

gchar *
line_iterator_get_line (LineIterator *line_iter)
{
    gchar *line;
    if (g_match_info_matches (line_iter->match_info))
    {
        line = g_match_info_fetch (line_iter->match_info, 0);
        g_match_info_next (line_iter->match_info, NULL);
    }
    else
        line = NULL;

    return line;
}

void
line_iterator_destroy (LineIterator *line_iter)
{
    g_object_unref (line_iter->regex);
    g_match_info_free (line_iter->match_info);
    //g_free (line_iter->text);
    g_free (line_iter);
}