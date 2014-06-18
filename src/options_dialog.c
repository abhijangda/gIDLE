#include "main.h"
#include "core_funcs.h"

#include <string.h>

static GtkBuilder *options_dlg_builder;
extern gIDLEOptions options;
static GtkWidget *treeview_envs;
static GtkTreeModel *treemodel_envs;

void
load_options_from_file (char *filepath);

void
update_dlg ();

void
cmd_ok_clicked (GtkWidget *widget);

void
cmd_add_clicked (GtkWidget *widget);

void
cmd_remove_clicked (GtkWidget *widget);

void
apply_options ();

void
save_options_to_file (char *);

void env_var_edited (GtkCellRendererText *cell,
                           gchar               *path_string,
                           gchar               *new_text,
                           gpointer             user_data)
{
    GtkTreeIter iter;
    if (gtk_tree_model_get_iter_from_string (treemodel_envs, &iter, path_string))
        gtk_list_store_set (GTK_LIST_STORE (treemodel_envs), &iter, 0, new_text, -1);
}
void
load_options_dlg()
{
    options_dlg_builder = gtk_builder_new ();
    gtk_builder_add_from_file (options_dlg_builder, "./ui/options_dialog.glade", NULL);

    GtkWidget *dialog = GTK_WIDGET (
        gtk_builder_get_object (options_dlg_builder, "dialog"));
    
    treeview_envs = GTK_WIDGET (gtk_builder_get_object (options_dlg_builder, "treeview_envs"));
    treemodel_envs = GTK_TREE_MODEL (gtk_list_store_new (1, G_TYPE_STRING));
     
    GtkCellRenderer *renderer = gtk_cell_renderer_text_new ();
    g_object_set (GTK_CELL_RENDERER_TEXT (renderer), "editable", TRUE, NULL);
    //g_object_set (GTK_CELL_RENDERER_TEXT (renderer), "mode", GTK_CELL_RENDERER_MODE_EDITABLE);
    g_signal_connect (G_OBJECT (renderer), "edited", G_CALLBACK (env_var_edited), NULL);

    GtkTreeViewColumn *column = gtk_tree_view_column_new ();   
    gtk_tree_view_column_pack_start (column, renderer, TRUE);
    gtk_tree_view_column_set_attributes (column, renderer, "text", 0, NULL);
    
    gtk_tree_view_set_model (GTK_TREE_VIEW (treeview_envs),
                              GTK_TREE_MODEL (treemodel_envs));
    gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (treeview_envs),
                                        FALSE);
    
    gtk_tree_view_append_column (GTK_TREE_VIEW (treeview_envs), column);

    load_options_from_file ("./options.inf");
    update_dlg ();
    
    g_signal_connect (G_OBJECT (gtk_builder_get_object (options_dlg_builder, "cmdOk")), "clicked", G_CALLBACK (cmd_ok_clicked), NULL);

    g_signal_connect (G_OBJECT (gtk_builder_get_object (options_dlg_builder, "cmdAdd")), "clicked", G_CALLBACK (cmd_add_clicked), NULL);
    g_signal_connect (G_OBJECT (gtk_builder_get_object (options_dlg_builder, "cmdRemove")), "clicked", G_CALLBACK (cmd_remove_clicked), NULL);
    
    gtk_font_button_set_show_style (GTK_FONT_BUTTON (gtk_builder_get_object (options_dlg_builder, "fontbutton")), FALSE);
    gtk_font_button_set_show_size (GTK_FONT_BUTTON (gtk_builder_get_object (options_dlg_builder, "fontbutton")), FALSE);
    gtk_widget_set_size_request (dialog, 400, 300);
    gtk_widget_show_all (dialog);
    gtk_dialog_run (GTK_DIALOG (dialog));
    gtk_widget_destroy (dialog);
}

void
cmd_ok_clicked (GtkWidget *widget)
{
    apply_options ();
    save_options_to_file ("./options.inf");
}

void
cmd_add_clicked (GtkWidget *widget)
{
    GtkTreeIter iter;
    gtk_list_store_append (GTK_LIST_STORE (treemodel_envs), &iter);
    gtk_tree_selection_select_iter (gtk_tree_view_get_selection (GTK_TREE_VIEW (treeview_envs)), &iter);
}

void
cmd_remove_clicked (GtkWidget *widget)
{
    GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (treeview_envs));
    GtkTreeIter iter;
    if (gtk_tree_selection_get_selected (selection, &treemodel_envs, &iter))
        gtk_list_store_remove (GTK_LIST_STORE (treemodel_envs), &iter);
}

void
update_dlg()
{
    /*Editor*/
    gtk_font_button_set_font_name (GTK_FONT_BUTTON (gtk_builder_get_object (options_dlg_builder, "fontbutton")), options.font_name);
    gtk_font_button_set_show_size (GTK_FONT_BUTTON (gtk_builder_get_object (options_dlg_builder, "fontbutton")), options.font_size);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (gtk_builder_get_object (options_dlg_builder, "chk_wordwrap")), options.word_wrap);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (gtk_builder_get_object (options_dlg_builder, "chk_curr_line")), options.highlight_curr_line);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (gtk_builder_get_object (options_dlg_builder, "chk_matching_brace")), options.matching_brace);

    /*Indentation*/
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (gtk_builder_get_object (options_dlg_builder, "chk_indentation")), options.indentation);
    gtk_spin_button_set_value (GTK_SPIN_BUTTON (gtk_builder_get_object (options_dlg_builder, "spin_indent_width")), options.indent_width);
    gtk_entry_set_text (GTK_ENTRY (gtk_builder_get_object (options_dlg_builder, "entry_inc_indent_syms")), options.inc_indent_syms);
    gtk_entry_set_text (GTK_ENTRY (gtk_builder_get_object (options_dlg_builder, "entry_dec_indent_syms")), options.dec_indent_syms);
    //gtk_spin_button_set_value (GTK_SPIN_BUTTON (gtk_builder_get_object (options_dlg_builder, "spin_tab_width")), options.tab_width);

    /*Code Folding*/
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (gtk_builder_get_object (options_dlg_builder, "chk_code_folding")), options.code_folding);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (gtk_builder_get_object (options_dlg_builder, "chk_fold_comments")), options.fold_comments);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (gtk_builder_get_object (options_dlg_builder, "chk_fold_classes")), options.fold_classes);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (gtk_builder_get_object (options_dlg_builder, "chk_fold_functions")), options.fold_functions);

    /*Code Completion*/
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (gtk_builder_get_object (options_dlg_builder, "chk_code_completion")), options.code_completion);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (gtk_builder_get_object (options_dlg_builder, "chk_variable_scoping")), options.variable_scoping);
    
    /*Line Numders*/
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (gtk_builder_get_object (options_dlg_builder, "chk_line_numbers")), options.line_numbers);
    gtk_spin_button_set_value (GTK_SPIN_BUTTON (gtk_builder_get_object (options_dlg_builder, "spin_line_numbers_font_size")), options.line_numbers_font_size);
    
    /*Syntax Highlighting*/
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (gtk_builder_get_object (options_dlg_builder, "chk_syntax_highlighting")), options.syntax_highlighting);
    
    /*Python Shell*/
    GtkTreeIter iter;
    gchar **p = options.env_vars;
    while (*p)
    {
        gtk_list_store_append (GTK_LIST_STORE (treemodel_envs), &iter);
        gtk_list_store_set 	(GTK_LIST_STORE (treemodel_envs), &iter, 0, *p, -1);
        p++;
    }
}

void
apply_options ()
{
    gchar *new_str_value = NULL;
    gint new_int_value;

    /*Editor*/
    new_str_value = (gchar *)gtk_font_chooser_get_font (GTK_FONT_CHOOSER (gtk_builder_get_object (options_dlg_builder, "fontbutton")));
    if (g_strcmp0 (new_str_value, options.font_name))
    {
        options.font_name = new_str_value;
        apply_changed_option ("font_name");
    }

    new_int_value = gtk_font_chooser_get_font_size (GTK_FONT_CHOOSER (gtk_builder_get_object (options_dlg_builder, "fontbutton")));
    if (new_int_value != options.font_size)
    {
        options.font_size = new_int_value;
        apply_changed_option ("font_size");
    }

    new_int_value = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (gtk_builder_get_object (options_dlg_builder, "chk_wordwrap")));
    if (new_int_value != options.word_wrap)
    {
        options.word_wrap = new_int_value;
        apply_changed_option ("word_wrap");
    }

    new_int_value = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (gtk_builder_get_object (options_dlg_builder, "chk_curr_line")));
    if (new_int_value != options.highlight_curr_line)
    {
        options.highlight_curr_line = new_int_value;
        apply_changed_option ("highlight_curr_line");
    }

    new_int_value = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (gtk_builder_get_object (options_dlg_builder, "chk_matching_brace")));
    if (new_int_value != options.matching_brace)
    {
        options.matching_brace = new_int_value;
        apply_changed_option ("matching_brace");
    }

    /*Indentation*/
    new_int_value = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (gtk_builder_get_object (options_dlg_builder, "chk_indentation")));
    if (new_int_value != options.indentation)
    {
        options.indentation = new_int_value;
        apply_changed_option ("indentation");
    }

    new_int_value = gtk_spin_button_get_value (GTK_SPIN_BUTTON (gtk_builder_get_object (options_dlg_builder, "spin_indent_width")));
    if (new_int_value != options.indent_width)
    {
        options.indent_width = new_int_value;
        apply_changed_option ("indent_width");
    }

    new_str_value = (gchar *)gtk_entry_get_text (GTK_ENTRY (gtk_builder_get_object (options_dlg_builder, "entry_inc_indent_syms")));
    if (g_strcmp0 (new_str_value, options.inc_indent_syms))
    {
        options.inc_indent_syms = new_str_value;
        apply_changed_option ("inc_indent_syms");
    }

    options.dec_indent_syms = (gchar *)gtk_entry_get_text (GTK_ENTRY (gtk_builder_get_object (options_dlg_builder, "entry_dec_indent_syms")));
    if (g_strcmp0 (new_str_value, options.dec_indent_syms))
    {
        options.dec_indent_syms = new_str_value;
        apply_changed_option ("dec_indent_syms");
    }

    new_int_value = gtk_spin_button_get_value (GTK_SPIN_BUTTON (gtk_builder_get_object (options_dlg_builder, "spin_tab_width")));
    if (new_int_value != options.tab_width)
    {
        options.tab_width = new_int_value;
        apply_changed_option ("tab_width");
    }

    /*Code Folding*/
    new_int_value = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (gtk_builder_get_object (options_dlg_builder, "chk_code_folding")));
    if (new_int_value != options.code_folding)
    {
        options.code_folding = new_int_value;
        apply_changed_option ("code_folding");
    }

    new_int_value = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (gtk_builder_get_object (options_dlg_builder, "chk_fold_comments")));
    if (new_int_value != options.fold_comments)
    {
        options.fold_comments = new_int_value;
        apply_changed_option ("fold_comments");
    }

    new_int_value= gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (gtk_builder_get_object (options_dlg_builder, "chk_fold_classes")));
    if (new_int_value != options.fold_classes)
    {
        options.fold_classes = new_int_value;
        apply_changed_option ("fold_classes");
    }

    new_int_value= gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (gtk_builder_get_object (options_dlg_builder, "chk_fold_functions")));
    if (new_int_value != options.fold_functions)
    {
        options.fold_functions = new_int_value;
        apply_changed_option ("fold_functions");
    }
    
    /*Code Completion*/
    options.code_completion = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (gtk_builder_get_object (options_dlg_builder, "chk_code_completion")));
    if (new_int_value != options.fold_functions)
    {
        options.fold_functions = new_int_value;
        apply_changed_option ("code_completion");
    }

    new_int_value = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (gtk_builder_get_object (options_dlg_builder, "chk_variable_scoping")));
    if (new_int_value != options.variable_scoping)
    {
        options.variable_scoping = new_int_value;
        apply_changed_option ("variable_scoping");
    }
    
    /*Line Numders*/
    new_int_value = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (gtk_builder_get_object (options_dlg_builder, "chk_line_numbers")));
    if (new_int_value != options.line_numbers)
    {
        options.line_numbers = new_int_value;
        apply_changed_option ("line_numbers");
    }

    new_int_value = gtk_spin_button_get_value (GTK_SPIN_BUTTON (gtk_builder_get_object (options_dlg_builder, "spin_line_numbers_font_size")));
    if (new_int_value != options.line_numbers_font_size)
    {
        options.line_numbers_font_size = new_int_value;
        apply_changed_option ("line_numbers_font_size");
    }
    
    /*Syntax Highlighting*/
    new_int_value = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (gtk_builder_get_object (options_dlg_builder, "chk_syntax_highlighting")));
    if (new_int_value != options.syntax_highlighting)
    {
        options.syntax_highlighting = new_int_value;
        apply_changed_option ("syntax_highlighting");
    }
    
    /*Python Shell*/
    GtkTreeIter iter;
    g_strfreev (options.env_vars);
    options.env_vars = NULL;
    if (gtk_tree_model_get_iter_first (treemodel_envs, &iter))
    {
        int size = 1;
        do 
        {
            gchar *var;
            gtk_tree_model_get (treemodel_envs, &iter, 0, &var, -1);
            options.env_vars = g_realloc (options.env_vars, sizeof (gchar *)*(size));
            options.env_vars [size - 1] = var;
            size++;
        }
        while (gtk_tree_model_iter_next (treemodel_envs, &iter));
        options.env_vars = g_realloc (options.env_vars, sizeof (gchar *)*(size));
        options.env_vars [size - 1] = NULL;
        gchar **p = options.env_vars;
        while (*p)
        {
            printf ("%s\n", *p);
            p++;
        }
    }
}

void
save_options_to_file (char *filepath)
{
    GString *contents = g_string_new("");
    char *str_value = NULL;

     /*Editor Options*/
    g_string_append(contents, "<font_name>");
    g_string_append(contents, options.font_name);
    g_string_append(contents, "</font_name>\n");

    g_string_append(contents, "<font_size>");
    str_value = g_strdup_printf("%d", options.font_size);
    g_string_append(contents, str_value);
    g_free (str_value);
    g_string_append(contents, "</font_size>\n");    

    g_string_append(contents, "<word_wrap>");
    str_value = g_strdup_printf("%d", options.word_wrap);
    g_string_append(contents, str_value);
    g_free (str_value);
    g_string_append(contents, "</word_wrap>\n");

    g_string_append(contents, "<curr_line>");
    str_value = g_strdup_printf("%d", options.highlight_curr_line);
    g_string_append(contents, str_value);
    g_free (str_value);
    g_string_append(contents, "</curr_line>\n");
    
    g_string_append(contents, "<matching_brace>");
    str_value = g_strdup_printf("%d", options.highlight_matching_brace);
    g_string_append(contents, str_value);
    g_free (str_value);
    g_string_append(contents, "</matching_brace>\n");
    
    /*Indentation Options*/
    g_string_append(contents, "<indentation>");
    str_value = g_strdup_printf("%d", options.indentation);
    g_string_append(contents, str_value);
    g_free (str_value);
    g_string_append(contents, "</indentation>\n");
    
    g_string_append(contents, "<indent_width>");
    str_value = g_strdup_printf("%d", options.indent_width);
    g_string_append(contents, str_value);
    g_free (str_value);
    g_string_append(contents, "</indent_width>\n");
    
    g_string_append(contents, "<inc_indent_syms>");
    g_string_append(contents, options.inc_indent_syms);
    g_string_append(contents, "</inc_indent_syms>\n");
    
    g_string_append(contents, "<dec_indent_syms>");
    g_string_append(contents, options.dec_indent_syms);
    g_string_append(contents, "</dec_indent_syms>\n");
    
    g_string_append(contents, "<tab_width>");
    str_value = g_strdup_printf("%d", options.tab_width);
    g_string_append(contents, str_value);
    g_free (str_value);
    g_string_append(contents, "</tab_width>\n");
    
    /*Code Folding*/
    g_string_append(contents, "<enable_folding>");
    str_value = g_strdup_printf("%d", options.enable_folding);
    g_string_append(contents, str_value);
    g_free (str_value);
    g_string_append(contents, "</enable_folding>\n");
    
    g_string_append(contents, "<fold_comments>");
    str_value = g_strdup_printf("%d", options.fold_comments);
    g_string_append(contents, str_value);
    g_free (str_value);
    g_string_append(contents, "</fold_comments>\n");
    
    g_string_append(contents, "<fold_classes>");
    str_value = g_strdup_printf("%d", options.fold_classes);
    g_string_append(contents, str_value);
    g_free (str_value);
    g_string_append(contents, "</fold_classes>");
    
    g_string_append(contents, "<fold_functions>");
    str_value = g_strdup_printf("%d", options.fold_functions);
    g_string_append(contents, str_value);
    g_free (str_value);
    g_string_append(contents, "</fold_functions>\n");
    
     /*Code Completion*/
    g_string_append(contents, "<completion>");
    str_value = g_strdup_printf("%d", options.completion);
    g_string_append(contents, str_value);
    g_free (str_value);
    g_string_append(contents, "</completion>\n"); 
    
    g_string_append(contents, "<variable_scoping>");
    str_value = g_strdup_printf("%d", options.variable_scoping);
    g_string_append(contents, str_value);
    g_free (str_value);
    g_string_append(contents, "</variable_scoping>\n"); 

    /*Line Numbers*/
    g_string_append(contents, "<line_numbers>");
    str_value = g_strdup_printf("%d", options.line_numbers);
    g_string_append(contents, str_value);
    g_free (str_value);
    g_string_append(contents, "</line_numbers>\n");
    
    g_string_append(contents, "<line_numbers_font_size>");
    str_value = g_strdup_printf("%d", options.line_numbers_font_size);
    g_string_append(contents, str_value);
    g_free (str_value);
    g_string_append(contents, "</line_numbers_font_size>\n");
    
    /*Syntax Highlighting*/
    g_string_append(contents, "<syntax_highlighting>");
    str_value = g_strdup_printf("%d", options.syntax_highlighting);
    g_string_append(contents, str_value);
    g_free (str_value);
    g_string_append(contents, "</syntax_highlighting>\n");
    
    /*Python Shell*/
    g_string_append(contents, "<env_vars>");    
    gchar **p = options.env_vars;
    while (*(p+1))
    {
        g_string_append (contents, *p);
        g_string_append (contents, "\n");
        p++;
    }
    g_string_append (contents, *p);
    g_string_append(contents, "</env_vars>\n");

    set_file_data (filepath, contents->str, contents->len);
    g_string_free (contents, TRUE);
}

void
load_options_from_file (char *filepath)
{
    char *contents = get_file_data (filepath), *string_end;
    if (!contents)
        return;

    /*Editor Options*/
    gchar *value = get_text_between_strings (contents, 
                                             g_strstr_len(contents, -1, "<font_name>") + strlen ("<font_name>"),
                                             g_strstr_len(contents, -1, "</font_name>") - 1);
    if (value)
        options.font_name = value;

    value = get_text_between_strings (contents,
                                      g_strstr_len(contents, -1, "<font_size>") + strlen ("<font_size>"), 
                                      g_strstr_len(contents, -1, "</font_size>") - 1);
    if (value)
        options.font_size = g_strtod (value, NULL);
    g_free (value);

    value = get_text_between_strings (contents,
                                      g_strstr_len(contents, -1, "<word_wrap>") + strlen ("<word_wrap>"), 
                                      g_strstr_len(contents, -1, "</word_wrap>") - 1);
    if (value)
    {
        if (g_strcmp0 (value, "0") == 0)
            options.word_wrap = FALSE;
        else
            options.word_wrap = TRUE;
    }

    g_free (value);

    value = get_text_between_strings (contents, 
                                      g_strstr_len(contents, -1, "<curr_line>") + strlen ("<curr_line>"), 
                                      g_strstr_len(contents, -1, "</curr_line>") - 1);
    if (value)
    {
        if (g_strcmp0 (value, "0") == 0)
            options.highlight_curr_line = FALSE;
        else
            options.highlight_curr_line = TRUE;
    }
    
    g_free (value);

    value = get_text_between_strings (contents,
                                      g_strstr_len(contents, -1, "<matching_brace>") + strlen ("<matching_brace>"),
                                      g_strstr_len(contents, -1, "</matching_brace>") - 1);
    if (value)
    {
        if (g_strcmp0 (value, "0") == 0)
            options.highlight_matching_brace = FALSE ;
        else
            options.highlight_matching_brace = TRUE;
    }
    
    /*Indentation Options*/
    value = get_text_between_strings (contents,
                                      g_strstr_len(contents, -1, "<indentation>") + strlen ("<indentation>"), 
                                      g_strstr_len(contents, -1, "</indentation>") - 1);
    if (value)
    {
        if (g_strcmp0 (value, "0") == 0)
            options.indentation = FALSE;
        else
            options.indentation = TRUE;
    }
    
    value = get_text_between_strings (contents, 
                                      g_strstr_len(contents, -1, "<indent_width>") + strlen ("<indent_width>"),
                                      g_strstr_len(contents, -1, "</indent_width>") - 1);
    if (value)
    {
        options.indent_width = g_strtod (value, NULL);
    }
    
    value = get_text_between_strings (contents, 
                                      g_strstr_len(contents, -1, "<inc_indent_syms>") + strlen ("<inc_indent_syms>"), 
                                      g_strstr_len(contents, -1, "</inc_indent_syms>") - 1);
    if (value)
    {
        options.inc_indent_syms = value;
    }

    value = get_text_between_strings (contents, 
                                      g_strstr_len(contents, -1, "<dec_indent_syms>") + strlen ("<dec_indent_syms>"), 
                                      g_strstr_len(contents, -1, "</dec_indent_syms>") - 1);
    if (value)
    {
        options.dec_indent_syms = value;
    }
    
    value = get_text_between_strings (contents,
                                      g_strstr_len(contents, -1, "<tab_width>") + strlen ("<tab_width>"),
                                      g_strstr_len(contents, -1, "</tab_width>") - 1);
    if (value)
    {
        if (g_strcmp0 (value, "0") == 0)
            options.tab_width = FALSE;
        else
            options.tab_width = TRUE;
    }
    
    /*Code Folding*/
    value = get_text_between_strings (contents,
                                      g_strstr_len(contents, -1, "<enable_folding>") + strlen ("<enable_folding>"),
                                      g_strstr_len(contents, -1, "</enable_folding>") - 1);
    if (value)
    {
        if (g_strcmp0 (value, "0") == 0)
            options.enable_folding = FALSE;
        else
            options.enable_folding = TRUE;
    }
    
    value = get_text_between_strings (contents,
                                      g_strstr_len(contents, -1, "<fold_comments>") + strlen ("<fold_comments>"),
                                      g_strstr_len(contents, -1, "</fold_comments>") - 1);
    if (value)
    {
        if (g_strcmp0 (value, "0") == 0)
            options.fold_comments = FALSE;
        else
            options.fold_comments = TRUE;
    }
    
    value = get_text_between_strings (contents,
                                      g_strstr_len(contents, -1, "<fold_classes>") + strlen ("<fold_classes>"),
                                      g_strstr_len(contents, -1, "</fold_classes>") - 1);
    if (value)
    {
        if (g_strcmp0 (value, "0") == 0)
            options.fold_classes = FALSE;
        else
            options.fold_classes = TRUE;
    }
    
    value = get_text_between_strings (contents,
                                      g_strstr_len(contents, -1, "<fold_functions>") + strlen ("<fold_functions>"),
                                      g_strstr_len(contents, -1, "</fold_functions>") - 1);
    if (value)
    {
        if (g_strcmp0 (value, "0") == 0)
            options.fold_functions = FALSE;
        else
            options.fold_functions = TRUE;
    }

    /*Code Completion*/
    value = get_text_between_strings (contents, 
                                      g_strstr_len(contents, -1, "<completion>") + strlen ("<completion>"),
                                      g_strstr_len(contents, -1, "</completion>") - 1);
    if (value)
    {
        if (g_strcmp0 (value, "0") == 0)
            options.completion = FALSE;
        else
            options.completion = TRUE;
    }
    
    value = get_text_between_strings (contents, 
                                      g_strstr_len(contents, -1, "<variable_scoping>") + strlen ("<variable_scoping>"), 
                                      g_strstr_len(contents, -1, "</variable_scoping>") - 1);
    if (value)
    {
        if (g_strcmp0 (value, "0") == 0)
            options.variable_scoping = FALSE;
        else
            options.variable_scoping = TRUE;
    }
    
    /*Line Numbers*/
    value = get_text_between_strings (contents, 
                                      g_strstr_len(contents, -1, "<line_numbers>") + strlen ("<line_numbers>"), 
                                      g_strstr_len(contents, -1, "</line_numbers>") - 1);
    if (value)
    {
        if (g_strcmp0 (value, "0") == 0)
            options.line_numbers = FALSE;
        else
            options.line_numbers = TRUE;
        
        printf ("line%d\n", options.line_numbers);
    }
    
    value = get_text_between_strings (contents, 
                                      g_strstr_len(contents, -1, "<line_numbers_font_size>") + strlen ("<line_numbers_font_size>"), 
                                      g_strstr_len(contents, -1, "</line_numbers_font_size>") - 1);
    if (value)
    {
        if (g_strcmp0 (value, "0") == 0)
            options.line_numbers_font_size = FALSE;
        else
            options.line_numbers_font_size = TRUE;
    }
    
    /*Syntax Highlighting*/
    value = get_text_between_strings (contents, 
                                      g_strstr_len(contents, -1, "<syntax_highlighting>") + strlen ("<syntax_highlighting>"), 
                                      g_strstr_len(contents, -1, "</syntax_highlighting>") - 1);
    if (value)
    {
        if (g_strcmp0 (value, "0") == 0)
            options.syntax_highlighting = FALSE;
        else
            options.syntax_highlighting = TRUE;
    }

    /*Python Shell*/
    value = get_text_between_strings (contents,
                                      g_strstr_len(contents, -1, "<env_vars>") + strlen ("<env_vars>"),
                                      g_strstr_len(contents, -1, "</env_vars>") - 1);

    if (value)
    {
        options.env_vars = g_strsplit (value, "\n", 0);
    }
}