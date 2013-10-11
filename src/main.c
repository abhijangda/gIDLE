#include "main.h"
#include "menus.h"
#include "toolbar.h"
#include "python_shell.h"
#include "py_variable.h"
#include "core_funcs.h"
#include "proj_notebook.h"
#include "path_browser.h"

int mode = -1;

char *env_python_path;

static gboolean
delete_event (GtkWidget *widget, GdkEvent *event);
static void
main_window_destroy (GtkWidget *widget);

GRegex *regex_import_as, *regex_import;
GRegex *regex_from_import, *regex_from_import_as;
GRegex *regex_class, *regex_func;
GRegex *regex_global_var, *regex_static_var;
GRegex *regex_local_var, *regex_self_var;
GRegex *regex_word;

GAsyncQueue *async_queue;

GtkBuilder *builder;
extern gchar *search_text;
GtkWidget *status_bar;

ChildProcessData *python_shell_data;
gboolean bash_loaded;
char *sys_path_string;
char *python_sys_script_path = "./scripts/path.py";

gIDLEOptions options;
FileMonitor *file_monitor;

static void 
file_monitor_changed (GFileMonitor *, GFile *, GFile *, GFileMonitorEvent, gpointer);

int
main (int argc, char *argv [])
{
    /*Running Shell*/
    GError *error  = NULL;

    python_shell_data = g_try_malloc (sizeof (ChildProcessData));
    python_shell_data->argv = NULL;
    python_shell_data->slave_termios = g_try_malloc (sizeof (struct termios));
    python_shell_data->current_dir = NULL;

    bash_loaded = ptyFork (python_shell_data, &error);
    gtk_init (&argc, &argv);

    python_shell_data->channel = g_io_channel_unix_new (python_shell_data->master_fd);
    g_io_add_watch (python_shell_data->channel, G_IO_IN,
                   (GIOFunc)read_masterFd, &(python_shell_data->master_fd));

    /**********/
    env_python_path = "/home/abhi/kivy_repo/kivy";
    /*Get sys.path*/
    char *sys_path_argv[] = {"python", "./scripts/path.py", NULL}; 
    g_spawn_sync (NULL, sys_path_argv, NULL, G_SPAWN_SEARCH_PATH,
                               NULL, NULL, &sys_path_string, NULL, NULL, NULL);
    /***********/
    
    char *_str = g_strconcat (sys_path_string, "\n", env_python_path, NULL);
    g_free (sys_path_string);
    sys_path_string = _str; 
    
    file_monitor = file_monitor_new (file_monitor_changed);

    /*Setting Main Window*/    
    GtkWidget *navigate_bookmarks;
    builder = gtk_builder_new ();
    gtk_builder_add_from_file (builder, "./ui/main.ui", NULL);

    content_box = GTK_WIDGET (gtk_builder_get_object (builder, "content_box"));

    window = gtk_builder_get_object (builder, "main_window");
    g_signal_connect (window, "destroy", G_CALLBACK (gtk_main_quit), NULL);

    path_browser = path_browser_new ();
    g_object_ref (path_browser);

    proj_notebook = project_notebook_new ();
    
    /**Setting proj_syms_tree_view**/
    symbols_view = symbols_view_new ();

    g_object_ref (symbols_view);
    /*************************/
    content_paned = gtk_paned_new (GTK_ORIENTATION_HORIZONTAL);
    g_object_ref (content_paned);

    status_bar = gtk_statusbar_new ();
    g_object_ref (status_bar);
    
    gtk_notebook_append_page (GTK_NOTEBOOK (proj_notebook),
                                   symbols_view, gtk_label_new ("Symbols"));
    gtk_notebook_append_page (GTK_NOTEBOOK (proj_notebook), 
                                   path_browser,
                                   gtk_label_new ("Path Browser"));

    navigate_bookmarks = GTK_WIDGET (gtk_builder_get_object (builder,
                                    "navigate_bookmarks"));
    bookmarks_menu = gtk_menu_new ();

    gtk_menu_item_set_submenu (GTK_MENU_ITEM (navigate_bookmarks),
                              bookmarks_menu);

    GtkAccelGroup *accelgroup = GTK_ACCEL_GROUP (gtk_builder_get_object (builder,
                                                                       "accelgroup"));

    /*Connecting menu item's signals*/ 
    //For File Menu
    g_signal_connect (gtk_builder_get_object (builder, "file_new"), "activate",
                      G_CALLBACK (file_new_activate), NULL);
    g_signal_connect (gtk_builder_get_object (builder, "file_new_tab"), "activate",
                      G_CALLBACK (file_new_tab_activate), NULL);
    g_signal_connect (gtk_builder_get_object (builder, "file_open"), "activate",
                      G_CALLBACK (file_open_activate), NULL);
    g_signal_connect (gtk_builder_get_object (builder, "recentchoosermenu"), "selection-done",
                      G_CALLBACK (file_recent_menu_selection_done), NULL);
    g_signal_connect (gtk_builder_get_object (builder, "file_save"), "activate",
                                   G_CALLBACK (file_save_activate), NULL);
    g_signal_connect (gtk_builder_get_object (builder, "file_save_as"), "activate",
                                   G_CALLBACK (file_save_as_activate), NULL);
    g_signal_connect (gtk_builder_get_object (builder, "file_save_copy_as"), "activate",
                                   G_CALLBACK (file_save_copy_as_activate), NULL);
    g_signal_connect (gtk_builder_get_object (builder, "file_save_all"), "activate",
                                   G_CALLBACK (file_save_all_activate), NULL);
    g_signal_connect (gtk_builder_get_object (builder, "file_close_tab"), "activate",
                                   G_CALLBACK (file_close_tab_activate), NULL);
    g_signal_connect (gtk_builder_get_object (builder, "file_close_all_tabs"), "activate",
                      G_CALLBACK (file_close_all_tabs_activate), NULL);
    g_signal_connect (gtk_builder_get_object (builder, "file_quit"), "activate",
                      G_CALLBACK (file_quit_activate), NULL);
    
    //For Edit Menu           
    g_signal_connect (gtk_builder_get_object (builder, "edit_undo"), "activate",
                                   G_CALLBACK (edit_undo_activate), NULL);
    g_signal_connect (gtk_builder_get_object (builder, "edit_redo"), "activate",
                                   G_CALLBACK (edit_redo_activate), NULL);
    g_signal_connect (gtk_builder_get_object (builder, "edit_cut"), "activate",
                                   G_CALLBACK (edit_cut_activate), NULL);
    g_signal_connect (gtk_builder_get_object (builder, "edit_copy"), "activate",
                                   G_CALLBACK (edit_copy_activate), NULL);
    g_signal_connect (gtk_builder_get_object (builder, "edit_paste"), "activate",
                                   G_CALLBACK (edit_paste_activate), NULL);
    g_signal_connect (gtk_builder_get_object (builder, "edit_delete"), "activate",
                                   G_CALLBACK (edit_delete_activate), NULL);
    g_signal_connect (gtk_builder_get_object (builder, "edit_select_all"), "activate",
                                   G_CALLBACK (edit_select_all_activate), NULL);
    g_signal_connect (gtk_builder_get_object (builder, "edit_select_function"), "activate",
                                   G_CALLBACK (edit_select_function_activate), NULL);
    g_signal_connect (gtk_builder_get_object (builder, "edit_select_block"), "activate",
                                   G_CALLBACK (edit_select_block_activate), NULL);
    g_signal_connect (gtk_builder_get_object (builder, "edit_fold_all_func"), "activate",
                                   G_CALLBACK (edit_fold_all_func_activate), NULL);
    g_signal_connect (gtk_builder_get_object (builder, "edit_unfold_all_func"), "activate",
                                   G_CALLBACK (edit_unfold_all_func_activate), NULL);
    g_signal_connect (gtk_builder_get_object (builder, "edit_fold_current_func"), "activate",
                                   G_CALLBACK (edit_fold_current_func_activate), NULL);
    g_signal_connect (gtk_builder_get_object (builder, "edit_autocomplete"), "activate",
                                   G_CALLBACK (edit_autocomplete_activate), NULL);
    g_signal_connect (gtk_builder_get_object (builder, "edit_matching_paranthesis"), "activate",
                                   G_CALLBACK (edit_matching_paranthesis_activate), NULL);
        
    //For Format Menu
    g_signal_connect (gtk_builder_get_object (builder, "format_inc_indent"), "activate",
                                   G_CALLBACK (format_inc_indent_activate), NULL);
    gtk_widget_add_accelerator (GTK_WIDGET (gtk_builder_get_object (builder, "format_inc_indent")),
                               "activate", accelgroup, GDK_KEY_bracketright, 
                                GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

    g_signal_connect (gtk_builder_get_object (builder, "format_dec_indent"), "activate",
                                   G_CALLBACK (format_dec_indent_activate), NULL);
    gtk_widget_add_accelerator (GTK_WIDGET (gtk_builder_get_object (builder, "format_dec_indent")),
                               "activate", accelgroup, GDK_KEY_bracketleft, 
                                GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

    g_signal_connect (gtk_builder_get_object (builder, "format_comment_out"), "activate",
                                   G_CALLBACK (format_comment_out_activate), NULL);
    gtk_widget_add_accelerator (GTK_WIDGET (gtk_builder_get_object (builder, "format_comment_out")),
                               "activate", accelgroup, GDK_KEY_3, 
                                GDK_MOD1_MASK, GTK_ACCEL_VISIBLE);

    g_signal_connect (gtk_builder_get_object (builder, "format_uncomment_out"), "activate",
                                   G_CALLBACK (format_uncomment_out_activate), NULL);
    gtk_widget_add_accelerator (GTK_WIDGET (gtk_builder_get_object (builder, "format_uncomment_out")),
                               "activate", accelgroup, GDK_KEY_4, 
                                GDK_MOD1_MASK, GTK_ACCEL_VISIBLE);

    g_signal_connect (gtk_builder_get_object (builder, "format_tabify_region"), "activate",
                                   G_CALLBACK (format_tabify_region_activate), NULL);
    g_signal_connect (gtk_builder_get_object (builder, "format_untabify_region"), "activate",
                                   G_CALLBACK (format_untabify_region_activate), NULL);
    g_signal_connect (gtk_builder_get_object (builder, "format_uppercase"), "activate",
                                   G_CALLBACK (format_uppercase_activate), NULL);
    g_signal_connect (gtk_builder_get_object (builder, "format_lowercase"), "activate",
                                   G_CALLBACK (format_lowercase_activate), NULL);
    g_signal_connect (gtk_builder_get_object (builder, "format_strip_spaces"), "activate",
                                   G_CALLBACK (format_strip_spaces_activate), NULL);
    
    //For Search Menu
    g_signal_connect (gtk_builder_get_object (builder, "search_find"), "activate",
                                   G_CALLBACK (search_find_activate), NULL);
     g_signal_connect (gtk_builder_get_object (builder, "search_find_next"), "activate",
                                   G_CALLBACK (search_find_next_activate), NULL);   
    g_signal_connect (gtk_builder_get_object (builder, "search_find_prev"), "activate",
                                   G_CALLBACK (search_find_prev_activate), NULL);   
    g_signal_connect (gtk_builder_get_object (builder, "search_find_replace"), "activate",
                                   G_CALLBACK (search_find_replace_activate), NULL);   
    g_signal_connect (gtk_builder_get_object (builder, "search_find_selected"), "activate",
                                   G_CALLBACK (search_find_selected_activate), NULL);   
    g_signal_connect (gtk_builder_get_object (builder, "search_find_in_text"), "activate",
                                   G_CALLBACK (search_find_in_text_activate), NULL);   
    g_signal_connect (gtk_builder_get_object (builder, "search_find_file"), "activate",
                                   G_CALLBACK (search_find_file_activate), NULL);   
    g_signal_connect (gtk_builder_get_object (builder, "search_replace_file"), "activate",
                                   G_CALLBACK (search_replace_file_activate), NULL);   
    g_signal_connect (gtk_builder_get_object (builder, "search_reg_exp"), "activate",
                                   G_CALLBACK (search_reg_exp_activate), NULL);   
    
    //For Navigation Menu
    g_signal_connect (gtk_builder_get_object (builder, "navigate_back"), "activate",
                                   G_CALLBACK (navigate_back_activate), NULL);   
    g_signal_connect (gtk_builder_get_object (builder, "navigate_forward"), "activate",
                                   G_CALLBACK (navigate_forward_activate), NULL);   
    g_signal_connect (gtk_builder_get_object (builder, "navigate_first_line"), "activate",
                                   G_CALLBACK (navigate_first_line_activate), NULL);   
    g_signal_connect (gtk_builder_get_object (builder, "navigate_last_line"), "activate",
                                   G_CALLBACK (navigate_last_line_activate), NULL);   
    g_signal_connect (gtk_builder_get_object (builder, "navigate_line_history"), "select",
                                   G_CALLBACK (navigate_line_history_select), NULL);   
    g_signal_connect (gtk_builder_get_object (builder, "naviagate_add_bookmark"), "activate",
                                   G_CALLBACK (naviagate_add_bookmark_activate), NULL);   
    g_signal_connect (gtk_builder_get_object (builder, "naviagate_clear_bookmarks"), "activate",
                                   G_CALLBACK (naviagate_clear_bookmarks_activate), NULL);   
    g_signal_connect (gtk_builder_get_object (builder, "navigate_prev_bookmarks"), "activate",
                                   G_CALLBACK (navigate_prev_bookmarks_activate), NULL);   
    g_signal_connect (gtk_builder_get_object (builder, "navigate_next_bookmark"), "activate",
                                   G_CALLBACK (navigate_next_bookmark_activate), NULL);   
    g_signal_connect (gtk_builder_get_object (builder, "navigate_bookmarks"), "activate",
                                   G_CALLBACK (navigate_bookmarks_activate), NULL);   
    g_signal_connect (gtk_builder_get_object (builder, "navigate_go_to_line"), "activate",
                                   G_CALLBACK (navigate_go_to_line_activate), NULL);   
    g_signal_connect (gtk_builder_get_object (builder, "navigate_go_to_block_start"), "activate",
                                   G_CALLBACK (navigate_go_to_block_start_activate), NULL);   
    g_signal_connect (gtk_builder_get_object (builder, "navigate_go_to_func_def"), "activate",
                                   G_CALLBACK (navigate_go_to_func_def_activate), NULL);   
    g_signal_connect (gtk_builder_get_object (builder, "navigate_go_to_next_func"), "activate",
                                   G_CALLBACK (navigate_go_to_next_func_activate), NULL);
    g_signal_connect (gtk_builder_get_object (builder, "navigate_go_to_prev_func"), "activate",
                                   G_CALLBACK (navigate_go_to_prev_func_activate), NULL);
    
    //For Project Menu
    g_signal_connect (gtk_builder_get_object (builder, "project_new"), "activate",
                                   G_CALLBACK (project_new_activate), NULL);   
    g_signal_connect (gtk_builder_get_object (builder, "project_open"), "activate",
                                   G_CALLBACK (project_open_activate), NULL);   
    g_signal_connect (gtk_builder_get_object (builder, "project_save"), "activate",
                                   G_CALLBACK (project_save_activate), NULL);   
    g_signal_connect (gtk_builder_get_object (builder, "project_save_as"), "activate",
                                   G_CALLBACK (project_save_as_activate), NULL);   
    g_signal_connect (gtk_builder_get_object (builder, "project_save_copy_as"), "activate",
                                   G_CALLBACK (project_save_copy_as_activate), NULL);   
    g_signal_connect (gtk_builder_get_object (builder, "project_empty"), "activate",
                                   G_CALLBACK (project_empty_activate), NULL);   
    g_signal_connect (gtk_builder_get_object (builder, "project_close"), "activate",
                                   G_CALLBACK (project_close_activate), NULL);   
    g_signal_connect (gtk_builder_get_object (builder, "project_preferences"), "activate",
                                   G_CALLBACK (project_preferences_activate), NULL);   
    g_signal_connect (gtk_builder_get_object (builder, "project_recent"), "activate",
                                   G_CALLBACK (project_recent_activate), NULL);
    
    //For Python Shell Menu
    g_signal_connect (gtk_builder_get_object (builder, "python_shell_open"), "activate",
                                   G_CALLBACK (python_shell_open_activate), NULL);   
    g_signal_connect (gtk_builder_get_object (builder, "python_shell_restart"), "activate",
                                   G_CALLBACK (python_shell_restart_activate), NULL);   
    g_signal_connect (gtk_builder_get_object (builder, "python_shell_close"), "activate",
                                   G_CALLBACK (python_shell_close_activate), NULL);   
    
    //For Run Menu
    g_signal_connect (gtk_builder_get_object (builder, "run_run_script"), "activate",
                                   G_CALLBACK (run_run_script_activate), NULL);   
    g_signal_connect (gtk_builder_get_object (builder, "run_debug_script"), "activate",
                                   G_CALLBACK (run_debug_script_activate), NULL);   
    g_signal_connect (gtk_builder_get_object (builder, "run_run_project"), "activate",
                                   G_CALLBACK (run_run_project_activate), NULL);   
    g_signal_connect (gtk_builder_get_object (builder, "run_debug_project"), "activate",
                                   G_CALLBACK (run_debug_project_activate), NULL);   
    
    //For Debug Menu
    g_signal_connect (gtk_builder_get_object (builder, "debug_open_pdb_shell"), "activate",
                                   G_CALLBACK (debug_open_pdb_shell_activate), NULL);
    
    //For Tools Menu
    g_signal_connect (gtk_builder_get_object (builder, "tools_options"), "activate",
                                   G_CALLBACK (tools_options_activate), NULL);   
    g_signal_connect (gtk_builder_get_object (builder, "tools_auto_indent"), "activate",
                                   G_CALLBACK (tools_auto_indent_activate), NULL);   
    g_signal_connect (gtk_builder_get_object (builder, "tools_class_browser"), "activate",
                                   G_CALLBACK (tools_class_browser_activate), NULL);   

    /*************************/
    
    /**Connecting Toolbar's signals**/

    g_signal_connect (gtk_builder_get_object (builder, "toolbar_new_menu"), "clicked",
                                   G_CALLBACK (toolbar_new_clicked), NULL);   
    g_signal_connect (gtk_builder_get_object (builder, "toolbar_new_file"), "activate",
                                   G_CALLBACK (toolbar_new_file_clicked), NULL);   
    g_signal_connect (gtk_builder_get_object (builder, "toolbar_new_project"), "activate",
                                   G_CALLBACK (toolbar_new_proj_clicked), NULL);   
    g_signal_connect (gtk_builder_get_object (builder, "toolbar_open_menu"), "clicked",
                                   G_CALLBACK (toolbar_open_clicked), NULL);   
    g_signal_connect (gtk_builder_get_object (builder, "toolbar_open_file"), "activate",
                                   G_CALLBACK (toolbar_open_file_clicked), NULL);   
    g_signal_connect (gtk_builder_get_object (builder, "toolbar_open_project"), "activate",
                                   G_CALLBACK (toolbar_open_proj_clicked), NULL);   
    g_signal_connect (gtk_builder_get_object (builder, "toolbar_save_menu"), "clicked",
                                   G_CALLBACK (toolbar_save_clicked), NULL);   
    g_signal_connect (gtk_builder_get_object (builder, "toolbar_save_file"), "activate",
                                   G_CALLBACK (toolbar_save_file_clicked), NULL);   
    g_signal_connect (gtk_builder_get_object (builder, "toolbar_save_project"), "activate",
                                   G_CALLBACK (toolbar_save_project_clicked), NULL);   
    g_signal_connect (gtk_builder_get_object (builder, "toolbar_save_as"), "clicked",
                                   G_CALLBACK (toolbar_save_as_clicked), NULL);   
    g_signal_connect (gtk_builder_get_object (builder, "toolbar_save_as_file"), "activate",
                                   G_CALLBACK (toolbar_save_as_file_clicked), NULL);   
    g_signal_connect (gtk_builder_get_object (builder, "toolbar_save_as_project"), "activate",
                                   G_CALLBACK (toolbar_save_as_project_clicked), NULL);  
    
    g_signal_connect (gtk_builder_get_object (builder, "toolbar_undo"), "clicked",
                                   G_CALLBACK (toolbar_undo_clicked), NULL);   
    g_signal_connect (gtk_builder_get_object (builder, "toolbar_redo"), "clicked",
                                   G_CALLBACK (toolbar_redo_clicked), NULL);   
                                
    g_signal_connect (gtk_builder_get_object (builder, "toolbar_cut"), "clicked",
                                   G_CALLBACK (toolbar_cut_clicked), NULL);   
    g_signal_connect (gtk_builder_get_object (builder, "toolbar_copy"), "clicked",
                                   G_CALLBACK (toolbar_copy_clicked), NULL);   
    g_signal_connect (gtk_builder_get_object (builder, "toolbar_paste"), "clicked",
                                   G_CALLBACK (toolbar_cut_clicked), NULL);   
    g_signal_connect (gtk_builder_get_object (builder, "toolbar_delete"), "clicked",
                                   G_CALLBACK (toolbar_delete_clicked), NULL);   
    g_signal_connect (gtk_builder_get_object (builder, "toolbar_select_all"), "clicked",
                                   G_CALLBACK (toolbar_select_all_clicked), NULL);   
                                
    g_signal_connect (gtk_builder_get_object (builder, "toolbar_find"), "clicked",
                                   G_CALLBACK (toolbar_find_clicked), NULL);   
    g_signal_connect (gtk_builder_get_object (builder, "toolbar_find_replace"), "clicked",
                                   G_CALLBACK (toolbar_find_replace_clicked), NULL);   
      
    g_signal_connect (gtk_builder_get_object (builder, "toolbar_inc_indent"), "clicked",
                                   G_CALLBACK (toolbar_inc_indent_clicked), NULL);   
    g_signal_connect (gtk_builder_get_object (builder, "toolbar_dec_indent"), "clicked",
                                   G_CALLBACK (toolbar_dec_indent_clicked), NULL);   
    
    g_signal_connect (gtk_builder_get_object (builder, "toolbar_next_line"), "clicked",
                                   G_CALLBACK (navigate_forward_activate), NULL); 
    g_signal_connect (gtk_builder_get_object (builder, "toolbar_prev_line"), "clicked",
                                   G_CALLBACK (navigate_back_activate), NULL); 
    g_signal_connect (gtk_builder_get_object (builder, "toolbar_add_bookmark"), "clicked",
                                   G_CALLBACK (naviagate_add_bookmark_activate), NULL); 
    g_signal_connect (gtk_builder_get_object (builder, "toolbar_next_bookmark"), "clicked",
                                   G_CALLBACK (navigate_next_bookmark_activate), NULL); 
    g_signal_connect (gtk_builder_get_object (builder, "toolbar_prev_bookmark"), "clicked",
                                   G_CALLBACK (navigate_prev_bookmarks_activate), NULL); 
    g_signal_connect (gtk_builder_get_object (builder, "toolbar_clear_bookmark"), "clicked",
                                   G_CALLBACK (naviagate_clear_bookmarks_activate), NULL); 
    g_signal_connect (gtk_builder_get_object (builder, "toolbar_run"), "clicked",
                                   G_CALLBACK (toolbar_run), NULL); 
    /*g_signal_connect (gtk_builder_get_object (builder, "toolbar_debug"), "clicked",
                                   G_CALLBACK (toolbar_dec_indent_clicked), NULL); */
    g_signal_connect (gtk_builder_get_object (builder, "toolbar_open_shell"), "clicked",
                                   G_CALLBACK (python_shell_open_activate), NULL); 
    g_signal_connect (gtk_builder_get_object (builder, "toolbar_options"), "clicked",
                                   G_CALLBACK (tools_options_activate), NULL); 
    
    gtk_tool_button_set_icon_widget (GTK_TOOL_BUTTON (gtk_builder_get_object (builder, "toolbar_add_bookmark")),
                                      gtk_image_new_from_pixbuf(gdk_pixbuf_new_from_file("./icons/bookmarksadd.png",NULL)));
    gtk_tool_button_set_icon_widget (GTK_TOOL_BUTTON (gtk_builder_get_object (builder, "toolbar_next_bookmark")),
                                      gtk_image_new_from_pixbuf(gdk_pixbuf_new_from_file("./icons/bookmarksnext.png",NULL)));
    gtk_tool_button_set_icon_widget (GTK_TOOL_BUTTON (gtk_builder_get_object (builder, "toolbar_prev_bookmark")),
                                      gtk_image_new_from_pixbuf(gdk_pixbuf_new_from_file("./icons/bookmarksprev.png",NULL)));
    gtk_tool_button_set_icon_widget (GTK_TOOL_BUTTON (gtk_builder_get_object (builder, "toolbar_clear_bookmark")),
                                      gtk_image_new_from_pixbuf(gdk_pixbuf_new_from_file("./icons/bookmarksclear.png",NULL)));

    gtk_tool_button_set_icon_widget (GTK_TOOL_BUTTON (gtk_builder_get_object (builder, "toolbar_debug")),
                                      gtk_image_new_from_pixbuf(gdk_pixbuf_new_from_file("./icons/debug.png",NULL)));

    gtk_tool_button_set_icon_widget (GTK_TOOL_BUTTON (gtk_builder_get_object (builder, "toolbar_open_shell")),
                                      gtk_image_new_from_pixbuf(gdk_pixbuf_new_from_file("./icons/python_shell.png",NULL)));
    /*************************/
    
    line_history_menu = gtk_menu_new ();
    gtk_menu_item_set_submenu (GTK_MENU_ITEM (gtk_builder_get_object (builder, "navigate_line_history")),
                              line_history_menu);
    
    GtkRecentFilter *py_recent_filter = gtk_recent_filter_new ();
    gtk_recent_filter_set_name (py_recent_filter, "Python Files");
    gtk_recent_filter_add_pattern (py_recent_filter, "*.py");

    gtk_recent_chooser_add_filter (GTK_RECENT_CHOOSER (gtk_builder_get_object (builder, "recentchoosermenu")),
                                  py_recent_filter);
    
    /* Connecting window's signals and events */
    g_signal_connect (window, "delete-event",
                      G_CALLBACK (delete_event), NULL);
    g_signal_connect (window, "destroy",
                      G_CALLBACK (main_window_destroy), NULL);
    /********************************/
    
    /*Loading Options*/
    options.indent_width = 4;
    options.indent_width_str = "    ";
    options.comment_out_str = "##";
    options.tab_width = 4;
    options.tab_width_str = "    ";
    options.is_code_completion = TRUE;
    options.is_code_folding = TRUE;
    options.show_line_numbers = TRUE;
    options.python_shell_path = "/usr/bin/python";
    options.font_name = "Liberation Mono";
    /*************/
    
    /*Other global variables*/
    search_text = NULL;
    bookmark_array= NULL;
    bookmark_array_size = 0;
    current_bookmark_index = -1;
    /******************/

    /*Initialize Regular Expressions*/
    regex_class = g_regex_new ("[ \\ \\t]*\\bclass\\b\\s*\\w+\\s*\\(*.*\\)*:", 0, 0, NULL);
    regex_func = g_regex_new ("[ \\ \\t]*def\\s+[\\w\\d_]+\\s*\\(.+\\)\\:", 0, 0, NULL);
    
    /*Regex if you don't want to search imports with in indentation*/
    /*regex_import = g_regex_new ("^import\\s+[\\w\\d_\\.]+", 0, 0, NULL);
    regex_import_as = g_regex_new ("^import\\s+[\\w\\d_\\.]+\\s+as\\s+[\\w\\d_]+", 0, 0, NULL);*/
    regex_from_import = g_regex_new ("^from\\s+[\\w\\d_\\.]+\\s+import\\s+[\\w\\d_]+", 0, 0, NULL);
    regex_from_import_as = g_regex_new ("^from\\s+[\\w\\d_\\.]+\\s+import\\s+[\\w\\d_]+as\\s+[\\w\\d_]", 0, 0, NULL);
    regex_global_var = g_regex_new ("^[\\w\\d_]+\\s*=\\s*[\\w\\d_]+\\s*\\(.+\\)", 0, 0, NULL);
    regex_static_var = g_regex_new ("^\\s*[\\w\\d_]+\\s*=.+", 0, 0, NULL);
    regex_word = g_regex_new ("[self]*[\\w\\d_\\.]+$", 0, 0, NULL);
    regex_local_var = g_regex_new ("^\\s*[\\w\\d_\\.]+\\s*=.+", 0, 0, NULL);
    regex_self_var = g_regex_new ("^\\s+self\\.[\\w\\d_]+\\s*=.+", 0, 0, NULL);

    /*Regex if you want to search imports with in indentation*/
    regex_import = g_regex_new ("^\\s*import\\s+[\\w\\d_\\.]+", 0, 0, NULL);
    regex_import_as = g_regex_new ("^\\s*import\\s+[\\w\\d_]+\\s+as\\s+[\\w\\d_]+", 0, 0, NULL);

    /***********************/
    
    async_queue = g_async_queue_new ();

    //Creating code_widget_array
    //code_widget_array = g_malloc0 (1*sizeof (CodeWidget *));
    //code_widget_array [0] = codewidget_new ();
    code_widget_array_size = 0;
    
    notebook = gtk_notebook_new ();
    g_object_ref (notebook);
    //gtk_notebook_append_page (GTK_NOTEBOOK (notebook), code_widget_array [0]->vbox,
    //                                                 gtk_label_new ("New File"));
    /*If bash is not loaded*/
    if (!bash_loaded)
    {
        gchar *msg = g_strdup_printf ("Cannot run Python Shell (%s)", error->message);
        gtk_statusbar_push (GTK_STATUSBAR (status_bar), 0, msg);
        g_free (msg);
        g_error_free (error);
    }
    /****************/

    gtk_window_maximize (GTK_WINDOW (window));
    gtk_widget_show_all (GTK_WIDGET (window));
    gtk_main ();
    return 0;
}


/* "delete-event" handler for
 * windowstatic gboolean can_read_to_text_view;
 */
static gboolean
delete_event (GtkWidget *widget, GdkEvent *event)
{
    int *not_saved_tabs_array = NULL;
    int not_saved_tabs_array_size = 0;
    int i;
    
    /* Checking which files has been modified */
    for (i = 0; i < code_widget_array_size; i++)
    {
        if (is_file_modified (i))
        {
            not_saved_tabs_array = g_realloc (not_saved_tabs_array,
                                             (not_saved_tabs_array_size + 1)*sizeof (int));
            not_saved_tabs_array [not_saved_tabs_array_size] = i;
            not_saved_tabs_array_size ++;
        }
    }

    if (not_saved_tabs_array_size == 0)
    {
        GtkBuilder *dialog_builder = gtk_builder_new ();
        gtk_builder_add_from_file (dialog_builder, "./ui/closing_dialog.glade", NULL);
        GObject *dialog = gtk_builder_get_object (dialog_builder, "dialog");
        int response = gtk_dialog_run (GTK_DIALOG (dialog));
        gtk_widget_destroy (GTK_WIDGET (dialog));
        if (response == 0)
            return TRUE;
        else
            return FALSE;
    }
    
    /* Creating confirmation dialog */
    GtkWidget *dialog = gtk_dialog_new_with_buttons ("gIDLE",
                                                    GTK_WINDOW (window),
                                                    GTK_DIALOG_DESTROY_WITH_PARENT,
                                                    "Close without saving", 2, 
                                                    "Save", 1, "Cancel", 0, NULL);
    
    GtkWidget *not_saved_chk_array [not_saved_tabs_array_size];
    GtkWidget *dialog_content_area = gtk_dialog_get_content_area (GTK_DIALOG (dialog));

    for (i = 0; i < not_saved_tabs_array_size; i++)
    {
        const gchar *label;
        label = gtk_notebook_get_tab_label_text (GTK_NOTEBOOK (notebook),
                                                code_widget_array [i]->vbox);

        not_saved_chk_array [i] = gtk_check_button_new_with_label (label);
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (not_saved_chk_array [i]), TRUE);
        gtk_box_pack_start (GTK_BOX (dialog_content_area),
                           not_saved_chk_array [i], FALSE, FALSE, 2);
    }
    
    gtk_widget_show_all (dialog);

    int run = gtk_dialog_run (GTK_DIALOG (dialog));
    if (run == 0)
    {
        /* If "Cancel" was clicked */
        g_free (not_saved_tabs_array);
        gtk_widget_destroy (dialog);
        return TRUE;
    }

    if (run == 1)
    {
        /* If "Save" was clicked */
        for (i = 0; i < not_saved_tabs_array_size; i++)
        {
            if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (not_saved_chk_array [i])))
            {
                gtk_notebook_set_current_page (GTK_NOTEBOOK (notebook), not_saved_tabs_array [i]);
                file_save_activate (NULL);
            }
        }
        
        return FALSE;
    }
    
    if (run == 2)
    {
        /* If "Close without saving" was clicked */
        g_free (not_saved_tabs_array);
        gtk_widget_destroy (dialog);
        return FALSE;
    }
}

void
set_mode (int _mode)
{
    if (_mode == mode)
        return;

    gtk_widget_unparent (status_bar);
    mode = _mode;
    //if (gtk_widget_get_parent (notebook))
    //    gtk_container_remove (GTK_CONTAINER (gtk_widget_get_parent (notebook)), notebook);
   
    //if (gtk_widget_get_parent (proj_notebook))
    //    gtk_container_remove (GTK_CONTAINER (gtk_widget_get_parent (proj_notebook)), proj_notebook);

    if (!gtk_widget_get_parent (content_paned))
        gtk_box_pack_start (GTK_BOX (content_box), content_paned, TRUE, TRUE, 0);
    
    if (!gtk_widget_get_parent (proj_notebook))
        gtk_paned_pack1 (GTK_PANED (content_paned), proj_notebook, FALSE, FALSE);

    //if (gtk_notebook_get_n_pages (GTK_NOTEBOOK (proj_notebook)) == 4)
    //    project_notebook_show_tree_views (PROJECT_NOTEBOOK (proj_notebook), FALSE);
    
    //if (gtk_widget_get_parent (symbols_view))
    //     gtk_container_remove (GTK_CONTAINER (gtk_widget_get_parent (symbols_view)), symbols_view);

    if (_mode == GIDLE_MODE_FILE)
    {
        project_notebook_show_tree_views (PROJECT_NOTEBOOK (proj_notebook), FALSE);
        gtk_paned_pack2 (GTK_PANED (content_paned), notebook, TRUE, TRUE);
    }
    else
    {
        project_notebook_show_tree_views (PROJECT_NOTEBOOK (proj_notebook), TRUE);        
        gtk_paned_pack2 (GTK_PANED (content_paned), notebook, TRUE, TRUE);
        gtk_widget_set_size_request (proj_notebook, 200, 200);
    }

    gtk_box_pack_start (GTK_BOX (content_box), status_bar, FALSE, FALSE, 0);
    gtk_widget_show_all (content_box);
    
    path_browser_show_paths (path_browser);
}

static void
main_window_destroy (GtkWidget *widget)
{
    g_regex_unref (regex_class);     
    g_regex_unref (regex_func);
    g_regex_unref (regex_import);
    g_regex_unref (regex_import_as);
    g_regex_unref (regex_from_import);
    g_regex_unref (regex_from_import_as);
    g_regex_unref (regex_global_var);
    g_regex_unref (regex_static_var);
    g_regex_unref (regex_local_var);
    g_regex_unref (regex_self_var);

    remove_all_code_widgets ();
    kill (python_shell_data->pid, SIGKILL);
    g_free (python_shell_data);
    g_async_queue_unref (async_queue);
}

void
apply_changed_option (char *option_name)
{
    if (!g_strcmp0 (option_name, "font_name"))
    {
        PangoFontDescription *font_desc;
        font_desc = pango_font_description_from_string (options.font_name);
        int i;
        for (i = 0; i < code_widget_array_size; i++)
            gtk_widget_modify_font (code_widget_array[i]->sourceview, font_desc);

        pango_font_description_free (font_desc);
    }
    else if (!g_strcmp0 (option_name, "font_size"))
    {
    }
    else if(!g_strcmp0 (option_name, "word_wrap"))
    {
        int i;
        GtkWrapMode wrap_mode;
        if (options.word_wrap)
            wrap_mode = GTK_WRAP_WORD;
        else
            wrap_mode = GTK_WRAP_NONE;
        for (i = 0; i < code_widget_array_size; i++)
            gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (code_widget_array[i]->sourceview), wrap_mode);
    }
    else if(!g_strcmp0 (option_name, "highlight_curr_line"))
    {
        int i;
        for (i = 0; i < code_widget_array_size; i++)
            gtk_source_view_set_highlight_current_line (GTK_SOURCE_VIEW (code_widget_array [i]->sourceview), options.highlight_curr_line);
    }
    else if(!g_strcmp0 (option_name, "matching_brace"))
    {
    }
    else if(!g_strcmp0 (option_name, "indentation"))
    {
    }
    else if(!g_strcmp0 (option_name, "indent_width"))
    {
    }
    else if(!g_strcmp0 (option_name, "inc_indent_syms"))
    {
    }
    else if(!g_strcmp0 (option_name, "dec_indent_syms"))
    {
    }
    else if(!g_strcmp0 (option_name, "tab_width"))
    {
    }
    else if(!g_strcmp0 (option_name, "code_folding"))
    {
    }
    else if(!g_strcmp0 (option_name, "fold_comments"))
    {
    }
    else if(!g_strcmp0 (option_name, "fold_classes"))
    {
    }
    else if(!g_strcmp0 (option_name, "fold_functions"))
    {
    }
    else if(!g_strcmp0 (option_name, "code_completion"))
    {
    }
    else if(!g_strcmp0 (option_name, "variable_scoping"))
    {
    }
    else if(!g_strcmp0 (option_name, "line_numbers"))
    {
    }
    else if(!g_strcmp0 (option_name, "line_numbers_font_size"))
    {
    }
    else if(!g_strcmp0 (option_name, "syntax_highlighting"))
    {
    }
}

static void 
file_monitor_changed (GFileMonitor *fm, GFile *file, GFile *otherfile, GFileMonitorEvent event, gpointer data)
{
    gboolean event_got = FALSE;

    if (G_FILE_MONITOR_EVENT_CHANGED & event)
        event_got  = TRUE;

    if (G_FILE_MONITOR_EVENT_DELETED & event)
        event_got  = TRUE;

    if (G_FILE_MONITOR_EVENT_CREATED & event)
        event_got  = TRUE;

    if (G_FILE_MONITOR_EVENT_ATTRIBUTE_CHANGED & event)
        event_got  = TRUE;

    if (G_FILE_MONITOR_EVENT_CHANGES_DONE_HINT & event)
        event_got  = TRUE;

    if (G_FILE_MONITOR_EVENT_MOVED & event)
        event_got  = TRUE;
    
    if (!event_got)
        return;
    
    int i;
    gchar *file_path = g_file_get_path (file);
    for (i = 0; i < code_widget_array_size; i++)
        if (strcmp (code_widget_array [i]->file_path, file_path) == 0)
            break;
        
    if (i == code_widget_array_size)
        return;

    codewidget_show_modified_dialog (code_widget_array [i]);    
}