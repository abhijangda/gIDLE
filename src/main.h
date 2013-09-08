#include <stdio.h>

#include "codewidget.h"
#include "bookmark.h"
#include "line_history_item.h"
#include "pty_fork.h"
#include "project_class.h"
#include "symbols_view.h"

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

GtkWidget *content_box;
GtkWidget *content_paned;
Project *current_project;

GtkWidget *symbols_view;

GtkWidget *proj_notebook;

/***Options****/
struct _opt
{
    int indent_width;
    char *indent_width_str;
    char *comment_out_str;
    int tab_width;
    char *tab_width_str;
    gboolean is_code_completion;
    gboolean is_code_folding;
    gboolean show_line_numbers;
    char *python_shell_path;
    char *font_name;
    int font_size;
    gboolean word_wrap;
    gboolean highlight_curr_line;
    gboolean highlight_matching_brace;
    gboolean enable_folding;
    gboolean fold_comments;
    gboolean fold_functions;
    gboolean fold_classes;
    gboolean variable_scoping;
    gboolean completion;
    gboolean line_numbers;
    int line_numbers_font_size;
    gboolean syntax_highlighting;
    gboolean matching_brace;
    gboolean indentation;
    char *inc_indent_syms;
    char *dec_indent_syms;
    gboolean code_folding;
    gboolean code_completion;
};
typedef struct _opt gIDLEOptions;

/*************/

/**Bookmark related**/
Bookmark **bookmark_array;
int bookmark_array_size;
int current_bookmark_index;
/****************/

void
set_mode (int mode);

void
apply_changed_option (char *option_name);
#endif /*MAIN_H*/