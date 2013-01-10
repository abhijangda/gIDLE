/*247 Lines*/
#include<gtk/gtk.h>
#include<stdio.h>
#include<stdlib.h>

GtkListStore *list_store;
GtkTreeIter iter;
static GtkWidget *dialog,*list_view,*txt_proj_name, *txt_main_file;
GString *main_file_gstring;
char *proj_path;
GtkCellRenderer *renderer;
GtkTreeViewColumn *column;
static char *filepatharray[10];

void cmd_add_existing_clicked(GtkWidget *widget)
{
    GtkWidget *open_dialog;
    open_dialog = gtk_file_chooser_dialog_new("Open File", GTK_WINDOW(dialog), GTK_FILE_CHOOSER_ACTION_OPEN,GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,GTK_STOCK_OPEN,GTK_RESPONSE_ACCEPT,NULL);
    if (gtk_dialog_run(GTK_DIALOG(open_dialog))==GTK_RESPONSE_ACCEPT)
    {
        char *file;
        file = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(open_dialog));
        int i;
        for (i=0;i<10;i++)
        {
            if(strcmp("",filepatharray[i])==0)
                break;
            }
        asprintf(&filepatharray[i],"%s",file);
        gtk_list_store_append(list_store,&iter);
        gtk_list_store_set(list_store,&iter,0,file,-1);
        }
    gtk_widget_destroy(open_dialog);
    gtk_widget_destroy(dialog);
    }

void cmd_add_new_clicked(GtkWidget *widget)
{
    GtkWidget *open_dialog;
    open_dialog = gtk_file_chooser_dialog_new("Save File", GTK_WINDOW(dialog), GTK_FILE_CHOOSER_ACTION_SAVE,GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,GTK_STOCK_SAVE,GTK_RESPONSE_ACCEPT,NULL);
    if (gtk_dialog_run(GTK_DIALOG(open_dialog))==GTK_RESPONSE_ACCEPT)
    {
        char *file;
        file = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(open_dialog));
        int i;
        for (i=0;i<10;i++)
        {
            if(strcmp("",filepatharray[i])==0)
                break;
            }
        asprintf(&filepatharray[i],"%s",file);
        gtk_list_store_append(list_store,&iter);
        gtk_list_store_set(list_store,&iter,0,file,-1);
        }
    gtk_widget_destroy(open_dialog);
    gtk_widget_destroy(dialog);
    }

void cmd_add_clicked(GtkWidget *widget)
{
    GtkWidget *label,*cmd_add_new,*cmd_add_existing;
    
    dialog = gtk_dialog_new();
    cmd_add_new = gtk_button_new_with_mnemonic("_Add New File");
    cmd_add_existing = gtk_button_new_with_mnemonic("_Add Existing File");
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->action_area),cmd_add_new,FALSE,FALSE,5);
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->action_area),cmd_add_existing,FALSE,FALSE,5);
    
    label = gtk_label_new("Please click on desired button?");
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),label,FALSE,FALSE,5);
    
    g_signal_connect(G_OBJECT(cmd_add_new),"clicked",G_CALLBACK(cmd_add_new_clicked),NULL);
    g_signal_connect(G_OBJECT(cmd_add_existing),"clicked",G_CALLBACK(cmd_add_existing_clicked),NULL);
    
    gtk_window_set_title(GTK_WINDOW(dialog),"Add File");
    gtk_widget_show_all(dialog);
    gtk_dialog_run(GTK_DIALOG(dialog));
    }

void cmd_remove_clicked(GtkWidget *widget)
{
    GtkTreeSelection *selection;    
    selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(list_view));    
    GtkTreeModel *model;
    char *string;    
    if(gtk_tree_selection_get_selected(GTK_TREE_SELECTION(selection),&model,&iter))
        gtk_tree_model_get(model,&iter,0,&string,-1);
    int i;
    for (i=0;i<10;i++)
    {
        if(strcmp(string,filepatharray[i])==0)
            break;
        }
    int j;
    for(j=i;j<9;j++)
        asprintf(&filepatharray[j],"%s",filepatharray[j+1]);
    if (i==9)
        filepatharray[9] = "";
    gtk_list_store_remove(GTK_LIST_STORE(list_store),&iter);
    }

void cmd_remove_all_clicked(GtkWidget *widget)
{
    gtk_list_store_clear(GTK_LIST_STORE(list_store));
    int i=0;
    for(i=0;i<10;i++)
    {
        filepatharray[i] = "";
        }
    }

void cmd_apply_clicked(GtkWidget *widget)
{
    GString *contents;
    contents = g_string_new("<name>");
    g_string_append(contents,gtk_entry_get_text(GTK_ENTRY(txt_proj_name)));
    g_string_append(contents,"</name>\n");
    g_string_append(contents,"<mainfile>");
    g_string_append(contents,gtk_entry_get_text(GTK_ENTRY(txt_main_file)));
    g_string_append(contents,"</mainfile>\n");
    asprintf(&filepatharray[0],"%s",gtk_entry_get_text(GTK_ENTRY(txt_main_file)));
    int i=1;
    while(strcmp("",filepatharray[i])!=0)
    {
        g_string_append(contents,"<file>");
        g_string_append(contents,filepatharray[i]);
        g_string_append(contents,"</file>\n");
        i++;
        }
    printf("%s",contents->str);
    g_file_set_contents(proj_path,contents->str,-1,NULL);
    /*g_string_free(contents,TRUE);*/
    }

void cmd_ok_clicked(GtkWidget *widget,GtkWidget *dialog)
{
    cmd_apply_clicked(widget);
    main_file_gstring = g_string_new(gtk_entry_get_text(GTK_ENTRY(txt_main_file)));
    GString *projname_gstring;
    projname_gstring = g_string_new(gtk_entry_get_text(GTK_ENTRY(txt_proj_name)));
    int i;
   
    proj_manager_destroyed(filepatharray,main_file_gstring,projname_gstring);
    /*g_string_free(projname_gstring,TRUE);*/
    /*g_string_free(main_file_gstring,TRUE);*/
    gtk_widget_destroy(dialog);
    }


void cmd_cancel_clicked(GtkWidget *widget,GtkWidget *dialog)
{
    gtk_widget_destroy(dialog);
    }

void proj_manager_init(GtkWindow *window,GString *main_file,GString *proj_name)
{
    GtkWidget *proj_manager_dialog, *hbox_buttons,*cmd_ok, *cmd_cancel, *cmd_apply, *hbox_proj_name, *lbl_proj_name,  *hbox_main_file, *lbl_main_file;
    GtkWidget *vbox_file_buttons, *cmd_add, *cmd_remove, *cmd_remove_all, *hbox_files,*lbl_files;
    
    proj_manager_dialog = gtk_dialog_new();

    hbox_buttons = gtk_hbox_new(FALSE,5);
    cmd_ok = gtk_button_new_with_mnemonic("Ok");
    cmd_apply = gtk_button_new_with_mnemonic("Apply");
    cmd_cancel = gtk_button_new_with_mnemonic("Cancel");
    gtk_widget_set_size_request(cmd_ok,95,31);
    gtk_widget_set_size_request(cmd_apply,95,31);
    gtk_widget_set_size_request(cmd_cancel,95,31);
    gtk_box_pack_end(GTK_BOX(hbox_buttons),cmd_ok,FALSE,FALSE,5);
    gtk_box_pack_end(GTK_BOX(hbox_buttons),cmd_apply,FALSE,FALSE,5);
    gtk_box_pack_end(GTK_BOX(hbox_buttons),cmd_cancel,FALSE,FALSE,5);
    
    g_signal_connect(G_OBJECT(cmd_ok),"clicked",G_CALLBACK(cmd_ok_clicked),proj_manager_dialog);
    g_signal_connect(G_OBJECT(cmd_apply),"clicked",G_CALLBACK(cmd_apply_clicked),NULL);
    g_signal_connect(G_OBJECT(cmd_cancel),"clicked",G_CALLBACK(cmd_cancel_clicked),proj_manager_dialog);

    hbox_files = gtk_hbox_new(FALSE,5);    
    vbox_file_buttons = gtk_vbox_new(FALSE,5);
    cmd_add = gtk_button_new_with_mnemonic("Add");
    cmd_remove = gtk_button_new_with_mnemonic("Remove");
    cmd_remove_all = gtk_button_new_with_mnemonic("Remove All");
    gtk_widget_set_size_request(cmd_add,95,31);
    gtk_widget_set_size_request(cmd_remove,95,31);
    gtk_widget_set_size_request(cmd_remove_all,95,31);
    g_signal_connect(G_OBJECT(cmd_add),"clicked",G_CALLBACK(cmd_add_clicked),NULL);
    g_signal_connect(G_OBJECT(cmd_remove),"clicked",G_CALLBACK(cmd_remove_clicked),NULL);
    g_signal_connect(G_OBJECT(cmd_remove_all),"clicked",G_CALLBACK(cmd_remove_all_clicked),NULL); 
    /*lbl_files = gtk_label_new("Files");*/
    list_view = gtk_tree_view_new();
   
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Files", renderer, "text", 0, NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW (list_view), column);

    list_store = gtk_list_store_new(1,G_TYPE_STRING);
    int i = 1;
    while(strcmp("",filepatharray[i])!=0)
    {
        gtk_list_store_append(list_store,&iter);
        gtk_list_store_set(list_store,&iter,0,filepatharray[i],-1);
        i++;
        }
    gtk_tree_view_set_model(GTK_TREE_VIEW(list_view),GTK_TREE_MODEL(list_store));
    /*g_object_unref(list_store);*/
    
    gtk_box_pack_end(GTK_BOX(vbox_file_buttons),cmd_remove_all,FALSE,FALSE,5);
    gtk_box_pack_end(GTK_BOX(vbox_file_buttons),cmd_remove,FALSE,FALSE,5);
    gtk_box_pack_end(GTK_BOX(vbox_file_buttons),cmd_add,FALSE,FALSE,5);
    /*gtk_box_pack_start(GTK_BOX(hbox_files),lbl_files,FALSE,FALSE,5);*/
    gtk_box_pack_start(GTK_BOX(hbox_files),list_view,TRUE,TRUE,5);
    gtk_box_pack_start(GTK_BOX(hbox_files),vbox_file_buttons,FALSE,FALSE,5);
    
    hbox_proj_name = gtk_hbox_new(FALSE,5);
    lbl_proj_name = gtk_label_new("Project Name");
    txt_proj_name = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(txt_proj_name),proj_name->str);
    gtk_box_pack_start(GTK_BOX(hbox_proj_name),lbl_proj_name,FALSE,FALSE,5);
    gtk_box_pack_start(GTK_BOX(hbox_proj_name),txt_proj_name,TRUE,TRUE,5);
    
    hbox_main_file = gtk_hbox_new(FALSE,5);
    lbl_main_file = gtk_label_new("Main File   ");
    txt_main_file = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(txt_main_file),main_file->str);
    gtk_box_pack_start(GTK_BOX(hbox_main_file),lbl_main_file,FALSE,FALSE,5);
    gtk_box_pack_start(GTK_BOX(hbox_main_file),txt_main_file,TRUE,TRUE,5);
    
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(proj_manager_dialog)->vbox),hbox_proj_name,FALSE,FALSE,5);
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(proj_manager_dialog)->vbox),hbox_main_file,FALSE,FALSE,5);
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(proj_manager_dialog)->vbox),hbox_files,TRUE,TRUE,5);
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(proj_manager_dialog)->vbox),hbox_buttons,FALSE,FALSE,5);
    
    gtk_widget_set_size_request(proj_manager_dialog,492,385);
    gtk_window_set_title(GTK_WINDOW(proj_manager_dialog),"Project Manager");
    gtk_widget_show_all(proj_manager_dialog);
    gtk_dialog_run(GTK_DIALOG(proj_manager_dialog));
    }

extern void show_proj_manager(GtkWindow *window, char *filearray[], GString *mainfile, GString *projname,char *projpath)
{
    int i;
    main_file_gstring = g_string_new(mainfile->str);
    asprintf(&proj_path,"%s",projpath);
    for (i=0;i<10;i++)
        asprintf(&filepatharray[i],"%s",filearray[i]);
    proj_manager_init(window,mainfile,projname);
    }