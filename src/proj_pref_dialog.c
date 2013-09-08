#include "main.h"
#include "proj_pref_dialog.h"
#include "proj_notebook.h"

static GtkBuilder *proj_pref_builder;
static GtkWidget *pref_proj_notebook;

void
load_settings ();

void
open_proj_pref ()
{
    if (!current_project)
        return;

    proj_pref_builder = gtk_builder_new ();
    gtk_builder_add_from_file (proj_pref_builder, "./ui/project_prefs.glade", NULL);

    GtkWidget *dialog = GTK_WIDGET (
        gtk_builder_get_object (proj_pref_builder, "dialog"));
    
    pref_proj_notebook = project_notebook_new ();
    gtk_box_pack_end (GTK_BOX (gtk_builder_get_object (proj_pref_builder, "box2")), pref_proj_notebook, TRUE, TRUE, 2);

    load_settings ();

    gtk_widget_set_size_request (dialog, 400, 300);
    gtk_widget_show_all (dialog);
    gtk_dialog_run (GTK_DIALOG (dialog));
    gtk_widget_destroy (dialog);    
}

void
load_settings ()
{
    gtk_entry_set_text (GTK_ENTRY (gtk_builder_get_object (proj_pref_builder, "entry_name")), current_project->name);
    gtk_entry_set_text (GTK_ENTRY (gtk_builder_get_object (proj_pref_builder, "entry_version")), current_project->version);
    gtk_entry_set_text (GTK_ENTRY (gtk_builder_get_object (proj_pref_builder, "entry_main_file")), current_project->main_file+1);
    project_notebook_open_project (PROJECT_NOTEBOOK (pref_proj_notebook), current_project);
    gtk_entry_set_text (GTK_ENTRY (gtk_builder_get_object (proj_pref_builder, "entry_curr_dir")), current_project->current_dir);
}