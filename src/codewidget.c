/*764 Lines*/ 
#include"codewidget.h" 
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<stdio.h>
#include<dirent.h>
#include<sys/stat.h>

//Remember to free all the memory in arrays of code widget

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
    }

void add_from_import_file(pymodule *module,char *text)
{
    //Code to get classes and its members in currently open file
    GRegex *regex_class,*regex_func;
    GMatchInfo *match_info_class,*match_info_func; 
    int indentation=0;
    
    /***Also remember to create cwidget->class_indentation_array**/
    /*getting class names and their positions*/
    regex_class = g_regex_new("[ \\ \\t]*\\bclass\\b\\s*\\w+",0,0,NULL);
    
    if (g_regex_match(regex_class,text,0,&match_info_class))
    {       
        printf("Regex found\n"); 
             
        int class_start=0,class_end=0;
        g_match_info_fetch_pos(match_info_class,0,&class_start,&class_end);
        
        char *class_name = g_match_info_fetch(match_info_class,0);
        //printf("CLASS NAME %s INDENTATION %d\n",class_name,indentation);        
        pymodule_add_class(module,class_name,indentation,class_start);
        g_free(class_name);
        g_match_info_next(match_info_class,NULL);      
        
        while(g_match_info_matches(match_info_class))
        {
            indentation = 0;            
            class_name = g_match_info_fetch(match_info_class,0);
            //printf("CLASS NAME %s\n",class_name);
            int i;
            for(i=0;i<strlen(class_name);i++)
            {
                if(isspace(class_name[i]))
                    indentation++;
                else
                    break;
                }
          
            g_match_info_fetch_pos(match_info_class,0,&class_start,&class_end);            
            //printf("CLASS NAME %s INDENTATION %d\n",class_name,indentation);
            char _class_name[strlen(class_name)-indentation+1];
            for(i=indentation;i<strlen(class_name);i++)
                _class_name[i-indentation]=class_name[i];
            _class_name[i] = '\0';
            
            pymodule_add_class(module,_class_name,indentation,class_start);            
            g_free(class_name);                   
            g_match_info_next(match_info_class,NULL);
            }                
        /***************************/
        g_match_info_free(match_info_class);
        g_regex_unref(regex_class);
        }
    int i=-1;
    //printf("MODULE CLASS  %d size\n",module->class_array_size)    
    regex_func = g_regex_new("([ \\ \\t]*\\bdef\\b\\s*.+)\\:",0,0,NULL); 
    if(g_regex_match(regex_func,text,0,&match_info_func))
    {   
        int class=0,class_prev=-1,indentation_prev=0;    
        
        GRegex *regex_def;
        GMatchInfo *match_info_def;
       
        regex_def = g_regex_new("\\bdef\\b\\s*",0,0,NULL);
    
        while(g_match_info_matches(match_info_func))
        {
            indentation = 0;                   
            char *word = g_match_info_fetch(match_info_func,0);
            for(i=0;i<strlen(word);i++)
            {
                if(isspace(word[i]))
                    indentation++;
                else
                    break;
                }
            int start,end;
           //printf("WORD %s INDENTATION %d\n",word,indentation);
            if (indentation==0)
            {
                //Add code to add functions here which are global functions
                g_match_info_fetch_pos(match_info_func,0,&start,&end);
                if(!g_regex_match(regex_def,word,0,&match_info_def))
                {
                    g_match_info_next(match_info_func,NULL); 
                    continue;
                    }
                int s=0,e=0;
                g_match_info_fetch_pos(match_info_def,0,&s,&e);
                g_match_info_free(match_info_def);              
        
                int index = e;
                i=0;
                char word1[strlen(word)-e+1];
                
                for(i=index;i<strlen(word)-1;i++)
                    word1[i-index] = word[i];
                word1[strlen(word)-index-1] = '\0';
                //printf("ADDING %s \n",word1);
                //printf("TO CLASS %s\n",module->class_array[0].class_name);
                pyclass_add_function(&module->class_array[0],word1,start);                
                }
            if (indentation>0)
            {                
                if(g_match_info_fetch_pos(match_info_func,0,&start,&end))
                {                    
                    for(class=module->class_array_size-1;class>0;class--) 
                    {
                        if(module->class_array[class].pos<start) 
                        {                            
                            //printf("CLASS INDENTATION %d FUNC INDENTATION %d\n",module->class_array[class].indentation,indentation);
                            if (word[indentation-1] == '\t' && module->class_array[class].indentation==indentation-1) //-1 for tabs 
                                    break; 
                            if (word[indentation-1] == ' ' && module->class_array[class].indentation==indentation-4) //4 for spaces remember to change it 
                                    break; 
                            }
                        }
                    //printf("CLASS DETECTED %d\n",class);
                    if (class == 0)
                    {
                        g_match_info_next(match_info_func,NULL); 
                        continue;
                        }
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
                    //printf("ADDING %s TO CLASS %s\n",word1,module->class_array[class].class_name);
                    if(class !=-1)                                        
                        pyclass_add_function(&module->class_array[class],word1,start);                                        
                    }
                class_prev = class;
                indentation_prev = indentation;
                }            
            g_free(word);  
            g_match_info_next(match_info_func,NULL);        
            }      
        g_match_info_free(match_info_func);
        g_regex_unref(regex_func);
        }
    i=-1;
    /*while(++i<module->class_array_size)
    {
        printf("MODULE CLASS %s\n",module->class_array[i].class_name);
        int j = -1;
        while(++j<module->class_array[i].function_array_size)
            printf("MODULE CLASS FUNCTION %s\n",module->class_array[i].function_array[j].func_name);
        }*/
    }

void get_import_files_class_and_members(CodeWidget * cwidget,char *filename)
{   
    if(cwidget->path_array==NULL)
    {
        int count = 0;
        cwidget->path_array = malloc(1*sizeof(char *));
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
        
        regex_path = g_regex_new("\'+.+?\'",0,0,NULL);
        if (g_regex_match(regex_path,output,0,&match_info))
        {
            while(g_match_info_matches(match_info))
            {         
                char *value = g_match_info_fetch(match_info,0);
                value[strlen(value)-1] = '\0';
                char path[strlen(value)-1];
                char *p = &path[0];
                value++;
                while(*p++=*value++);
                path[strlen(value)-2] = '\0';                
                asprintf(&cwidget->path_array[count],"%s",path);
                count++;
                cwidget->path_array = realloc(cwidget->path_array,(count+1)*sizeof(char *));
                g_match_info_next(match_info,NULL);
                }            
            }    
        cwidget->path_array[count] = NULL;            
        }
    
    int i = -1;
    char *contents, *filepath;
    int isFile = -1;
    gsize length;
    while(cwidget->path_array[++i]!=NULL)
    {
        asprintf(&filepath,"%s/%s.py",cwidget->path_array[i],filename);               
        if(g_file_test(filepath,G_FILE_TEST_IS_REGULAR))
        {           
            isFile = 1;
            break;
            }
        else 
        {            
            free(filepath);
            asprintf(&filepath,"%s/%s",cwidget->path_array[i],filename);       
            
            if(g_file_test(filepath,G_FILE_TEST_IS_DIR))
            {
                free(filepath);
                asprintf(&filepath,"%s/%s/__init__.py",cwidget->path_array[i],filename);
                
                isFile = -1;
                if(g_file_test(filepath,G_FILE_TEST_IS_REGULAR))
                {                    
                    isFile = 0;
                    free(filepath);
                    asprintf(&filepath,"%s/%s",cwidget->path_array[i],filename);
                    break;
                    }
                }  
            free(filepath);                      
            }
        }
    if(cwidget->path_array[i]==NULL)
        return;
    int module_index=0;
    if(isFile!=-1)
    {
        if(cwidget->module_array==NULL)
        {
            cwidget->module_array_size = 1;
            module_index  = 0;
            cwidget->module_array=(pymodule *)malloc(1*sizeof(pymodule));
            }
        else
        {
            int j=-1;
            while(++j<cwidget->module_array_size)
            {
                if(strcmp(cwidget->module_array[j].module_path,filepath))
                     return;
                }
            module_index = j;
            cwidget->module_array = (pymodule *)realloc(cwidget->module_array,(cwidget->module_array_size+1)*sizeof(pymodule));
            cwidget->module_array_size++;
            }
        
        pymodule_init(&cwidget->module_array[module_index],filename,filepath);
        }
    //Open each file in the module or open the file then get all the classes and its members
    char *text=NULL;
    gsize l=0;
    //printf("Filepath%s\n",filepath);
    if(strchr(filepath,'.')!=NULL)
    {
        g_file_get_contents(filepath,&text,&l,NULL);
        if(text==NULL)
            return;
        //printf("MODULE PATH%s NAME %s\n",cwidget->module_array[module_index].module_path,cwidget->module_array[module_index].module_name);
        add_from_import_file(&cwidget->module_array[module_index],text);            
        //printf("FREEING TEXT\n");
        g_free(text);
        //printf("FREED TEXT\n");
        }
    else
    {
        /*struct dirent *entry;
        struct stat statbuf;
        
        dp = opendir(filepath);
        chdir(fromdir);
        while((entry= readdir(dp)) !=NULL) 
        {
            lstat(entry->d_name,&statbuf);            
            if(!S_ISDIR(statbuf.st_mode))                
                printf("%s\n",entry->d_name);
            }*/
        }
   
    }

void codewidget_get_import_files_class(CodeWidget *cwidget, char *text)
{
    GRegex *regex_import = g_regex_new("\\bimport\\b\\s*\\w+.+",0,0,NULL);
    GMatchInfo *match_info_import;
    GRegex *regex_import_pos=g_regex_new("\\bimport\\b\\s*",0,0,NULL);
    GMatchInfo *match_info_import_pos;
    if(g_regex_match(regex_import,text,0,&match_info_import))
    {
        int import_pos_end,import_pos_start,i=0;
        char *match_text;        
        while(g_match_info_matches(match_info_import))
        {
            match_text=g_match_info_fetch(match_info_import,0);
            g_regex_match(regex_import_pos,text,0,&match_info_import_pos);
            g_match_info_fetch_pos(match_info_import_pos,0,&import_pos_start,&import_pos_end);
            GString *import_filename = g_string_new("");
            for(i=import_pos_end;i<strlen(match_text);i++)
            {
                if(match_text[i]!=',')
                    g_string_append_c(import_filename,match_text[i]);
                else
                {
                    //printf("FILENAME%s\n",import_filename->str);
                    //g_string_append(import_filename,".py");
                    get_import_files_class_and_members(cwidget,import_filename->str);
                    g_string_free(import_filename,TRUE);
                    import_filename = g_string_new("");                    
                    }
                }
            //printf("FILENAME%s\n",import_filename->str);
            //g_string_append(import_filename,".py");
            get_import_files_class_and_members(cwidget,import_filename->str);
            g_string_free(import_filename,TRUE);
            g_match_info_next(match_info_import,NULL);            
            }
        g_match_info_free(match_info_import);
        g_regex_unref(regex_import);
        }
    }

void codewidget_get_class_and_members(CodeWidget *cwidget)
{
    cwidget->pyclass_array = (pyclass *)malloc(1*sizeof(pyclass));
    
    pyclass_init(&cwidget->pyclass_array[0],"Global Scope",0,0);
    cwidget->pyclass_count=1;
    int i =-1;   
    
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(cwidget->txtinput));
    GtkTextIter start_iter, end_iter;
    gtk_text_buffer_get_start_iter(buffer,&start_iter);
    gtk_text_buffer_get_end_iter(buffer,&end_iter);
    char *text = gtk_text_buffer_get_text(buffer,&start_iter,&end_iter,FALSE);         
    
    //Code to get classes and its members in currently open file
    GRegex *regex_class,*regex_func;
    GMatchInfo *match_info_class,*match_info_func;
    int indentation=0;
    
    /***Also remember to create cwidget->class_indentation_array**/
    /*getting class names and their positions*/
    regex_class = g_regex_new("[ \\ \\t]*\\bclass\\b\\s*\\w+",0,0,NULL);
    
    if (g_regex_match(regex_class,text,0,&match_info_class))
    {       
        printf("Class Regex found\n");     
        cwidget->pyclass_array = (pyclass *)realloc(cwidget->pyclass_array,(cwidget->pyclass_count+1)*sizeof(pyclass));      
             
        int class_start=0,class_end=0;
        g_match_info_fetch_pos(match_info_class,0,&class_start,&class_end);
        
        char *class_name = g_match_info_fetch(match_info_class,0);
        pyclass_init(&cwidget->pyclass_array[cwidget->pyclass_count],class_name,class_start,0);
        g_free(class_name);
        cwidget->pyclass_count++; 
        g_match_info_next(match_info_class,NULL);

        
        
        while(g_match_info_matches(match_info_class))
        {
            indentation = 0;
            
            cwidget->pyclass_array = (pyclass *)realloc(cwidget->pyclass_array,(cwidget->pyclass_count+1)*sizeof(pyclass));
            
            class_name = g_match_info_fetch(match_info_class,0);
            //printf("CLASS NAME %s\n",class_name);
            int i;
            for(i=0;i<strlen(class_name);i++)
            {
                if(isspace(class_name[i]))
                    indentation++;
                else
                    break;
                }
          
            g_match_info_fetch_pos(match_info_class,0,&class_start,&class_end);
            
            pyclass_init(&cwidget->pyclass_array[cwidget->pyclass_count],class_name,class_start,indentation);
            g_free(class_name);
             i =-1;
            cwidget->pyclass_count++;
            g_match_info_next(match_info_class,NULL);
            }        
        
        /***************************/
        g_match_info_free(match_info_class);
        g_regex_unref(regex_class);
        }
    
    /**************for functions*********/
    regex_func = g_regex_new("([ \\ \\t]*\\bdef\\b\\s*.+)\\:",0,0,NULL); 
    
    if(g_regex_match(regex_func,text,0,&match_info_func))
    {   
        int class=0,class_prev=-1,indentation_prev=0;    
        
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
                    
                pyclass_add_function(&cwidget->pyclass_array[0],word1,start);                
                }
            if (indentation>0)
            {
                if(g_match_info_fetch_pos(match_info_func,0,&start,&end))
                {                   
                    for(class=cwidget->pyclass_count-1;class>0;class--)
                    {                        
                        if(cwidget->pyclass_array[class].pos<start)
                        {                           
                            if (word[indentation-1] == '\t' && cwidget->pyclass_array[class].indentation==indentation-1) //-1 for tabs
                                    break;
                            if (word[indentation-1] == ' ' && cwidget->pyclass_array[class].indentation==indentation-4) //4 for spaces remember to change it
                                    break;
                            }                    
                        }
                    if (class == 0)
                    {
                        g_match_info_next(match_info_func,NULL); 
                        continue;
                        }
                    g_match_info_fetch_pos(match_info_func,0,&start,&end);
                    if(!g_regex_match(regex_def,word,0,&match_info_def))
                    {
                        g_match_info_next(match_info_func,NULL); 
                        continue;
                        }
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
                       
                    if(class !=-1)                                        
                        pyclass_add_function(&cwidget->pyclass_array[class],word1,start);                                        
                    }
                class_prev = class;
                indentation_prev = indentation;
                }            
            g_match_info_next(match_info_func,NULL);   
            g_free(word);     
            }        
        g_match_info_free(match_info_func);
        g_regex_unref(regex_func);
        //g_regex_unref(regex_def);
        }   
    
    GRegex* regex_self = g_regex_new("[\\ \\t]*\\bself\\.+.+=",0,0,NULL);
    GMatchInfo* match_info_self;
    
     if(g_regex_match(regex_self,text,0,&match_info_self))
    {   
        int class=0,class_prev=-1,indentation_prev=0;    
        
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
                    for(class=cwidget->pyclass_count-1;class>0;class--)
                    {                        
                        //printf("CLASS POS %d start %d\n",cwidget->class_pos_array[class][0],start);
                        if(cwidget->pyclass_array[class].pos<start)
                        {                            
                            if (word[indentation-1] == '\t' && cwidget->pyclass_array[class].indentation==indentation-2*1) //-1 for tabs
                                    break;
                            if (word[indentation-1] == ' ' && cwidget->pyclass_array[class].indentation==indentation-2*4) //4 for spaces remember to change it
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
                        int b=-1;                             
                                                
                        while(++b<cwidget->pyclass_array[class].self_array_size)
                        {                            
                            //printf("SELF ARRAY%sWORD %s\n",cwidget->pyclass_array[class].self_array[b],word1);                            
                            if(strcmp(cwidget->pyclass_array[class].self_array[b],word1)==0)
                            {
                                g_match_info_next(match_info_self,NULL); 
                                break;
                                }
                            }
                        if(b!=cwidget->pyclass_array[class].self_array_size)
                            continue;
                        //printf("SELF %s\n",word1);                       
                        pyclass_add_self(&cwidget->pyclass_array[class],word1);                         
                        }                    
                    }
                class_prev = class;
                indentation_prev = indentation;
                }            
            g_match_info_next(match_info_self,NULL);        
            g_free(word);
            }               
        g_match_info_free(match_info_self);
        g_regex_unref(regex_self);
        }
    i =-1; 
    codewidget_get_import_files_class(cwidget,text); 
    int j;
    /*while(++i<cwidget->pyclass_count)
    {
        j=-1;
        printf("CLASS%s\n",cwidget->pyclass_array[i].class_name);
        printf("SIZE%d\n",cwidget->pyclass_array[i].function_array_size);
        while(++j<cwidget->pyclass_array[i].function_array_size)
        printf("FUNC%s\n",cwidget->pyclass_array[i].function_array[j].func_name);
        j=-1;
                while(++j<cwidget->pyclass_array[i].self_array_size)
        printf("self%s\n",cwidget->pyclass_array[i].self_array[j]);
        }    */
    
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
    i=-1;
    while(++i<cwidget->pyclass_count)    
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(cwidget->class_combo_box),cwidget->pyclass_array[i].class_name);
        
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
        i=-1;
        while(++i<cwidget->pyclass_array[0].function_array_size) 
            gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(cwidget->func_combo_box),cwidget->pyclass_array[0].function_array[i].func_name);        
 
        gtk_combo_box_set_active(GTK_COMBO_BOX(cwidget->func_combo_box),0);
        }
    }

void codewidget_destroy(CodeWidget *cwidget)
{
    if(GTK_IS_WIDGET(cwidget->txtinput))
        gtk_widget_destroy(cwidget->txtinput);
    if(GTK_IS_WIDGET(cwidget->func_combo_box))
        gtk_widget_destroy(cwidget->func_combo_box);
    if(GTK_IS_WIDGET(cwidget->class_combo_box))
        gtk_widget_destroy(cwidget->class_combo_box);
    if(GTK_IS_WIDGET(cwidget->fixed_txtinput))
        gtk_widget_destroy(cwidget->fixed_txtinput);
    if(GTK_IS_WIDGET(cwidget->list_view))
        gtk_widget_destroy(cwidget->list_view);
    if(GTK_IS_WIDGET(cwidget->hbox))
        gtk_widget_destroy(cwidget->hbox);
    codewidget_clear_arrays(cwidget);
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
    if(cwidget->pyclass_array==NULL)
        return;
    
    int class_count=cwidget->pyclass_count,member_count=0,i=-1,j=-1;
    
    while(--class_count)
    {
        member_count=cwidget->pyclass_array[class_count].function_array_size;
        while(--member_count)        
            free(cwidget->pyclass_array[class_count].function_array[member_count].func_name);            
            
        free(cwidget->pyclass_array[class_count].function_array);
        
        member_count = cwidget->pyclass_array[class_count].self_array_size;        
        while(--member_count)
            free(cwidget->pyclass_array[class_count].self_array[member_count]);
        
        free(cwidget->pyclass_array[class_count].self_array);        
        }
    free(cwidget->pyclass_array);
    
    int module_count = cwidget->module_array_size;    
    while(--module_count>=0)
    {
        free(cwidget->module_array[module_count].module_name);
        free(cwidget->module_array[module_count].module_path);
        class_count=cwidget->module_array[module_count].class_array_size;
    
        while(--class_count)
        {
            member_count=cwidget->module_array[module_count].class_array[class_count].function_array_size;
            while(--member_count)        
                free(cwidget->module_array[module_count].class_array[class_count].function_array[member_count].func_name);            
                
            free(cwidget->module_array[module_count].class_array[class_count].function_array);         
            }
        free(cwidget->module_array[module_count].class_array);
        }
    //cwidget->module_array_size=0;
    cwidget->module_array=NULL;
    free(cwidget->module_array);
    }