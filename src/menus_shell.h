/*557 Lines*/
#include<gtk/gtk.h>
#include<gtksourceview/gtksourceview.h>
#include<gtksourceview/gtksourcelanguage.h>
#include<gtksourceview/gtksourcelanguagemanager.h>
#include<string.h>
#include<stdlib.h>

#define BUF_SIZE 256
#define MAX_SNAME 1000
#define STDIN_FILENO 0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

GtkWidget *window, *txtInput,*txtfind,*txtreplace;
int min_cursor_offset = 0,offset=0;
char slaveName[MAX_SNAME];
int masterFd, scriptFd;
struct winsize ws;
fd_set inFds;
ssize_t numRead;
pid_t childPid;    
struct termios ttyOrig;
int indent_width = 4;
char *filepath="";
int shellPid=0;
gboolean read_masterFd(GIOChannel *channel, GIOCondition condition,gpointer data);
pid_t ptyFork(int *masterFd, char *slaveName, size_t snLen, struct termios *slaveTermios, const struct winsize *slaveWS);
GIOChannel *masterFd_channel;

//Shell menu starts here **********************************************

void run_run_module_activated(GtkWidget *widget)
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
    dialog = gtk_file_chooser_dialog_new("Open", GTK_WINDOW(window), GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,GTK_STOCK_OPEN,GTK_RESPONSE_ACCEPT,NULL);
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog),filefilter_py);
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog),filefilter_txt);
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog),filefilter_all);
    
    if (gtk_dialog_run(GTK_DIALOG(dialog))== GTK_RESPONSE_ACCEPT)
    {
        //restart the shell first
        char *filepath,*text;
        char *run_command;
        gssize length = -1;
        filepath = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        write(masterFd,"import os\n",10);
        char dir[strlen(filepath)];
        strcpy(dir,filepath);
        int i=strlen(filepath);
        while(filepath[--i]!='/');
        dir[i] = '\0';
        GtkTextBuffer *buffer;
        buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(txtInput));
        asprintf(&run_command,"os.chdir('%s')\n",dir);
        gtk_text_buffer_insert_at_cursor(buffer,"\n",1);
        write(masterFd,run_command,strlen(run_command));
        asprintf(&run_command,"execfile('%s')\n",filepath);
        write(masterFd,run_command,strlen(run_command));
        free(filepath);
        }
    gtk_widget_destroy(dialog);
    //after completing restart the shell
    }

void shell_restart_shell_activated(GtkWidget *widget)
{
    write(masterFd,"quit()\n",strlen("quit()\n"));
    GtkTextBuffer* buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(txtInput));
    gtk_text_buffer_insert_at_cursor(buffer,"\n",strlen("\n"));
    
    write(masterFd,"/usr/bin/python\n",strlen("/usr/bin/python\n"));
    }
//Shell menu ends here ************************************

//File menu starts here **************************************************************************

extern void cmdclose_clicked(GtkWidget *widget, GtkWidget *dialog)
{
    gtk_widget_destroy(dialog);
    }

void cmdimport_clicked(GtkWidget *widget, GtkEntry *txtmodule)
{
    char *module;
    asprintf(&module,"import %s\n",gtk_entry_get_text(txtmodule));
    write(masterFd,module, strlen(module));
    free(module);
    GtkTextBuffer *buffer;
    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(txtInput));
    gtk_text_buffer_insert_at_cursor(buffer,"\n",-1);
    }

void file_import_module_activated(GtkWidget *widget)
{
    GtkWidget *dialog, *cmdimport, *label, *txtmodule,*hbox,*cmdclose;
    
    dialog = gtk_dialog_new();
    label = gtk_label_new("The module name to search in sys.path");
    txtmodule = gtk_entry_new();
    cmdimport = gtk_button_new_with_mnemonic("OK");
    cmdclose = gtk_button_new_with_mnemonic("Close");
    
    hbox = gtk_hbox_new(FALSE,5);
    gtk_container_set_border_width(GTK_CONTAINER(hbox),10);
    gtk_box_pack_start_defaults(GTK_BOX(hbox),label);
    gtk_box_pack_start_defaults(GTK_BOX(hbox),txtmodule);
    gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox),hbox);
    
    gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->action_area),cmdimport);
    gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->action_area),cmdclose);
    g_signal_connect(G_OBJECT(cmdimport),"clicked",G_CALLBACK(cmdimport_clicked),GTK_ENTRY(txtmodule));
    g_signal_connect(G_OBJECT(cmdclose),"clicked",G_CALLBACK(cmdclose_clicked),(gpointer)dialog);
    
    gtk_window_set_modal(GTK_WINDOW(dialog),FALSE);
    gtk_window_set_title(GTK_WINDOW(dialog),"Import Module");
    gtk_widget_show_all(dialog);
    gtk_dialog_run(GTK_DIALOG(dialog));
    }

void file_savecopyas_activated(GtkWidget *widget, GtkWindow *parent)
{
    GtkWidget *dialog;
    GtkTextIter start_iter, end_iter;
    GtkTextBuffer *buffer;
    GtkSourceLanguageManager *languagemanager = gtk_source_language_manager_new();
    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(txtInput));
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

void file_saveas_activated(GtkWidget *widget, GtkWindow *parent)
{
    GtkWidget *dialog;
    GtkTextIter start_iter, end_iter;
    GtkTextBuffer *buffer;
    GtkSourceLanguageManager *languagemanager = gtk_source_language_manager_new();
    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(txtInput));
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
        gtk_text_buffer_get_end_iter(buffer,&end_iter);
        gtk_text_buffer_get_start_iter(buffer,&start_iter);
        text = gtk_text_buffer_get_text(buffer,&start_iter,&end_iter,FALSE);
        g_file_set_contents(filepath,text,length,NULL);
        GtkSourceLanguage *language = gtk_source_language_manager_guess_language(languagemanager,filepath,NULL);
         filename = strrchr(filepath,'/');
        *filename++;
        gtk_source_buffer_set_language(GTK_SOURCE_BUFFER(buffer),language);   
        g_free(filepath);
        g_free(text);
        }
    gtk_widget_destroy(dialog);
    }

void file_save_activated(GtkWidget *widget, GtkWindow *parent)
{     
    if (filepath == "")
    {
        file_saveas_activated(widget,parent);
        }
    else
    {
        gssize length = -1;
        GtkTextBuffer *buffer;
        GtkTextIter start_iter, end_iter;
        buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(txtInput));
        gtk_text_buffer_get_start_iter(buffer, &start_iter);
        gtk_text_buffer_get_end_iter(buffer, &end_iter);
        char *text;
        text = gtk_text_buffer_get_text(buffer,&start_iter,&end_iter,FALSE);
        g_file_set_contents(filepath,text,length,NULL);
        }
    }

void file_exit_activated(GtkWidget *widget,GtkWidget *dialog)
{
    if (GTK_IS_WIDGET(dialog) == TRUE)
    {
        gtk_widget_destroy(dialog);
        }
    gtk_widget_destroy(window);
    }

//File menu ends here ***************************************************************************

//Edit menu starts here **************************************************************************

void cmdsearch_clicked(GtkWidget *widget,GtkEntry *entry)
{
    static int counter = 0;
    const char *txt_regexp = gtk_entry_get_text(GTK_ENTRY(entry));
    GtkTextBuffer *buffer;
    buffer = GTK_TEXT_BUFFER(gtk_text_view_get_buffer(GTK_TEXT_VIEW(txtInput)));
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

void edit_reg_exp_search_activated(GtkWidget *widget)
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

void cmdgotoline_clicked(GtkWidget *widget, GtkEntry *entry)
{
    int linenumber = 0;
    const char *charlinenumber;
    charlinenumber = gtk_entry_get_text(entry);
    linenumber = atoi(charlinenumber);
    GtkTextIter line_iter;
    GtkTextBuffer *buffer;
    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(txtInput));
    gtk_text_buffer_get_iter_at_line(buffer,&line_iter,linenumber);
    gtk_text_buffer_select_range(buffer,&line_iter,&line_iter);
    gtk_text_view_scroll_to_iter(GTK_TEXT_VIEW(txtInput),&line_iter,0.1,FALSE,0.5,0.5);
    }

void edit_gotoline_activated(GtkWidget *widget)
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

void cmdfindprevious_clicked(GtkWidget *widget,GtkToggleButton *checkbutton)
{
    const gchar *textfind;
    textfind = gtk_entry_get_text(GTK_ENTRY(txtfind));
    GtkTextBuffer *buffer;
    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(txtInput));
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
            gtk_text_view_scroll_to_iter(GTK_TEXT_VIEW(txtInput),&match_end_iter,0.1,FALSE,0.5,0.5);
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
                gtk_text_view_scroll_to_iter(GTK_TEXT_VIEW(txtInput),&match_end_iter,0.1,FALSE,0.5,0.5);
                }                        
            }
        }
    }

void cmdfindnext_clicked(GtkWidget *widget,GtkToggleButton *checkbutton)
{
    GtkTextBuffer *buffer;
    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(txtInput));
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
            gtk_text_view_scroll_to_iter(GTK_TEXT_VIEW(txtInput),&match_end_iter,0.1,FALSE,0.5,0.5);
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
                gtk_text_view_scroll_to_iter(GTK_TEXT_VIEW(txtInput),&match_end_iter,0.1,FALSE,0.5,0.5);
                }                        
            }
        }
    }

void cmdfind_clicked(GtkWidget *widget,GtkToggleButton *checkbutton)
{
    const gchar *textfind;
    textfind = gtk_entry_get_text(GTK_ENTRY(txtfind));
    GtkTextBuffer *buffer;
    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(txtInput));
    GtkTextIter match_start_iter,match_end_iter;
    GtkTextIter start_iter;
    gtk_text_buffer_get_iter_at_offset(buffer,&start_iter,0);
    if(gtk_text_iter_forward_search(&start_iter,textfind,GTK_TEXT_SEARCH_TEXT_ONLY,&match_start_iter,&match_end_iter,NULL) == TRUE)
    {
        if(gtk_toggle_button_get_active(checkbutton) == FALSE)
        {
            gtk_text_buffer_select_range(buffer,&match_start_iter,&match_end_iter);
            offset = gtk_text_iter_get_offset(&match_end_iter);
            gtk_text_view_scroll_to_iter(GTK_TEXT_VIEW(txtInput),&match_end_iter,0.1,FALSE,0.5,0.5);
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
                gtk_text_view_scroll_to_iter(GTK_TEXT_VIEW(txtInput),&match_end_iter,0.1,FALSE,0.5,0.5);
                }                        
            }
        }
    }

void edit_find_activated(GtkWidget *widget)
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

void edit_undo_activated(GtkWidget *widget)
{
    GtkSourceBuffer *buffer;
    buffer = GTK_SOURCE_BUFFER(gtk_text_view_get_buffer(GTK_TEXT_VIEW(txtInput)));
    if (gtk_source_buffer_can_undo(buffer) == TRUE)
    {
        gtk_source_buffer_undo(buffer);
        }
    }

void edit_redo_activated(GtkWidget *widget)
{
    GtkSourceBuffer *buffer;
    buffer = GTK_SOURCE_BUFFER(gtk_text_view_get_buffer(GTK_TEXT_VIEW(txtInput)));
    if (gtk_source_buffer_can_redo(buffer) == TRUE)
    {
        gtk_source_buffer_redo(buffer);
        }
    }

void edit_copy_activated(GtkWidget *widget,GtkWidget *paste)
{
    GtkClipboard *clipboard;
    GtkTextBuffer *buffer;
    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(txtInput));
    clipboard = gtk_clipboard_get(GDK_NONE);
    gtk_text_buffer_copy_clipboard(buffer,clipboard);
    gtk_widget_set_sensitive(paste,TRUE);
    }

void edit_cut_activated(GtkWidget *widget,GtkWidget *paste)
{
    GtkClipboard *clipboard;
    GtkTextBuffer *buffer;
    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(txtInput));
    clipboard = gtk_clipboard_get(GDK_NONE);
    gtk_text_buffer_cut_clipboard(buffer,clipboard,TRUE);
    gtk_widget_set_sensitive(paste,TRUE);
    }

void edit_paste_activated(GtkWidget *widget)
{
    GtkClipboard *clipboard;
    GtkTextBuffer *buffer;
    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(txtInput));
    clipboard = gtk_clipboard_get(GDK_NONE);
    gtk_text_buffer_paste_clipboard(buffer,clipboard,NULL,TRUE);
    }

void edit_selectall_activated(GtkWidget *widget)
{
    GtkTextBuffer *buffer;
    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(txtInput));
    GtkTextIter start_iter,end_iter;
    gtk_text_buffer_get_start_iter(buffer,&start_iter);
    gtk_text_buffer_get_end_iter(buffer,&end_iter);
    gtk_text_buffer_select_range(buffer,&start_iter,&end_iter);
    }
//Edit Menu Ends Here ***********************************************************************
