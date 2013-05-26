#include "run_script_dialog.h"

static GtkBuilder *run_script_dialog_builder;
static char *script_path;
static GtkWidget *combo_args;
static GtkWidget *combo_env_vars;
static gchar **combo_args_text_array = NULL;
static gchar **combo_env_vars_text_array = NULL;
static int combo_args_text_array_size = 0;
static int combo_env_vars_text_array_size = 0;
static GtkWidget *dialog;
static GtkFileChooserButton *cmdSelectDir;

/*To load 
 * run_script_dialog
 */
void
load_run_script_dialog (char *filename)
{
    g_return_if_fail (filename != NULL);

    run_script_dialog_builder = gtk_builder_new ();
    gtk_builder_add_from_file (run_script_dialog_builder, 
                              "./ui/run_script_dialog.glade", NULL);
    
    dialog = GTK_WIDGET (gtk_builder_get_object (run_script_dialog_builder,
                                                 "run_script_dialog"));

    g_signal_connect (gtk_builder_get_object (run_script_dialog_builder, "cmdRun"), "clicked",
                     G_CALLBACK (run_script_dialog_run_clicked), NULL);
    
    g_signal_connect (gtk_builder_get_object (run_script_dialog_builder, "cmdCancel"), "clicked",
                     G_CALLBACK (run_script_dialog_cancel_clicked), dialog);

    combo_args = gtk_combo_box_text_new_with_entry ();
    gtk_box_pack_start (GTK_BOX (gtk_builder_get_object (run_script_dialog_builder, "box1")),
                       combo_args, TRUE, TRUE, 2);

    combo_env_vars = gtk_combo_box_text_new_with_entry ();
    gtk_box_pack_start (GTK_BOX (gtk_builder_get_object (run_script_dialog_builder, "box3")),
                       combo_env_vars, TRUE, TRUE, 2);
    
    gtk_widget_set_tooltip_text (combo_env_vars, "Add Environment Variables in form of 'VAR1=VALUE1 VAR2=VALUE2'");

    if (combo_args_text_array_size > 0)
    {
        int i;
        for (i = 0; i < combo_args_text_array_size; i++)
            gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (combo_args), combo_args_text_array [i]);
    }

    if (combo_env_vars_text_array_size > 0)
    {
        int i;
        for (i = 0; i < combo_env_vars_text_array_size; i++)
           gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (combo_env_vars), combo_env_vars_text_array [i]);
    }
    
    char *dir_path = g_path_get_dirname (filename);
    cmdSelectDir = GTK_FILE_CHOOSER_BUTTON (gtk_builder_get_object (run_script_dialog_builder,
                                                 "cmdSelectDir"));
    if (dir_path)
        gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (cmdSelectDir), dir_path);

    g_free (dir_path);

    gtk_widget_show_all (dialog);
    
    gtk_label_set_text (GTK_LABEL (gtk_builder_get_object (run_script_dialog_builder,
                                                 "lblscriptpath")), filename);

    script_path = filename;
}

/* When cmdRun is clicked
 * in run_script_dialog
 */
void
run_script_dialog_run_clicked (GtkWidget *widget)
{
    gchar **combo_box_env_vars = NULL;
    gchar **combo_box_argvs = NULL;       

    if (g_strcmp0 (gtk_combo_box_text_get_active_text (GTK_COMBO_BOX_TEXT (combo_args)), "") != 0)
    {
        combo_box_argvs = g_strsplit (gtk_combo_box_text_get_active_text (GTK_COMBO_BOX_TEXT (combo_args)), " ", 0);
        combo_args_text_array = g_realloc (combo_args_text_array,
                                              (combo_args_text_array_size + 1) * sizeof (gchar *));
        combo_args_text_array [combo_args_text_array_size] = g_strdup (gtk_combo_box_text_get_active_text (GTK_COMBO_BOX_TEXT (combo_args)));
        combo_args_text_array_size ++;
    }

    if (g_strcmp0 (gtk_combo_box_text_get_active_text (GTK_COMBO_BOX_TEXT (combo_env_vars)), "") != 0)
    {
        combo_box_env_vars = g_strsplit (gtk_combo_box_text_get_active_text (GTK_COMBO_BOX_TEXT (combo_env_vars)), " ", 0);
        combo_env_vars_text_array = g_realloc (combo_env_vars_text_array,
                                              (combo_env_vars_text_array_size + 1) * sizeof (gchar *));
        combo_env_vars_text_array [combo_env_vars_text_array_size] = g_strdup (gtk_combo_box_text_get_active_text (GTK_COMBO_BOX_TEXT (combo_args)));
        combo_env_vars_text_array_size ++;
    }
   
    char *dir = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (cmdSelectDir));

    run_file_in_python_shell (script_path, dir, combo_box_argvs, combo_box_env_vars);
    gtk_widget_destroy (dialog);
}

/* When cmdCancel is 
 * clicked in run_script_dialog
 */
void
run_script_dialog_cancel_clicked (GtkWidget *widget, gpointer dialog)
{
    gtk_widget_destroy (dialog);
}