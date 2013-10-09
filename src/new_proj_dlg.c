#include "new_proj_dlg.h"
#include "project_class.h"

#include <string.h>

static
void cmdProjDir_clicked(GtkWidget *widget, gpointer data);

static void
update_entry_main_file_text ();

static void
entry_proj_dir_delete (GtkEntry *entry, GtkDeleteType type, gint count);

static void
entry_proj_dir_insert (GtkEntry *entry, GtkMovementStep, gint count, gboolean ext_sel);

static GtkWidget *entry_name ;
static GtkWidget *entry_proj_dir;
static GtkWidget *entry_main_file;

Project *
create_new_project (GtkWindow *parent)
{
    GtkBuilder *class_browser_builder;

    class_browser_builder = gtk_builder_new ();
    gtk_builder_add_from_file (class_browser_builder, "./ui/new_project_dialog.glade", NULL);
    
    GtkWidget *dialog = GTK_WIDGET (
        gtk_builder_get_object (class_browser_builder, "dialog"));
    entry_name = GTK_WIDGET (
        gtk_builder_get_object (class_browser_builder, "entry_name"));
    entry_proj_dir = GTK_WIDGET (
        gtk_builder_get_object (class_browser_builder, "entry_proj_dir"));
    entry_main_file = GTK_WIDGET (
        gtk_builder_get_object (class_browser_builder, "entry_main_file"));
    GtkWidget *entry_version = GTK_WIDGET (
        gtk_builder_get_object (class_browser_builder, "entry_version"));
    
    g_signal_connect (G_OBJECT (entry_proj_dir), "delete-from-cursor",
                      G_CALLBACK (entry_proj_dir_delete), NULL);
    g_signal_connect (G_OBJECT (entry_proj_dir), "insert-at-cursor",
                      G_CALLBACK (entry_proj_dir_insert), NULL);

    g_signal_connect (gtk_builder_get_object (class_browser_builder,
                                              "cmdProjDir"),
                      "clicked", G_CALLBACK (cmdProjDir_clicked),
                      entry_proj_dir);

    gtk_widget_show_all (dialog);
    
    if (gtk_dialog_run (GTK_DIALOG (dialog)) == 0)
    {
        gtk_widget_destroy (dialog);
        return NULL;
    }

    gchar *path = g_build_path ("/", gtk_entry_get_text (GTK_ENTRY (entry_proj_dir)), 
                                gtk_entry_get_text (GTK_ENTRY (entry_name)), NULL);
    gchar *proj_file_path = g_strconcat (path, ".py_proj", NULL);

    Project *new_proj = project_new (gtk_entry_get_text (GTK_ENTRY (entry_name)), 
                                     gtk_entry_get_text (GTK_ENTRY (entry_main_file)),
                                     proj_file_path, 
                                     gtk_entry_get_text (GTK_ENTRY (entry_proj_dir)),
                                     gtk_entry_get_text (GTK_ENTRY (entry_version)), 
                                     NULL, NULL);

    gtk_widget_destroy (dialog);

    g_free (path);
    g_free (proj_file_path);
    return new_proj;
}

static
void cmdProjDir_clicked(GtkWidget *widget, gpointer data)
{
    GtkWidget *dialog = gtk_file_chooser_dialog_new ("Open File", NULL,
                                                     GTK_FILE_CHOOSER_ACTION_CREATE_FOLDER,
                                                     GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                                     GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
                                                     NULL);

    if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
    {
        gchar *proj_dir = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
        GtkEntry *entry = GTK_ENTRY (data);
        gchar *text = proj_dir;
        if (!strrchr (text, '/') + 1)
            text = strrchr (text, '/') - 1;
        
        gchar *base_name = g_path_get_basename (text);
        if (g_strcmp0 (base_name, gtk_entry_get_text (GTK_ENTRY (entry_name))))
        {
            text = g_build_path ("/", proj_dir, gtk_entry_get_text (GTK_ENTRY (entry_name)), NULL);
            gtk_entry_set_text (entry, text);
            update_entry_main_file_text ();
        }
    }
    gtk_widget_destroy (dialog);    
}

static void
update_entry_main_file_text ()
{
    gchar *text = g_build_path ("/",
                                gtk_entry_get_text (GTK_ENTRY (entry_proj_dir)),
                                "main.py", NULL);
    gtk_entry_set_text (GTK_ENTRY (entry_main_file), text);
}

static void
entry_proj_dir_delete (GtkEntry *entry, GtkDeleteType type, gint count)
{
    update_entry_main_file_text ();
}

static void
entry_proj_dir_insert (GtkEntry *entry, GtkMovementStep step, gint count, gboolean ext_sel)
{
    update_entry_main_file_text ();
}