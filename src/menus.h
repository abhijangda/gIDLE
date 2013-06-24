#include <gtk/gtk.h>

#ifndef MENUS_H
#define MENUS_H

//File Menu
void
file_new_activate (GtkWidget *);
void
file_new_tab_activate (GtkWidget *);
void
file_open_activate (GtkWidget *);
void
file_recent_menu_selection_done (GtkMenuShell *, GtkWidget *);
void
file_save_activate (GtkWidget *);
void
file_save_as_activate (GtkWidget *);
void
file_save_copy_as_activate (GtkWidget *);
void
file_save_all_activate (GtkWidget *);
void
file_close_all_tabs_activate (GtkWidget *);
void
file_close_tab_activate (GtkWidget *);
void
file_quit_activate (GtkWidget *);

//Edit Menu
void
edit_undo_activate (GtkWidget *);
void
edit_redo_activate (GtkWidget *);
void
edit_copy_activate (GtkWidget *);
void
edit_cut_activate (GtkWidget *);
void
edit_paste_activate (GtkWidget *);
void
edit_select_all_activate (GtkWidget *);
void
edit_delete_activate (GtkWidget *);
void
edit_select_function_activate (GtkWidget *);
void
edit_select_block_activate (GtkWidget *);
void
edit_fold_all_func_activate (GtkWidget *);
void
edit_unfold_all_func_activate (GtkWidget *);
void
edit_fold_current_func_activate (GtkWidget *);
void
edit_autocomplete_activate (GtkWidget *);
void
edit_matching_paranthesis_activate (GtkWidget *);

//Format Menu
void
format_inc_indent_activate (GtkWidget *);
void
format_dec_indent_activate (GtkWidget *);
void
format_comment_out_activate (GtkWidget *);
void
format_uncomment_out_activate (GtkWidget *);
void
format_tabify_region_activate (GtkWidget *);
void
format_untabify_region_activate (GtkWidget *);
void
format_uppercase_activate (GtkWidget *);
void
format_lowercase_activate (GtkWidget *);
void
format_strip_spaces_activate (GtkWidget *);

//Search Menu
void
search_find_activate (GtkWidget *);
void
search_find_next_activate (GtkWidget *);
void
search_find_prev_activate (GtkWidget *);
void
search_find_replace_activate (GtkWidget *);
void
search_find_selected_activate  (GtkWidget *);
void
search_find_in_text_activate  (GtkWidget *);
void
search_find_file_activate  (GtkWidget *);
void
search_replace_file_activate  (GtkWidget *);
void
search_reg_exp_activate  (GtkWidget *);

//Navigation Menu

void
navigate_back_activate (GtkWidget *);
void
navigate_forward_activate (GtkWidget *);
void
navigate_first_line_activate  (GtkWidget *);
void
navigate_last_line_activate  (GtkWidget *);
void
navigate_line_history_select  (GtkWidget *);
void
naviagate_add_bookmark_activate  (GtkWidget *);
void
naviagate_clear_bookmarks_activate  (GtkWidget *);
void
navigate_prev_bookmarks_activate  (GtkWidget *);
void
navigate_next_bookmark_activate  (GtkWidget *);
void
navigate_bookmarks_activate  (GtkWidget *);
void
navigate_go_to_block_start_activate  (GtkWidget *);
void
navigate_go_to_func_def_activate  (GtkWidget *);
void
navigate_go_to_line_activate  (GtkWidget *);

//Project Menu
void
project_new_activate (GtkWidget *);
void
project_open_activate (GtkWidget *);
void
project_save_activate (GtkWidget *);
void
project_save_as_activate (GtkWidget *);
void
project_save_copy_as_activate (GtkWidget *);
void
project_empty_activate (GtkWidget *);
void
project_close_activate (GtkWidget *);
void
project_preferences_activate (GtkWidget *);
void
project_recent_activate (GtkWidget *);

//Python Shell Menu
void
python_shell_open_activate (GtkWidget *);
void
python_shell_restart_activate (GtkWidget *);
void
python_shell_close_activate (GtkWidget *);

//Run Menu
void
run_run_script_activate (GtkWidget *);
void
run_debug_script_activate (GtkWidget *);
void
run_run_project_activate (GtkWidget *);
void
run_debug_project_activate (GtkWidget *);

//Debug Menu
void
debug_open_pdb_shell_activate (GtkWidget *);

//Tools Menu
void
tools_options_activate (GtkWidget *);
void
tools_auto_indent_activate (GtkWidget *);
void
tools_class_browser_activate (GtkWidget *);

#endif /*MENUS_H*/