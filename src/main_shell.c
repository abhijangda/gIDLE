/*938 Total Lines*/
/*381 Lines*/
#define _XOPEN_SOURCE 600
#include<stdlib.h>
#include<fcntl.h>
#include<string.h>
#include<errno.h>
#include<termios.h>
#include<sys/ioctl.h>
#include<sys/types.h>
#include<sys/select.h>
#include<unistd.h>
#include<stdio.h>
#include"menus_shell.h"

void destroy(GtkWidget *window);
gboolean delete_event(GtkWidget*, GdkEvent*);
void empty_stream();
GtkWidget *window, *txtInput;
static gboolean txtinput_key_press_event(GtkWidget *widget,GdkEventKey *event);
void gtk_text_buffer_append_output(GtkTextBuffer *buffer, char *text, int len);
gboolean read_masterFd(GIOChannel *channel, GIOCondition condition,gpointer data);
pid_t ptyFork(int *masterFd, char *slaveName, size_t snLen, struct termios *slaveTermios, const struct winsize *slaveWS);
static void ttyReset(void);

int main(int argc, char *argv[])
{  
    GtkWidget *menubar, *file, *edit, *shell, *run;
    GtkWidget *filemenu, *file_import_module, *file_save, *file_save_as, *file_save_copy_as, *file_exit;
    GtkWidget *editmenu, *edit_undo,*edit_redo,*edit_cut, *edit_copy, *edit_paste, *edit_selectall, *edit_find, *edit_go_to_line, *edit_reg_exp_search;
    GtkWidget *shellmenu, *shell_restart_shell;
    GtkWidget *runmenu, *run_run_module;
    GtkWidget *vbox;
    
    tcgetattr(STDIN_FILENO, &ttyOrig);    
    ioctl(STDIN_FILENO, TIOCGWINSZ, &ws);
   
    childPid = ptyFork(&masterFd, slaveName, MAX_SNAME, &ttyOrig, &ws);
    
    if (childPid == 0) 
    {  
        char *shell;
        if(argc>1)
        {
            shell = "/usr/bin/python";
            asprintf(&argv[0],"/usr/bin/python");           
            char *filepath;
            asprintf(&filepath,"%s",argv[1]);
            char dir[strlen(filepath)];
            strcpy(dir,filepath);
            int i=strlen(filepath);
            while(filepath[--i]!='/');
            dir[i] = '\0';
            chdir(dir);
            }        
        else if(argc==1)
        {
            shell = "/usr/bin/sh";
            asprintf(&argv[0],"/usr/bin/sh");
            }
        //execlp(shell, shell, (char *) NULL);
        execv(shell,argv);
        }  
        
    atexit(ttyReset);
    
    GtkSourceBuffer *buffer;
    GtkWidget *scrollwin;
    
    gtk_init(&argc, &argv);
    
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    buffer = gtk_source_buffer_new(NULL);
    txtInput = gtk_source_view_new_with_buffer(buffer);
    
    menubar = gtk_menu_bar_new();
    vbox = gtk_vbox_new(FALSE,2);
    
    filemenu = gtk_menu_new();
    file = gtk_menu_item_new_with_label("File");
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(file),filemenu);
    
    file_import_module = gtk_menu_item_new_with_label("Import Module");
    file_save = gtk_menu_item_new_with_label("Save");
    file_save_as = gtk_menu_item_new_with_label("Save As");
    file_save_copy_as = gtk_menu_item_new_with_label("Save Copy As");
    file_exit = gtk_menu_item_new_with_label("Exit");
    
    gtk_menu_shell_append(GTK_MENU_SHELL(filemenu),file_import_module);
    gtk_menu_shell_append(GTK_MENU_SHELL(filemenu),file_save);
    gtk_menu_shell_append(GTK_MENU_SHELL(filemenu),file_save_as);
    gtk_menu_shell_append(GTK_MENU_SHELL(filemenu),file_save_copy_as);
    gtk_menu_shell_append(GTK_MENU_SHELL(filemenu),file_exit);
    
    g_signal_connect(G_OBJECT(file_import_module),"activate",G_CALLBACK(file_import_module_activated),NULL);
    g_signal_connect(G_OBJECT(file_save_as),"activate",G_CALLBACK(file_saveas_activated),(gpointer)GTK_WINDOW(window));
    g_signal_connect(G_OBJECT(file_save),"activate",G_CALLBACK(file_save_activated),(gpointer)GTK_WINDOW(window));
    g_signal_connect(G_OBJECT(file_save_copy_as),"activate",G_CALLBACK(file_savecopyas_activated),(gpointer)GTK_WINDOW(window));
    g_signal_connect(G_OBJECT(file_exit),"activate",G_CALLBACK(file_exit_activated),NULL);     

    editmenu = gtk_menu_new();
    edit = gtk_menu_item_new_with_label("Edit");
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(edit),editmenu);
    
    edit_undo = gtk_menu_item_new_with_label("Undo");
    edit_redo = gtk_menu_item_new_with_label("Redo");
    edit_cut = gtk_menu_item_new_with_label("Cut");
    edit_copy = gtk_menu_item_new_with_label("Copy");
    edit_paste = gtk_menu_item_new_with_label("Paste");
    edit_selectall = gtk_menu_item_new_with_label("Select All");
    edit_find = gtk_menu_item_new_with_label("Find");
    edit_go_to_line = gtk_menu_item_new_with_label("Go To Line");
    edit_reg_exp_search = gtk_menu_item_new_with_label("Regular Expression Search");
    
    g_signal_connect(G_OBJECT(edit_undo),"activate",G_CALLBACK(edit_undo_activated),NULL);
    g_signal_connect(G_OBJECT(edit_redo),"activate",G_CALLBACK(edit_redo_activated),NULL);
    g_signal_connect(G_OBJECT(edit_cut),"activate",G_CALLBACK(edit_cut_activated),edit_paste);
    g_signal_connect(G_OBJECT(edit_paste),"activate",G_CALLBACK(edit_paste_activated),NULL);
    g_signal_connect(G_OBJECT(edit_copy),"activate",G_CALLBACK(edit_copy_activated),edit_paste);
    g_signal_connect(G_OBJECT(edit_find),"activate",G_CALLBACK(edit_find_activated),NULL);
    g_signal_connect(G_OBJECT(edit_reg_exp_search),"activate",G_CALLBACK(edit_reg_exp_search_activated),NULL);
    g_signal_connect(G_OBJECT(edit_selectall),"activate",G_CALLBACK(edit_selectall_activated),NULL);    
    
    gtk_menu_shell_append(GTK_MENU_SHELL(editmenu),edit_undo);
    gtk_menu_shell_append(GTK_MENU_SHELL(editmenu),edit_redo);
    gtk_menu_shell_append(GTK_MENU_SHELL(editmenu),edit_cut);
    gtk_menu_shell_append(GTK_MENU_SHELL(editmenu),edit_copy);
    gtk_menu_shell_append(GTK_MENU_SHELL(editmenu),edit_paste);
    gtk_menu_shell_append(GTK_MENU_SHELL(editmenu),edit_selectall);
    gtk_menu_shell_append(GTK_MENU_SHELL(editmenu),edit_find);
    gtk_menu_shell_append(GTK_MENU_SHELL(editmenu),edit_go_to_line);
    gtk_menu_shell_append(GTK_MENU_SHELL(editmenu),edit_reg_exp_search);
    
    shellmenu = gtk_menu_new();
    shell = gtk_menu_item_new_with_label("Shell");
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(shell),shellmenu);    
    
    shell_restart_shell  = gtk_menu_item_new_with_label("Restart Shell");
    gtk_menu_shell_append(GTK_MENU_SHELL(shellmenu),shell_restart_shell);
    
    g_signal_connect(G_OBJECT(shell_restart_shell),"activate",G_CALLBACK(shell_restart_shell_activated),NULL);
    
    runmenu = gtk_menu_new();
    run = gtk_menu_item_new_with_label("Run");
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(run),runmenu);        
    
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(run),runmenu);    
    
    run_run_module  = gtk_menu_item_new_with_label("Run Module");
    gtk_menu_shell_append(GTK_MENU_SHELL(runmenu),run_run_module);
    
    g_signal_connect(G_OBJECT(run_run_module),"activate",G_CALLBACK(run_run_module_activated),NULL);
    /*use import sys sys.argv = ['lkj','kj'] and then execfile('script.py')*/
    
    //g_signal_connect(G_OBJECT(run_run_module),"activated",G_CALLBACK(run_run_module_activated),NULL);
    
    gtk_menu_shell_append(GTK_MENU_SHELL(menubar),file);
    gtk_menu_shell_append(GTK_MENU_SHELL(menubar),edit);
    gtk_menu_shell_append(GTK_MENU_SHELL(menubar),shell);
    gtk_menu_shell_append(GTK_MENU_SHELL(menubar),run);
    
    GtkSourceLanguageManager *languagemanager = gtk_source_language_manager_new();
    GtkSourceLanguage *language = gtk_source_language_manager_guess_language(languagemanager,"abc.py",NULL);
    gtk_source_buffer_set_language(GTK_SOURCE_BUFFER(buffer),language); 

    scrollwin = gtk_scrolled_window_new(NULL,NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrollwin),GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC);    
    gtk_container_add(GTK_CONTAINER(scrollwin),txtInput);
       
    masterFd_channel = g_io_channel_unix_new(masterFd);
    g_io_add_watch(masterFd_channel,G_IO_IN,(GIOFunc)read_masterFd,NULL);
    
    gtk_box_pack_start(GTK_BOX(vbox),menubar,FALSE,FALSE,0);
    gtk_box_pack_start(GTK_BOX(vbox),scrollwin,TRUE,TRUE,0);
    gtk_container_add(GTK_CONTAINER(window),vbox);
    
    gtk_widget_set_size_request(window,600,400);
    gtk_window_set_title(GTK_WINDOW(window),"Python Shell");
    g_signal_connect(G_OBJECT(txtInput),"key-press-event",G_CALLBACK(txtinput_key_press_event),NULL);
    g_signal_connect(G_OBJECT(window),"destroy",G_CALLBACK(destroy),NULL);
    g_signal_connect(G_OBJECT(window),"delete_event",G_CALLBACK(delete_event),NULL);
    gtk_widget_show_all(window);      
    
     if (argc==1) //Run sh then run python
        write(masterFd, "/usr/bin/python\n", strlen("/usr/bin/python\n"));

    gtk_main();  
    
    return 0;
    }

static void ttyReset(void)
{
    if (tcsetattr(STDIN_FILENO, TCSANOW, &ttyOrig) == -1);
    }

pid_t ptyFork(int *masterFd, char *slaveName, size_t snLen, struct termios *slaveTermios, const struct winsize *slaveWS)
{
    int mfd, slaveFd, savedErrno;
    pid_t childPid;
    char slname[100];
    char *p;
    mfd = posix_openpt(O_RDWR | O_NOCTTY);     
    grantpt(mfd);     
    unlockpt(mfd);
    p = ptsname(mfd);
    strncpy(slname, p, 100);
    if (slaveName != NULL)
    {
        if (strlen(slname) < snLen) 
        {
            strncpy(slaveName, slname, snLen);
            }
        }
    
    childPid = fork();
    if (childPid != 0) 
    { 
        *masterFd = mfd; 
        return childPid; 
        } 
    
    setsid();
    close(mfd);
    slaveFd = open(slname, O_RDWR);  
    #ifdef TIOCSCTTY
    ioctl(slaveFd, TIOCSCTTY, 0);    
    #endif
    if (slaveTermios != NULL)    
    {
        slaveTermios->c_lflag &= ~ECHO;
        slaveTermios->c_lflag |= ECHOE;
        slaveTermios->c_lflag |= ECHOK;
        //slaveTermios->c_lflag |= ECHOKE;
        //slaveTermios->c_lflag |= ECHOCTL;
        slaveTermios->c_lflag |= ICANON;        
        slaveTermios->c_lflag |= ISIG;        
        slaveTermios->c_lflag |= IEXTEN;        
        slaveTermios->c_cflag |= HUPCL;
        slaveTermios->c_iflag |= BRKINT;
        slaveTermios->c_iflag |= ICRNL;
        slaveTermios->c_iflag |= IXON;
        slaveTermios->c_iflag |= IXANY;
        slaveTermios->c_iflag |= IUTF8;
        slaveTermios->c_iflag |= IMAXBEL;
        slaveTermios->c_oflag |= OPOST;
        slaveTermios->c_oflag |= ONLCR;
        cfsetspeed(slaveTermios,B38400);        
        slaveTermios->c_cc[VMIN]=1;
        if (tcsetattr(slaveFd, TCSANOW, slaveTermios) == -1);
        }
        
    if (slaveWS != NULL)    
        if (ioctl(slaveFd, TIOCSWINSZ, slaveWS) == -1);           
    
    dup2(slaveFd, 0);        
    dup2(slaveFd, 1);        
    dup2(slaveFd, 2);
        
    if (slaveFd > 2)
        close(slaveFd); 
    return 0;
    }

void gtk_text_buffer_append_output(GtkTextBuffer *buffer, char *text, int len)
{
    if (strcmp(text,"sh-4.2$ ")==0)
    {
        printf("sh-4.2$\n");
        return;
        }
    
    if(strcmp(text,"... ")==0)
    {
        GtkTextIter iter,prev_line_iter,prev_line_end_iter,current_line_iter;
        gtk_text_buffer_get_iter_at_mark(buffer,&iter,gtk_text_buffer_get_insert(buffer));
        min_cursor_offset = gtk_text_iter_get_offset(&iter);
        int prev_line = gtk_text_iter_get_line(&iter)-1,i;
        gtk_text_buffer_get_iter_at_line(buffer,&current_line_iter,gtk_text_iter_get_line(&iter));
        gtk_text_buffer_get_iter_at_line(buffer,&prev_line_iter,prev_line);
        gtk_text_buffer_get_iter_at_offset(buffer,&prev_line_end_iter,gtk_text_iter_get_offset(&current_line_iter)-1);
        char *line_text = gtk_text_buffer_get_text(buffer, &prev_line_iter, &prev_line_end_iter,FALSE);
        int space_count = 0;
        
        for(i=0;i<strlen(line_text);i++)
        {
            if(isspace(line_text[i]))
                space_count++;
            else
                break;
            }

        for (i=0;i<strlen(line_text);i++)
        {
            if(line_text[i] == ':')
                break;
            }
        if(i==strlen(line_text))
        {
            for (i=0;i<strlen(line_text);i++)
                if(isalnum(line_text[i]))
                    break;
            if (i!=strlen(line_text))
            {
                for(i=0;i<space_count;i++)
                    gtk_text_buffer_insert_at_cursor(buffer," ",-1);
                }
            return;
            }
        for(i=0;i<space_count+indent_width;i++)
            gtk_text_buffer_insert_at_cursor(buffer," ",-1);
        }
    else
    {
        gtk_text_buffer_insert_at_cursor(buffer,text,len);
        GtkTextIter iter;
        gtk_text_buffer_get_iter_at_mark(buffer,&iter,gtk_text_buffer_get_insert(buffer));
        min_cursor_offset = gtk_text_iter_get_offset(&iter);
        }
    //scroll to iter also
    }

gboolean read_masterFd(GIOChannel *channel, GIOCondition condition,gpointer data)
{
    char buf[BUF_SIZE];
    memset(&buf,0,sizeof(buf));
    numRead = read(masterFd, buf, BUF_SIZE);
    buf[numRead] = '\0';
    //printf("RECIEVING FROM PTY %s\n",buf);           
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(txtInput));
    gtk_text_buffer_append_output(buffer,buf,-1);  
    }

static gboolean txtinput_key_press_event(GtkWidget *widget,GdkEventKey *event)
{
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(widget));
    GtkTextIter current_iter,start_iter;
    gtk_text_buffer_get_iter_at_mark(buffer,&current_iter,gtk_text_buffer_get_insert(buffer));
    int current_offset = gtk_text_iter_get_offset(&current_iter);
    if (current_offset<min_cursor_offset)
        return TRUE;
    gtk_text_buffer_get_iter_at_offset(buffer,&start_iter,min_cursor_offset);
    char *text,*text2;
    asprintf(&text2,"");    
    switch(event->keyval)
    {
        case 65293: // 65293 for Return Key                        
            //printf("getting text\n");
            text = gtk_text_buffer_get_text(buffer,&start_iter,&current_iter,FALSE);
            asprintf(&text2,"%s\n",text);            
            if (strcmp(text2,"\n")==0)
            {
                GtkTextIter start_line_iter;
                gtk_text_buffer_get_iter_at_line(buffer,&start_line_iter,gtk_text_iter_get_line_index(&current_iter));
                text = gtk_text_buffer_get_text(buffer,&start_line_iter,&current_iter,FALSE);
                }
            gtk_text_buffer_insert_at_cursor(buffer,"\n",-1);            
            break;
        case 65288: //for Backspace key
            if(current_offset == min_cursor_offset)
                return TRUE;
            break;
        }
    if(strcmp(text2,"")==0)
        return FALSE;
    //printf("SENDING TO PTY %s\n",text2);
    write(masterFd, text2, strlen(text2));
    free(text2);   
    return TRUE;
    }

void destroy(GtkWidget *window)
{
    gtk_main_quit();
    }

gboolean delete_event(GtkWidget *window, GdkEvent *event)
{
    return FALSE;
    }   

