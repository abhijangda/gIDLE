/*64 Lines*/
#include<gtk/gtk.h>
#include<string.h>
#include"pyclass.h"

extern void class_browser_show(pyclass *class_array, int class_array_size, char *filepath, GtkWindow *window)
{    
    GtkWidget *view;
    GtkTreeViewColumn *col;
    GtkCellRenderer *renderer;
    GtkTreeModel *model;
    GtkTreeStore *treestore;
    GtkTreeIter toplevel,class_iter,member_iter;
    
    treestore = gtk_tree_store_new(1,G_TYPE_STRING);
    gtk_tree_store_append(treestore,&toplevel,NULL);
    gtk_tree_store_set(treestore,&toplevel,0,filepath,-1);
    int i=-1,j=-1;    
    
    while(++i<class_array_size)
    {        
        gtk_tree_store_append(treestore,&class_iter,&toplevel);
        gtk_tree_store_set(treestore,&class_iter,0,class_array[i].class_name,-1);
        
        j=-1;
        while(++j<class_array[i].self_array_size)
        {
            gtk_tree_store_append(treestore,&member_iter,&class_iter);
            gtk_tree_store_set(treestore,&member_iter,0,class_array[i].self_array[j],-1);
            }
        
        j=-1;
        while(++j<class_array[i].function_array_size)
        {
            gtk_tree_store_append(treestore,&member_iter,&class_iter);
            gtk_tree_store_set(treestore,&member_iter,0,class_array[i].function_array[j].func_name,-1);
            }
        }

    view = gtk_tree_view_new();
    gtk_tree_view_set_model(GTK_TREE_VIEW(view),GTK_TREE_MODEL(treestore));
    
    col = gtk_tree_view_column_new();
    gtk_tree_view_column_set_title(col,"Class Browser");
    gtk_tree_view_append_column(GTK_TREE_VIEW(view),col);    
            
    renderer = gtk_cell_renderer_text_new();
    gtk_tree_view_column_pack_start(col,renderer,TRUE);
    gtk_tree_view_column_add_attribute(col,renderer,"text",0);
        
    GtkWidget *dialog;
    
    dialog = gtk_dialog_new_with_buttons("gIDLE",GTK_WINDOW(window),GTK_DIALOG_DESTROY_WITH_PARENT,GTK_STOCK_OK,GTK_RESPONSE_NONE,NULL);
    GtkWidget *scroll;
    scroll = gtk_scrolled_window_new(NULL,NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(scroll),view);
    gtk_box_pack_start_defaults(GTK_BOX(GTK_DIALOG(dialog)->vbox),scroll);
    gtk_widget_set_size_request(dialog,400,300);
     int result;
    gtk_widget_show_all(dialog);
    result = gtk_dialog_run(GTK_DIALOG(dialog));
    if (result == GTK_RESPONSE_NONE)
        gtk_widget_destroy(dialog);
    }