#include "main.h"
#include "menus.h"
#include "toolbar.h"

static gboolean
delete_event (GtkWidget *widget, GdkEvent *event);
static void
main_window_destroy (GtkWidget *widget);

GtkBuilder *builder;
extern gchar *search_text;
GtkWidget *status_bar;
ChildProcessData *python_shell_data;

int
main (int argc, char *argv [])
{    
    GtkWidget *navigate_bookmarks;
    
    gtk_init (&argc, &argv);

    builder = gtk_builder_new ();
    gtk_builder_add_from_file (builder, "./ui/main.ui", NULL);

    window = gtk_builder_get_object (builder, "main_window");
    g_signal_connect (window, "destroy", G_CALLBACK (gtk_main_quit), NULL);
    
    status_bar = GTK_WIDGET (gtk_builder_get_object (builder, "status_bar"));
    
    navigate_bookmarks = GTK_WIDGET (gtk_builder_get_object (builder,
                                    "navigate_bookmarks"));
    bookmarks_menu = gtk_menu_new ();

    gtk_menu_item_set_submenu (GTK_MENU_ITEM (navigate_bookmarks),
                              bookmarks_menu);

    /*Connecting menu item's signals*/ 
    //For File Menu
    g_signal_connect (gtk_builder_get_object (builder, "file_new"), "activate",
                                   G_CALLBACK (file_new_activate), NULL);
    g_signal_connect (gtk_builder_get_object (builder, "file_new_tab"), "activate",
                                   G_CALLBACK (file_new_tab_activate), NULL);
    g_signal_connect (gtk_builder_get_object (builder, "file_open"), "activate",
                                   G_CALLBACK (file_open_activate), NULL);
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
    g_signal_connect (gtk_builder_get_object (builder, "format_dec_indent"), "activate",
                                   G_CALLBACK (format_dec_indent_activate), NULL);
    g_signal_connect (gtk_builder_get_object (builder, "format_comment_out"), "activate",
                                   G_CALLBACK (format_comment_out_activate), NULL);
    g_signal_connect (gtk_builder_get_object (builder, "format_uncomment_out"), "activate",
                                   G_CALLBACK (format_uncomment_out_activate), NULL);
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

    /*************************/
    
    line_history_menu = gtk_menu_new ();
    gtk_menu_item_set_submenu (GTK_MENU_ITEM (gtk_builder_get_object (builder, "navigate_line_history")),
                              line_history_menu);

    /* Connecting window's signals and events */
    g_signal_connect (window, "delete-event",
                      G_CALLBACK (delete_event), NULL);
    g_signal_connect (window, "destroy",
                      G_CALLBACK (main_window_destroy), NULL);
    /********************************/

    //Creating code_widget_array
    code_widget_array = g_malloc0 (1*sizeof (CodeWidget *));
    code_widget_array [0] = codewidget_new ();
    code_widget_array_size = 1;
    
    notebook = gtk_builder_get_object (builder, "notebook");
    gtk_notebook_append_page (GTK_NOTEBOOK (notebook), code_widget_array [0]->vbox,
                                                     gtk_label_new ("New File"));
    
    /*Loading Options*/
    indent_width = 4;
    indent_width_str = "    ";
    comment_out_str = "##";
    tab_width = 4;
    tab_width_str = "    ";
    search_text = NULL;
    bookmark_array= NULL;
    bookmark_array_size = 0;
    current_bookmark_index = -1;
    mode = GIDLE_MODE_FILE;
    is_code_completion = TRUE;
    is_code_folding = TRUE;
    show_line_numbers = TRUE;
    python_shell_path = "/usr/bin/python";
    /*************/
    
    gtk_window_maximize (GTK_WINDOW (window));
    gtk_widget_show_all (GTK_WIDGET (window));
    gtk_main ();
    return 0;
}

/* "delete-event" handler for
 * window
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
        return FALSE;
    
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

static void
main_window_destroy (GtkWidget *widget)
{
    int i;

    for (i = 0; i < code_widget_array_size; i++)
        codewidget_destroy (code_widget_array [i]);
    
    g_free (code_widget_array);
    g_free (python_shell_data);
}