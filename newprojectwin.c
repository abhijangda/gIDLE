/* 81 Lines*/
#include<gtk/gtk.h>

GtkWidget *new_proj_window,*lblprjname,*txtprjname,*cmdpath,*lblpath,*txtpath,*cmdcreate,*cmdcancel;
extern char *projpath;
extern GString *proj_name;
void new_proj_window_destroyed(GtkWidget *widget);

void cmdcancel_clicked(GtkWidget *widget)
{
    gtk_widget_destroy(new_proj_window);
    }

void cmdcreate_clicked(GtkWidget *widget)
{
    proj_name  = g_string_new(gtk_entry_get_text(GTK_ENTRY(txtprjname)));
    GString *projstring;
    projstring = g_string_new("<name>");
    g_string_append(projstring,proj_name->str);
    g_string_append(projstring,"</name>");
    g_file_set_contents(projpath,projstring->str,-1,NULL);
    g_string_free(projstring,TRUE);
    cmdcancel_clicked(widget);
    }

void cmdpath_clicked(GtkWidget *widget)
{
    GtkWidget *dialog;
    GtkFileFilter *filefilter_pyproj;
    filefilter_pyproj = gtk_file_filter_new();
    gtk_file_filter_set_name(filefilter_pyproj,"Python Project Files(*.pyproj)");
    gtk_file_filter_add_pattern(filefilter_pyproj,"*.pyproj");
    dialog = gtk_file_chooser_dialog_new("Save File", GTK_WINDOW(new_proj_window), GTK_FILE_CHOOSER_ACTION_SAVE,GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,GTK_STOCK_SAVE,GTK_RESPONSE_ACCEPT,NULL);
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog),filefilter_pyproj);
    int result;
    if ((result=gtk_dialog_run(GTK_DIALOG(dialog)))==GTK_RESPONSE_ACCEPT)
    {
        projpath = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        gtk_entry_set_text(GTK_ENTRY(txtpath),projpath);
        }
    gtk_widget_destroy(dialog);
    }

extern void new_proj_activated(GtkWidget *widget, GtkWindow *parent)
{
    new_proj_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    
    lblprjname = gtk_label_new("Project Name");
    txtprjname = gtk_entry_new();
    lblpath = gtk_label_new("Save to         ");
    txtpath = gtk_entry_new();
    cmdpath = gtk_button_new_with_mnemonic("....");
    cmdcreate = gtk_button_new_with_mnemonic("Create");
    cmdcancel = gtk_button_new_with_mnemonic("Cancel");
    
    GtkWidget *hbox1,*hbox2,*hbox3,*vbox;    
    hbox1 = gtk_hbox_new(FALSE,2);
    gtk_box_pack_start(GTK_BOX(hbox1),lblprjname,FALSE,FALSE,5);
    gtk_box_pack_start(GTK_BOX(hbox1),txtprjname,TRUE,TRUE,5);
    hbox2 = gtk_hbox_new(FALSE,2);
    gtk_box_pack_start(GTK_BOX(hbox2),lblpath,FALSE,FALSE,5);
    gtk_box_pack_start(GTK_BOX(hbox2),txtpath,TRUE,TRUE,5);
    gtk_box_pack_start(GTK_BOX(hbox2),cmdpath,FALSE,FALSE,5);
    hbox3 = gtk_hbox_new(FALSE,2);
    gtk_box_pack_end(GTK_BOX(hbox3),cmdcreate,FALSE,FALSE,5);
    gtk_box_pack_end(GTK_BOX(hbox3),cmdcancel,FALSE,FALSE,5);
    vbox = gtk_vbox_new(FALSE,2);
    gtk_box_pack_start(GTK_BOX(vbox),hbox1,FALSE,FALSE,25);
    gtk_box_pack_start(GTK_BOX(vbox),hbox2,FALSE,FALSE,25);
    gtk_box_pack_start(GTK_BOX(vbox),hbox3,FALSE,FALSE,25);
    
    g_signal_connect(G_OBJECT(cmdcreate),"clicked",G_CALLBACK(cmdcreate_clicked),NULL);
    g_signal_connect(G_OBJECT(cmdcancel),"clicked",G_CALLBACK(cmdcancel_clicked),NULL);
    g_signal_connect(G_OBJECT(cmdpath),"clicked",G_CALLBACK(cmdpath_clicked),NULL);
    g_signal_connect(G_OBJECT(new_proj_window),"destroy",G_CALLBACK(new_proj_window_destroyed),NULL);
    
    gtk_widget_set_size_request(new_proj_window,400,250);
    gtk_window_set_title(GTK_WINDOW(new_proj_window),"Create New Project");
    gtk_container_add(GTK_CONTAINER(new_proj_window),vbox);
    gtk_widget_show_all(new_proj_window);
    }