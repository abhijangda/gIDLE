/*2336 Lines*/
//main filepath is stored in GString mainfile
//In project, cursor is not responding to the change of scope 
//Option including the backup location for files

#include"key_press_event.h"
#include<ctype.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<pthread.h>

#ifndef __MENUS__H
#define __MENUS__H

int bookmarks_line_array[MAX_TABS],bookmarks_tabindex_array[MAX_TABS],bookmark_index=-1;
GSList *bookmarks_menu_item_list = NULL;
GtkWidget *bookmarks_menu_item_array[MAX_TABS],*next_bookmark,*previous_bookmark,*clear_bookmarks;
GtkWidget *addfile,*addfile_main;
GtkWidget *window;
GtkWidget *lbl_status,*entry_find_in_file,*entry_file;
char *filepathsarray[MAX_TABS];
GtkWidget *file_path_menu_itemsarray[5];
GtkWidget *recentfilesmenu;
int offset = 0,proj_filecount=0;
GtkWidget *txtfind;
GtkWidget *txtreplace;
void file_path_menu_item_activated(GtkWidget *widget);
void cmdclose_clicked(GtkWidget *widget, GtkWidget *dialog);
void cmdfindnext_clicked(GtkWidget *widget,GtkToggleButton*);
void cmdfind_clicked(GtkWidget *widget,GtkToggleButton*);
void cmdfindprevious_clicked(GtkWidget *widget,GtkToggleButton*);
void saveas_activated(GtkWidget *widget, GtkWindow *parent);
void save_activated(GtkWidget *widget, GtkWindow *parent);
GString *indentation,*indent_width,*auto_save,*auto_save_time,*show_line_numbers,*highlight_current_line,*run_shell;
GString *mainfile,*proj_name,*recent_files,*tabwidth;
char *mode = "File",*projpath;
GtkWidget *proj_tree_view;
GtkTreeViewColumn *proj_col;
GtkCellRenderer *proj_renderer;
GtkTreeModel *proj_model;
GtkTreeStore *proj_treestore;
GtkTreeIter toplevel,file_child_array[MAX_TABS];
extern char **command;
extern GtkWidget *runvte;

int is_file_already_present(char *filepath)
{
    int i;
    for(i=0;i<proj_filecount;i++)
    {
        if(!strcmp(filepathsarray[i],filepath))
            return 1;
        }
    return 0;
    }

int is_filename_already_present(char *filename)
{
    int i;
    for(i=0;i<proj_filecount;i++)
    {
        char *_filename;
        _filename = strrchr(filepathsarray[i],'/');        
        _filename++;
        if(strcmp(_filename,filename)==0)
            return 1;        
        }
    return 0;
    }

void watch_file_file_modified(void *watch_file, char *filepath)
{
    ///Just implement a message dialog
    //I think the problem is with threads
    
    GtkTextBuffer *buffer;
    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(codewidget[gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook))].txtinput));
    //gtk_text_buffer_set_text(buffer,"lllll\n",-1);
    /*watch_file_stop(&watch_file_object);
    GtkWidget *dialog,*label,*hbox; 
    char *message, *filename; 
    filename = strrchr(filepath,'/'); 
    filename++; 
    asprintf(&message,"File %s has been modified. Do you want to reload it?",filename);
    dialog  = gtk_dialog_new();
    label = gtk_label_new(message);
    gtk_box_pack_start_defaults(GTK_BOX(GTK_DIALOG(dialog)->vbox),label);
    gtk_widget_show_all(dialog);
    gtk_dialog_run(GTK_DIALOG(dialog));*/
    
    /*watch_file_stop(&watch_file_object);
    GtkWidget *dialog,*label,*hbox;
    char *message, *filename;
    filename = strrchr(filepath,'/');
    filename++;
    asprintf(&message,"File %s has been modified. Do you want to reload it?",filename);
    printf("%s\n",message);
    label = gtk_label_new(message);
    dialog = gtk_dialog_new_with_buttons("gIDLE", GTK_WINDOW(window), GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT,GTK_STOCK_YES,GTK_RESPONSE_YES,GTK_STOCK_NO,GTK_RESPONSE_NO,NULL);
    hbox = gtk_hbox_new(FALSE,5);
    gtk_box_pack_start_defaults(GTK_BOX(hbox),label);
    gtk_box_pack_start_defaults(GTK_BOX(GTK_DIALOG(dialog)->vbox),hbox);
    gtk_widget_show_all(dialog);*/
    /*int result = gtk_dialog_run(GTK_DIALOG(dialog)); 
    if (result == GTK_RESPONSE_YES) 
    { 
        int i=0; 
        for(i=0;i<gtk_notebook_get_n_pages(GTK_NOTEBOOK(notebook));i++) 
            if(strcmp(filepathsarray[i],filepath)==0) 
                break; 
        if(i==gtk_notebook_get_n_pages(GTK_NOTEBOOK(notebook))+1)  
            return; 
        char *contents;
        gssize length = -1; 
        g_file_get_contents(filepath,&contents,&length,NULL); 
        GtkTextBuffer *textbuffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(txtinput[i])); 
        gtk_text_buffer_set_text(textbuffer,contents,-1);         
        g_free(contents); 
        } hhkhkhkhk
    free(mes
    gtk_widget_destroy(dialog); */
    } 

void proj_manager_activated(GtkWidget *widget, GtkWindow *window)
{
    if (strcmp(mode,"Proj")==0)
        show_proj_manager(window,filepathsarray,mainfile,proj_name,projpath);
    }

void clear_bookmarks_activated(GtkWidget *widget,GtkMenuShell *menu)
{
    int i=0;
    while(i<MAX_TABS && bookmarks_line_array[i++]!=-1)
        gtk_container_remove(GTK_CONTAINER(menu),bookmarks_menu_item_array[i-1]);
                
    for(i=0;i<MAX_TABS;i++)
    {
        bookmarks_line_array[i] = -1;
        bookmarks_tabindex_array[i] = -1;
        }
    }

void bookmarks_menu_item_activated(GtkWidget *widget)
{
    const char *label = gtk_menu_item_get_label(GTK_MENU_ITEM(widget));
    int i;
    while(strcmp(label,gtk_menu_item_get_label(GTK_MENU_ITEM(bookmarks_menu_item_array[i++])))!=0);
    bookmark_index = i-1;
    GString *_filename = g_string_new("");
    
    for(i=0;i<strlen(label);i++)
    {
        if(label[i]==' ')
            break;
        else
            g_string_append_c(_filename,label[i]);
        }
    for(i=0;i<gtk_notebook_get_n_pages(GTK_NOTEBOOK(notebook));i++)
    {
        if (strcmp(_filename->str,gtk_notebook_get_tab_label_text(GTK_NOTEBOOK(notebook),vbox_txtinput[i]))==0)
            break;
        }
    int tab_index = i;
    GString *line = g_string_new("");
    for(i=find_strstr(label,"Line:",0) + strlen("Line:");i<strlen(label);i++)
        g_string_append_c(line,label[i]);
    int line_number = atoi(line->str);
    GtkTextBuffer *buffer;
    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(codewidget[tab_index].txtinput));
    GtkTextIter bookmark_iter;        
    gtk_text_buffer_get_iter_at_line(buffer,&bookmark_iter,line_number);
    gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook),tab_index);
    gtk_text_buffer_place_cursor(buffer,&bookmark_iter);
    gtk_text_view_scroll_to_iter(GTK_TEXT_VIEW(codewidget[gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook))].txtinput),&bookmark_iter,0.1,FALSE,0.5,0.5);
    }

void add_bookmarks_activated(GtkWidget *widget,GtkMenuShell *menu)
{
    GtkTextBuffer *buffer;
    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(codewidget[gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook))].txtinput));
    GtkTextIter current_iter;
    gtk_text_buffer_get_iter_at_mark(buffer,&current_iter,gtk_text_buffer_get_insert(buffer));
    int i = 0;
    while(i<MAX_TABS && bookmarks_line_array[i++]!=-1);
    bookmarks_line_array[i-1] = gtk_text_iter_get_line(&current_iter)+1;
    bookmarks_tabindex_array[i-1] = gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook));
    GString *string;
    string = g_string_new(gtk_notebook_get_tab_label_text(GTK_NOTEBOOK(notebook),vbox_txtinput[gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook))]));
    g_string_append(string,"    Line:");
    char *s;
    asprintf(&s,"%d",bookmarks_line_array[i-1]);
    g_string_append(string,s);
    if (i!=1)
        bookmarks_menu_item_list = gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(bookmarks_menu_item_array[i-2]));
    bookmarks_menu_item_array[i-1] = gtk_radio_menu_item_new_with_label(bookmarks_menu_item_list,string->str);
    g_signal_connect(G_OBJECT(bookmarks_menu_item_array[i-1]),"activate",G_CALLBACK(bookmarks_menu_item_activated),NULL);
    g_free(s);
    g_string_free(string,TRUE);
    gtk_menu_shell_append(menu,bookmarks_menu_item_array[i-1]);
    gtk_widget_show(bookmarks_menu_item_array[i-1]);
    gtk_widget_set_sensitive(next_bookmark,TRUE);    
    gtk_widget_set_sensitive(previous_bookmark,TRUE);
    gtk_widget_set_sensitive(clear_bookmarks,TRUE);
    }

void next_bookmarks_activated(GtkWidget *widget,GtkMenuShell *menu)
{
    int i = 0;
    while(i<MAX_TABS && bookmarks_line_array[i++]!=-1);
    
    gboolean bookmarks_valid=FALSE;
    if (i>0 && i < MAX_TABS)
        bookmarks_valid=TRUE;
    if(i=1 && bookmarks_line_array[0] !=-1)
        bookmarks_valid = TRUE;
    if(i=MAX_TABS && bookmarks_line_array[MAX_TABS-1] !=-1)
        bookmarks_valid = TRUE;
    if (bookmark_index==MAX_TABS-1)
        bookmark_index = -1;
    
    if (bookmarks_valid==TRUE)
    {
        GtkTextBuffer *buffer;
        buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(codewidget[bookmarks_tabindex_array[++bookmark_index]].txtinput));
        GtkTextIter bookmark_iter;        
        gtk_text_buffer_get_iter_at_line(buffer,&bookmark_iter,bookmarks_line_array[bookmark_index]);
        gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook),bookmarks_tabindex_array[bookmark_index]);
        gtk_text_buffer_place_cursor(buffer,&bookmark_iter);
        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(bookmarks_menu_item_array[bookmark_index]),TRUE);
        gtk_text_view_scroll_to_iter(GTK_TEXT_VIEW(codewidget[gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook))].txtinput),&bookmark_iter,0.1,FALSE,0.5,0.5);
        }    
    }

void previous_bookmarks_activated(GtkWidget *widget,GtkMenuShell *menu)
{
    int i = 0;
    while(i<MAX_TABS && bookmarks_line_array[i++]!=-1);
    gboolean bookmarks_valid=FALSE;
    if (i>0 && i < MAX_TABS)
        bookmarks_valid=TRUE;
    if(i=1 && bookmarks_line_array[0] !=-1)
        bookmarks_valid = TRUE;
    if(i=MAX_TABS && bookmarks_line_array[MAX_TABS-1] !=-1)
        bookmarks_valid = TRUE;
    if (bookmark_index==0)
        bookmark_index = MAX_TABS;
    if (bookmarks_valid==TRUE)
    {
        GtkTextBuffer *buffer;
        buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(codewidget[bookmarks_tabindex_array[--bookmark_index]].txtinput));
        GtkTextIter bookmark_iter;        
        gtk_text_buffer_get_iter_at_line(buffer,&bookmark_iter,bookmarks_line_array[bookmark_index]);
        gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook),bookmarks_tabindex_array[bookmark_index]);
        gtk_text_buffer_place_cursor(buffer,&bookmark_iter);
        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(bookmarks_menu_item_array[bookmark_index]),TRUE);
        gtk_text_view_scroll_to_iter(GTK_TEXT_VIEW(codewidget[gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook))].txtinput),&bookmark_iter,0.1,FALSE,0.5,0.5);
        }
    }

void lowercase_selection_activated(GtkWidget *widget)
{
    GtkTextBuffer *buffer;
    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(codewidget[gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook))].txtinput));
    GtkTextIter selection_start_iter,selection_end_iter;
    gtk_text_buffer_get_selection_bounds(buffer,&selection_start_iter,&selection_end_iter);
    int i,selection_start_offset = gtk_text_iter_get_offset(&selection_start_iter);
    char *text = gtk_text_buffer_get_text(buffer,&selection_start_iter,&selection_end_iter,TRUE);
    gtk_text_buffer_delete(buffer,&selection_start_iter,&selection_end_iter);
    char lower_text[strlen(text)+1];
    for(i=0;i<strlen(text);i++)
    {
        if(isalpha(text[i]))
            lower_text[i] = tolower(text[i]);
        else
            lower_text[i] = text[i];
        }
    lower_text[i] = '\0';
    gtk_text_buffer_get_iter_at_offset(buffer,&selection_start_iter,selection_start_offset);
    gtk_text_buffer_insert(buffer,&selection_start_iter,lower_text,-1);
    }

void uppercase_selection_activated(GtkWidget *widget)
{
    GtkTextBuffer *buffer;
    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(codewidget[gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook))].txtinput));
    GtkTextIter selection_start_iter,selection_end_iter;
    gtk_text_buffer_get_selection_bounds(buffer,&selection_start_iter,&selection_end_iter);
    int i,selection_start_offset = gtk_text_iter_get_offset(&selection_start_iter);
    char *text = gtk_text_buffer_get_text(buffer,&selection_start_iter,&selection_end_iter,TRUE);
    gtk_text_buffer_delete(buffer,&selection_start_iter,&selection_end_iter);
    char upper_text[strlen(text)+1];
    for(i=0;i<strlen(text);i++)
    {
        if(isalpha(text[i]))
            upper_text[i] = toupper(text[i]);
        else
            upper_text[i] = text[i];
        }
    upper_text[i] = '\0';
    gtk_text_buffer_get_iter_at_offset(buffer,&selection_start_iter,selection_start_offset);
    gtk_text_buffer_insert(buffer,&selection_start_iter,upper_text,-1);
    }

void tabify_region_activated(GtkWidget *widget)
{
    GtkTextBuffer *buffer;
    int i,tab_width = 4;/***********Remember to add Tab Width****************/
    if (strcmp(tabwidth->str,"")!=0)
        tab_width = atoi(tabwidth->str);
    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(codewidget[gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook))].txtinput));
    GtkTextIter selection_start_iter,selection_end_iter;
    gtk_text_buffer_get_selection_bounds(buffer,&selection_start_iter,&selection_end_iter);
    int selection_start_iter_line = gtk_text_iter_get_line(&selection_start_iter),selection_end_iter_line = gtk_text_iter_get_line(&selection_end_iter);
    int selection_start_iter_offset = gtk_text_iter_get_offset(&selection_start_iter);
    GtkTextIter start_line_iter,end_line_iter;
    for (i=selection_start_iter_line;i<=selection_end_iter_line;i++)
    {
        gtk_text_buffer_get_iter_at_line(buffer,&start_line_iter,i);
        gtk_text_buffer_get_iter_at_line(buffer,&end_line_iter,i+1);
        char *line = gtk_text_buffer_get_text(buffer,&start_line_iter,&end_line_iter,TRUE);
        int spaces=0,j=0;
        for(j=0;j<strlen(line);j++)
        {
            if(line[j] == ' ')
                spaces++;
            else
                break;
            }
        GtkTextIter iter;
        gtk_text_buffer_get_iter_at_line_offset(buffer,&iter,i,spaces);
        gtk_text_buffer_delete(buffer,&start_line_iter,&iter);
        gtk_text_buffer_get_iter_at_line(buffer,&start_line_iter,i);
        for(j=0;j<spaces/tab_width;j++)
            gtk_text_buffer_insert(buffer,&start_line_iter,"\t",1);        
        }
    }

void untabify_region_activated(GtkWidget *widget)
{
    GtkTextBuffer *buffer;
    int i,tab_width = 4;/***********Remember to add Tab Width****************/
    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(codewidget[gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook))].txtinput));
    GtkTextIter selection_start_iter,selection_end_iter;
    gtk_text_buffer_get_selection_bounds(buffer,&selection_start_iter,&selection_end_iter);
    int selection_start_iter_line = gtk_text_iter_get_line(&selection_start_iter),selection_end_iter_line = gtk_text_iter_get_line(&selection_end_iter);
    int selection_start_iter_offset = gtk_text_iter_get_offset(&selection_start_iter);
    GtkTextIter start_line_iter,end_line_iter;
    for (i=selection_start_iter_line;i<=selection_end_iter_line;i++)
    {
        gtk_text_buffer_get_iter_at_line(buffer,&start_line_iter,i);
        gtk_text_buffer_get_iter_at_line(buffer,&end_line_iter,i+1);
        char *line = gtk_text_buffer_get_text(buffer,&start_line_iter,&end_line_iter,TRUE);
        int tabs=0,j=0;
        for(j=0;j<strlen(line);j++)
        {
            if(line[j] == '\t')
                tabs++;
            else
                break;
            }
        GtkTextIter iter;
        gtk_text_buffer_get_iter_at_line_offset(buffer,&iter,i,tabs);
        gtk_text_buffer_delete(buffer,&start_line_iter,&iter);
        gtk_text_buffer_get_iter_at_line(buffer,&start_line_iter,i);
        for(j=0;j<tabs;j++)
        {
            int k=0;
            for(k=0;k<tabs*tab_width;k++)
                gtk_text_buffer_insert(buffer,&start_line_iter," ",1);
            }
        }
    }

int find_strstr(const char *str1, const char *str2,int index)
{
    gboolean found = FALSE;
    int i,j;
    for (i=index;i<strlen(str1);i++)
    {
        if (str1[i] == str2[0])
        {
            for (j=0;j<strlen(str2);j++)
            {
                if (str1[i+j] == str2[j])
                    found = TRUE;
                else
                {
                    found = FALSE;
                    break;
                    }
                }
            }
        if (found == TRUE)
           return i;        	
        }
    return -1;
    }

void back_line_activated(GtkWidget *widget)
{
    GtkTextBuffer *buffer;
    int current_page = gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook));
    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(codewidget[current_page].txtinput));
    GtkTextIter current_iter;
    gtk_text_buffer_get_iter_at_mark(buffer,&current_iter,gtk_text_buffer_get_insert(buffer));
    int line=0,i=0;
    line = gtk_text_iter_get_line(&current_iter);
    while(lines_track_array[current_page][i]!=line+1&&i<5)
        i++;
    if(i<5)
        gtk_text_buffer_get_iter_at_line(buffer,&current_iter,lines_track_array[current_page][i-1]-1);
        gtk_text_view_scroll_to_iter(GTK_TEXT_VIEW(codewidget[current_page].txtinput),&current_iter,0.1,FALSE,0.5,0.5);
        gtk_text_buffer_place_cursor(buffer,&current_iter);
    }

void forward_line_activated(GtkWidget *widget)
{
    GtkTextBuffer *buffer;
    int current_page = gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook));
    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(codewidget[current_page].txtinput));
    GtkTextIter current_iter;
    gtk_text_buffer_get_iter_at_mark(buffer,&current_iter,gtk_text_buffer_get_insert(buffer));
    int line=0,i=0;
    line = gtk_text_iter_get_line(&current_iter);
    while(lines_track_array[current_page][i]!=line+1&&i<5)
        i++;
    if(i<4)
        gtk_text_buffer_get_iter_at_line(buffer,&current_iter,lines_track_array[current_page][i+1]-1);
        gtk_text_view_scroll_to_iter(GTK_TEXT_VIEW(codewidget[current_page].txtinput),&current_iter,0.1,FALSE,0.5,0.5);
        gtk_text_buffer_place_cursor(buffer,&current_iter);
    }

void beggining_line_activated(GtkWidget *widget)
{
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(codewidget[gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook))].txtinput));
    GtkTextIter start_iter;
    gtk_text_buffer_get_start_iter(buffer,&start_iter);
    gtk_text_view_scroll_to_iter(GTK_TEXT_VIEW(codewidget[gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook))].txtinput),&start_iter,0.1,FALSE,0.5,0.5);
    gtk_text_buffer_place_cursor(buffer,&start_iter);
    }

void end_line_activated(GtkWidget *widget)
{
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(codewidget[gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook))].txtinput));
    GtkTextIter end_iter;
    gtk_text_buffer_get_end_iter(buffer,&end_iter);
    gtk_text_view_scroll_to_iter(GTK_TEXT_VIEW(codewidget[gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook))].txtinput),&end_iter,0.1,FALSE,0.5,0.5);
    gtk_text_buffer_place_cursor(buffer,&end_iter);
    }

void show_parenthesis_activated(GtkWidget *widget)
{
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(codewidget[gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook))].txtinput));
    GtkTextIter iter;
    gtk_text_buffer_get_iter_at_mark(buffer,&iter,gtk_text_buffer_get_insert(buffer));
    GtkTextIter line_start_iter,line_end_iter;
    gtk_text_buffer_get_iter_at_line(buffer,&line_start_iter,gtk_text_iter_get_line(&iter));
    gtk_text_buffer_get_iter_at_line(buffer,&line_end_iter,gtk_text_iter_get_line(&iter)+1);
    gtk_text_buffer_get_iter_at_offset(buffer,&line_end_iter,gtk_text_iter_get_offset(&line_end_iter)-1);
    char *line_text = gtk_text_buffer_get_text(buffer,&line_start_iter,&line_end_iter,TRUE);
    int current_line_offset = gtk_text_iter_get_line_offset(&iter);
    int i,open_bracket_count=0,close_bracket_count=0;
    for (i=0;i<strlen(line_text);i++)
    {
        if(line_text[i]=='(')
            open_bracket_count++;
        if (line_text[i]==')')
            close_bracket_count++;
        }
    int open_array[open_bracket_count],close_array[close_bracket_count];
    int j=0,k=0;
    int open_bracket_before=0,open_bracket_after=0,close_bracket_after=0,close_bracket_before=0,closest_open_bracket=0;
    for (i=0;i<strlen(line_text);i++)
    {
        if(line_text[i]=='(')
        {
            open_array[j++] = i;
            if (i<current_line_offset)
            {
                closest_open_bracket = i;
                open_bracket_before++;
                }
            else
                open_bracket_after++;
            }
        if (line_text[i]==')')
        {
            close_array[k++] = i;
            if (i>current_line_offset)
                close_bracket_after++;
            }
        }
    for (i=0;i<close_bracket_count;i++)
    {
        if(close_array[i]>closest_open_bracket&&close_array[i]<current_line_offset)
            close_bracket_before++;
        }
    GtkTextIter bracket_start_iter,bracket_close_iter;
    gtk_text_buffer_get_iter_at_line_offset(buffer,&bracket_start_iter,gtk_text_iter_get_line(&iter),open_array[open_bracket_before-close_bracket_before-1]);
    gtk_text_buffer_get_iter_at_line_offset(buffer,&bracket_close_iter,gtk_text_iter_get_line(&iter),close_array[close_bracket_count-close_bracket_after+open_bracket_after]);
    gtk_text_buffer_select_range(buffer,&bracket_start_iter,&bracket_close_iter);
    }

void class_browser_activated(GtkWidget *widget, GtkWindow *window)
{    
    int current_page = gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook));
    class_browser_show(codewidget[current_page].pyclass_array,codewidget[current_page].pyclass_count,filepathsarray[gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook))],window);    
    }

void cmdsearch_clicked(GtkWidget *widget,GtkEntry *entry)
{
    static int counter = 0;
    const char *txt_regexp = gtk_entry_get_text(GTK_ENTRY(entry));
    GtkTextBuffer *buffer;
    buffer = GTK_TEXT_BUFFER(gtk_text_view_get_buffer(GTK_TEXT_VIEW(codewidget[gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook))].txtinput)));
    GtkTextIter start_iter, end_iter;
    gtk_text_buffer_get_start_iter(buffer, &start_iter);
    gtk_text_buffer_get_end_iter(buffer, &end_iter);
    char *text = gtk_text_buffer_get_text(buffer,&start_iter,&end_iter,FALSE);
    GRegex *regex;
    GMatchInfo *match_info;
    int i,start=0,end=0;
    regex = g_regex_new(txt_regexp,0,0,NULL);
    gboolean ans1= g_regex_match(regex,text,0,&match_info);
    for(i=0;i<counter;i++)
        g_match_info_next(match_info,NULL);
    gboolean ans2 = g_match_info_fetch_pos(match_info,0,&start,&end);
    if(ans1==TRUE && ans2 == TRUE)
    {
        gtk_text_buffer_get_iter_at_offset(buffer, &start_iter,start);
        gtk_text_buffer_get_iter_at_offset(buffer, &end_iter,end);
        gtk_text_buffer_select_range(buffer,&start_iter,&end_iter);
        counter++;
        }
    else
        counter=0;
    } 

void reg_exp_search_activated(GtkWidget *widget)
{
    GtkWidget *dialog,*cmdsearch,*cmdclose,*lblregexp,*entry_regexp,*hbox;
    
    dialog = gtk_dialog_new();
    
    cmdsearch = gtk_button_new_with_mnemonic("_Search");
    cmdclose = gtk_button_new_with_mnemonic("_Close");
    
    lblregexp = gtk_label_new("Regular Expression");
    entry_regexp = gtk_entry_new();
    
    hbox = gtk_hbox_new(FALSE,5);
    gtk_box_pack_start(GTK_BOX(hbox),lblregexp,FALSE,FALSE,5);
    gtk_box_pack_start(GTK_BOX(hbox),entry_regexp,TRUE,TRUE,5);
    
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),hbox,TRUE,TRUE,5);
    gtk_box_pack_end(GTK_BOX(GTK_DIALOG(dialog)->action_area),cmdsearch,FALSE,FALSE,5);
    gtk_box_pack_end(GTK_BOX(GTK_DIALOG(dialog)->action_area),cmdclose,FALSE,FALSE,5);
    
    g_signal_connect(G_OBJECT(cmdsearch),"clicked",G_CALLBACK(cmdsearch_clicked),entry_regexp);
    g_signal_connect(G_OBJECT(cmdclose),"clicked",G_CALLBACK(cmdclose_clicked),dialog);
    
    gtk_window_set_title(GTK_WINDOW(dialog),"Regular Expression Search");
    gtk_widget_show_all(dialog);
    }

void file_information_activated(GtkWidget *widget)
{
    GtkWidget *dialog, *lbl_file_name, *label_file_name, *lbl_file_path, *label_file_path, *lbl_lines, *label_lines, *lbl_characters, *label_characters;
    
    dialog = gtk_dialog_new_with_buttons("gIDLE",GTK_WINDOW(window),GTK_DIALOG_DESTROY_WITH_PARENT,GTK_STOCK_OK,GTK_RESPONSE_NONE,NULL);
    gtk_widget_set_size_request(dialog,400,300);
    label_lines = gtk_label_new("Lines");
    label_characters = gtk_label_new("Characters");
    label_file_name = gtk_label_new("File Name");
    label_file_path = gtk_label_new("File Path");
    
    GtkTextBuffer *buffer;
    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(codewidget[gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook))].txtinput));
    
    lbl_characters = gtk_label_new(g_strdup_printf("%d",gtk_text_buffer_get_char_count(buffer)));
    lbl_lines = gtk_label_new(g_strdup_printf("%d",gtk_text_buffer_get_line_count(buffer)));
    lbl_file_name = gtk_label_new(gtk_notebook_get_tab_label_text(GTK_NOTEBOOK(notebook),vbox_txtinput[gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook))]));
    lbl_file_path = gtk_label_new(filepathsarray[gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook))]);
    
    GtkWidget *hbox,*vbox1,*vbox2;
    int i;
    hbox = gtk_hbox_new(FALSE,2);
    gtk_box_pack_start_defaults(GTK_BOX(GTK_DIALOG(dialog)->vbox),hbox);
    vbox1 = gtk_vbox_new(FALSE,2);
    vbox2 = gtk_vbox_new(FALSE,2);
    gtk_box_pack_start_defaults(GTK_BOX(hbox),vbox1);
    gtk_box_pack_start_defaults(GTK_BOX(hbox),vbox2);
    
    gtk_box_pack_start_defaults(GTK_BOX(vbox1),label_file_name);
    gtk_box_pack_start_defaults(GTK_BOX(vbox1),label_file_path);
    gtk_box_pack_start_defaults(GTK_BOX(vbox1),label_lines);
    gtk_box_pack_start_defaults(GTK_BOX(vbox1),label_characters);
      
    gtk_box_pack_start_defaults(GTK_BOX(vbox2),lbl_file_name);
    gtk_box_pack_start_defaults(GTK_BOX(vbox2),lbl_file_path);
    gtk_box_pack_start_defaults(GTK_BOX(vbox2),lbl_lines);
    gtk_box_pack_start_defaults(GTK_BOX(vbox2),lbl_characters);
    
    int result;
    gtk_widget_show_all(dialog);
    result = gtk_dialog_run(GTK_DIALOG(dialog));
    if (result == GTK_RESPONSE_NONE)
        gtk_widget_destroy(dialog);
    }
    
void striptrailingspaces_activated(GtkWidget *widget)
{
    GtkTextBuffer *buffer;
    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(codewidget[gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook))].txtinput));
    GtkTextIter start_iter,end_iter,iter;
    int i;
    for(i=0;i<gtk_text_buffer_get_line_count(buffer)-1;i++)
    {
        gtk_text_buffer_get_iter_at_line(buffer,&start_iter,i);
        gtk_text_buffer_get_iter_at_line(buffer,&end_iter,i+1);
        gtk_text_buffer_get_iter_at_offset(buffer,&end_iter,gtk_text_iter_get_offset(&end_iter)-1);
        char *line_text = gtk_text_buffer_get_text(buffer,&start_iter,&end_iter,FALSE);
        int j,space_count=0;
        for(j=strlen(line_text)-1;j>=0;j--)
        {
            if(line_text[j] == ' ')
                space_count++;
            else
                break;
            }
        gtk_text_buffer_get_iter_at_line_offset(buffer,&start_iter,gtk_text_iter_get_line(&start_iter),strlen(line_text)-space_count-1);
        gtk_text_buffer_delete(buffer,&start_iter,&end_iter);
        }    
    gtk_text_buffer_get_iter_at_line(buffer,&start_iter,i);
    gtk_text_buffer_get_end_iter(buffer,&end_iter);
    char *line_text = gtk_text_buffer_get_text(buffer,&start_iter,&end_iter,FALSE);
    int j,space_count=0;
    for(j=strlen(line_text)-1;j>=0;j--)
    {
        if(line_text[j] == ' ')
            space_count++;
        else
            break;
        }
    gtk_text_buffer_get_iter_at_line_offset(buffer,&start_iter,gtk_text_iter_get_line(&start_iter),strlen(line_text)-space_count-1);
    gtk_text_buffer_delete(buffer,&start_iter,&end_iter);    
    }

void *run_gnome_terminal(void *arg)
{
    system((char *)arg);
    }

void gnome_terminal_activated(GtkWidget *widget)
{
    pthread_t gnome_thread;
     
    int res = pthread_create(&gnome_thread,NULL,run_gnome_terminal,(void *) "gnome-terminal");
}

void pdb_activated(GtkWidget *widget)
{
    if (filepathsarray[gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook))]!="")
    {
        GtkWidget *pdb_window,*pdb_vte;
        //pdb_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
        //pdb_vte = vte_terminal_new();
        char *command[5] = { "./shell", "-m","pdb",filepathsarray[gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook))],NULL};
        int pid;
        GPid child;
        GError *error=NULL;
        g_spawn_async(NULL,command,NULL,G_SPAWN_DO_NOT_REAP_CHILD,NULL,NULL,&child,&error);
        }
    else
    {
        GtkWidget *dialog,*label;
        
        dialog = gtk_dialog_new_with_buttons("gIDLE",GTK_WINDOW(window),GTK_DIALOG_DESTROY_WITH_PARENT,GTK_STOCK_OK,GTK_RESPONSE_NONE,NULL);
        label = gtk_label_new("Please save file before debugging.");
        
        gtk_box_pack_start_defaults(GTK_BOX(GTK_DIALOG(dialog)->vbox),label);
        int result;
        gtk_widget_show_all(dialog);
        result = gtk_dialog_run(GTK_DIALOG(dialog));
        if (result == GTK_RESPONSE_NONE)
            gtk_widget_destroy(dialog);
        }
    }

void uncommentout_activated(GtkWidget *widget)
{
    GtkTextBuffer *buffer;
    int i;
    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(codewidget[gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook))].txtinput));
    GtkTextIter selection_start_iter,selection_end_iter;
    gtk_text_buffer_get_selection_bounds(buffer,&selection_start_iter,&selection_end_iter);
    int selection_start_iter_line = gtk_text_iter_get_line(&selection_start_iter),selection_end_iter_line = gtk_text_iter_get_line(&selection_end_iter);
    int selection_start_iter_line_offset = gtk_text_iter_get_line_offset(&selection_start_iter);
    for (i=selection_start_iter_line;i<=selection_end_iter_line;i++)
    {
        GtkTextIter new_iter,line_iter;
        gtk_text_buffer_get_iter_at_line_offset(buffer,&new_iter,i,1);
        gtk_text_buffer_get_iter_at_line_offset(buffer,&line_iter,i,0);
        if (strcmp("#",gtk_text_buffer_get_text(buffer,&line_iter,&new_iter,TRUE))==0)
            gtk_text_buffer_delete(buffer,&line_iter,&new_iter);
        }
    }

void commentout_activated(GtkWidget *widget)
{
    GtkTextBuffer *buffer;
    int i;
    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(codewidget[gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook))].txtinput));
    GtkTextIter selection_start_iter,selection_end_iter;
    gtk_text_buffer_get_selection_bounds(buffer,&selection_start_iter,&selection_end_iter);
    int selection_start_iter_line = gtk_text_iter_get_line(&selection_start_iter),selection_end_iter_line = gtk_text_iter_get_line(&selection_end_iter);
    int selection_start_iter_offset = gtk_text_iter_get_offset(&selection_start_iter);
    GtkTextIter line_iter;
    for (i=selection_start_iter_line;i<=selection_end_iter_line;i++)
    {
        gtk_text_buffer_get_iter_at_line(buffer,&line_iter,i);
        gtk_text_buffer_insert(buffer,&line_iter,"#",-1);
        }
    }
    
void exit_activated(GtkWidget *widget,GtkWidget *dialog)
{
    if (GTK_IS_WIDGET(dialog) == TRUE)
    {
        gtk_widget_destroy(dialog);
        }
    gtk_widget_destroy(window);
    }

void openshell_activated(GtkWidget *widget)
{
    char *arg[2];
    arg[0]="./shell";
    arg[1]=NULL;
    GPid child;
    GError *error=NULL;
    g_spawn_async(NULL,arg,NULL,G_SPAWN_DO_NOT_REAP_CHILD,NULL,NULL,&child,&error);
    }
    
void runmodule_activated(GtkWidget *widget)
{
    pid_t k;
    GString *command_string = g_string_new(""),*gstring = g_string_new("");
    char *settings;
    int count_spaces=0,j=0,i;
    gsize length = -1;
    if (g_file_get_contents("./settings.ini",&settings,&length,NULL) == TRUE)
    {
        int opening_index = find_strstr(settings,"<command>",0) + strlen("<command>");
        int closing_index = find_strstr(settings,"</command>",0);
        for(i=opening_index;i<closing_index;i++)
        {
            g_string_append_c(command_string,settings[i]);
            if (settings[i] ==' ')
                count_spaces++;
            }
        }
    
    char *_command[count_spaces+2];
    _command[0] = "./shell";    
    if (strcmp(mode,"File") == 0)
        _command[1] = filepathsarray[gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook))];
    else
        _command[1] = mainfile->str;
    for(i=0;i<=strlen(command_string->str);i++)
    {
        if(command_string->str[i]==' ' || i == strlen(command_string->str))
        {
            if(j>1)
                asprintf(&_command[j],"%s",gstring->str);
            
            gstring = g_string_erase(gstring,0,-1);
            j++;
            }
        else
            g_string_append_c(gstring,command_string->str[i]);
        }
    GtkTextBuffer *buffer;
    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(codewidget[gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook))].txtinput));
    GtkTextIter start_iter, end_iter;
    gtk_text_buffer_get_start_iter(buffer, &start_iter);
    gtk_text_buffer_get_end_iter(buffer, &end_iter);
    if (function_filechanged(filepathsarray[gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook))],gtk_text_buffer_get_text(buffer,&start_iter,&end_iter,FALSE)) == TRUE)
    {
        GtkWidget *dialog,*label,*cmdok;
        
        dialog = gtk_dialog_new();
        label = gtk_label_new("Current file is not saved. Save current file before running.");
        cmdok = gtk_button_new_with_mnemonic("_OK");
        gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox),label);
        gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->action_area),cmdok);
        g_signal_connect(G_OBJECT(cmdok),"clicked",G_CALLBACK(cmdclose_clicked),dialog);
        gtk_window_set_title(GTK_WINDOW(dialog),"gIDLE");
        gtk_widget_show_all(dialog);
        }
    else
    {
        _command[0]="./shell";
         _command[count_spaces+1] = NULL;
        int i =-1;
        while(_command[++i]!=NULL)
            printf("%s\n",_command[i]);
        GPid child;
        GError *error=NULL;
        g_spawn_async(NULL,_command,NULL,G_SPAWN_DO_NOT_REAP_CHILD,NULL,NULL,&child,&error);        
        }
    }

void find_activated(GtkWidget *widget)
{
    GtkWidget *dialog,*cmdfind,*cmdfindnext,*cmdfindprevious,*cmdclose,*hbox,*label,*checkbutton_match_word;
    
    dialog = gtk_dialog_new();
    cmdfind = gtk_button_new_with_mnemonic("_Find");
    cmdfindnext = gtk_button_new_with_mnemonic("_Find Next");
    cmdfindprevious = gtk_button_new_with_mnemonic("_Find Previous");
    cmdclose = gtk_button_new_with_mnemonic("_Close");
    txtfind = gtk_entry_new();
    label = gtk_label_new("Find");
    checkbutton_match_word = gtk_check_button_new_with_label("Match Word");
    
    hbox = gtk_hbox_new(FALSE,5);
    
    gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,5);   
    gtk_box_pack_start(GTK_BOX(hbox),txtfind,TRUE,TRUE,5);
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),hbox,FALSE,FALSE,0);
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),checkbutton_match_word,FALSE,FALSE,0);
    
    gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->action_area),cmdfind);
    gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->action_area),cmdfindnext);
    gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->action_area),cmdfindprevious);
    gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->action_area),cmdclose);
    
    g_signal_connect(G_OBJECT(cmdclose),"clicked",G_CALLBACK(cmdclose_clicked),(gpointer)dialog);
    g_signal_connect(G_OBJECT(cmdfindnext),"clicked",G_CALLBACK(cmdfindnext_clicked),(gpointer)checkbutton_match_word);
    g_signal_connect(G_OBJECT(cmdfind),"clicked",G_CALLBACK(cmdfind_clicked),(gpointer)checkbutton_match_word);
    g_signal_connect(G_OBJECT(cmdfindprevious),"clicked",G_CALLBACK(cmdfindprevious_clicked),(gpointer)checkbutton_match_word);
    
    gtk_window_set_title(GTK_WINDOW(dialog),"Find");
    gtk_window_set_modal(GTK_WINDOW(dialog),FALSE);
    gtk_widget_show_all(dialog);
    gtk_dialog_run(GTK_DIALOG(dialog));
    }
        
void cmdgotoline_clicked(GtkWidget *widget, GtkEntry *entry)
{
    int linenumber = 0;
    const char *charlinenumber;
    charlinenumber = gtk_entry_get_text(entry);
    linenumber = atoi(charlinenumber);
    GtkTextIter line_iter;
    GtkTextBuffer *buffer;
    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(codewidget[gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook))].txtinput));
    gtk_text_buffer_get_iter_at_line(buffer,&line_iter,linenumber);
    gtk_text_buffer_select_range(buffer,&line_iter,&line_iter);
    gtk_text_view_scroll_to_iter(GTK_TEXT_VIEW(codewidget[gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook))].txtinput),&line_iter,0.1,FALSE,0.5,0.5);
    }

extern void cmdclose_clicked(GtkWidget *widget, GtkWidget *dialog)
{
    gtk_widget_destroy(dialog);
    }

void gotoline_activated(GtkWidget *widget)
{
    GtkWidget *dialog, *cmdgotoline, *label, *txtgotoline,*hbox,*cmdclose;
    
    dialog = gtk_dialog_new();
    label = gtk_label_new("Go To Line");
    txtgotoline = gtk_entry_new();
    cmdgotoline = gtk_button_new_with_mnemonic("OK");
    cmdclose = gtk_button_new_with_mnemonic("Close");
    
    hbox = gtk_hbox_new(FALSE,5);
    gtk_container_set_border_width(GTK_CONTAINER(hbox),10);
    gtk_box_pack_start_defaults(GTK_BOX(hbox),label);
    gtk_box_pack_start_defaults(GTK_BOX(hbox),txtgotoline);
    gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox),hbox);
    
    gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->action_area),cmdgotoline);
    gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->action_area),cmdclose);
    g_signal_connect(G_OBJECT(cmdgotoline),"clicked",G_CALLBACK(cmdgotoline_clicked),(gpointer)GTK_ENTRY(txtgotoline));
    g_signal_connect(G_OBJECT(cmdclose),"clicked",G_CALLBACK(cmdclose_clicked),(gpointer)dialog);
    gtk_window_set_modal(GTK_WINDOW(dialog),FALSE);
    gtk_window_set_title(GTK_WINDOW(dialog),"Go To Line");
    gtk_widget_show_all(dialog);
    gtk_dialog_run(GTK_DIALOG(dialog));
    }

void cmd_find_in_file_clicked(GtkWidget *widget)
{
    const char *file_path = gtk_entry_get_text(GTK_ENTRY(entry_file));
    const char *find_text = gtk_entry_get_text(GTK_ENTRY(entry_find_in_file));
    
    char *contents;
    gsize length = -1;
    g_file_get_contents(file_path,&contents,&length,NULL);
    if(find_strstr(contents,find_text,0)!=-1)
        gtk_label_set_text(GTK_LABEL(lbl_status),"Text found in the given file");
    }

void cmd_open_file_clicked(GtkWidget *widget)
{
    GtkWidget *dialog;
    GtkFileFilter *filefilter_py,*filefilter_txt,*filefilter_all;
    filefilter_py = gtk_file_filter_new();
    filefilter_txt = gtk_file_filter_new();
    filefilter_all = gtk_file_filter_new();
    gtk_file_filter_set_name(filefilter_py,"Python Files(*.py)");
    gtk_file_filter_add_pattern(filefilter_py,"*.py");
    gtk_file_filter_set_name(filefilter_txt,"Text Files(*.txt)");
    gtk_file_filter_add_pattern(filefilter_txt,"*.txt");
    gtk_file_filter_set_name(filefilter_all,"All Files(*.*)");
    gtk_file_filter_add_pattern(filefilter_all,"*.*");
    dialog = gtk_file_chooser_dialog_new("Save File", GTK_WINDOW(window), GTK_FILE_CHOOSER_ACTION_SAVE,GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,GTK_STOCK_SAVE,GTK_RESPONSE_ACCEPT,NULL);
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog),filefilter_py);
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog),filefilter_txt);
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog),filefilter_all);
    if(gtk_dialog_run(GTK_DIALOG(dialog))== GTK_RESPONSE_ACCEPT)
    {
        char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        gtk_entry_set_text(GTK_ENTRY(entry_file),filename);
        }
    gtk_widget_destroy(dialog);
    }

void find_in_file_activated(GtkWidget *widget)
{
    GtkWidget *dialog,*lbl_find,*lbl_file,*cmd_find,*cmd_open_file,*cmd_close,*fixed;
    
    dialog = gtk_dialog_new();
    
    cmd_find = gtk_button_new_with_mnemonic("_Find");
    cmd_open_file = gtk_button_new_with_mnemonic("_Open File");
    cmd_close = gtk_button_new_with_mnemonic("_Close");
    
    entry_file = gtk_entry_new();
    entry_find_in_file = gtk_entry_new();
    
    lbl_find = gtk_label_new("Find");
    lbl_file = gtk_label_new("File");
    lbl_status = gtk_label_new("");
    
    fixed = gtk_fixed_new();
    gtk_widget_set_size_request(cmd_find,95,31);
    gtk_widget_set_size_request(cmd_open_file,95,31);
    gtk_widget_set_size_request(cmd_close,95,31);
    gtk_widget_set_size_request(entry_file,271,31);
    gtk_widget_set_size_request(entry_find_in_file,271,31);
    gtk_widget_set_size_request(lbl_find,66,21);
    gtk_widget_set_size_request(lbl_file,66,21);
    gtk_widget_set_size_request(lbl_status,350,21);
    
    gtk_fixed_put(GTK_FIXED(fixed),cmd_find,360,10);
    gtk_fixed_put(GTK_FIXED(fixed),cmd_open_file,360,50);
    gtk_fixed_put(GTK_FIXED(fixed),cmd_close,360,90);
    gtk_fixed_put(GTK_FIXED(fixed),entry_file,80,50);
    gtk_fixed_put(GTK_FIXED(fixed),entry_find_in_file,80,10);
    gtk_fixed_put(GTK_FIXED(fixed),lbl_find,5,16);
    gtk_fixed_put(GTK_FIXED(fixed),lbl_file,5,50);
    gtk_fixed_put(GTK_FIXED(fixed),lbl_status,5,93);
    
    g_signal_connect(G_OBJECT(cmd_find),"clicked",G_CALLBACK(cmd_find_in_file_clicked),NULL);
    g_signal_connect(G_OBJECT(cmd_open_file),"clicked",G_CALLBACK(cmd_open_file_clicked),NULL);
    g_signal_connect(G_OBJECT(cmd_close),"clicked",G_CALLBACK(cmdclose_clicked),dialog);
    
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),fixed,TRUE,TRUE,5);
    
    gtk_window_set_title(GTK_WINDOW(dialog),"Find in File");
    gtk_widget_show_all(dialog);
    }

void cmdfindprevious_clicked(GtkWidget *widget,GtkToggleButton *checkbutton)
{
    const gchar *textfind;
    textfind = gtk_entry_get_text(GTK_ENTRY(txtfind));
    GtkTextBuffer *buffer;
    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(codewidget[gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook))].txtinput));
    GtkTextIter current_iter;
    gtk_text_buffer_get_iter_at_mark(buffer,&current_iter,gtk_text_buffer_get_insert(buffer));
    offset = gtk_text_iter_get_offset(&current_iter);
    GtkTextIter match_start_iter,match_end_iter;
    GtkTextIter start_iter;
    gtk_text_buffer_get_iter_at_offset(buffer,&start_iter,offset-1);
    if(gtk_text_iter_backward_search(&start_iter,textfind,GTK_TEXT_SEARCH_TEXT_ONLY,&match_start_iter,&match_end_iter,NULL) == TRUE)
    {
         if(gtk_toggle_button_get_active(checkbutton) == FALSE)
        {
            gtk_text_buffer_select_range(buffer,&match_start_iter,&match_end_iter);
            offset = gtk_text_iter_get_offset(&match_end_iter);
            gtk_text_view_scroll_to_iter(GTK_TEXT_VIEW(codewidget[gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook))].txtinput),&match_end_iter,0.1,FALSE,0.5,0.5);
            }
        else
        {
            int line_offset_end = gtk_text_iter_get_line_offset(&match_end_iter);
            int line_offset_start = gtk_text_iter_get_line_offset(&match_start_iter);
            int line_start = gtk_text_iter_get_line(&match_start_iter);
            int line_end = line_start++;
            GtkTextIter line_start_iter,line_end_iter;
            gtk_text_buffer_get_iter_at_line(buffer,&line_start_iter,line_start);
            gtk_text_buffer_get_iter_at_line(buffer,&line_end_iter,line_end);
            char *line_text;
            line_text = g_strdup_printf("%c%s%c",' ',gtk_text_buffer_get_text(buffer,&line_start_iter,&line_end_iter,FALSE),' ');
                        
            if (!((isalpha(line_text[line_offset_end+1]) && isalpha(line_text[line_offset_start]))))
            {
                gtk_text_buffer_select_range(buffer,&match_start_iter,&match_end_iter);
                offset = gtk_text_iter_get_offset(&match_end_iter);
                gtk_text_view_scroll_to_iter(GTK_TEXT_VIEW(codewidget[gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook))].txtinput),&match_end_iter,0.1,FALSE,0.5,0.5);
                }                        
            }
        }
    }

void cmdfind_clicked(GtkWidget *widget,GtkToggleButton *checkbutton)
{
    const gchar *textfind;
    textfind = gtk_entry_get_text(GTK_ENTRY(txtfind));
    GtkTextBuffer *buffer;
    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(codewidget[gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook))].txtinput));
    GtkTextIter match_start_iter,match_end_iter;
    GtkTextIter start_iter;
    gtk_text_buffer_get_iter_at_offset(buffer,&start_iter,0);
    if(gtk_text_iter_forward_search(&start_iter,textfind,GTK_TEXT_SEARCH_TEXT_ONLY,&match_start_iter,&match_end_iter,NULL) == TRUE)
    {
        if(gtk_toggle_button_get_active(checkbutton) == FALSE)
        {
            gtk_text_buffer_select_range(buffer,&match_start_iter,&match_end_iter);
            offset = gtk_text_iter_get_offset(&match_end_iter);
            gtk_text_view_scroll_to_iter(GTK_TEXT_VIEW(codewidget[gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook))].txtinput),&match_end_iter,0.1,FALSE,0.5,0.5);
            }
        else
        {
            int line_offset_end = gtk_text_iter_get_line_offset(&match_end_iter);
            int line_offset_start = gtk_text_iter_get_line_offset(&match_start_iter);
            int line_start = gtk_text_iter_get_line(&match_start_iter);
            int line_end = line_start++;
            GtkTextIter line_start_iter,line_end_iter;
            gtk_text_buffer_get_iter_at_line(buffer,&line_start_iter,line_start);
            gtk_text_buffer_get_iter_at_line(buffer,&line_end_iter,line_end);
            char *line_text;
            line_text = g_strdup_printf("%c%s%c",' ',gtk_text_buffer_get_text(buffer,&line_start_iter,&line_end_iter,FALSE),' ');
                        
            if (!((isalpha(line_text[line_offset_end+1]) && isalpha(line_text[line_offset_start]))))
            {
                gtk_text_buffer_select_range(buffer,&match_start_iter,&match_end_iter);
                offset = gtk_text_iter_get_offset(&match_end_iter);
                gtk_text_view_scroll_to_iter(GTK_TEXT_VIEW(codewidget[gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook))].txtinput),&match_end_iter,0.1,FALSE,0.5,0.5);
                }                        
            }
        }
    }
          
void cmdfindnext_clicked(GtkWidget *widget,GtkToggleButton *checkbutton)
{
    GtkTextBuffer *buffer;
    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(codewidget[gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook))].txtinput));
    GtkTextIter current_iter;
    gtk_text_buffer_get_iter_at_mark(buffer,&current_iter,gtk_text_buffer_get_insert(buffer));
    offset = gtk_text_iter_get_offset(&current_iter);
    const gchar *textfind;
    textfind = gtk_entry_get_text(GTK_ENTRY(txtfind));
    GtkTextIter match_start_iter,match_end_iter;
    GtkTextIter start_iter;
    gtk_text_buffer_get_iter_at_offset(buffer,&start_iter,offset+1);
    if(gtk_text_iter_forward_search(&start_iter,textfind,GTK_TEXT_SEARCH_TEXT_ONLY,&match_start_iter,&match_end_iter,NULL) == TRUE)
    {
         if(gtk_toggle_button_get_active(checkbutton) == FALSE)
        {
            gtk_text_buffer_select_range(buffer,&match_start_iter,&match_end_iter);
            offset = gtk_text_iter_get_offset(&match_end_iter);
            gtk_text_view_scroll_to_iter(GTK_TEXT_VIEW(codewidget[gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook))].txtinput),&match_end_iter,0.1,FALSE,0.5,0.5);
            }
        else
        {
            int line_offset_end = gtk_text_iter_get_line_offset(&match_end_iter);
            int line_offset_start = gtk_text_iter_get_line_offset(&match_start_iter);
            int line_start = gtk_text_iter_get_line(&match_start_iter);
            int line_end = line_start++;
            GtkTextIter line_start_iter,line_end_iter;
            gtk_text_buffer_get_iter_at_line(buffer,&line_start_iter,line_start);
            gtk_text_buffer_get_iter_at_line(buffer,&line_end_iter,line_end);
            char *line_text;
            line_text = g_strdup_printf("%c%s%c",' ',gtk_text_buffer_get_text(buffer,&line_start_iter,&line_end_iter,FALSE),' ');
                        
            if (!((isalpha(line_text[line_offset_end+1]) && isalpha(line_text[line_offset_start]))))
            {
                gtk_text_buffer_select_range(buffer,&match_start_iter,&match_end_iter);
                offset = gtk_text_iter_get_offset(&match_end_iter);
                gtk_text_view_scroll_to_iter(GTK_TEXT_VIEW(codewidget[gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook))].txtinput),&match_end_iter,0.1,FALSE,0.5,0.5);
                }                        
            }
        }
    }

void cmdreplace_clicked(GtkWidget *widget,GtkToggleButton *checkbutton)
{
    const gchar *textfind,*textreplace;
    textfind = gtk_entry_get_text(GTK_ENTRY(txtfind));
    textreplace = gtk_entry_get_text(GTK_ENTRY(txtreplace));
    GtkTextBuffer *buffer;
    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(codewidget[gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook))].txtinput));
    GtkTextIter current_iter;
    gtk_text_buffer_get_iter_at_mark(buffer,&current_iter,gtk_text_buffer_get_insert(buffer));
    offset = gtk_text_iter_get_offset(&current_iter);
    GtkTextIter match_start_iter,match_end_iter;
    GtkTextIter start_iter;
    gtk_text_buffer_get_iter_at_offset(buffer,&start_iter,offset);
    
    if(gtk_text_iter_forward_search(&start_iter,textfind,GTK_TEXT_SEARCH_TEXT_ONLY,&match_start_iter,&match_end_iter,NULL) == TRUE)
    {
        GtkTextIter replace_text_start_iter, replace_text_end_iter;
        int replace_text_start_iter_offset =  gtk_text_iter_get_offset(&match_start_iter);
        int replace_text_end_iter_offset = replace_text_start_iter_offset + strlen(textreplace);
        if(gtk_toggle_button_get_active(checkbutton) == FALSE)
        {            
            gtk_text_buffer_select_range(buffer,&match_start_iter,&match_end_iter);
            offset = gtk_text_iter_get_offset(&match_end_iter);
            gtk_text_view_scroll_to_iter(GTK_TEXT_VIEW(codewidget[gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook))].txtinput),&match_end_iter,0.1,FALSE,0.5,0.5);
            gtk_text_buffer_delete_selection(buffer,FALSE,FALSE);
            gtk_text_buffer_insert_at_cursor(buffer,textreplace,-1);
            gtk_text_buffer_get_iter_at_offset(buffer,&replace_text_start_iter,replace_text_start_iter_offset);
            gtk_text_buffer_get_iter_at_offset(buffer,&replace_text_end_iter,replace_text_end_iter_offset);
            gtk_text_buffer_select_range(buffer,&replace_text_start_iter,&replace_text_end_iter);
            }
        else
        {
            int line_offset_end = gtk_text_iter_get_line_offset(&match_end_iter);
            int line_offset_start = gtk_text_iter_get_line_offset(&match_start_iter);
            int line_start = gtk_text_iter_get_line(&match_start_iter);
            int line_end = line_start++;
            GtkTextIter line_start_iter,line_end_iter;
            gtk_text_buffer_get_iter_at_line(buffer,&line_start_iter,line_start);
            gtk_text_buffer_get_iter_at_line(buffer,&line_end_iter,line_end);
            char *line_text;
            line_text = g_strdup_printf("%c%s%c",' ',gtk_text_buffer_get_text(buffer,&line_start_iter,&line_end_iter,FALSE),' ');
                        
            if (!((isalpha(line_text[line_offset_end+1]) && isalpha(line_text[line_offset_start]))))
            {
                gtk_text_buffer_select_range(buffer,&match_start_iter,&match_end_iter);
                offset = gtk_text_iter_get_offset(&match_end_iter);
                gtk_text_view_scroll_to_iter(GTK_TEXT_VIEW(codewidget[gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook))].txtinput),&match_end_iter,0.1,FALSE,0.5,0.5);
                gtk_text_buffer_delete_selection(buffer,FALSE,FALSE);
                gtk_text_buffer_insert_at_cursor(buffer,textreplace,-1);
                gtk_text_buffer_get_iter_at_offset(buffer,&replace_text_start_iter,replace_text_start_iter_offset);
                gtk_text_buffer_get_iter_at_offset(buffer,&replace_text_end_iter,replace_text_end_iter_offset);
                gtk_text_buffer_select_range(buffer,&replace_text_start_iter,&replace_text_end_iter);
                }
            }            
        }
    } 

void cmdreplaceall_clicked(GtkWidget *widget,GtkToggleButton *checkbutton)
{
    const gchar *textfind,*textreplace;
    textfind = gtk_entry_get_text(GTK_ENTRY(txtfind));
    textreplace = gtk_entry_get_text(GTK_ENTRY(txtreplace));
    GtkTextBuffer *buffer;
    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(codewidget[gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook))].txtinput));
    GtkTextIter current_iter;
    GtkTextIter match_start_iter,match_end_iter;
    GtkTextIter start_iter;
    gtk_text_buffer_get_iter_at_mark(buffer,&current_iter,gtk_text_buffer_get_insert(buffer));
    offset = gtk_text_iter_get_offset(&current_iter);
    while(1)
    {
        gtk_text_buffer_get_iter_at_offset(buffer,&start_iter,offset);
        if(gtk_text_iter_forward_search(&start_iter,textfind,GTK_TEXT_SEARCH_TEXT_ONLY,&match_start_iter,&match_end_iter,NULL) == TRUE)
        {
            if(gtk_toggle_button_get_active(checkbutton) == FALSE)
            {
                gtk_text_buffer_select_range(buffer,&match_start_iter,&match_end_iter);
                offset = gtk_text_iter_get_offset(&match_end_iter);
                gtk_text_buffer_delete_selection(buffer,FALSE,FALSE);
                gtk_text_buffer_insert_at_cursor(buffer,textreplace,-1);  
                }
            else
            {
                int line_offset_end = gtk_text_iter_get_line_offset(&match_end_iter);
                int line_offset_start = gtk_text_iter_get_line_offset(&match_start_iter);
                int line_start = gtk_text_iter_get_line(&match_start_iter);
                int line_end = line_start++;
                GtkTextIter line_start_iter,line_end_iter;
                gtk_text_buffer_get_iter_at_line(buffer,&line_start_iter,line_start);
                gtk_text_buffer_get_iter_at_line(buffer,&line_end_iter,line_end);
                char *line_text;
                line_text = g_strdup_printf("%c%s%c",' ',gtk_text_buffer_get_text(buffer,&line_start_iter,&line_end_iter,FALSE),' ');
                if (!((isalpha(line_text[line_offset_end+1]) && isalpha(line_text[line_offset_start]))))
                {
                    gtk_text_buffer_select_range(buffer,&match_start_iter,&match_end_iter);
                    offset = gtk_text_iter_get_offset(&match_end_iter);
                    gtk_text_buffer_delete_selection(buffer,FALSE,FALSE);
                    gtk_text_buffer_insert_at_cursor(buffer,textreplace,-1);
                    }
                }
            }
        else
        break;
        }
    } 
        
void findandreplace_activated(GtkWidget *widget)
{
    GtkWidget *findandreplacewindow;
    
    GtkWidget *cmdfind;
    GtkWidget *cmdreplace,*cmdreplaceall;    
    GtkWidget *frame,*cmdclose;
    GtkWidget *lblfind,*lblreplace,*checkbutton_match_word;
    
    findandreplacewindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    frame = gtk_fixed_new();
    txtfind = gtk_entry_new();
    gtk_widget_set_size_request(txtfind,230,31);
    gtk_fixed_put(GTK_FIXED(frame),txtfind,100,10);
    checkbutton_match_word = gtk_check_button_new_with_label("Match Word");
    gtk_fixed_put(GTK_FIXED(frame),checkbutton_match_word,10,90);
    
    lblfind = gtk_label_new("Find");
    gtk_widget_set_size_request(lblfind,66,21);
    gtk_fixed_put(GTK_FIXED(frame),lblfind,10,17);
    lblreplace = gtk_label_new("Replace");
    gtk_widget_set_size_request(lblreplace,66,21);
    gtk_fixed_put(GTK_FIXED(frame),lblreplace,10,51);
    
    cmdfind = gtk_button_new_with_mnemonic("_Find");
    gtk_widget_set_size_request(cmdfind,95,31);
    gtk_fixed_put(GTK_FIXED(frame),cmdfind,350,10);
    txtreplace = gtk_entry_new();
    gtk_widget_set_size_request(txtreplace,231,31);
    gtk_fixed_put(GTK_FIXED(frame),txtreplace,100,50);
    cmdreplace = gtk_button_new_with_mnemonic("_Replace");
    gtk_widget_set_size_request(cmdreplace,95,31);
    gtk_fixed_put(GTK_FIXED(frame),cmdreplace,350,50);
    cmdreplaceall = gtk_button_new_with_mnemonic("_Replace All");
    gtk_widget_set_size_request(cmdreplaceall,95,31);
    gtk_fixed_put(GTK_FIXED(frame),cmdreplaceall,350,90);
    cmdclose = gtk_button_new_with_mnemonic("_Close");
    gtk_widget_set_size_request(cmdclose,95,31);
    gtk_fixed_put(GTK_FIXED(frame),cmdclose,350,130);
    
    g_signal_connect(G_OBJECT(cmdfind),"clicked",G_CALLBACK(cmdfindnext_clicked),checkbutton_match_word);
    g_signal_connect(G_OBJECT(cmdreplaceall),"clicked",G_CALLBACK(cmdreplaceall_clicked),checkbutton_match_word);
    g_signal_connect(G_OBJECT(cmdreplace),"clicked",G_CALLBACK(cmdreplace_clicked),checkbutton_match_word);
    g_signal_connect(G_OBJECT(cmdclose),"clicked",G_CALLBACK(cmdclose_clicked),findandreplacewindow);
    
    gtk_container_add(GTK_CONTAINER(findandreplacewindow),frame);
    gtk_window_set_transient_for(GTK_WINDOW(findandreplacewindow),GTK_WINDOW(window));
    gtk_window_set_destroy_with_parent(GTK_WINDOW(findandreplacewindow),TRUE);
    
    gtk_widget_set_size_request(GTK_WIDGET(findandreplacewindow),453,173);
    gtk_window_set_title(GTK_WINDOW(findandreplacewindow),"Find and Replace");
    gtk_widget_show_all(findandreplacewindow);
    }

void find_selected_activated(GtkWidget *widget)
{
    GtkTextBuffer *buffer;
    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(codewidget[gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook))].txtinput));
    if (gtk_text_buffer_get_has_selection(buffer)== TRUE)
    {
        GtkTextIter current_iter;
        gtk_text_buffer_get_iter_at_mark(buffer,&current_iter,gtk_text_buffer_get_insert(buffer));
        offset = gtk_text_iter_get_offset(&current_iter);
        char *textfind;
        GtkTextIter match_start_iter,match_end_iter;
        gtk_text_buffer_get_selection_bounds(buffer,&match_start_iter,&match_end_iter);
        textfind = gtk_text_buffer_get_text(buffer,&match_start_iter,&match_end_iter,TRUE);
        GtkTextIter start_iter;
        gtk_text_buffer_get_iter_at_offset(buffer,&start_iter,offset+1);
        if(gtk_text_iter_forward_search(&start_iter,textfind,GTK_TEXT_SEARCH_TEXT_ONLY,&match_start_iter,&match_end_iter,NULL) == TRUE)
        {
             gtk_text_buffer_select_range(buffer,&match_start_iter,&match_end_iter);
             offset = gtk_text_iter_get_offset(&match_end_iter);
             gtk_text_view_scroll_to_iter(GTK_TEXT_VIEW(codewidget[gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook))].txtinput),&match_end_iter,0.1,FALSE,0.5,0.5);
             }
        }
    }

void removetab_activated(GtkWidget *widget)
{
    int i;
    if (gtk_notebook_get_n_pages(GTK_NOTEBOOK(notebook)) !=0)
    {
        for (i = gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook)); i<MAX_TABS;i++)
        {
            codewidget[i].txtinput = codewidget[i+1].txtinput;
            vbox_txtinput[i] = vbox_txtinput[i+1];
            }
        GtkSourceBuffer *buffer;
        buffer = GTK_SOURCE_BUFFER(gtk_source_buffer_new(NULL));
        codewidget[MAX_TABS-1].txtinput = gtk_source_view_new_with_buffer(buffer);
        vbox_txtinput[MAX_TABS-1] = gtk_vbox_new(FALSE,2);
        gtk_notebook_remove_page(GTK_NOTEBOOK(notebook),gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook)));
        }
    }

void removealltab_activated(GtkWidget *widget)
{
    int i;
    for (i=gtk_notebook_get_n_pages(GTK_NOTEBOOK(notebook))-1;i>=0;i--)
    {
        gtk_notebook_remove_page(GTK_NOTEBOOK(notebook),i);
        }
    
    for (i=0;i<MAX_TABS;i++)
    {
        GtkSourceBuffer *buffer;
        buffer = GTK_SOURCE_BUFFER(gtk_source_buffer_new(NULL));
        codewidget[i].txtinput = gtk_source_view_new_with_buffer(buffer);    
        vbox_txtinput[i] = gtk_vbox_new(FALSE,2);
        filepathsarray[i] = "";
        }        
    }

void dedentregion_activated(GtkWidget *widget)
{
    GtkTextBuffer *buffer;
    int i,indent_width=gtk_source_view_get_indent_width(GTK_SOURCE_VIEW(codewidget[gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook))].txtinput));
    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(codewidget[gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook))].txtinput));
    GtkTextIter selection_start_iter,selection_end_iter;
    gtk_text_buffer_get_selection_bounds(buffer,&selection_start_iter,&selection_end_iter);
    int selection_start_iter_line = gtk_text_iter_get_line(&selection_start_iter),selection_end_iter_line = gtk_text_iter_get_line(&selection_end_iter);
    int selection_start_iter_line_offset = gtk_text_iter_get_line_offset(&selection_start_iter);
    if (selection_start_iter_line_offset>=indent_width)
    {
        GtkTextIter new_iter;
        gtk_text_buffer_get_iter_at_line_offset(buffer,&new_iter,selection_start_iter_line,selection_start_iter_line_offset-indent_width);
        gtk_text_buffer_delete(buffer,&selection_start_iter,&new_iter);
        }
    else
    {
        GtkTextIter new_iter;
        gtk_text_buffer_get_iter_at_line_offset(buffer,&new_iter,selection_start_iter_line,selection_start_iter_line_offset+indent_width);
        gtk_text_buffer_delete(buffer,&selection_start_iter,&new_iter);
        }
    for (i=selection_start_iter_line+1;i<=selection_end_iter_line;i++)
    {
        GtkTextIter new_iter,line_iter;
        gtk_text_buffer_get_iter_at_line_offset(buffer,&new_iter,i,indent_width);
        gtk_text_buffer_get_iter_at_line_offset(buffer,&line_iter,i,0);
        gtk_text_buffer_delete(buffer,&line_iter,&new_iter);
        }
    }

void indentregion_activated(GtkWidget *widget)
{
    GtkTextBuffer *buffer;
    int i,indent_width=gtk_source_view_get_indent_width(GTK_SOURCE_VIEW(codewidget[gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook))].txtinput));
    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(codewidget[gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook))].txtinput));
    GtkTextIter selection_start_iter,selection_end_iter;
    gtk_text_buffer_get_selection_bounds(buffer,&selection_start_iter,&selection_end_iter);
    int selection_start_iter_line = gtk_text_iter_get_line(&selection_start_iter),selection_end_iter_line = gtk_text_iter_get_line(&selection_end_iter);
    int selection_start_iter_offset = gtk_text_iter_get_offset(&selection_start_iter);
    char indent_string[indent_width+1];
    for (i=0;i<indent_width;i++)
    {
        indent_string[i] = ' ';
        }
    indent_string[indent_width] = '\0';
    gtk_text_buffer_insert(buffer,&selection_start_iter,indent_string,-1);
    GtkTextIter line_iter;
    for (i=selection_start_iter_line+1;i<=selection_end_iter_line;i++)
    {
        gtk_text_buffer_get_iter_at_line(buffer,&line_iter,i);
        gtk_text_buffer_insert(buffer,&line_iter,indent_string,-1);
        }
    }

void selectall_activated(GtkWidget *widget)
{
    GtkTextBuffer *buffer;
    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(codewidget[gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook))].txtinput));
    GtkTextIter start_iter,end_iter;
    gtk_text_buffer_get_start_iter(buffer,&start_iter);
    gtk_text_buffer_get_end_iter(buffer,&end_iter);
    gtk_text_buffer_select_range(buffer,&start_iter,&end_iter);
    }
    
void newtab_activated(GtkWidget *widget)
{
    int pages  = gtk_notebook_get_n_pages(GTK_NOTEBOOK(notebook));
    
    codewidget_new(&codewidget[pages]);    
    vbox_txtinput[pages] = gtk_vbox_new(FALSE,2);   
    //codewidget[pages].class_combo_box = gtk_combo_box_text_new();
    scrollwin[pages] = gtk_scrolled_window_new(NULL,NULL);    
    
    gtk_container_add(GTK_CONTAINER(codewidget[pages].txtinput),codewidget[pages].fixed_txtinput);
    gtk_container_add(GTK_CONTAINER(scrollwin[pages]),codewidget[pages].txtinput);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrollwin[pages]),GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC);
    GtkWidget *hbox_combo;
    hbox_combo = gtk_hbox_new(FALSE,2);
    gtk_box_pack_start(GTK_BOX(hbox_combo),codewidget[pages].class_combo_box,FALSE,FALSE,0);    
    gtk_box_pack_start(GTK_BOX(hbox_combo),codewidget[pages].func_combo_box,FALSE,FALSE,0);
    g_signal_connect(G_OBJECT(codewidget[pages].class_combo_box),"changed",G_CALLBACK(toolbar_combo_class_changed),NULL);    
    g_signal_connect(G_OBJECT(codewidget[pages].func_combo_box),"changed",G_CALLBACK(toolbar_combo_func_changed),NULL);
    gtk_box_pack_start(GTK_BOX(vbox_txtinput[pages]),hbox_combo,FALSE,FALSE,0);
    gtk_box_pack_start(GTK_BOX(vbox_txtinput[pages]),scrollwin[pages],TRUE,TRUE,0);
    GtkWidget *label1;
    label1 = gtk_label_new("New File");
    
    if (pages == 0)
    {
        char *settings;
        gsize length = -1;
            
        indentation = g_string_new("");
        indent_width = g_string_new("");   
        show_line_numbers = g_string_new("");
        highlight_current_line = g_string_new("");    
            
        if (g_file_get_contents("./settings.ini",&settings,&length,NULL) == TRUE)
        {
            int i;
            int opening_index=0,closing_index=0,value_index=0;
            opening_index = find_string(settings,"<indentation>") + strlen("<indentation>");
            closing_index = find_string(settings,"</indentation>");
            for(i=opening_index;i<closing_index;i++)
            {
                g_string_append_c(indentation,settings[i]);
                }
            if (strcmp(indentation->str,"ENABLED") == 0)
            {
                opening_index = find_string(settings,"<indent width>") + strlen("<indent width>");
                closing_index = find_string(settings,"</indent width>");
                for(i=opening_index;i<closing_index;i++)
                {
                    g_string_append_c(indent_width,settings[i]);
                    }
                }      
                 
            opening_index = find_string(settings,"<show line numbers>") + strlen("<show line numbers>");
            closing_index = find_string(settings,"</show line numbers>");
            for(i=opening_index;i<closing_index;i++)
            {
                g_string_append_c(show_line_numbers,settings[i]);
                }
            
            opening_index = find_string(settings,"<highlight current line>") + strlen("<highlight current line>");
            closing_index = find_string(settings,"</highlight current line>");
            for(i=opening_index;i<closing_index;i++)
            {
                g_string_append_c(highlight_current_line,settings[i]);
                }             
            }        
        GtkSourceView *sourceview;    
        sourceview = GTK_SOURCE_VIEW(codewidget[0].txtinput);
        
        if (strcmp(indentation->str,"ENABLED") == 0)
        {
            gtk_source_view_set_auto_indent(sourceview,TRUE);
            gtk_source_view_set_indent_width(sourceview,atoi(indent_width->str));
            }
        else
            gtk_source_view_set_auto_indent(sourceview,FALSE);            
        
        if (strcmp(show_line_numbers->str,"ENABLED") == 0)
            gtk_source_view_set_show_line_numbers(sourceview,TRUE);
        else
            gtk_source_view_set_show_line_numbers(sourceview,FALSE);
        
        if (strcmp(highlight_current_line->str,"ENABLED") == 0)
            gtk_source_view_set_highlight_current_line(sourceview,TRUE);        
        }    
    else
    {    
        if (gtk_source_view_get_auto_indent(GTK_SOURCE_VIEW(codewidget[0].txtinput)) == TRUE)
        {
            gtk_source_view_set_auto_indent(GTK_SOURCE_VIEW(codewidget[pages].txtinput),TRUE);
            gtk_source_view_set_indent_width(GTK_SOURCE_VIEW(codewidget[pages].txtinput),gtk_source_view_get_indent_width(GTK_SOURCE_VIEW(codewidget[0].txtinput)));
            }
        else
            gtk_source_view_set_auto_indent(GTK_SOURCE_VIEW(codewidget[pages].txtinput),FALSE);
        
        if (gtk_source_view_get_show_line_numbers(GTK_SOURCE_VIEW(codewidget[0].txtinput)) == TRUE)
            gtk_source_view_set_show_line_numbers(GTK_SOURCE_VIEW(codewidget[pages].txtinput),TRUE);        
        else
            gtk_source_view_set_show_line_numbers(GTK_SOURCE_VIEW(codewidget[pages].txtinput),FALSE);
            
        if (gtk_source_view_get_highlight_current_line(GTK_SOURCE_VIEW(codewidget[0].txtinput)) == TRUE)
            gtk_source_view_set_highlight_current_line(GTK_SOURCE_VIEW(codewidget[pages].txtinput),TRUE);
        else
            gtk_source_view_set_highlight_current_line(GTK_SOURCE_VIEW(codewidget[pages].txtinput),FALSE);
        }
    /*g_signal_connect(G_OBJECT(txtinput[pages]),"key-press-event",G_CALLBACK(txtinput_key_press_event),NULL);
    g_signal_connect(G_OBJECT(txtinput[pages]),"button-release-event",G_CALLBACK(txtinput_button_release_event),NULL);*/   
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook),vbox_txtinput[pages],label1);     
    gtk_widget_show_all(notebook);    
    }
    
void undo_activated(GtkWidget *widget)
{
    GtkSourceBuffer *buffer;
    buffer = GTK_SOURCE_BUFFER(gtk_text_view_get_buffer(GTK_TEXT_VIEW(codewidget[gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook))].txtinput)));
    if (gtk_source_buffer_can_undo(buffer) == TRUE)
    {
        gtk_source_buffer_undo(buffer);
        }
    }

void redo_activated(GtkWidget *widget)
{
    GtkSourceBuffer *buffer;
    buffer = GTK_SOURCE_BUFFER(gtk_text_view_get_buffer(GTK_TEXT_VIEW(codewidget[gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook))].txtinput)));
    if (gtk_source_buffer_can_redo(buffer) == TRUE)
    {
        gtk_source_buffer_redo(buffer);
        }
    }

void copy_activated(GtkWidget *widget,GtkWidget *paste)
{
    GtkClipboard *clipboard;
    GtkTextBuffer *buffer;
    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(codewidget[gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook))].txtinput));
    clipboard = gtk_clipboard_get(GDK_NONE);
    gtk_text_buffer_copy_clipboard(buffer,clipboard);
    gtk_widget_set_sensitive(paste,TRUE);
    }

void cut_activated(GtkWidget *widget,GtkWidget *paste)
{
    GtkClipboard *clipboard;
    GtkTextBuffer *buffer;
    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(codewidget[gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook))].txtinput));
    clipboard = gtk_clipboard_get(GDK_NONE);
    gtk_text_buffer_cut_clipboard(buffer,clipboard,TRUE);
    gtk_widget_set_sensitive(paste,TRUE);
    }

void paste_activated(GtkWidget *widget)
{
    GtkClipboard *clipboard;
    GtkTextBuffer *buffer;
    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(codewidget[gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook))].txtinput));
    clipboard = gtk_clipboard_get(GDK_NONE);
    gtk_text_buffer_paste_clipboard(buffer,clipboard,NULL,TRUE);
    }

void update_recent_files_menu(const char *filepath)
{
    int i,j=0,k,present = 0,empty = 0;
    for (i = 0; i<5; i++)
    {
        if (GTK_IS_MENU_ITEM(file_path_menu_itemsarray[i])== TRUE)
        {
            if(strcmp(filepath,gtk_menu_item_get_label(GTK_MENU_ITEM(file_path_menu_itemsarray[i]))) == 0)
            {
                present = 1;
                break;
                }
            }
        }
   
    if (present == 0) /*filepath not present in file_path_menu_itemsarray*/
    {
        for (j = 0; j<5;j++)
        {
            if (GTK_IS_MENU_ITEM(file_path_menu_itemsarray[j])==FALSE)
            {
                empty = 1;
                break;
                }
            }
        
        if (empty == 1)
        {            
            file_path_menu_itemsarray[j] = gtk_menu_item_new_with_label(filepath);            
            gtk_menu_shell_prepend(GTK_MENU_SHELL(recentfilesmenu),file_path_menu_itemsarray[j]);
            g_signal_connect(G_OBJECT(file_path_menu_itemsarray[j]),"activate",G_CALLBACK(file_path_menu_item_activated),NULL);
            gtk_widget_show(file_path_menu_itemsarray[j]);  
            }
        else
        {
            for (k = 0;k<4;k++)
            {
                gtk_menu_item_set_label(GTK_MENU_ITEM(file_path_menu_itemsarray[k]),gtk_menu_item_get_label(GTK_MENU_ITEM(file_path_menu_itemsarray[k+1])));                
                }
            gtk_menu_item_set_label(GTK_MENU_ITEM(file_path_menu_itemsarray[4]),filepath);
            }
        }
    else
    {
        for (j = i;j<5;j++)
        {
            if (GTK_IS_MENU_ITEM(file_path_menu_itemsarray[j]) == FALSE)
                break;
            }
        
        for(k=i;k<j-1;k++)
        {
            gtk_menu_item_set_label(GTK_MENU_ITEM(file_path_menu_itemsarray[k]),gtk_menu_item_get_label(GTK_MENU_ITEM(file_path_menu_itemsarray[k+1])));
            }
        gtk_menu_item_set_label(GTK_MENU_ITEM(file_path_menu_itemsarray[j-1]),filepath);
        }
    GString *files = g_string_new("");
    for(i=0;i<5;i++)
    {
        if(GTK_IS_MENU_ITEM(file_path_menu_itemsarray[i])==TRUE)
        {
            g_string_append(files,"<file>");
            g_string_append(files,gtk_menu_item_get_label(GTK_MENU_ITEM(file_path_menu_itemsarray[i])));
            g_string_append(files,"</file>");
            }
        }
    g_file_set_contents("./recentfiles.ini",files->str,-1,NULL);
    g_string_free(files,TRUE);
    }                  

void addfile_new_activated(GtkWidget *widget, GtkWindow *parent)
{
    GtkWidget *dialog;
    GtkFileFilter *filefilter_py,*filefilter_txt,*filefilter_all;
    filefilter_py = gtk_file_filter_new();
    filefilter_txt = gtk_file_filter_new();
    filefilter_all = gtk_file_filter_new();
    gtk_file_filter_set_name(filefilter_py,"Python Files(*.py)");
    gtk_file_filter_add_pattern(filefilter_py,"*.py");
    gtk_file_filter_set_name(filefilter_txt,"Text Files(*.txt)");
    gtk_file_filter_add_pattern(filefilter_txt,"*.txt");
    gtk_file_filter_set_name(filefilter_all,"All Files(*.*)");
    gtk_file_filter_add_pattern(filefilter_all,"*.*");
    GtkSourceLanguageManager *languagemanager = gtk_source_language_manager_new();
    dialog = gtk_file_chooser_dialog_new("Save File", GTK_WINDOW(window), GTK_FILE_CHOOSER_ACTION_SAVE,GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,GTK_STOCK_SAVE,GTK_RESPONSE_ACCEPT,NULL);
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog),filefilter_py);
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog),filefilter_txt);
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog),filefilter_all);
    if(gtk_dialog_run(GTK_DIALOG(dialog))== GTK_RESPONSE_ACCEPT)
    {
        char *filepath,*filename;
        gsize length = -1;
        filepath = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        filename = strrchr(filepath,'/');
        int i;
        filename++; 
        if(is_filename_already_present(filename))
        {
            gtk_widget_destroy(dialog);
            GtkWidget *msg_dialog,*label,*hbox; 
            char *message;
            asprintf(&message,"File already present with this filename");
            msg_dialog  = gtk_dialog_new();
            label = gtk_label_new(message);
            gtk_box_pack_start_defaults(GTK_BOX(GTK_DIALOG(msg_dialog)->vbox),label);
            gtk_dialog_add_button(GTK_DIALOG(msg_dialog),"OK",0);
            gtk_widget_show_all(msg_dialog);
            gtk_dialog_run(GTK_DIALOG(msg_dialog));
            gtk_widget_destroy(msg_dialog);
            g_free(filepath);            
            return;           
            }
        
        filepathsarray[proj_filecount] = g_strconcat(filepath,NULL);
        /*GtkSourceBuffer *buffer1;
        buffer1 = GTK_SOURCE_BUFFER(gtk_source_buffer_new(NULL));
        codewidget[proj_filecount].txtinput = gtk_source_view_new_with_buffer(buffer1);    */
        newtab_activated(widget);        
        GtkTextBuffer *buffer;
        buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(codewidget[proj_filecount].txtinput));
        GtkSourceLanguage *language = gtk_source_language_manager_guess_language(languagemanager,filepath,NULL);
        gtk_source_buffer_set_language(GTK_SOURCE_BUFFER(buffer),language); 
        GtkWidget *label;        
        label = gtk_label_new(filename);
        gtk_notebook_set_tab_label(GTK_NOTEBOOK(notebook),vbox_txtinput[proj_filecount],label);
        gtk_tree_store_append(proj_treestore,&file_child_array[proj_filecount],&toplevel);
        gtk_tree_store_set(proj_treestore,&file_child_array[proj_filecount],0,filename,-1);
        proj_filecount++;
        
        GString *projstring;
        char *contents;
        g_file_get_contents(projpath,&contents,&length,NULL);
        projstring = g_string_new(contents);
        g_string_append(projstring,"<file>");
        g_string_append(projstring,filepath);
        g_string_append(projstring,"</file>");
        g_file_set_contents(projpath,projstring->str,-1,NULL);
        gtk_widget_show_all(notebook);
        if(strcmp(recent_files->str,"ENABLED")==0)
            update_recent_files_menu(filepath);
        g_string_free(projstring,TRUE);
        g_free(contents);
        g_free(filepath);
        }
    gtk_widget_destroy(dialog);
    }

void addfile_existing_activated(GtkWidget *widget, GtkWindow *parent)
{
    GtkTextBuffer *buffer;
    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(codewidget[gtk_notebook_get_n_pages(GTK_NOTEBOOK(notebook))].txtinput));
    GtkWidget *dialog;
    GtkFileFilter *filefilter_py,*filefilter_txt,*filefilter_all;
    filefilter_py = gtk_file_filter_new();
    filefilter_txt = gtk_file_filter_new();
    filefilter_all = gtk_file_filter_new();
    gtk_file_filter_set_name(filefilter_py,"Python Files(*.py)");
    gtk_file_filter_add_pattern(filefilter_py,"*.py");
    gtk_file_filter_set_name(filefilter_txt,"Text Files(*.txt)");
    gtk_file_filter_add_pattern(filefilter_txt,"*.txt");
    gtk_file_filter_set_name(filefilter_all,"All Files(*.*)");
    gtk_file_filter_add_pattern(filefilter_all,"*.*");
    GtkSourceLanguageManager *languagemanager = gtk_source_language_manager_new();
    dialog = gtk_file_chooser_dialog_new("Open File", GTK_WINDOW(window), GTK_FILE_CHOOSER_ACTION_OPEN,GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,GTK_STOCK_OPEN,GTK_RESPONSE_ACCEPT,NULL);
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog),filefilter_py);
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog),filefilter_txt);
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog),filefilter_all);
    
    if(gtk_dialog_run(GTK_DIALOG(dialog))== GTK_RESPONSE_ACCEPT)
    {
        char *filepath,*string,*filename;
        gsize length = -1;
        filepath = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        if(is_file_already_present(filepath))
        {
            gtk_widget_destroy(dialog);
            GtkWidget *msg_dialog,*label,*hbox; 
            char *message;
            asprintf(&message,"File already present");
            msg_dialog  = gtk_dialog_new();
            label = gtk_label_new(message);
            gtk_box_pack_start_defaults(GTK_BOX(GTK_DIALOG(msg_dialog)->vbox),label);
            gtk_dialog_add_button(GTK_DIALOG(msg_dialog),"OK",0);
            gtk_widget_show_all(msg_dialog);
            gtk_dialog_run(GTK_DIALOG(msg_dialog));
            gtk_widget_destroy(msg_dialog);
            g_free(filepath);            
            return;           
            }
        filepathsarray[gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook))] = g_strconcat(filepath,NULL);
        GtkSourceBuffer *buffer1;
        buffer1 = GTK_SOURCE_BUFFER(gtk_source_buffer_new(NULL));
        codewidget[proj_filecount].txtinput = gtk_source_view_new_with_buffer(buffer1);    
        GtkTextBuffer *buffer;
        buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(codewidget[gtk_notebook_get_n_pages(GTK_NOTEBOOK(notebook))].txtinput));
        gtk_text_buffer_set_text(buffer,string,-1);         
        newtab_activated(widget);
        GtkWidget *label;
        g_file_get_contents(filepath,&string,&length,NULL); 
        GtkSourceLanguage *language = gtk_source_language_manager_guess_language(languagemanager,filepath,NULL);
        gtk_source_buffer_set_language(GTK_SOURCE_BUFFER(buffer),language); 
        gtk_text_buffer_set_text(buffer,string,-1);
        filename = strrchr(filepath,'/');
        int i;
        filename++; 
        label = gtk_label_new(filename);
        gtk_notebook_set_tab_label(GTK_NOTEBOOK(notebook),vbox_txtinput[proj_filecount],label);
        gtk_tree_store_append(proj_treestore,&file_child_array[proj_filecount],&toplevel);
        gtk_tree_store_set(proj_treestore,&file_child_array[proj_filecount],0,filename,-1);
        proj_filecount++;
        GString *projstring;
        char *contents;
        g_file_get_contents(projpath,&contents,&length,NULL);
        projstring = g_string_new(contents);
        g_string_append(projstring,"<file>");
        g_string_append(projstring,filepath);
        g_string_append(projstring,"</file>");
        g_file_set_contents(projpath,projstring->str,-1,NULL);
        gtk_widget_show_all(notebook);
        if(strcmp(recent_files->str,"ENABLED")==0)
            update_recent_files_menu(filepath);
        g_string_free(projstring,TRUE);
        g_free(contents);
        g_free(filepath);
        g_free(string);
        }
    gtk_widget_destroy(dialog);
    }

void addfile_main_new_activated(GtkWidget *widget, GtkWindow *parent)
{
    GtkWidget *dialog;
    if(mainfile->str || mainfile->len)
    {
        //Already contains a main file        
        GtkWidget *label,*hbox; 
        char *message;
        asprintf(&message,"Project already contains a main file");
        dialog=gtk_dialog_new();
        label = gtk_label_new(message);
        gtk_box_pack_start_defaults(GTK_BOX(GTK_DIALOG(dialog)->vbox),label);
        gtk_dialog_add_button(GTK_DIALOG(dialog),"OK",0);
        gtk_widget_show_all(dialog);
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);        
        return;
        }
    GtkFileFilter *filefilter_py,*filefilter_txt,*filefilter_all;
    filefilter_py = gtk_file_filter_new();
    filefilter_txt = gtk_file_filter_new();
    filefilter_all = gtk_file_filter_new();
    gtk_file_filter_set_name(filefilter_py,"Python Files(*.py)");
    gtk_file_filter_add_pattern(filefilter_py,"*.py");
    gtk_file_filter_set_name(filefilter_txt,"Text Files(*.txt)");
    gtk_file_filter_add_pattern(filefilter_txt,"*.txt");
    gtk_file_filter_set_name(filefilter_all,"All Files(*.*)");
    gtk_file_filter_add_pattern(filefilter_all,"*.*");
    GtkSourceLanguageManager *languagemanager = gtk_source_language_manager_new();
    dialog = gtk_file_chooser_dialog_new("Save File", GTK_WINDOW(window), GTK_FILE_CHOOSER_ACTION_SAVE,GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,GTK_STOCK_SAVE,GTK_RESPONSE_ACCEPT,NULL);
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog),filefilter_py);
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog),filefilter_txt);
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog),filefilter_all);
    
    if(gtk_dialog_run(GTK_DIALOG(dialog))== GTK_RESPONSE_ACCEPT)
    {
        char *filepath,*filename;
        gsize length = -1;
        filepath = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        filename = strrchr(filepath,'/');
        int i;
        filename++; 
        if(is_filename_already_present(filename))
        {
            gtk_widget_destroy(dialog);
            GtkWidget *msg_dialog,*label,*hbox; 
            char *message;
            asprintf(&message,"File already present with this filename");
            msg_dialog  = gtk_dialog_new();
            label = gtk_label_new(message);
            gtk_box_pack_start_defaults(GTK_BOX(GTK_DIALOG(msg_dialog)->vbox),label);
            gtk_dialog_add_button(GTK_DIALOG(msg_dialog),"OK",0);
            gtk_widget_show_all(msg_dialog);
            gtk_dialog_run(GTK_DIALOG(msg_dialog));
            gtk_widget_destroy(msg_dialog);
            g_free(filepath);            
            return;           
            }
        filepathsarray[proj_filecount] = g_strconcat(filepath,NULL);        
        mainfile = g_string_new(filepath);        
        
        gtk_tree_store_append(proj_treestore,&file_child_array[proj_filecount],&toplevel);
        gtk_tree_store_set(proj_treestore,&file_child_array[proj_filecount],0,filename,-1);
        
        newtab_activated(widget);
        GtkTextBuffer *buffer;
        buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(codewidget[proj_filecount].txtinput));
        GtkSourceLanguage *language = gtk_source_language_manager_guess_language(languagemanager,filepath,NULL);
        gtk_source_buffer_set_language(GTK_SOURCE_BUFFER(buffer),language);           
        
        GtkWidget *label;
        label = gtk_label_new(filename);        
        gtk_notebook_set_tab_label(GTK_NOTEBOOK(notebook),vbox_txtinput[proj_filecount],label);
        proj_filecount++;    
        
        GString *projstring;
        char *contents;
        g_file_get_contents(projpath,&contents,&length,NULL);
        projstring = g_string_new(contents);
        g_string_append(projstring,"<mainfile>");
        g_string_append(projstring,filepath);
        g_string_append(projstring,"</mainfile>");
        g_file_set_contents(projpath,projstring->str,-1,NULL);
        gtk_widget_show_all(notebook);
        if(strcmp(recent_files->str,"ENABLED")==0)
            update_recent_files_menu(filepath);
        g_string_free(projstring,TRUE);
        g_free(contents);
        g_free(filepath);
        gtk_widget_set_state(addfile_main,GTK_STATE_INSENSITIVE);
        }
    gtk_widget_destroy(dialog);
    }

void addfile_main_existing_activated(GtkWidget *widget, GtkWindow *parent)
{
    GtkWidget *dialog;
    if(mainfile->str || mainfile->len)
    {
        //Already contains a main file        
        GtkWidget *label,*hbox; 
        char *message;
        asprintf(&message,"Project already contains a main file");
        dialog=gtk_dialog_new();
        label = gtk_label_new(message);
        gtk_box_pack_start_defaults(GTK_BOX(GTK_DIALOG(dialog)->vbox),label);
        gtk_dialog_add_button(GTK_DIALOG(dialog),"OK",0);
        gtk_widget_show_all(dialog);
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);       
        return;
        }
    GtkFileFilter *filefilter_py,*filefilter_txt,*filefilter_all;
    filefilter_py = gtk_file_filter_new();
    filefilter_txt = gtk_file_filter_new();
    filefilter_all = gtk_file_filter_new();
    gtk_file_filter_set_name(filefilter_py,"Python Files(*.py)");
    gtk_file_filter_add_pattern(filefilter_py,"*.py");
    gtk_file_filter_set_name(filefilter_txt,"Text Files(*.txt)");
    gtk_file_filter_add_pattern(filefilter_txt,"*.txt");
    gtk_file_filter_set_name(filefilter_all,"All Files(*.*)");
    gtk_file_filter_add_pattern(filefilter_all,"*.*");
    GtkSourceLanguageManager *languagemanager = gtk_source_language_manager_new();
    dialog = gtk_file_chooser_dialog_new("Open File", GTK_WINDOW(window), GTK_FILE_CHOOSER_ACTION_OPEN,GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,GTK_STOCK_OPEN,GTK_RESPONSE_ACCEPT,NULL);
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog),filefilter_py);
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog),filefilter_txt);
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog),filefilter_all);
    
    if(gtk_dialog_run(GTK_DIALOG(dialog))== GTK_RESPONSE_ACCEPT)
    {
        char *filepath,*string,*filename;
        gsize length = -1;
        filepath = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        if(is_file_already_present(filepath))
        {
            gtk_widget_destroy(dialog);
            GtkWidget *msg_dialog,*label,*hbox; 
            char *message;
            asprintf(&message,"File already present");
            msg_dialog  = gtk_dialog_new();
            label = gtk_label_new(message);
            gtk_box_pack_start_defaults(GTK_BOX(GTK_DIALOG(msg_dialog)->vbox),label);
            gtk_dialog_add_button(GTK_DIALOG(msg_dialog),"OK",0);
            gtk_widget_show_all(msg_dialog);
            gtk_dialog_run(GTK_DIALOG(msg_dialog));
            gtk_widget_destroy(msg_dialog);
            g_free(filepath);            
            return;           
            }
        char *text;
        filepathsarray[proj_filecount] = g_strconcat(filepath,NULL);
        mainfile = g_string_new(filepath);
        g_file_get_contents(filepath,&string,&length,NULL); 
        if (proj_filecount+1 <= gtk_notebook_get_n_pages(GTK_NOTEBOOK(notebook)))
            gtk_notebook_remove_page(GTK_NOTEBOOK(notebook),proj_filecount);
        GtkSourceBuffer *buffer1;
        buffer1 = GTK_SOURCE_BUFFER(gtk_source_buffer_new(NULL));
        codewidget[proj_filecount].txtinput = gtk_source_view_new_with_buffer(buffer1);    
        GtkTextBuffer *buffer;
        buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(codewidget[gtk_notebook_get_n_pages(GTK_NOTEBOOK(notebook))].txtinput));
        GtkSourceLanguage *language = gtk_source_language_manager_guess_language(languagemanager,filepath,NULL);
        gtk_source_buffer_set_language(GTK_SOURCE_BUFFER(buffer),language); 
        gtk_text_buffer_set_text(buffer,string,-1);        
        newtab_activated(widget);
        filename = strrchr(filepath,'/');
        int i;
        filename++; 
        GtkWidget *label;
        label = gtk_label_new(filename);
        gtk_notebook_set_tab_label(GTK_NOTEBOOK(notebook),vbox_txtinput[proj_filecount],label);
        gtk_tree_store_append(proj_treestore,&file_child_array[proj_filecount],&toplevel);
        gtk_tree_store_set(proj_treestore,&file_child_array[proj_filecount],0,filename,-1);
        proj_filecount++;        
        GString *projstring;
        char *contents;
        g_file_get_contents(projpath,&contents,&length,NULL);
        projstring = g_string_new(contents);
        g_string_append(projstring,"<mainfile>");
        g_string_append(projstring,filepath);
        g_string_append(projstring,"</mainfile>");
        g_file_set_contents(projpath,projstring->str,-1,NULL);
        gtk_widget_show_all(notebook);
        if(strcmp(recent_files->str,"ENABLED")==0)
            update_recent_files_menu(filepath);
        g_string_free(projstring,TRUE);
        g_free(contents);
        g_free(filepath);
        g_free(string);
        gtk_widget_set_state(addfile_main,GTK_STATE_INSENSITIVE);
        }
    gtk_widget_destroy(dialog);
    }

void file_path_menu_item_activated(GtkWidget *widget)
{
    GtkTextBuffer *buffer;    
    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(codewidget[gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook))].txtinput));
    GtkSourceLanguageManager *languagemanager = gtk_source_language_manager_new();
    char *string,*filename;
    gsize length = -1;
    const char *filepath1 = gtk_menu_item_get_label(GTK_MENU_ITEM(widget));
    filepath1 = gtk_menu_item_get_label(GTK_MENU_ITEM(widget));
    
    filepathsarray[gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook))] = g_strconcat(filepath1,NULL);
    g_file_get_contents(filepath1,&string,&length,NULL); 
    GtkSourceLanguage *language = gtk_source_language_manager_guess_language(languagemanager,filepath1,NULL);
    filename = strrchr(filepath1,'/');
    *filename++;
    gtk_notebook_set_tab_label_text(GTK_NOTEBOOK(notebook),vbox_txtinput[gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook))],filename);
    gtk_source_buffer_set_language(GTK_SOURCE_BUFFER(buffer),language);       
    gtk_text_buffer_set_text(buffer,string,-1);
    
    if(strcmp(recent_files->str,"ENABLED")==0)
    {
        update_recent_files_menu(filepath1);
        printf("update_recent_files filepath %s\n",filepath1);
        }
    g_free(string);
    }

void saveasall_activated(GtkWidget *widget,GtkWindow *parent)
{
    int i,j;
    j = gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook));
    for (i=0;i<gtk_notebook_get_n_pages(GTK_NOTEBOOK(notebook));i++)
    {
        gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook),i);
        saveas_activated(widget,parent);
       }
    gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook),j);
    }

void saveall_activated(GtkWidget *widget, GtkWindow *parent)
{
    int i,j;
    j = gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook));
    for (i=0;i<gtk_notebook_get_n_pages(GTK_NOTEBOOK(notebook));i++)
    {
        gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook),i);
        save_activated(widget,parent);
        }
    gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook),j);
    }

void savecopyas_activated(GtkWidget *widget, GtkWindow *parent)
{
    GtkWidget *dialog;
    GtkTextIter start_iter, end_iter;
    GtkTextBuffer *buffer;
    GtkSourceLanguageManager *languagemanager = gtk_source_language_manager_new();
    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(codewidget[gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook))].txtinput));
    GtkFileFilter *filefilter_py,*filefilter_txt,*filefilter_all;
    filefilter_py = gtk_file_filter_new();
    filefilter_txt = gtk_file_filter_new();
    filefilter_all = gtk_file_filter_new();
    gtk_file_filter_set_name(filefilter_py,"Python Files(*.py)");
    gtk_file_filter_add_pattern(filefilter_py,"*.py");
    gtk_file_filter_set_name(filefilter_txt,"Text Files(*.txt)");
    gtk_file_filter_add_pattern(filefilter_txt,"*.txt");
    gtk_file_filter_set_name(filefilter_all,"All Files(*.*)");
    gtk_file_filter_add_pattern(filefilter_all,"*.*");
    dialog = gtk_file_chooser_dialog_new("Save Copy As", parent, GTK_FILE_CHOOSER_ACTION_SAVE, GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,GTK_STOCK_SAVE,GTK_RESPONSE_ACCEPT,NULL);
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog),filefilter_py);
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog),filefilter_txt);
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog),filefilter_all);
    
    if (gtk_dialog_run(GTK_DIALOG(dialog))== GTK_RESPONSE_ACCEPT)
    {
        char *filepath,*text;
        char *filename;
        gssize length = -1;
        filepath = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        gtk_text_buffer_get_end_iter(buffer,&end_iter);
        gtk_text_buffer_get_start_iter(buffer,&start_iter);
        text = gtk_text_buffer_get_text(buffer,&start_iter,&end_iter,FALSE);
        g_file_set_contents(filepath,text,length,NULL);
        g_free(filepath);
        g_free(text);
        }
    gtk_widget_destroy(dialog);
    }

void saveas_activated(GtkWidget *widget, GtkWindow *parent)
{
    GtkWidget *dialog;
    GtkTextIter start_iter, end_iter;
    GtkTextBuffer *buffer;
    GtkSourceLanguageManager *languagemanager = gtk_source_language_manager_new();
    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(codewidget[gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook))].txtinput));
    GtkFileFilter *filefilter_py,*filefilter_txt,*filefilter_all;
    filefilter_py = gtk_file_filter_new();
    filefilter_txt = gtk_file_filter_new();
    filefilter_all = gtk_file_filter_new();
    gtk_file_filter_set_name(filefilter_py,"Python Files(*.py)");
    gtk_file_filter_add_pattern(filefilter_py,"*.py");
    gtk_file_filter_set_name(filefilter_txt,"Text Files(*.txt)");
    gtk_file_filter_add_pattern(filefilter_txt,"*.txt");
    gtk_file_filter_set_name(filefilter_all,"All Files(*.*)");
    gtk_file_filter_add_pattern(filefilter_all,"*.*");
    dialog = gtk_file_chooser_dialog_new("Save File", parent, GTK_FILE_CHOOSER_ACTION_SAVE, GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,GTK_STOCK_SAVE,GTK_RESPONSE_ACCEPT,NULL);
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog),filefilter_py);
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog),filefilter_txt);
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog),filefilter_all);
    
    if (gtk_dialog_run(GTK_DIALOG(dialog))== GTK_RESPONSE_ACCEPT)
    {
        char *filepath,*text;
        char *filename;
        gssize length = -1;
        filepath = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        filepathsarray[gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook))] = g_strconcat(filepath,NULL);
        gtk_text_buffer_get_end_iter(buffer,&end_iter);
        gtk_text_buffer_get_start_iter(buffer,&start_iter);
        text = gtk_text_buffer_get_text(buffer,&start_iter,&end_iter,FALSE);
        g_file_set_contents(filepath,text,length,NULL);
        GtkSourceLanguage *language = gtk_source_language_manager_guess_language(languagemanager,filepath,NULL);
         filename = strrchr(filepath,'/');
        *filename++;
        gtk_notebook_set_tab_label_text(GTK_NOTEBOOK(notebook),vbox_txtinput[gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook))],filename);
        gtk_source_buffer_set_language(GTK_SOURCE_BUFFER(buffer),language);   
        if(strcmp(recent_files->str,"ENABLED")==0)
            update_recent_files_menu(filepath);    
        g_free(filepath);
        g_free(text);
        }
    gtk_widget_destroy(dialog);
    }

gboolean function_filechanged(char *filename1,char *buffertext)
{
    if (strcmp(filename1,"") !=0)
    {
        char *filecontents;
        gssize length = -1;
        g_file_get_contents(filename1,&filecontents,&length,NULL);
        if (filecontents !=NULL)
        {
            if (strcmp(buffertext,filecontents) == 0)
                return FALSE;
            else
                return TRUE;
            }
        }
    else
    {
        if(strcmp(buffertext,"") != 0)
            return TRUE;
        else
            return FALSE;
        }
    }

void restore_activated(GtkWidget *widget)
{
    GString *backup_filepath;
    backup_filepath = g_string_new("/media/sda6/C/GTK+/gIDLE/backup/");
    g_string_append(backup_filepath,gtk_notebook_get_tab_label_text(GTK_NOTEBOOK(notebook),vbox_txtinput[gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook))]));
    char *text;
    gsize length = -1;
    if (g_file_get_contents(backup_filepath->str,&text,&length,NULL)==TRUE);
        gtk_text_buffer_set_text(gtk_text_view_get_buffer(GTK_TEXT_VIEW(codewidget[gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook))].txtinput)),text,-1);
    }

gboolean auto_save_func()
{
    if (strcmp("",filepathsarray[gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook))])!=0)
    {
        gssize length = -1;
        GtkTextBuffer *buffer;      
        buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(codewidget[gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook))].txtinput));
        GtkTextIter start_iter, end_iter;
        gtk_text_buffer_get_start_iter(buffer, &start_iter);
        gtk_text_buffer_get_end_iter(buffer, &end_iter);
        char *text;
        text = gtk_text_buffer_get_text(buffer,&start_iter,&end_iter,FALSE);
        g_file_set_contents(filepathsarray[gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook))],text,length,NULL);
        }
    return TRUE;
    }
void save_activated(GtkWidget *widget, GtkWindow *parent)
{     
    if (strcmp(filepathsarray[gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook))],"")==0)
    {
        saveas_activated(widget,parent);
        }
    else
    {
        gssize length = -1;
        GtkTextBuffer *buffer;
        GtkTextIter start_iter, end_iter;
        buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(codewidget[gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook))].txtinput));
        gtk_text_buffer_get_start_iter(buffer, &start_iter);
        gtk_text_buffer_get_end_iter(buffer, &end_iter);
        /*********************Code for creating backup before saving****************/
        char *previous_text,*filename;
        gsize l = -1;
        if(g_file_get_contents(filepathsarray[gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook))],&previous_text,&l,NULL)==TRUE)
        {
            filename = strrchr(filepathsarray[gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook))],'/');
            *filename++;
            GString *backup_filepath;
            backup_filepath = g_string_new("/media/sda6/C/GTK+/gIDLE/backup/");
            g_string_append(backup_filepath, filename);
            g_file_set_contents(backup_filepath->str,previous_text,length,NULL);
            }
        /*****************************************************************/
        char *text;
        text = gtk_text_buffer_get_text(buffer,&start_iter,&end_iter,FALSE);
        g_file_set_contents(filepathsarray[gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook))],text,length,NULL);
        }
    }

void function_open(GtkWidget *widget,GtkWidget *previous_dialog)
{
    if (GTK_IS_WIDGET(previous_dialog) == TRUE)
    {
        gtk_widget_destroy(previous_dialog);
        }
    GtkTextBuffer *buffer;
    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(codewidget[gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook))].txtinput));
    GtkTextIter start_iter, end_iter;
    gtk_text_buffer_get_start_iter(buffer, &start_iter);
    gtk_text_buffer_get_end_iter(buffer, &end_iter);
    GtkWidget *dialog;
    GtkFileFilter *filefilter_py,*filefilter_txt,*filefilter_all;
    filefilter_py = gtk_file_filter_new();
    filefilter_txt = gtk_file_filter_new();
    filefilter_all = gtk_file_filter_new();
    gtk_file_filter_set_name(filefilter_py,"Python Files(*.py)");
    gtk_file_filter_add_pattern(filefilter_py,"*.py");
    gtk_file_filter_set_name(filefilter_txt,"Text Files(*.txt)");
    gtk_file_filter_add_pattern(filefilter_txt,"*.txt");
    gtk_file_filter_set_name(filefilter_all,"All Files(*.*)");
    gtk_file_filter_add_pattern(filefilter_all,"*.*");
    GtkSourceLanguageManager *languagemanager = gtk_source_language_manager_new();
    dialog = gtk_file_chooser_dialog_new("Open File", GTK_WINDOW(window), GTK_FILE_CHOOSER_ACTION_OPEN,GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,GTK_STOCK_OPEN,GTK_RESPONSE_ACCEPT,NULL);
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog),filefilter_py);
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog),filefilter_txt);
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog),filefilter_all);
    
    if(gtk_dialog_run(GTK_DIALOG(dialog))== GTK_RESPONSE_ACCEPT)
    {
        char *filepath,*string,*filename;
        gsize length = -1;
        filepath = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        char *text;
        filepathsarray[gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook))] = g_strconcat(filepath,NULL);
        g_file_get_contents(filepath,&string,&length,NULL); 
        GtkSourceLanguage *language = gtk_source_language_manager_guess_language(languagemanager,filepath,NULL);
        gtk_source_buffer_set_language(GTK_SOURCE_BUFFER(buffer),language); 
        gtk_text_buffer_set_text(buffer,string,-1);
        /*********remember to remove it************/
        codewidget_get_class_and_members(&codewidget[gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook))]);
        codewidget_fill_combo_boxes(&codewidget[gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook))]);
        //func(&codewidget[gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook))]);
        /*******************/
        filename = strrchr(filepath,'/');        
        filename++;
        mode = "File";
        gtk_notebook_set_tab_label_text(GTK_NOTEBOOK(notebook),vbox_txtinput[gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook))],filename);
        if(strcmp(recent_files->str,"ENABLED")==0)        
            update_recent_files_menu(filepath);        
        
        g_signal_connect(G_OBJECT(buffer),"insert-text",G_CALLBACK(buffer_insert_text),NULL);
        g_signal_connect(G_OBJECT(buffer),"delete-range",G_CALLBACK(buffer_delete_range),NULL);
        g_free(filepath);
        g_free(string);
        }
    gtk_widget_destroy(dialog);
    }
    
void open_file_activated(GtkWidget *widget, GtkWindow *parent)
{
    GtkTextBuffer *buffer;
    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(codewidget[gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook))].txtinput));
    GtkTextIter start_iter, end_iter;
    gtk_text_buffer_get_start_iter(buffer, &start_iter);
    gtk_text_buffer_get_end_iter(buffer, &end_iter);
    if (function_filechanged(filepathsarray[gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook))],gtk_text_buffer_get_text(buffer,&start_iter,&end_iter,FALSE)) == FALSE)
    {
        codewidget_clear_arrays(&codewidget[gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook))]);
        //codewidget_destroy(&codewidget[gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook))]);
        //codewidget_new(&codewidget[gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook))]);
        //codewidget[gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook))].class_combo_box = gtk_combo_box_text_new();
        function_open(widget,NULL);
        }
    else
    {
        GtkWidget *dialog,*label,*cmdyes,*cmdno;
        
        dialog = gtk_dialog_new();
        label = gtk_label_new("Current file is not saved. Do you want to open a new file without saving current file?");
        cmdyes = gtk_button_new_with_mnemonic("_Yes");
        cmdno = gtk_button_new_with_mnemonic("_No");
        gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox),label);
        gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->action_area),cmdyes);
        gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->action_area),cmdno);
        g_signal_connect(G_OBJECT(cmdno),"clicked",G_CALLBACK(cmdclose_clicked),dialog);
        g_signal_connect(G_OBJECT(cmdyes),"clicked",G_CALLBACK(function_open),dialog);
                
        gtk_window_set_title(GTK_WINDOW(dialog),"gIDLE");
        gtk_widget_show_all(dialog);
        }                
    }

void open_proj_activated(GtkWidget *widget,GtkWindow *parent)
{    
    GtkWidget *dialog;
    GtkFileFilter *filefilter_pyproj,*filefilter_all;
    filefilter_pyproj = gtk_file_filter_new();
    filefilter_all = gtk_file_filter_new();
    gtk_file_filter_set_name(filefilter_pyproj,"Python Project Files(*.pyproj)");
    gtk_file_filter_add_pattern(filefilter_pyproj,"*.pyproj");
    gtk_file_filter_set_name(filefilter_all,"All Files(*.*)");
    gtk_file_filter_add_pattern(filefilter_all,"*.*");
    dialog = gtk_file_chooser_dialog_new("Open Project", GTK_WINDOW(window), GTK_FILE_CHOOSER_ACTION_OPEN,GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,GTK_STOCK_OPEN,GTK_RESPONSE_ACCEPT,NULL);
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog),filefilter_pyproj);
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog),filefilter_all);
    GtkSourceLanguageManager *languagemanager = gtk_source_language_manager_new();
    
    if(gtk_dialog_run(GTK_DIALOG(dialog))== GTK_RESPONSE_ACCEPT)
    {
        int i,total_pages;
        if(strcmp(mode,"Proj")==0&&proj_filecount>=0)
        {
            //Closing  current project
            for(i=0;i<proj_filecount;i++)
                if(filepathsarray[i])
                    free(filepathsarray[i]);
            gtk_tree_store_clear(proj_treestore);       
            gtk_tree_view_remove_column(GTK_TREE_VIEW(proj_tree_view),proj_col);
            if(GTK_IS_OBJECT(proj_col))
                gtk_object_destroy(GTK_OBJECT(proj_col));
            if(GTK_IS_OBJECT(proj_renderer))
                gtk_object_destroy(GTK_OBJECT(proj_renderer));
            }
        proj_filecount = 0;
        total_pages = gtk_notebook_get_n_pages(GTK_NOTEBOOK(notebook));
        for (i=total_pages-1; i>=0;i--)
        {
            gtk_widget_destroy(vbox_txtinput[i]);
            codewidget_destroy(&codewidget[i]);
            gtk_notebook_remove_page(GTK_NOTEBOOK(notebook),i);            
            }
        proj_treestore = gtk_tree_store_new(1,G_TYPE_STRING);
        gtk_tree_store_append(proj_treestore,&toplevel,NULL);
        
        gtk_tree_view_set_model(GTK_TREE_VIEW(proj_tree_view),GTK_TREE_MODEL(proj_treestore));
        proj_col = gtk_tree_view_column_new();
        proj_renderer = gtk_cell_renderer_text_new();
        gtk_tree_view_column_pack_start(proj_col,proj_renderer,TRUE);
        gtk_tree_view_column_add_attribute(proj_col,proj_renderer,"text",0);
        
        char *string;
        gsize length = -1;
        asprintf(&projpath,"%s",gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog)));
        gtk_widget_destroy(dialog);
        g_file_get_contents(projpath,&string,&length,NULL);
        
        mode = "Proj";
        int name_close = find_strstr(string,"</name>",0);
        proj_name = g_string_new("");        
        gtk_tree_view_append_column(GTK_TREE_VIEW(proj_tree_view),proj_col);
        gtk_tree_view_column_set_title(proj_col,"Project");
        for (i=6;i<name_close;i++)
        {
            g_string_append_c(proj_name,string[i]);
            }
        gtk_tree_store_set(proj_treestore,&toplevel,0,proj_name->str,-1);
        
        int mainfile_open = find_strstr(string,"<mainfile>",name_close), mainfile_close = find_strstr(string,"</mainfile>",mainfile_open);
        if (mainfile_open !=-1 && mainfile_close !=-1)
        {            
            mainfile = g_string_new("");
            for(i=mainfile_open+10;i<mainfile_close;i++)
            {
                g_string_append_c(mainfile,string[i]);
                }
            asprintf(&filepathsarray[0],"%s\0", mainfile->str);
            gtk_tree_store_append(proj_treestore,&file_child_array[0],&toplevel);
            char *filename;
            filename = strrchr(mainfile->str,'/');
            filename++;            
            gtk_tree_store_set(proj_treestore,&file_child_array[0],0,filename,-1);
            GtkTextBuffer *buffer;
            codewidget_new(&codewidget[0]);
            buffer= gtk_text_view_get_buffer(GTK_TEXT_VIEW(codewidget[0].txtinput));
            vbox_txtinput[0] = gtk_vbox_new(FALSE,2);
            scrollwin[0] = gtk_scrolled_window_new(NULL,NULL);
            newtab_activated(widget);
            GtkSourceLanguage *language = gtk_source_language_manager_guess_language(languagemanager,mainfile->str,NULL);
            gtk_source_buffer_set_language(GTK_SOURCE_BUFFER(buffer),language);
            char *contents;
            if(g_file_get_contents(mainfile->str,&contents,&length,NULL))
            {
                buffer= gtk_text_view_get_buffer(GTK_TEXT_VIEW(codewidget[0].txtinput));
                GtkSourceLanguage *language = gtk_source_language_manager_guess_language(languagemanager,mainfile->str,NULL);
                gtk_source_buffer_set_language(GTK_SOURCE_BUFFER(buffer),language);
                gtk_text_buffer_set_text(buffer,contents,-1);
                codewidget_get_class_and_members(&codewidget[0]);
                codewidget_fill_combo_boxes(&codewidget[0]);
                g_free(contents);
                }
            buffer= gtk_text_view_get_buffer(GTK_TEXT_VIEW(codewidget[0].txtinput));
            
            gtk_notebook_set_tab_label_text(GTK_NOTEBOOK(notebook),vbox_txtinput[0],filename);
            proj_filecount++;            
            }
        int file_open=0,file_close=mainfile_close;        
        while((file_open=find_strstr(string,"<file>",file_close+1))!=-1)
        {
            file_close = find_strstr(string,"</file>",file_open+1);
            GString *file;
            file = g_string_new("");
            for(i=file_open+6;i<file_close;i++)
            {
                g_string_append_c(file,string[i]);
                }
            if (strcmp("",file->str)!=0)
            {
                asprintf(&filepathsarray[proj_filecount],"%s\0", file->str);
                codewidget_new(&codewidget[proj_filecount]);
                vbox_txtinput[proj_filecount] = gtk_vbox_new(FALSE,2);
                scrollwin[proj_filecount] = gtk_scrolled_window_new(NULL,NULL);        
                newtab_activated(widget);
                GtkTextBuffer *buffer;
                buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(codewidget[proj_filecount].txtinput));
                GtkSourceLanguage *language = gtk_source_language_manager_guess_language(languagemanager,file->str,NULL);
                gtk_source_buffer_set_language(GTK_SOURCE_BUFFER(buffer),language);
                char *contents;
                if (g_file_get_contents(file->str,&contents,&length,NULL))
                {
                    gtk_text_buffer_set_text(buffer,contents,-1);
                    //g_signal_connect(G_OBJECT(buffer),"insert-text",G_CALLBACK(buffer_insert_text),NULL);
                    //g_signal_connect(G_OBJECT(buffer),"delete-range",G_CALLBACK(buffer_delete_range),NULL);
                    codewidget_get_class_and_members(&codewidget[proj_filecount]);
                    codewidget_fill_combo_boxes(&codewidget[proj_filecount]);
                    g_free(contents);
                    }                
                char *filename;
                filename = strrchr(file->str,'/');
                filename++;
                GtkWidget *label;
                label = gtk_label_new(filename);
                gtk_notebook_set_tab_label_text(GTK_NOTEBOOK(notebook),vbox_txtinput[proj_filecount],filename);
                gtk_tree_store_append(proj_treestore,&file_child_array[proj_filecount],&toplevel);
                gtk_tree_store_set(proj_treestore,&file_child_array[proj_filecount],0,filename,-1);
                proj_filecount++;
                g_string_free(file,TRUE);
                }
            }
        for(i=0;i<proj_filecount;i++)
        {
            GtkTextBuffer *buffer= gtk_text_view_get_buffer(GTK_TEXT_VIEW(codewidget[i].txtinput));
            //g_signal_connect(G_OBJECT(codewidget[i].txtinput),"key-press-event",G_CALLBACK(txtinput_key_press_event),NULL);
            //g_signal_connect(G_OBJECT(codewidget[i].txtinput),"button-release-event",G_CALLBACK(txtinput_button_release_event),NULL);
            //g_signal_connect(G_OBJECT(buffer),"mark-set",G_CALLBACK(txtinput_mark_set),NULL);
            g_signal_connect(G_OBJECT(buffer),"insert-text",G_CALLBACK(buffer_insert_text),NULL);    
            g_signal_connect(G_OBJECT(buffer),"delete-range",G_CALLBACK(buffer_delete_range),NULL);
            }
        int w,h;
        gtk_window_get_size(GTK_WINDOW(window),&w,&h);
        gtk_widget_set_size_request(proj_tree_view,200,h);
        g_free(string);
        }
    else
        gtk_widget_destroy(dialog);
    }

void filenew_activated(GtkWidget *widget)
{
    GtkTextBuffer *textbuffer;
    textbuffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(codewidget[gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook))].txtinput));
    GtkTextIter start_iter, end_iter;
    gtk_text_buffer_get_start_iter(textbuffer, &start_iter);
    gtk_text_buffer_get_end_iter(textbuffer, &end_iter);
    codewidget_clear_arrays(&codewidget[gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook))]);
    if (function_filechanged(filepathsarray[gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook))],gtk_text_buffer_get_text(textbuffer,&start_iter,&end_iter,FALSE)) == FALSE)
    {
        gtk_text_buffer_set_text(textbuffer,"",-1);
        filepathsarray[gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook))] = "";
        gtk_notebook_set_tab_label_text(GTK_NOTEBOOK(notebook),vbox_txtinput[gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook))],"New File");
        mode = "File";
        }
    else
    {
        GtkWidget *dialog,*label,*hbox;
        label = gtk_label_new("Current file is not saved. Do you want to create a new file without saving current file?");
        dialog = gtk_dialog_new_with_buttons("gIDLE", GTK_WINDOW(window), GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT,GTK_STOCK_YES,GTK_RESPONSE_YES,GTK_STOCK_NO,GTK_RESPONSE_NO,NULL);
        hbox = gtk_hbox_new(FALSE,5);
        gtk_box_pack_start_defaults(GTK_BOX(hbox),label);
        gtk_box_pack_start_defaults(GTK_BOX(GTK_DIALOG(dialog)->vbox),hbox);
        gtk_widget_show_all(dialog);
        int result = gtk_dialog_run(GTK_DIALOG(dialog));
  
        if (result == GTK_RESPONSE_YES)
        {
            gtk_text_buffer_set_text(textbuffer,"",-1);
            filepathsarray[gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook))] = "";
            gtk_notebook_set_tab_label_text(GTK_NOTEBOOK(notebook),vbox_txtinput[gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook))],"New File");
            mode = "File";
            gtk_widget_destroy(dialog);
            }
        if(result ==GTK_RESPONSE_NO)
        {
            gtk_widget_destroy(dialog);
            }
        }
    }
#endif