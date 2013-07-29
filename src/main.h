#include <stdio.h>
#include "codewidget.h"
#include "bookmark.h"
#include "line_history_item.h"
#include "pty_fork.h"
#include "project_class.h"

#ifndef MAIN_H
#define MAIN_H

/**Modes for gIDLE**/
#define GIDLE_MODE_FILE            0
#define GIDLE_MODE_PROJECT    1
/****************/

GObject *window;
CodeWidget **code_widget_array;
GtkWidget *bookmarks_menu;
int code_widget_array_size;
GtkWidget *notebook;
GtkMenu *bookmark_menu;
GtkFileFilter *py_filter;
GtkFileFilter *all_filter;
GtkFileFilter *pyproj_file;
GtkWidget *line_history_menu;
LineHistoryItem **line_history_item_array;
GtkWidget *proj_tree_view;
GtkTreeStore *proj_tree_store;
GtkWidget *content_box;
GtkWidget *content_paned;
Project *current_project;

/***Options****/
int indent_width;
char *indent_width_str;
char *comment_out_str;
int tab_width;
char *tab_width_str;
gboolean is_code_completion;
gboolean is_code_folding;
gboolean show_line_numbers;
char *python_shell_path;
/*************/

/**Bookmark related**/
Bookmark **bookmark_array;
int bookmark_array_size;
int current_bookmark_index;
/****************/

void
set_mode (int mode);

#endif /*MAIN_H*/