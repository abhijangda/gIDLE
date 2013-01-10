/*243 Lines*/ 
#include"codewidget.h" 
#include<stdlib.h>
#include<string.h>

void codewidget_new(CodeWidget *cwidget)
{
    GtkSourceBuffer *buffer = gtk_source_buffer_new(NULL);
    cwidget->txtinput = gtk_source_view_new_with_buffer(buffer);
    cwidget->func_combo_box = gtk_combo_box_text_new();
    cwidget->class_combo_box = gtk_combo_box_text_new();
    cwidget->fixed_txtinput = gtk_fixed_new();
    cwidget->list_view = gtk_tree_view_new();
    GtkListStore *list_store;
    list_store = gtk_list_store_new(1,G_TYPE_STRING);
    GtkTreeIter iter;
    GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
    GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes("Class Methods", renderer, "text", 0, NULL);
    cwidget->hbox = gtk_hbox_new(FALSE,2);       
    cwidget->class_array=NULL;     
    }

void codewidget_get_class_and_members(CodeWidget *cwidget)
{    
    cwidget->class_array = (char **)malloc(1*sizeof(char **));
    asprintf(&cwidget->class_array[0],"Global Scope");
    cwidget->class_pos_array = (int **)malloc(1*sizeof(int *));
    cwidget->class_pos_array[0] = (int*)malloc(2*sizeof(int));
    cwidget->class_pos_array[0][0]=0;
    cwidget->class_pos_array[0][1]=0;   
    cwidget->class_indentation_array=(int*)malloc(1*sizeof(int));
    cwidget->class_indentation_array[0] = 0;
    
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(cwidget->txtinput));
    GtkTextIter start_iter, end_iter;
    gtk_text_buffer_get_start_iter(buffer,&start_iter);
    gtk_text_buffer_get_end_iter(buffer,&end_iter);
    char *text = gtk_text_buffer_get_text(buffer,&start_iter,&end_iter,FALSE);
    GRegex *regex_class,*regex_func;
    GMatchInfo *match_info_class,*match_info_func;
    int class_count = 1,indentation=0,global_func_count=0;
    /***Also remember to create cwidget->class_indentation_array**/
    /*getting class names and their positions*/
    regex_class = g_regex_new("[ \\ \\t]*\\bclass\\b\\s*\\w+",0,0,NULL);
    
    if (g_regex_match(regex_class,text,0,&match_info_class))
    {          
        class_count++; 
        cwidget->class_array = (char **)realloc(cwidget->class_array,2*sizeof(char **));
        cwidget->class_pos_array = (int **)realloc(cwidget->class_pos_array,2*sizeof(int *));    
        asprintf(&cwidget->class_array[1],"%s",g_match_info_fetch(match_info_class,0));    
        cwidget->class_pos_array[1] = (int*)malloc(2*sizeof(int));        
        g_match_info_fetch_pos(match_info_class,0,&cwidget->class_pos_array[1][0],&cwidget->class_pos_array[1][1]);
        g_match_info_next(match_info_class,NULL);
        cwidget->class_indentation_array = (int *)realloc(cwidget->class_indentation_array,2*sizeof(int));
        cwidget->class_indentation_array[1] = 0;        
        
        while(g_match_info_matches(match_info_class))
        {
            indentation = 0;
            class_count++;
            cwidget->class_array = (char **)realloc(cwidget->class_array,class_count*sizeof(char **));
            asprintf(&cwidget->class_array[class_count-1],"%s",g_match_info_fetch(match_info_class,0));
            int i;
            for(i=0;i<strlen(cwidget->class_array[class_count-1]);i++)
            {
                if(isspace(cwidget->class_array[class_count-1][i]))
                    indentation++;
                else
                    break;
                }
            cwidget->class_indentation_array = (int *)realloc(cwidget->class_indentation_array,class_count*sizeof(int));
            cwidget->class_indentation_array[class_count-1] = indentation;
            cwidget->class_pos_array = (int **)realloc(cwidget->class_pos_array,class_count*sizeof(int *));
            cwidget->class_pos_array[class_count-1] = (int *)malloc(2*sizeof(int));
            g_match_info_fetch_pos(match_info_class,0,&cwidget->class_pos_array[class_count-1][0],&cwidget->class_pos_array[class_count-1][1]);
            cwidget->class_pos_array[class_count-1][0] += indentation;
            g_match_info_next(match_info_class,NULL);
            }
        
        cwidget->class_array = (char **)realloc(cwidget->class_array,(class_count+1)*sizeof(char **));
        cwidget->class_array[class_count] = NULL;
        cwidget->class_pos_array = (int **)realloc(cwidget->class_pos_array,(class_count+1)*sizeof(int *));
        cwidget->class_pos_array[class_count] = NULL;   
        /***************************/
        g_match_info_free(match_info_class);
        g_regex_unref(regex_class);
        }
    /**************for functions*********/
    regex_func = g_regex_new("([ \\ \\t]*\\bdef\\b\\s*.+)\\:",0,0,NULL); 
    
    if(class_count>0 && g_regex_match(regex_func,text,0,&match_info_func))
    {   
        int func_count=0,class=0,class_prev=-1;
        //cwidget->func_array is equivalent to cwidget->func_array[class_count][func_count][character]
        int indentation_prev=0;    
        cwidget->func_array = (char ***)malloc((class_count+1)*sizeof(char **));
        cwidget->func_pos_array = (int **)malloc(1*sizeof(int *));
        cwidget->func_pos_array[0] = (int *)malloc(1*sizeof(int));
        cwidget->func_array[0] = (char **)malloc(1*sizeof(char *));
        GRegex *regex_def;
        GMatchInfo *match_info_def;
        regex_def = g_regex_new("\\bdef\\b\\s*",0,0,NULL);
        
        while(g_match_info_matches(match_info_func))
        {
            indentation = 0;        
            int i;
            char *word = g_match_info_fetch(match_info_func,0);
            for(i=0;i<strlen(word);i++)
            {
                if(isspace(word[i]))
                    indentation++;
                else
                    break;
                }
            int start,end;
            if (indentation==0)
            {
                //Add code to add functions here which are global functions
                g_match_info_fetch_pos(match_info_func,0,&start,&end);
                g_regex_match(regex_def,word,0,&match_info_def);
                g_match_info_fetch(match_info_def,0);
                int s=0,e=0;
                g_match_info_fetch_pos(match_info_def,0,&s,&e);
                g_match_info_free(match_info_def);
                              
                int index = e;
                i=0;
                char word1[strlen(word)-e+1];
                
                for(i=index;i<strlen(word)-1;i++)
                    word1[i-index] = word[i];
                word1[strlen(word)-index-1] = '\0';
                if (class==-1)
                    class=class_prev;
                
                cwidget->func_array[0] = (char **)realloc(cwidget->func_array[0],(global_func_count+1)*sizeof(char *));
                asprintf(&cwidget->func_array[0][global_func_count],"%s",word1);
                //printf("global_func_count%d\n",global_func_count);
                cwidget->func_pos_array[0] = (int *)realloc(cwidget->func_pos_array[0],(global_func_count+1)*sizeof(int));
                cwidget->func_pos_array[0][global_func_count] = start;
                global_func_count++;
                }
            if (indentation>0)
            {
                if(g_match_info_fetch_pos(match_info_func,0,&start,&end))
                {                   
                    for(class=class_count-1;class>0;class--)
                    {                        
                        if(cwidget->class_pos_array[class][0]<start)
                        {                           
                            if (word[indentation-1] == '\t' && cwidget->class_indentation_array[class]==indentation-1) //-1 for tabs
                                    break;
                            if (word[indentation-1] == ' ' && cwidget->class_indentation_array[class]==indentation-4) //4 for spaces remember to change it
                                    break;
                            }                    
                        }
                    if (class == 0)
                    {
                        g_match_info_next(match_info_func,NULL); 
                        continue;
                        }
                    int index = find_strstr(word,"def",0)+strlen("def")-1;            
                    for(i=index+1;i<strlen(word);i++)
                    {
                        if(!(isspace(word[i])))
                            break;
                        }
                    char word1[strlen(word)-i+1];
                    index=i;
                    for(i=index;i<strlen(word)-1;i++)
                        word1[i-index] = word[i];
                    word1[strlen(word)-index-1] = '\0';
                    if (class==-1)
                        class=class_prev;
                    //printf("word1%s\n",word1);
                    //printf("class%d prev%d\n",class,class_prev);
                    if(class_prev==class && class_prev!=-1)
                    {
                        cwidget->func_array[class] = (char **)realloc(cwidget->func_array[class],(func_count+1)*sizeof(char *));
                        cwidget->func_pos_array[class] = (int *)realloc(cwidget->func_pos_array[class],(func_count+1)*sizeof(int));
                        cwidget->func_pos_array[class][func_count] = start;
                        asprintf(&cwidget->func_array[class][func_count],"%s",word1);
                        //printf("%s\n",cwidget->func_array[class][func_count]);
                        func_count++;
                        }
                    else
                    {
                        if(class_prev!=-1)
                        {
                            //printf("class_prev !=-1\n");                    
                            cwidget->func_array[class_prev] = (char **)realloc(cwidget->func_array[class_prev],(func_count+1)*sizeof(char *));
                            cwidget->func_array[class_prev][func_count] = NULL;
                            cwidget->func_pos_array[class_prev] = (int *)realloc(cwidget->func_pos_array[class_prev],(func_count+1)*sizeof(int));                            
                            cwidget->func_pos_array[class_prev][func_count] = -1;
                            }
                        func_count=0;
                        //cwidget->func_array = (char ***)realloc(cwidget->func_array,(class+1)*sizeof(char **));
                        cwidget->func_array[class] = (char **)malloc(1*sizeof(char *));           
                        asprintf(&cwidget->func_array[class][func_count],"%s",word1);
                        cwidget->func_pos_array[class] = (int *)malloc((func_count+1)*sizeof(int));
                        cwidget->func_pos_array[class][func_count] = start;
                        //printf("%s\n",cwidget->func_array[class][func_count]);
                        func_count++;
                        }
                    }
                class_prev = class;
                indentation_prev = indentation;
                }            
            g_match_info_next(match_info_func,NULL);        
            }        
        cwidget->func_array[class_prev] = (char **)realloc(cwidget->func_array[class_prev],(func_count+1)*sizeof(char *));
        cwidget->func_array[class_prev][func_count] = (char *)0;
        cwidget->func_pos_array[class] = (int *)realloc(cwidget->func_pos_array[class],(func_count+1)*sizeof(int));
        cwidget->func_pos_array[class][func_count] = -1;
        
        cwidget->func_array[0] = (char **)realloc(cwidget->func_array[0],(global_func_count+1)*sizeof(char *));
        cwidget->func_array[0][global_func_count] = NULL;
        cwidget->func_pos_array[0] = (int *)realloc(cwidget->func_pos_array[0],(global_func_count+1)*sizeof(int));
        cwidget->func_pos_array[0][global_func_count] = -1;
        
        g_match_info_free(match_info_func);
        g_regex_unref(regex_func);
        }
    else
    {        
        cwidget->class_array = (char **)malloc(1*sizeof(char **));
        cwidget->class_array[0] = NULL;
        cwidget->class_pos_array = (int **)malloc(1*sizeof(int *));    
        cwidget->class_pos_array[0] = (int *)malloc(2*sizeof(int));
        cwidget->class_pos_array[0][0] = -1;
        cwidget->class_pos_array[0][1] = -1;
        cwidget->func_array = (char ***)malloc(sizeof(char **));
        cwidget->func_array[0] = (char **)malloc(sizeof(char *));
        cwidget->func_array[0][0] = (char *)0;
        cwidget->func_pos_array = (int **)malloc(sizeof(int *));
        cwidget->func_pos_array[0] = (int *)malloc(sizeof(int));
        cwidget->func_pos_array[0][0] = -1;
        }
    
    //For class attributes other than member functions
    GRegex* regex_self = g_regex_new("[\\ \\t]*\\bself\\.+.+=",0,0,NULL);
    GMatchInfo* match_info_self;
    
    if(class_count>1 && g_regex_match(regex_self,text,0,&match_info_self))
    {   
        int self_count=0,class=0,class_prev=-1;
        //cwidget->func_array is equivalent to cwidget->func_array[class_count][func_count][character]
        int indentation_prev=0;    
        cwidget->self_array = (char ***)malloc((class_count+1)*sizeof(char **));        
        cwidget->self_array[0] = (char **)malloc(1*sizeof(char *));        
        cwidget->self_array[0][0]=NULL;
        
        while(g_match_info_matches(match_info_self))
        {            
            indentation = 0;        
            int i;
            char *word = g_match_info_fetch(match_info_self,0);
            for(i=0;i<strlen(word);i++)
            {
                if(isspace(word[i]))
                    indentation++;
                else
                    break;
                }            
            int start,end;            
            if (indentation>0)
            {
                if(g_match_info_fetch_pos(match_info_self,0,&start,&end))
                {
                    for(class=class_count-1;class>0;class--)
                    {                        
                        //printf("CLASS POS %d start %d\n",cwidget->class_pos_array[class][0],start);
                        if(cwidget->class_pos_array[class][0]<start)
                        {                            
                            if (word[indentation-1] == '\t' && cwidget->class_indentation_array[class]==indentation-2*1) //-1 for tabs
                                    break;
                            if (word[indentation-1] == ' ' && cwidget->class_indentation_array[class]==indentation-2*4) //4 for spaces remember to change it
                                    break;
                            }              
                        }
                    
                    if (class == 0)
                    {
                        g_match_info_next(match_info_self,NULL); 
                        continue;
                        }
                    int index = find_strstr(word,"self.",0)+strlen("self.");
                    
                    char word1[strlen(word)+1];
                    
                    for(i=index;i<strlen(word)-1;i++)
                        word1[i-index] = word[i];
                    word1[strlen(word)-index-1] = '\0';
                    if (class==-1)
                        class=class_prev;           
                    
                    //printf("word1%s\n",word1);
                    //printf("class%d prev%d\n",class,class_prev);
                    if(class_prev==class && class_prev!=-1)
                    {   
                        int b=0;                             
                                                
                        while(b<self_count)
                        {                            
                            //printf("SELF ARRAY%sWORD %s\n",cwidget->self_array[class][b],word1);                            
                            if(strcmp(cwidget->self_array[class][b],word1)==0)
                            {
                                g_match_info_next(match_info_self,NULL); 
                                break;
                                }
                            b++;
                            }
                        if(b!=self_count)
                            continue;
                        
                        cwidget->self_array[class] = (char **)realloc(cwidget->self_array[class],(self_count+1)*sizeof(char *));                        
                        asprintf(&cwidget->self_array[class][self_count],"%s",word1);                        
                        self_count++;
                        }
                    else
                    {
                        if(class_prev!=-1)
                        {
                            //printf("class_prev !=-1\n");                    
                            cwidget->self_array[class_prev] = (char **)realloc(cwidget->self_array[class_prev],(self_count+1)*sizeof(char *));
                            cwidget->self_array[class_prev][self_count] = NULL;                            
                            }
                        self_count=0;
                        cwidget->self_array = (char ***)realloc(cwidget->self_array,(class+1)*sizeof(char **));
                        cwidget->self_array[class] = (char **)malloc(1*sizeof(char *));           
                        asprintf(&cwidget->self_array[class][self_count],"%s",word1);                       
                        //printf("%s\n",cwidget->self_array[class][self_count]);
                        self_count++;
                        }
                    }
                class_prev = class;
                indentation_prev = indentation;
                }            
            g_match_info_next(match_info_self,NULL);        
            }        
        //printf("REALLOC CLASS PREV %d\n",class_prev);
        cwidget->self_array[class_prev] = (char **)realloc(cwidget->self_array[class_prev],(self_count+1)*sizeof(char *));
        cwidget->self_array[class_prev][self_count] = (char *)0;
        g_match_info_free(match_info_self);
        g_regex_unref(regex_self);
        }
    /*int i=-1,j=-1;
    while(cwidget->class_array[++i]!=NULL)
    {
        j=-1;
        while(cwidget->self_array[i][++j]!=NULL)
            printf("%s\n",cwidget->self_array[i][j]);
        }*/
    }

void codewidget_fill_combo_boxes(CodeWidget *cwidget)
{
    int j=0;
    int i=0;
    if (GTK_IS_COMBO_BOX(cwidget->func_combo_box))
    {
        int i=0,count =0;        
        GtkTreeModel *model = gtk_combo_box_get_model(GTK_COMBO_BOX(cwidget->func_combo_box));
        GtkTreeIter iter;
        if(gtk_tree_model_get_iter_first(model,&iter))
        {
            count++;
            while(gtk_tree_model_iter_next(model,&iter)==TRUE)
                count++;
            for(i=count-1;i>=0;i--)
                gtk_combo_box_text_remove(GTK_COMBO_BOX_TEXT(cwidget->class_combo_box),i);
            }
        }
    
    while(cwidget->class_array[i]!=NULL)
    {
        //j=0;
        //printf("%s\n",class_array[i]);
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(cwidget->class_combo_box),cwidget->class_array[i]);
        //printf("class_indentation%d\n",class_indentation_array[j]);
        //printf("class_pos%d\n",class_pos_array[i][0]);
        //while(func_pos_array[i][j]!=-1)
        //{
          //  printf("%d\n",func_pos_array[i][j]);
            //j++;
          //  }
        i++;
        }        
    gtk_combo_box_set_active(GTK_COMBO_BOX(cwidget->class_combo_box),0);

    if (GTK_IS_COMBO_BOX(cwidget->func_combo_box))
    {
        i=0;
        int count = 0;
        GtkTreeModel *model = gtk_combo_box_get_model(GTK_COMBO_BOX(cwidget->func_combo_box));
        GtkTreeIter iter;
        if(gtk_tree_model_get_iter_first(model,&iter)) 
        {
            count++;
            while(gtk_tree_model_iter_next(model,&iter)==TRUE)
                count++;
            for(i=count-1;i>=0;i--)
                gtk_combo_box_text_remove(GTK_COMBO_BOX_TEXT(cwidget->func_combo_box),i);
            }
        i=0;
        while(cwidget->func_array[0][i]!=NULL) 
        {
            gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(cwidget->func_combo_box),cwidget->func_array[0][i]);
            i++;
            }
        gtk_combo_box_set_active(GTK_COMBO_BOX(cwidget->func_combo_box),0);
        }
    }

void codewidget_clear_arrays(CodeWidget *cwidget)
{
    if (GTK_IS_COMBO_BOX(cwidget->func_combo_box))
    {
        int i=0,count =0;        
        GtkTreeModel *model = gtk_combo_box_get_model(GTK_COMBO_BOX(cwidget->class_combo_box));
        GtkTreeIter iter;
        if(gtk_tree_model_get_iter_first(model,&iter))
        {
            count++;
            while(gtk_tree_model_iter_next(model,&iter)==TRUE)
                count++;
            for(i=count-1;i>=0;i--)
                gtk_combo_box_text_remove(GTK_COMBO_BOX_TEXT(cwidget->class_combo_box),i);
            }
        
        model = gtk_combo_box_get_model(GTK_COMBO_BOX(cwidget->func_combo_box));
        
        if(gtk_tree_model_get_iter_first(model,&iter))
        {
            count++;
            while(gtk_tree_model_iter_next(model,&iter)==TRUE)
                count++;
            for(i=count-1;i>=0;i--)
                gtk_combo_box_text_remove(GTK_COMBO_BOX_TEXT(cwidget->func_combo_box),i);
            }
        }
    free(cwidget->class_array); //Remember to correct it
    free(cwidget->func_array); //Remember to correct it
    free(cwidget->self_array); //Remember to correct it
    free(cwidget->class_pos_array); //Remember to correct it
    free(cwidget->func_pos_array); //Remember to correct it
    free(cwidget->class_indentation_array); //Remember to correct it  
    }