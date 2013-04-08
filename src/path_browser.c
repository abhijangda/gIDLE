/*85 Lines*/ 
#include<gtk/gtk.h>
#include<stdlib.h>
#include<stdio.h>
#include<string.h>

extern void path_browser_activated(GtkWidget *widget, GtkWindow *window)
{
    GtkWidget *dialog, *cmdok, *cmdcancel, *view, *scrollwin;
    GtkTreeViewColumn *col;
    GtkTreeModel *model;
    GtkTreeStore *treestore;
    GtkTreeIter toplevel,iter;
    GtkCellRenderer *renderer;
    
    dialog = gtk_dialog_new_with_buttons("gIDLE",GTK_WINDOW(window),GTK_DIALOG_DESTROY_WITH_PARENT,GTK_STOCK_OK,GTK_RESPONSE_NONE,NULL);
    gtk_widget_set_size_request(dialog,400,300);
    
    view = gtk_tree_view_new();
    scrollwin = gtk_scrolled_window_new(NULL,NULL);
    gtk_container_add(GTK_CONTAINER(scrollwin),view);
    gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox),scrollwin);
    
    char *argv[] = {"python","./path.py",NULL}; 
    int pid,in,out,err;
    g_spawn_async_with_pipes(NULL,argv,NULL,G_SPAWN_SEARCH_PATH,NULL,NULL,&pid,&in,&out,&err,NULL);     
    char c;     
    char *output = (char *)malloc(1*sizeof(char));
    read(out,&c,1);
    output[0] = c;
    int size = 1;
    while(read(out,&c,1)==1)
    {
        output = (char *)realloc(output,(size+1)*sizeof(char));
        output[size] = c;
        size++;
        }
    output = (char *)realloc(output,(size+1)*sizeof(char));
    output[size] = '\0';
    GRegex *regex_path;
    GMatchInfo *match_info;
    
    treestore = gtk_tree_store_new(1,G_TYPE_STRING);
    gtk_tree_store_append(treestore,&toplevel,NULL);
    gtk_tree_store_set(treestore,&toplevel,0,"sys.path",-1);
    
    regex_path = g_regex_new("\'+.+?\'",0,0,NULL);
    if (g_regex_match(regex_path,output,0,&match_info))
    {
        while(g_match_info_matches(match_info))
        {            
            gtk_tree_store_append(treestore,&iter,&toplevel);            
            char *value = g_match_info_fetch(match_info,0);
            value[strlen(value)-1] = '\0';
            char path[strlen(value)-1];
            char *p = &path[0];
            value++;
            while(*p++=*value++);
            path[strlen(value)-2] = '\0';
            gtk_tree_store_set(treestore,&iter,0,path,-1);        
            g_match_info_next(match_info,NULL);
            }
        }
    
    gtk_tree_view_set_model(GTK_TREE_VIEW(view),GTK_TREE_MODEL(treestore));
    
    col = gtk_tree_view_column_new();
    gtk_tree_view_column_set_title(col,"Path Browser");
    gtk_tree_view_append_column(GTK_TREE_VIEW(view),col);
    
    renderer = gtk_cell_renderer_text_new();
    gtk_tree_view_column_pack_start(col,renderer,TRUE);
    gtk_tree_view_column_add_attribute(col,renderer,"text",0);
    
    g_match_info_free(match_info);
    g_regex_unref(regex_path);
    
    int result;
    gtk_widget_show_all(dialog);
    result = gtk_dialog_run(GTK_DIALOG(dialog));
    if (result == GTK_RESPONSE_NONE)
    {
        free(output);
        gtk_widget_destroy(dialog);
        }
    }