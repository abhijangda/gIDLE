#include "main.h"

#ifndef CORE_FUNCS_H
#define CORE_FUNCS_H

gboolean
open_project_from_file (gchar *proj_file);

gchar *
g_file_input_stream_read_line (GFileInputStream *istream);

int
get_current_index ();

int
get_total_pages ();

gchar *
remove_text_between_strings (gchar *text, gchar *start, gchar *end);

gchar *
get_text_between_strings (gchar *text, gchar *start, gchar *end);

gchar *
get_file_data (char *file_path);

gboolean
open_file_at_index (char *file_path, int index);

gboolean
set_file_data (char *file_path, gchar *file_data, gsize length);

int
get_line_pos (GtkTextBuffer *buffer, int line);

int
get_indent_spaces_in_string (char *string);

gchar *
get_text_at_index (int index);

gchar *
remove_char (gchar *string, gchar c);

CodeWidget *
add_new_code_widget ();

gboolean
is_file_modified ();

int
str_count_str (gchar *str1, gchar *str2);

char *
get_doc_string_between_lines (GtkTextBuffer *buffer, int start, int end);

void
gtk_text_buffer_append (GtkTextBuffer *buffer, gchar *text, int len);

void
gtk_text_buffer_get_line_end_iter (GtkTextBuffer *buffer, GtkTextIter *iter, int line_index);

gdouble
gtk_text_buffer_get_matching_parethesis_pos (GtkTextBuffer *buffer, gint pos, gchar bracket);

gint
gtk_text_buffer_get_first_unmatched_parenthesis_pos (GtkTextBuffer *buffer, gint pos);

gchar *
gtk_text_buffer_get_line_text (GtkTextBuffer *buffer, int line_index, gboolean strip_strings);

gchar *
gtk_text_buffer_get_selected_text (GtkTextBuffer *buffer);

gboolean
find_next (GtkTextView *text_view, const gchar *text, GtkTextSearchFlags flags);

gboolean
find_previous (GtkTextView *text_view, const gchar *text, GtkTextSearchFlags flags);

void
go_to_file_at_line (gchar *file_path, gint line);

void
go_to_line (GtkTextView *view, int line);

void
go_to_pos_and_select_line (GtkTextView *view, int pos);

void
show_error_message_dialog (gchar *message);

void
init_file_filters ();

void
init_line_history_array ();

void
clean_line_history_array ();

void
add_bookmark (int index, int line, char *file_name);

void
remove_all_bookmarks ();

void
bookmark_item_activate (GtkWidget *widget, gpointer bookmark);

void
remove_code_widget_at (int index);

void
remove_all_code_widgets ();

void
free_code_widget_array ();
#endif /*CORE_FUNCS_H*/