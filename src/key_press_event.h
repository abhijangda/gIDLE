/*749 Lines*/
#include<gtk/gtk.h>
#include<string.h>
#include<stdio.h>
#include<stdlib.h>
#define MAX_TABS  10
#include "codewidget.h"
#include"string_stack.h"

#ifndef __KEY_PRESS_EVENT_H__
#define __KEY_PRESS_EVENT_H__
//Add code so that tooltip text can display many functions

GtkWidget *exit_dialog, *cut, *copy,*undo,*redo,*openshell,*recentfiles;
GtkWidget *vbox_txtinput[MAX_TABS];
GtkWidget *scrollwin[MAX_TABS];
CodeWidget codewidget[MAX_TABS];
GtkWidget *notebook;
GtkWidget *statusbar;
int lines_track_array[MAX_TABS][5],prev_line = -1;
GtkTreeSelection *selection;
GRegex *regex_func;
GMatchInfo *match_info_func;
void toolbar_combo_class_changed(GtkComboBox *combo_box);
gboolean can_combo_func_change = TRUE;
gboolean show_tooltip = FALSE;
char *tooltip_text;
string_stack tooltip_text_stack;

void buffer_delete_range(GtkTextBuffer *buffer,GtkTextIter *start,GtkTextIter *end)
{
    //printf("BUFFER DELETE RANGE for page %d\n",gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook)));
    int len = gtk_text_iter_get_offset(start) - gtk_text_iter_get_offset(end);
    int class_index = gtk_combo_box_get_active(GTK_COMBO_BOX(codewidget[gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook))].class_combo_box));
    if (class_index == -1)
        return;
    int func_index = gtk_combo_box_get_active(GTK_COMBO_BOX(codewidget[gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook))].func_combo_box));
    
    int i=class_index-1;
    int j=func_index;
    
    char *regex_func_string;
    GString *word = g_string_new("");
    int k;
    gboolean current_function_exists=TRUE;
    if(func_index!=-1)
    {        
        for(k=0;k<strlen(codewidget[gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook))].pyclass_array[i+1].function_array[j].func_name);k++)
        {
            if (codewidget[gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook))].pyclass_array[i+1].function_array[j].func_name[k]=='(' || codewidget[gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook))].pyclass_array[i+1].function_array[j].func_name[k]==')' )
                g_string_append(word,"\\");
            g_string_append_c(word,codewidget[gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook))].pyclass_array[i+1].function_array[j].func_name[k]);
            }
        asprintf(&regex_func_string,"\\bdef\\b\\s*%s",word->str);
        g_string_free(word,TRUE);    
        regex_func = g_regex_new(regex_func_string,0,0,NULL);       
    
        GtkTextIter end_iter,start_iter;
        gtk_text_buffer_get_end_iter(buffer,&end_iter);
        gtk_text_buffer_get_start_iter(buffer,&start_iter);
        char *text = gtk_text_buffer_get_text(buffer,&start_iter,&end_iter,FALSE);
        if (g_regex_match(regex_func,text,0,&match_info_func) == FALSE)
        {        
            pyclass_remove_function(&codewidget[gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook))].pyclass_array[i+1],codewidget[gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook))].pyclass_array[i+1].function_array[j].func_name);
            current_function_exists=FALSE;
            }
        g_match_info_free(match_info_func);
        }
    i=class_index-1;
    j=func_index;
    while(++i<codewidget[gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook))].pyclass_count)
    {
        codewidget[gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook))].pyclass_array[i].pos+=len;
        while(++j<codewidget[gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook))].pyclass_array[i].function_array_size)
            codewidget[gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook))].pyclass_array[i].function_array[j].pos += len;
        }
    if(current_function_exists==FALSE)
        toolbar_combo_class_changed(GTK_COMBO_BOX(codewidget[gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook))].class_combo_box));  
        
    free(regex_func_string);  
    }

void buffer_insert_text(GtkTextBuffer *buffer, GtkTextIter *iter, gchar *text, gint len)
{    
    //printf("BUFFER INSERT TEXT for page %d\n",gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook)));
    if(GTK_IS_COMBO_BOX_TEXT(codewidget[gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook))].class_combo_box)==FALSE)
        return;
    int class_index = gtk_combo_box_get_active(GTK_COMBO_BOX(codewidget[gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook))].class_combo_box));
    if (class_index == -1)
        return;
    int func_index = gtk_combo_box_get_active(GTK_COMBO_BOX(codewidget[gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook))].func_combo_box));
    int i=class_index-1;
    int j=func_index;  
    
    while(++i<codewidget[gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook))].pyclass_count)
    {
        while(++j<codewidget[gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook))].pyclass_array[i].function_array_size)
            codewidget[gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook))].pyclass_array[i].function_array[j].pos += len;            
        }
    }

void toolbar_combo_class_changed(GtkComboBox *combo_box)
{
    int index = gtk_combo_box_get_active(combo_box),current_page=0;
    
    if(gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook))>0)
        current_page=gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook));
    //printf("cc%d\n",current_page);
    if (index == -1) 
        return;
    if (GTK_IS_COMBO_BOX(codewidget[current_page].func_combo_box))
    {
        //printf("CC%d\n",current_page);
        int i=0,count =0;        
        GtkTreeModel *model = gtk_combo_box_get_model(GTK_COMBO_BOX(codewidget[current_page].func_combo_box));
        GtkTreeIter iter;
        if(gtk_tree_model_get_iter_first(model,&iter)) 
        {
            count++;
            while(gtk_tree_model_iter_next(model,&iter)==TRUE)
                count++;
            for(i=count-1;i>=0;i--)
                gtk_combo_box_text_remove(GTK_COMBO_BOX_TEXT(codewidget[current_page].func_combo_box),i);
            }
        i=-1;
        while(++i<codewidget[current_page].pyclass_array[index].function_array_size) 
            gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(codewidget[current_page].func_combo_box),codewidget[current_page].pyclass_array[index].function_array[i].func_name);

        /*GtkTextBuffer *buffer; 
        buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(codewidget[current_page].txtinput));
        GtkTextIter current_iter;
        gtk_text_buffer_get_iter_at_offset(buffer,&current_iter,codewidget[current_page].class_pos_array[index][0]);
        gtk_text_buffer_place_cursor(buffer,&current_iter);
        gtk_text_view_scroll_to_iter(GTK_TEXT_VIEW(codewidget[current_page].txtinput),&current_iter,0.1,FALSE,0.5,0.5);*/
        gtk_combo_box_set_active(GTK_COMBO_BOX(codewidget[current_page].func_combo_box),0);
        }
    }

void toolbar_combo_func_changed(GtkComboBox *combo_box)
{ 
    //printf("CAN COMBO FUNC CHANGE %d\n",can_combo_func_change);
    if (can_combo_func_change==FALSE)
        return;
           
    int index_func = gtk_combo_box_get_active(combo_box);
    int current_page = 0;
    if(gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook))>0)
        current_page=gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook));
    int index_class = gtk_combo_box_get_active(GTK_COMBO_BOX(codewidget[current_page].class_combo_box));  
    
    if (index_func !=-1 && index_class !=-1)
    {
        GtkTextBuffer *buffer;
        buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(codewidget[current_page].txtinput));
        GtkTextIter current_iter;
        gtk_text_buffer_get_iter_at_offset(buffer,&current_iter,codewidget[current_page].pyclass_array[index_class].function_array[index_func].pos);
        gtk_text_buffer_place_cursor(buffer,&current_iter);
        gtk_text_view_scroll_to_iter(GTK_TEXT_VIEW(codewidget[current_page].txtinput),&current_iter,0.1,FALSE,0.5,0.5);
        }    
    }

void row_activated(GtkTreeView *tree_view,GtkTreePath *path,GtkTreeViewColumn *column,GtkTreeSelection *selection)
{
    GtkTreeIter iter;
    GtkTreeModel *model;
    char *value;
    if(gtk_tree_selection_get_selected(selection,&model,&iter))
    {
        gtk_tree_model_get(model,&iter,0,&value,-1);
        char attribute[strlen(value)];
        int i;
        for(i=0;i<strlen(value);i++)
        {
            if(value[i]=='(')
                break;
            attribute[i]=value[i];
            }
        attribute[i]='\0';
        GtkTextBuffer *buffer;
        buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(codewidget[gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook))].txtinput));
        gtk_text_buffer_insert_at_cursor(buffer,attribute,-1);
        gtk_widget_grab_focus(codewidget[gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook))].txtinput);
        if(i!=strlen(value))
        {
            tooltip_text = (char *)malloc(strlen(value)*sizeof(char));    
            strcpy(tooltip_text,value);
            }
        else
            tooltip_text=NULL;
        g_free(value);
        }
    string_stack_push(&tooltip_text_stack,tooltip_text);    
    free(tooltip_text);
    show_tooltip=TRUE;
    gtk_widget_destroy(GTK_WIDGET(tree_view));
    }

static gboolean txtinput_key_press_event(GtkWidget *widget,GdkEventKey *event)
{
    printf("txtinput key press for page %d\n",gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook)));
    can_combo_func_change=FALSE;
    
    GtkTextBuffer *buffer;
    int current_page = gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook));
    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(codewidget[current_page].txtinput));
    GtkTextIter current_iter;
    gtk_text_buffer_get_iter_at_mark(buffer,&current_iter,gtk_text_buffer_get_insert(buffer));
    int line=0,col=0,chars=0,i;
    chars = gtk_text_iter_get_offset(&current_iter);
    line = gtk_text_iter_get_line(&current_iter);
    col = gtk_text_iter_get_line_offset(&current_iter);
    
    
    if(event == NULL)
       return;
    //printf("EVENT NOT NULL\n");
    if (gtk_source_view_get_auto_indent(GTK_SOURCE_VIEW(codewidget[current_page].txtinput)) == TRUE)
    {        
        //printf("ss%d\n",event->keyval);
        gint indent_width;
        indent_width = gtk_source_view_get_indent_width(GTK_SOURCE_VIEW(codewidget[current_page].txtinput));
        GtkTextIter line_iter,next_line_iter;
        int next_line,offset_end_of_line;
        switch (event->keyval)
        {
            case GDK_Return:
                if(GTK_IS_WIDGET(codewidget[current_page].list_view)){gtk_widget_destroy(codewidget[current_page].list_view);}
                gtk_text_buffer_get_iter_at_line(buffer,&line_iter,line);
                char *line_text;
                line_text = gtk_text_buffer_get_text(buffer,&line_iter,&current_iter,TRUE);
                /**Code to add func when enter is pressed */
                regex_func = g_regex_new("([ \\ \\t]*\\bdef\\b\\s*.+)\\:",0,0,NULL);
                g_regex_match(regex_func,line_text,0,&match_info_func);
                while(g_match_info_matches(match_info_func))
                {
                    int start,end;
                    g_match_info_fetch_pos(match_info_func,0,&start,&end);
                    if (start == 0)
                    {                        
                        GString *word;
                        word = g_string_new(g_match_info_fetch(match_info_func,0));
                        int index = find_strstr(word->str,"def",0)+strlen("def")-1;            
                        for(i=index+1;i<strlen(word->str);i++)
                        {
                            if(!(isspace(word->str[i])))
                                break;
                            }
                        char word1[strlen(word->str)-i+1];
                        index=i;
                        for(i=index;i<strlen(word->str)-1;i++)
                            word1[i-index] = word->str[i];
                        word1[strlen(word->str)-index-1] = '\0';
                       
                        int class_index = gtk_combo_box_get_active(GTK_COMBO_BOX(codewidget[current_page].class_combo_box));
                        int func_index = gtk_combo_box_get_active(GTK_COMBO_BOX(codewidget[current_page].func_combo_box));                        
                        pyclass_insert_function(&codewidget[current_page].pyclass_array[class_index],word1,chars,func_index);
                                                     
                        toolbar_combo_class_changed(GTK_COMBO_BOX(codewidget[current_page].class_combo_box));
                        g_string_free(word,TRUE);
                        }
                    g_match_info_next(match_info_func,NULL);
                    }
                /**************************** ****/
                int spaces=0,count=0,i,break_index= find_strstr(line_text,"break",0),continue_index= find_strstr(line_text,"continue",0);
                if (break_index !=-1 || continue_index !=-1)
                {
                    int index_new = 0;
                    if (break_index>0)
                        index_new = break_index;
                    if (continue_index >0)
                        index_new = continue_index;
                    gtk_text_buffer_insert(buffer,&current_iter,"\n",-1);
                    if (line_text[index_new-2] == ' ')
                    {
                        int k,j,i;
                        for (j=0;j<spaces/indent_width -1;j++)
                        {
                            for (i=0;i<indent_width;i++)
                                gtk_text_buffer_insert(buffer,&current_iter," ",-1);
                            }
                        }
                    }                    
                else
                {
                    for (i=0; i<strlen(line_text);i++)
                    {
                        if(line_text[i] == ' ')
                        spaces++;
                        else
                        break;
                        }
                    for (i=0;i<strlen(line_text);i++)
                    {
                        if (line_text[i]==':')
                        count++;
                        }
                    int count_open=0,count_close=0;
                    for(i=0;i<strlen(line_text);i++)
                    {
                        if(line_text[i]=='(')
                        count_open++;
                        if(line_text[i]==')')
                        count_close++;
                        }                
                    gtk_text_buffer_insert(buffer,&current_iter,"\n",-1);
                    if (count==1)
                    {                     
                        int i,j;
                        for (j=0;j<spaces/indent_width + 1;j++)
                        {
                            for (i=0;i<indent_width;i++)
                            {
                                /*gtk_text_buffer_get_iter_at_mark(buffer,&current_iter,gtk_text_buffer_get_insert(buffer));*/                            
                                gtk_text_buffer_insert(buffer,&current_iter," ",-1);
                                 }
                            }
                        }    
                    else
                    {
                        int i,j;
                        for (j=0;j<spaces/indent_width;j++)
                        {
                            for (i=0;i<indent_width;i++)
                            {
                                /*gtk_text_buffer_get_iter_at_mark(buffer,&current_iter,gtk_text_buffer_get_insert(buffer));*/
                                gtk_text_buffer_insert(buffer,&current_iter," ",-1);
                                }                                                      
                            }
                        }
                    if(count_open>count_close)
                    {
                        int k,j;
                        for(k=0;k<count_open-count_close;k++)
                        {
                            for (j=0;j<spaces/indent_width + 1;j++)
                            {
                                for (i=0;i<indent_width;i++)
                                gtk_text_buffer_insert(buffer,&current_iter," ",-1);
                                }
                            }
                        }
                    }              
                g_free(line_text);
                return TRUE;
                
            case GDK_KEY_Left:
                if(GTK_IS_WIDGET(codewidget[current_page].list_view)){gtk_widget_destroy(codewidget[current_page].list_view);}
                gtk_text_buffer_get_iter_at_line(buffer,&line_iter,line);
                GtkTextIter end_of_line_iter;
                if (line+1 == gtk_text_buffer_get_line_count(buffer))
                    gtk_text_buffer_get_end_iter(buffer,&end_of_line_iter);
                else
                {
                    next_line = line+1;
                    gtk_text_buffer_get_iter_at_line(buffer,&next_line_iter,next_line);
                    offset_end_of_line = gtk_text_iter_get_offset(&next_line_iter)-1;
                    gtk_text_buffer_get_iter_at_offset(buffer,&end_of_line_iter,offset_end_of_line);
                    }                
                line_text = gtk_text_buffer_get_text(buffer,&line_iter,&end_of_line_iter,FALSE);
                spaces = 0;
                for(i=0;i<strlen(line_text);i++)
                {
                    if (line_text[i] == ' ')
                    spaces++;
                    else
                    break;
                    }
                if(col<=spaces&&col>0)
                {             
                    int new_line_offset = 0;
                    if (col%indent_width == 0)
                    {new_line_offset = col-indent_width;}
                    else
                    {new_line_offset = col-(col%indent_width);}
                    GtkTextIter new_iter;
                    gtk_text_buffer_get_iter_at_line_offset(buffer,&new_iter,line,new_line_offset);
                    gtk_text_buffer_place_cursor(buffer,&new_iter);
                    }
                else
                return FALSE;
                break;
            case GDK_KEY_Home:
                gtk_text_buffer_get_iter_at_line(buffer,&line_iter,line);
                if (line+1 == gtk_text_buffer_get_line_count(buffer))
                    gtk_text_buffer_get_end_iter(buffer,&end_of_line_iter);
                else
                {
                    next_line = line+1;
                    gtk_text_buffer_get_iter_at_line(buffer,&next_line_iter,next_line);
                    offset_end_of_line = gtk_text_iter_get_offset(&next_line_iter)-1;
                    gtk_text_buffer_get_iter_at_offset(buffer,&end_of_line_iter,offset_end_of_line);
                    }
                line_text = gtk_text_buffer_get_text(buffer,&line_iter,&end_of_line_iter,FALSE);
                spaces = 0;
                for(i=0;i<strlen(line_text);i++)
                {
                    if (line_text[i] == ' ')
                    spaces++;
                    else
                    break;
                    }
                int new_line_pos = spaces;
                GtkTextIter new_pos_iter;
                gtk_text_buffer_get_iter_at_line_offset(buffer,&new_pos_iter,line,new_line_pos);
                if (event->state == GDK_SHIFT_MASK)
                    gtk_text_buffer_select_range(buffer,&new_pos_iter,&current_iter);
                else
                    gtk_text_buffer_place_cursor(buffer,&new_pos_iter);
                if(GTK_IS_WIDGET(codewidget[current_page].list_view)){gtk_widget_destroy(codewidget[current_page].list_view);}
                break;             
            case GDK_KEY_BackSpace:
                if(GTK_IS_WIDGET(codewidget[current_page].list_view)){gtk_widget_destroy(codewidget[current_page].list_view);}
                gtk_text_buffer_get_iter_at_line(buffer,&line_iter,line);                                
                if (line+1 == gtk_text_buffer_get_line_count(buffer))
                    gtk_text_buffer_get_end_iter(buffer,&end_of_line_iter);
                else
                {
                    next_line = line+1;
                    gtk_text_buffer_get_iter_at_line(buffer,&next_line_iter,next_line);
                    offset_end_of_line = gtk_text_iter_get_offset(&next_line_iter)-1;
                    gtk_text_buffer_get_iter_at_offset(buffer,&end_of_line_iter,offset_end_of_line);
                    }                
                line_text = gtk_text_buffer_get_text(buffer,&line_iter,&end_of_line_iter,FALSE);
                spaces = 0;
                for(i=0;i<strlen(line_text);i++)
                {
                    if (line_text[i] == ' ')
                        spaces++;
                    else
                       break;
                    }
                if(col<=spaces&&col>0)
                {             
                    int new_line_offset = 0;
                    if (col%indent_width == 0)
                        new_line_offset = col-indent_width;
                    else
                        new_line_offset = col-(col%indent_width);
                    GtkTextIter new_iter;
                    gtk_text_buffer_get_iter_at_line_offset(buffer,&new_iter,line,new_line_offset);
                    gboolean ans = gtk_text_buffer_delete_interactive(buffer,&new_iter,&current_iter,TRUE);
                    gtk_text_buffer_place_cursor(buffer,&new_iter);
                    }
                else
                {
                    can_combo_func_change=TRUE;
                	return FALSE;
                    }
                break;
            case 65505: // For Shift Key
                return FALSE;
                
            case 46:                
                //printf("DOT\n");         
                gtk_text_buffer_get_iter_at_line(buffer,&line_iter,line);
                if (line+1 == gtk_text_buffer_get_line_count(buffer))
                    gtk_text_buffer_get_end_iter(buffer,&end_of_line_iter);
                else
                {
                    next_line = line+1;
                    gtk_text_buffer_get_iter_at_line(buffer,&next_line_iter,next_line);
                    offset_end_of_line = gtk_text_iter_get_offset(&next_line_iter)-1;
                    gtk_text_buffer_get_iter_at_offset(buffer,&end_of_line_iter,offset_end_of_line);
                    }
                line_text = gtk_text_buffer_get_text(buffer,&line_iter,&end_of_line_iter,FALSE);
                char *self = "self";
                //printf("col%d\n",col);
                for(i=col-1;i>=col-strlen(self);i--)
                {
                    //printf("line_text %c self %c\n",line_text[i],self[strlen(self)-col+i]);
                    if(line_text[i]==self[strlen(self)-col+i])
                        continue;
                    else
                        break;
                    }
                
                if(i==col-strlen(self)-1 &&!isalnum(line_text[i]))
                {                   
                    int indentation_level=0;
                    for(i=0;i<strlen(line_text);i++)
                    {
                        if (isspace(line_text[i]))
                            indentation_level++;
                        else
                           break;
                        }                        
                   
                    for(i=codewidget[current_page].pyclass_count-1;i>=0;i--)
                    {
                        if(codewidget[current_page].pyclass_array[i].pos<=chars && codewidget[current_page].pyclass_array[i].indentation<indentation_level)                                
                                break;
                        }
                    if(i!=-1)
                    {
                        /****successfull in finding class***/
                        /*printf("class %s class_pos %d class_indentation %d\n",class_array[i],class_pos_array[i][0],class_indentation_array[i]);    */
                         /***calculating cursor position in x,y coordinates w.r.t. source view***/                            
                        GdkRectangle location;
                        gtk_text_view_get_iter_location(GTK_TEXT_VIEW(codewidget[current_page].txtinput),&current_iter,&location);
                        int window_x,window_y,class = i;
                        gtk_text_view_buffer_to_window_coords(GTK_TEXT_VIEW(codewidget[current_page].txtinput),GTK_TEXT_WINDOW_WIDGET,location.x+location.width,location.y+location.height,&window_x,&window_y);
                        //printf("Window X %d  Window Y %d\n",window_x,window_y);                            
                        /****************************************************/                       
                        /***showing list view**/
                        gtk_widget_set_tooltip_text(codewidget[current_page].txtinput,"");
                        
                        codewidget[current_page].list_view = gtk_tree_view_new();
                        GtkListStore *list_store;
                        list_store = gtk_list_store_new(1,G_TYPE_STRING);
                        GtkTreeIter iter;
                        GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
                        GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes("Class Methods", renderer, "text", 0, NULL);
                        selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(codewidget[current_page].list_view));
                        g_signal_connect(G_OBJECT(codewidget[current_page].list_view),"row-activated",G_CALLBACK(row_activated),selection); 
                        
                        for(i=0;i<codewidget[current_page].pyclass_array[class].self_array_size;i++)
                        {
                            gtk_list_store_append(list_store,&iter);                                
                            gtk_list_store_set(list_store,&iter,0,codewidget[current_page].pyclass_array[class].self_array[i],-1);
                            }
                                                          
                        for(i=0;i<codewidget[current_page].pyclass_array[class].function_array_size;i++)
                        {
                            gtk_list_store_append(list_store,&iter);
                            gtk_list_store_set(list_store,&iter,0,codewidget[current_page].pyclass_array[class].function_array[i],-1,-1);
                            }
                        GtkWidget *scroll_list = gtk_scrolled_window_new(NULL,NULL);
                        GtkWidget *vbox = gtk_vbox_new(2,FALSE);
                        /*gtk_container_set_border_width(GTK_CONTAINER(scroll_list),5);*/
                        gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll_list),GTK_POLICY_ALWAYS,GTK_POLICY_ALWAYS);
                        gtk_tree_view_set_model(GTK_TREE_VIEW(codewidget[current_page].list_view),GTK_TREE_MODEL(list_store));
                        gtk_tree_view_append_column (GTK_TREE_VIEW (codewidget[current_page].list_view), column);
                        gtk_container_add(GTK_CONTAINER(scroll_list),codewidget[current_page].list_view);
                        gtk_box_pack_start(GTK_BOX(vbox),scroll_list,TRUE,TRUE,0);
                        gtk_fixed_put(GTK_FIXED(codewidget[current_page].fixed_txtinput),vbox,window_x,window_y);
                        gtk_widget_set_size_request(codewidget[current_page].list_view,181,151);
                        gtk_widget_show_all(vbox);                            
                        /**********************/
                        }                                       
                    }
                else
                {
                    printf("FINDING MODULE\n");
                    int j;
                    for(i=0;i<codewidget[current_page].module_array_size;i++)
                    {
                        printf("MODULE NAME %s\n",codewidget[current_page].module_array[i].module_name);
                        char *module_name = codewidget[current_page].module_array[i].module_name;
                        for(j=col-1;j>=col-strlen(module_name);j--)
                        {
                            printf("line_text %c module %c\n",line_text[j],module_name[strlen(module_name)-col+j]);
                            if(line_text[j]==module_name[strlen(module_name)-col+j])
                                continue;
                            else
                                break;
                            }
                        if(j==col-strlen(module_name)-1)
                            break;
                        }
                    if(i!=codewidget[current_page].module_array_size && !isalnum(line_text[i]))
                    {
                        GdkRectangle location;
                        gtk_text_view_get_iter_location(GTK_TEXT_VIEW(codewidget[current_page].txtinput),&current_iter,&location);
                        int window_x,window_y,class = i;
                        gtk_text_view_buffer_to_window_coords(GTK_TEXT_VIEW(codewidget[current_page].txtinput),GTK_TEXT_WINDOW_WIDGET,location.x+location.width,location.y+location.height,&window_x,&window_y); 
                        /***showing list view**/
                        gtk_widget_set_tooltip_text(codewidget[current_page].txtinput,"");
                        
                        codewidget[current_page].list_view = gtk_tree_view_new();
                        GtkListStore *list_store;
                        list_store = gtk_list_store_new(1,G_TYPE_STRING);
                        GtkTreeIter iter;
                        GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
                        GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes("Module Attributes", renderer, "text", 0, NULL);
                        selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(codewidget[current_page].list_view));
                        g_signal_connect(G_OBJECT(codewidget[current_page].list_view),"row-activated",G_CALLBACK(row_activated),selection); 
                        
                        for(j=0;j<codewidget[current_page].module_array[i].class_array[0].function_array_size;j++)
                        {
                            gtk_list_store_append(list_store,&iter);                                
                            gtk_list_store_set(list_store,&iter,0,codewidget[current_page].module_array[i].class_array[0].function_array[j].func_name,-1);
                            }
                                                          
                        for(j=1;j<codewidget[current_page].module_array[i].class_array_size;j++)
                        {
                            gtk_list_store_append(list_store,&iter);
                            gtk_list_store_set(list_store,&iter,0,codewidget[current_page].module_array[i].class_array[j].class_name,-1,-1);
                            }
                        GtkWidget *scroll_list = gtk_scrolled_window_new(NULL,NULL);
                        GtkWidget *vbox = gtk_vbox_new(2,FALSE);
                        /*gtk_container_set_border_width(GTK_CONTAINER(scroll_list),5);*/
                        gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll_list),GTK_POLICY_ALWAYS,GTK_POLICY_ALWAYS);
                        gtk_tree_view_set_model(GTK_TREE_VIEW(codewidget[current_page].list_view),GTK_TREE_MODEL(list_store));
                        gtk_tree_view_append_column (GTK_TREE_VIEW (codewidget[current_page].list_view), column);
                        gtk_container_add(GTK_CONTAINER(scroll_list),codewidget[current_page].list_view);
                        gtk_box_pack_start(GTK_BOX(vbox),scroll_list,TRUE,TRUE,0);
                        gtk_fixed_put(GTK_FIXED(codewidget[current_page].fixed_txtinput),vbox,window_x,window_y);
                        gtk_widget_set_size_request(codewidget[current_page].list_view,181,151);
                        gtk_widget_show_all(vbox);            
                        }
                    }
                can_combo_func_change=TRUE;
                return FALSE;
                break;
            case 40: 
                if(event->state & GDK_SHIFT_MASK && show_tooltip==TRUE) //40 for Key 9 or Key (               
                {                    
                    //string_stack_push(&tooltip_text_stack,tooltip_text);    
                    gtk_widget_set_tooltip_text(codewidget[current_page].txtinput,tooltip_text_stack.text_array[tooltip_text_stack.count-1]);                    
                    }      
                break;
            case 41:
                if(event->state & GDK_SHIFT_MASK && show_tooltip==TRUE) //41 for Key 0 or Key )
                {                    
                    if(tooltip_text_stack.count>0 && string_stack_pop(&tooltip_text_stack))
                    {                        
                        if(tooltip_text_stack.count!=0)
                            gtk_widget_set_tooltip_text(codewidget[current_page].txtinput,tooltip_text_stack.text_array[tooltip_text_stack.count-1]);
                        else
                        {
                            show_tooltip=FALSE;
                            gtk_widget_set_tooltip_text(codewidget[current_page].txtinput,"");
                            }
                        }                                         
                    }
                break;
            default:
                if(show_tooltip==TRUE && tooltip_text_stack.text_array[tooltip_text_stack.count-1]!=NULL)                
                    gtk_widget_set_tooltip_text(codewidget[current_page].txtinput,tooltip_text_stack.text_array[tooltip_text_stack.count-1]);                
                
                if(GTK_IS_WIDGET(codewidget[current_page].list_view))
                {                    
                    gtk_widget_destroy(codewidget[current_page].list_view);
                    }
                can_combo_func_change=TRUE;
                return FALSE;
            }
        }
    else
    {
        can_combo_func_change=TRUE;
        return FALSE;
        }
    can_combo_func_change=TRUE;
    return FALSE;
    }

gboolean txtinput_button_release_event(GtkWidget *widget,GdkEvent *event)
{
    printf("txtinput button release for page %d\n",gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook)));
    if (gtk_text_buffer_get_has_selection(gtk_text_view_get_buffer(GTK_TEXT_VIEW(codewidget[gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook))].txtinput)))==TRUE)
    {
        gtk_widget_set_sensitive(copy,TRUE);
        gtk_widget_set_sensitive(cut,TRUE);        
        }
    else
    {
        gtk_widget_set_state(copy,GTK_STATE_INSENSITIVE);
        gtk_widget_set_state(cut,GTK_STATE_INSENSITIVE);
        }
    if(gtk_source_buffer_can_undo(GTK_SOURCE_BUFFER(gtk_text_view_get_buffer(GTK_TEXT_VIEW(codewidget[gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook))].txtinput))))==TRUE)
        gtk_widget_set_sensitive(undo,TRUE); 
    if(gtk_source_buffer_can_redo(GTK_SOURCE_BUFFER(gtk_text_view_get_buffer(GTK_TEXT_VIEW(codewidget[gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook))].txtinput))))==TRUE)
        gtk_widget_set_sensitive(redo,TRUE); 
    return FALSE;
    }

void txtinput_mark_set(GtkTextBuffer *buffer, GtkTextIter *current_iter, GtkTextMark *mark, gpointer data)
{
    printf("txtinput mark set for page %d\n",gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook)));
    can_combo_func_change=FALSE;
    
    int current_page = gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook));
    //GtkTextIter current_iter;
    gtk_text_buffer_get_iter_at_mark(buffer,current_iter,gtk_text_buffer_get_insert(buffer));
    int line=0,col=0,chars=0;
    chars = gtk_text_iter_get_offset(current_iter);
    line = gtk_text_iter_get_line(current_iter);
    col = gtk_text_iter_get_line_offset(current_iter);
    
    gtk_statusbar_pop(GTK_STATUSBAR(statusbar),0);
    gchar *msg;
    msg = g_strdup_printf("Char %d Line %d Col %d", chars +1,line +1, col +1);
    gtk_statusbar_push(GTK_STATUSBAR(statusbar),0,msg);
    int i=-1,j=-1;
    /**This code will detect in which class and its method the current cursor is  ***/
    if(codewidget[gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook))].pyclass_array==NULL)
       return;
    
    if (prev_line != line)
    {
        i=codewidget[current_page].pyclass_count;
        while(--i>=0)
        {
            if(codewidget[current_page].pyclass_array[i].pos < chars)
                break;
            }     
        //printf("line %d prev_line %d i%d\n",line,prev_line,i);   
        if (i!=-1)
        {            
            //printf("%s\n",codewidget[gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook))].class_array[i]);
            j=codewidget[current_page].pyclass_array[i].function_array_size; /**Size of func_pos_array[i] **/
            while(--j>=0)
            {
                if(codewidget[gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook))].pyclass_array[i].function_array[j].pos<=chars)
                    break;
                }
         
            if(j!=-1)
            {
                gtk_combo_box_set_active(GTK_COMBO_BOX(codewidget[current_page].class_combo_box),i);
                gtk_combo_box_set_active(GTK_COMBO_BOX(codewidget[current_page].func_combo_box),j);                                
                }
            else if(i==0)
                gtk_combo_box_set_active(GTK_COMBO_BOX(codewidget[current_page].class_combo_box),0);                
            }
        }
    can_combo_func_change=TRUE;
    /*********************************************************/
    i=0;
    while(lines_track_array[current_page][i]!=-1&&i<5)
        i++;
    
    if(i<5)
    {
        if (lines_track_array[current_page][i-1]!=line+1&&i!=0)
            lines_track_array[current_page][i]=line+1;
        else
            if(i==0)
                lines_track_array[current_page][i]=line+1;
        }
    j=0;
    if (i==5)
    {
        i=0;
        while(lines_track_array[current_page][i]!=line+1&&i<5)      /*LIFO*/
            i++;
        
        if(i==5)
        {
            for(j=0;j<=3;j++)
                lines_track_array[current_page][j]=lines_track_array[current_page][j+1];
            lines_track_array[current_page][4]= line+1;
            }
        if(i<4)
        {
            for(j=i;j<=3;j++)
                lines_track_array[current_page][j]=lines_track_array[current_page][j+1];
            lines_track_array[current_page][4]= line+1;
            }
        }
    prev_line = line;       
    }
#endif