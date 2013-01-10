/*639 Lines*/
#include<gtk/gtk.h>
#include<string.h>
                    
GtkWidget *options_window;
GtkWidget *label_indentation, *check_button_indentation, *label_indentwidth, *txt_indentwidth, *check_button_show_line_numbers, *check_button_run_shell, *check_button_highlight_current_line;
GtkWidget *check_button_auto_save, *label_auto_save_time, *txt_auto_save_time,*check_button_recent_files,*lbltabwidth,*txttabwidth;
GtkWidget *chk_E,*chk_O,*chk_d,*chk_i,*chk_s,*chk_t,*chk_OO,*chk_W,*chk_v,*chk_S,*chk_B;
GtkWidget *entry_W,*entry_command,*lbl_command;
GtkWidget *vbox, *hbox1, *hbox2, *hbox3,*vbox_general,*fixed,*notebook_options;
GtkWidget *cmdOk, *cmdCancel, *cmdApply;
extern void options_window_destroyed(GtkWidget *options_window);

void check_button_auto_save_toggled(GtkToggleButton *widget)
{
    if (gtk_toggle_button_get_active(widget)==TRUE)
    {
        gtk_widget_set_sensitive(label_auto_save_time,TRUE);
        gtk_widget_set_sensitive(txt_auto_save_time,TRUE);
        }
    else
    {
        gtk_widget_set_state(label_auto_save_time,GTK_STATE_INSENSITIVE);
        gtk_widget_set_state(txt_auto_save_time,GTK_STATE_INSENSITIVE);
        }
    }

void check_button_indentation_toggled(GtkToggleButton *widget)
{
    if (gtk_toggle_button_get_active(widget)==TRUE)
    {
        gtk_widget_set_sensitive(label_indentwidth,TRUE);
        gtk_widget_set_sensitive(txt_indentwidth,TRUE);
        }
    else
    {
        gtk_widget_set_state(label_indentwidth,GTK_STATE_INSENSITIVE);
        gtk_widget_set_state(txt_indentwidth,GTK_STATE_INSENSITIVE);
        }
    }

void cmdCancel_clicked(GtkWidget *widget)
{
    gtk_widget_destroy(options_window);
    }

int save_settings(GtkWindow *window)
{
    GString *settings_string;
    
    settings_string = g_string_new("");
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button_indentation)) == TRUE)
    {
        if (strcmp(gtk_entry_get_text(GTK_ENTRY(txt_indentwidth)),"") == 0)
        {
            GtkWidget *dialog,*label;
        
            dialog = gtk_dialog_new_with_buttons("gIDLE",GTK_WINDOW(window),GTK_DIALOG_DESTROY_WITH_PARENT,GTK_STOCK_OK,GTK_RESPONSE_NONE,NULL);
            label = gtk_label_new("Please specify Indentation Width.");
            gtk_box_pack_start_defaults(GTK_BOX(GTK_DIALOG(dialog)->vbox),label);
            int result;
            gtk_widget_show_all(dialog);
            result = gtk_dialog_run(GTK_DIALOG(dialog));
            if (result == GTK_RESPONSE_NONE)
                gtk_widget_destroy(dialog);
            return -1;
            }
        else
        {
            g_string_append_len(settings_string,"<indentation>ENABLED</indentation>\n", strlen("<indentation>ENABLED</indentation>\n"));
            g_string_append_len(settings_string,"<indent width>", strlen("<indent width>"));
            g_string_append_len(settings_string,gtk_entry_get_text(GTK_ENTRY(txt_indentwidth)), strlen(gtk_entry_get_text(GTK_ENTRY(txt_indentwidth))));
            g_string_append_len(settings_string,"</indent width>\n", strlen("</indent width>\n"));
            }
        }
    else
        g_string_append_len(settings_string,"<indentation>DISABLED</indentation>\n", strlen("<indentation>DISABLED</indentation>\n"));
    
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button_auto_save)) == TRUE)
    {
        if (strcmp(gtk_entry_get_text(GTK_ENTRY(txt_auto_save_time)),"") == 0)
        {
            GtkWidget *dialog,*label;
        
            dialog = gtk_dialog_new_with_buttons("gIDLE",GTK_WINDOW(window),GTK_DIALOG_DESTROY_WITH_PARENT,GTK_STOCK_OK,GTK_RESPONSE_NONE,NULL);
            label = gtk_label_new("Please specify Auto Save Time.");
            gtk_box_pack_start_defaults(GTK_BOX(GTK_DIALOG(dialog)->vbox),label);
            int result;
            gtk_widget_show_all(dialog);
            result = gtk_dialog_run(GTK_DIALOG(dialog));
            if (result == GTK_RESPONSE_NONE)
                gtk_widget_destroy(dialog);
            return -1;
            }
        else
        {
            g_string_append_len(settings_string,"<auto save>ENABLED</auto save>\n", strlen("<auto save>ENABLED</auto save>\n"));
            g_string_append_len(settings_string,"<auto save time>", strlen("<auto save time>"));
            g_string_append_len(settings_string,gtk_entry_get_text(GTK_ENTRY(txt_auto_save_time)), strlen(gtk_entry_get_text(GTK_ENTRY(txt_auto_save_time))));
            g_string_append_len(settings_string,"</auto save time>\n", strlen("</auto save time>\n"));
            }
        }
    else
        g_string_append_len(settings_string,"<auto save>DISABLED</auto save>\n", strlen("<auto save>DISABLED</auto save>\n"));
        
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button_show_line_numbers)) == TRUE)
        g_string_append_len(settings_string,"<show line numbers>ENABLED</show line numbers>\n", strlen("<show line numbers>ENABLED</show line numbers>\n"));
    else
        g_string_append_len(settings_string,"<show line numbers>DISABLED</show line numbers>\n", strlen("<show line numbers>DISABLED</show line numbers>\n"));
    
     if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button_highlight_current_line)) == TRUE)
        g_string_append_len(settings_string,"<highlight current line>ENABLED</highlight current line>\n", strlen("<highlight current line>ENABLED</highlight current line>\n"));
    else
        g_string_append_len(settings_string,"<highlight current line>DISABLED</highlight current line>\n", strlen("<highlight current line>DISABLED</highlight current line>\n"));
        
     if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button_run_shell)) == TRUE)
        g_string_append_len(settings_string,"<run shell>ENABLED</run shell>\n", strlen("<run shell>ENABLED</run shell>\n"));
    else
        g_string_append_len(settings_string,"<run shell>DISABLED</run shell>\n", strlen("<run shell>DISABLED</run shell>\n"));
    
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button_recent_files)) == TRUE)
        g_string_append_len(settings_string,"<recentfiles>ENABLED</recentfiles>\n", strlen("<recentfiles>ENABLED</recentfiles>\n"));
    else
        g_string_append_len(settings_string,"<recentfiles>DISABLED</recentfiles>\n", strlen("<recentfiles>DISABLED</recentfiles>\n"));
        
    g_string_append(settings_string,"<command>");
    g_string_append(settings_string,gtk_entry_get_text(GTK_ENTRY(entry_command)));
    g_string_append(settings_string,"</command>\n");
    
    g_string_append(settings_string,"<tabwidth>");
    g_string_append(settings_string,gtk_entry_get_text(GTK_ENTRY(txttabwidth)));
    g_string_append(settings_string,"</tabwidth>\n");
    
    g_file_set_contents("./settings.ini",settings_string->str,-1,NULL);
    return 0;
    }

void cmdApply_clicked(GtkWidget *widget,GtkWindow *window)
{
    int i = save_settings(window);
    }

void cmdOk_clicked(GtkWidget *widget,GtkWindow *window)
{
    int i = save_settings(window);
    if (i==0)
        cmdCancel_clicked(widget);
    }

extern int find_string(char *str1, char *str2)
{
    gboolean found = FALSE;
    int i,j;
    for (i=0;i<strlen(str1);i++)
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

void chk_E_toggled(GtkWidget *widget,GtkEntry *entry)
{
    int i;
    GString *command_string;
    command_string = g_string_new(gtk_entry_get_text(GTK_ENTRY(entry)));
    if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)))
        g_string_append(command_string," -E");
    else
    {
        if ((i=find_string(command_string->str," -E"))!=-1)
            g_string_erase(command_string,i,sizeof(char)*3);
        }
    gtk_entry_set_text(GTK_ENTRY(entry),command_string->str);
    }

void chk_O_toggled(GtkWidget *widget,GtkEntry *entry)
{
    int i;
    GString *command_string;
    command_string = g_string_new(gtk_entry_get_text(GTK_ENTRY(entry)));
    if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)))
        g_string_append(command_string," -O");
    else
    {
        if ((i=find_string(command_string->str," -O"))!=-1)
            g_string_erase(command_string,i,sizeof(char)*3);
        }
    gtk_entry_set_text(GTK_ENTRY(entry),command_string->str);
    }

void chk_d_toggled(GtkWidget *widget,GtkEntry *entry)
{
    int i;
    GString *command_string;
    command_string = g_string_new(gtk_entry_get_text(GTK_ENTRY(entry)));
    if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)))
        g_string_append(command_string," -d");
    else
    {
        if ((i=find_string(command_string->str," -d"))!=-1)
            g_string_erase(command_string,i,sizeof(char)*3);
        }
    gtk_entry_set_text(GTK_ENTRY(entry),command_string->str);
    }

void chk_i_toggled(GtkWidget *widget,GtkEntry *entry)
{
    int i;
    GString *command_string;
    command_string = g_string_new(gtk_entry_get_text(GTK_ENTRY(entry)));
    if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)))
        g_string_append(command_string," -i");
    else
    {
        if ((i=find_string(command_string->str," -i"))!=-1)
            g_string_erase(command_string,i,sizeof(char)*3);
        }
    gtk_entry_set_text(GTK_ENTRY(entry),command_string->str);
    }

void chk_s_toggled(GtkWidget *widget,GtkEntry *entry)
{
    int i;
    GString *command_string;
    command_string = g_string_new(gtk_entry_get_text(GTK_ENTRY(entry)));
    if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)))
        g_string_append(command_string," -s");
    else
    {
        if ((i=find_string(command_string->str," -s"))!=-1)
            g_string_erase(command_string,i,sizeof(char)*3);
        }
    gtk_entry_set_text(GTK_ENTRY(entry),command_string->str);
    }

void chk_t_toggled(GtkWidget *widget,GtkEntry *entry)
{
    int i;
    GString *command_string;
    command_string = g_string_new(gtk_entry_get_text(GTK_ENTRY(entry)));
    if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)))
        g_string_append(command_string," -t");
    else
    {
        if ((i=find_string(command_string->str," -t"))!=-1)
            g_string_erase(command_string,i,sizeof(char)*3);
        }
    gtk_entry_set_text(GTK_ENTRY(entry),command_string->str);
    }

void chk_OO_toggled(GtkWidget *widget,GtkEntry *entry)
{
    int i;
    GString *command_string;
    command_string = g_string_new(gtk_entry_get_text(GTK_ENTRY(entry)));
    if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)))
        g_string_append(command_string," -OO");
    else
    {
        if ((i=find_string(command_string->str," -OO"))!=-1)
            g_string_erase(command_string,i,sizeof(char)*4);
        }
    gtk_entry_set_text(GTK_ENTRY(entry),command_string->str);
    }
/*****************Remember to edit -W option************/
void chk_W_toggled(GtkWidget *widget,GtkEntry *entry)
{
    int i;
    GString *command_string;
    command_string = g_string_new(gtk_entry_get_text(GTK_ENTRY(entry)));
    if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)))
        g_string_append(command_string,gtk_entry_get_text(GTK_ENTRY(entry_W)));
    else
    {
        if ((i=find_string(command_string->str," -W"))!=-1)
            g_string_erase(command_string,i,sizeof(char)*3);
        }
    gtk_entry_set_text(GTK_ENTRY(entry),command_string->str);
    }
/********************************************************/
void chk_v_toggled(GtkWidget *widget,GtkEntry *entry)
{
    int i;
    GString *command_string;
    command_string = g_string_new(gtk_entry_get_text(GTK_ENTRY(entry)));
    if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)))
        g_string_append(command_string," -V");
    else
    {
        if ((i=find_string(command_string->str," -V"))!=-1)
            g_string_erase(command_string,i,sizeof(char)*3);
        }
    gtk_entry_set_text(GTK_ENTRY(entry),command_string->str);
    }

void chk_S_toggled(GtkWidget *widget,GtkEntry *entry)
{
    int i;
    GString *command_string;
    command_string = g_string_new(gtk_entry_get_text(GTK_ENTRY(entry)));
    if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)))
        g_string_append(command_string," -S");
    else
    {
        if ((i=find_string(command_string->str," -S"))!=-1)
            g_string_erase(command_string,i,sizeof(char)*3);
        }
    gtk_entry_set_text(GTK_ENTRY(entry),command_string->str);
    }

void chk_B_toggled(GtkWidget *widget,GtkEntry *entry)
{
    int i;
    GString *command_string;
    command_string = g_string_new(gtk_entry_get_text(GTK_ENTRY(entry)));
    if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)))
        g_string_append(command_string," -B");
    else
    {
        if ((i=find_string(command_string->str," -B"))!=-1)
            g_string_erase(command_string,i,sizeof(char)*3);
        }
    gtk_entry_set_text(GTK_ENTRY(entry),command_string->str);
    }

extern void options_activated(GtkWidget *widget, GtkWindow *window)
{
    char *settings;
    gsize length = -1;
    GString *indentation,*indent_width,*auto_save,*auto_save_time,*show_line_numbers,*highlight_current_line,*run_shell,*command_string,*recent_files,*tabwidth;
    
    indentation = g_string_new("");
    indent_width = g_string_new("");
    auto_save = g_string_new("");
    auto_save_time = g_string_new("");
    show_line_numbers = g_string_new("");
    highlight_current_line = g_string_new("");
    run_shell = g_string_new("");
    command_string = g_string_new("");
    recent_files = g_string_new("");
    tabwidth = g_string_new("");
    
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
               
        opening_index = find_string(settings,"<auto save>") + strlen("<auto save>");
        closing_index = find_string(settings,"</auto save>");
        for(i=opening_index;i<closing_index;i++)
        {
            g_string_append_c(auto_save,settings[i]);
            }
        if (strcmp(auto_save->str,"ENABLED") == 0)
        {
            opening_index = find_string(settings,"<auto save time>") + strlen("<auto save time>");
            closing_index = find_string(settings,"</auto save time>");
            for(i=opening_index;i<closing_index;i++)
             {
                 g_string_append_c(auto_save_time,settings[i]);
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
        
        opening_index = find_string(settings,"<run shell>") + strlen("<run shell>");
        closing_index = find_string(settings,"</run shell>");
        for(i=opening_index;i<closing_index;i++)
        {
            g_string_append_c(run_shell,settings[i]);
            }
        
        opening_index = find_string(settings,"<command>") + strlen("<command>");
        closing_index = find_string(settings,"</command>");
        for(i=opening_index;i<closing_index;i++)
            g_string_append_c(command_string,settings[i]);
        
        opening_index = find_string(settings,"<recentfiles>") + strlen("<recentfiles>");
        closing_index = find_string(settings,"</recentfiles>");
        for(i=opening_index;i<closing_index;i++)
            g_string_append_c(recent_files,settings[i]);
            
        opening_index = find_string(settings,"<tabwidth>") + strlen("<tabwidth>");
        closing_index = find_string(settings,"</tabwidth>");
        for(i=opening_index;i<closing_index;i++)
            g_string_append_c(tabwidth,settings[i]);
        }

    options_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_widget_set_size_request(options_window,600,400);
    gtk_window_set_transient_for(GTK_WINDOW(options_window),window);
    gtk_window_set_modal(GTK_WINDOW(options_window),TRUE);
    gtk_window_set_title(GTK_WINDOW(options_window),"Options");
    
    vbox = gtk_vbox_new(FALSE,2);
    
    check_button_indentation = gtk_check_button_new_with_label("Enable/Disable Indentation");
    label_indentwidth = gtk_label_new("Indent Width");
    txt_indentwidth = gtk_entry_new();
    
    if (strcmp(indentation->str,"ENABLED") == 0)
    {
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button_indentation),TRUE);
        gtk_entry_set_text(GTK_ENTRY(txt_indentwidth),indent_width->str);
        }
    else
    {
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button_indentation),FALSE);
        gtk_widget_set_state(label_indentwidth,GTK_STATE_INSENSITIVE);
        gtk_widget_set_state(txt_indentwidth,GTK_STATE_INSENSITIVE);
        }
        
    g_signal_connect(G_OBJECT(check_button_indentation),"toggled",G_CALLBACK(check_button_indentation_toggled),NULL);
   
    check_button_show_line_numbers = gtk_check_button_new_with_label("Show Line Numbers");
     if (strcmp(show_line_numbers->str,"ENABLED") == 0)
         gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button_show_line_numbers),TRUE);
        
    check_button_run_shell = gtk_check_button_new_with_label("Run Python Shell at program startup");
    if (strcmp(run_shell->str,"ENABLED") == 0)
         gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button_run_shell),TRUE);
        
    check_button_highlight_current_line = gtk_check_button_new_with_label("Highlight Current Line");
    if (strcmp(highlight_current_line->str,"ENABLED") == 0)
         gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button_highlight_current_line),TRUE);
        
    check_button_auto_save = gtk_check_button_new_with_label("Enable/Disable Auto Save");
    g_signal_connect(G_OBJECT(check_button_auto_save),"toggled",G_CALLBACK(check_button_auto_save_toggled),NULL);
    label_auto_save_time = gtk_label_new("Time interval after which gIDLE saves file automatically");
    txt_auto_save_time = gtk_entry_new();
    if (strcmp(auto_save->str,"ENABLED") == 0)
    {
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button_auto_save),TRUE);
        gtk_entry_set_text(GTK_ENTRY(txt_auto_save_time),auto_save_time->str);
        }
    else
    {
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button_auto_save),FALSE);
        gtk_widget_set_state(label_auto_save_time,GTK_STATE_INSENSITIVE);
        gtk_widget_set_state(txt_auto_save_time,GTK_STATE_INSENSITIVE);
        }
    check_button_recent_files = gtk_check_button_new_with_label("Enable/Disable Recent Files");
    if(strcmp(recent_files->str,"ENABLED")==0)
         gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button_recent_files),TRUE);
    else
         gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button_highlight_current_line),FALSE);
    
    lbltabwidth = gtk_label_new("Tab Width");
    txttabwidth = gtk_entry_new();
    if (strcmp(tabwidth->str,"")!=0)
        gtk_entry_set_text(GTK_ENTRY(txttabwidth),tabwidth->str);
        
    cmdOk = gtk_button_new_with_mnemonic("_Ok");
    gtk_widget_set_size_request(cmdOk, 95,31);
    g_signal_connect(G_OBJECT(cmdOk),"clicked",G_CALLBACK(cmdOk_clicked),window);

    cmdCancel = gtk_button_new_with_mnemonic("_Cancel");
    gtk_widget_set_size_request(cmdCancel, 95,31);
    g_signal_connect(G_OBJECT(cmdCancel),"clicked",G_CALLBACK(cmdCancel_clicked),NULL);
    
    cmdApply = gtk_button_new_with_mnemonic("_Apply");
    gtk_widget_set_size_request(cmdApply, 95,31);
    g_signal_connect(G_OBJECT(cmdApply),"clicked",G_CALLBACK(cmdApply_clicked),window);
    
    vbox_general = gtk_vbox_new(FALSE,5);
    gtk_box_pack_start_defaults(GTK_BOX(vbox_general),check_button_indentation);
    
    hbox1 = gtk_hbox_new(FALSE,2);
    gtk_box_pack_start(GTK_BOX(hbox1),label_indentwidth,TRUE,TRUE,0);
    gtk_box_pack_start(GTK_BOX(hbox1),txt_indentwidth,FALSE,FALSE,0);
    gtk_box_pack_start_defaults(GTK_BOX(vbox_general),hbox1);
    
    GtkWidget *hbox_tabwidth = gtk_hbox_new(FALSE,2);
    gtk_box_pack_start(GTK_BOX(hbox_tabwidth),lbltabwidth,FALSE,FALSE,10);
    gtk_box_pack_start(GTK_BOX(hbox_tabwidth),txttabwidth,FALSE,FALSE,10);
    gtk_box_pack_start_defaults(GTK_BOX(vbox_general),hbox_tabwidth);
    
    gtk_box_pack_start_defaults(GTK_BOX(vbox_general),check_button_show_line_numbers);
    gtk_box_pack_start_defaults(GTK_BOX(vbox_general),check_button_run_shell);
    gtk_box_pack_start_defaults(GTK_BOX(vbox_general),check_button_highlight_current_line);
    gtk_box_pack_start_defaults(GTK_BOX(vbox_general),check_button_auto_save);
        
    hbox2 = gtk_hbox_new(FALSE,2);
    gtk_box_pack_start(GTK_BOX(hbox2),label_auto_save_time,TRUE,TRUE,0);
    gtk_box_pack_start(GTK_BOX(hbox2),txt_auto_save_time,FALSE,FALSE,0);
    gtk_box_pack_start_defaults(GTK_BOX(vbox_general),hbox2);
    
    gtk_box_pack_start_defaults(GTK_BOX(vbox_general),check_button_recent_files);
    
    hbox3 = gtk_hbox_new(FALSE,2);
    gtk_box_pack_end(GTK_BOX(hbox3),cmdApply,FALSE,FALSE,0);
    gtk_box_pack_end(GTK_BOX(hbox3),cmdOk,FALSE,FALSE,0);
    gtk_box_pack_end(GTK_BOX(hbox3),cmdCancel,FALSE,FALSE,0);
    
    notebook_options = gtk_notebook_new();
    GtkWidget *label = gtk_label_new("General");
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook_options),vbox_general,label);
    
    fixed = gtk_fixed_new();
    
    lbl_command = gtk_label_new("The following command will be executed:");
    gtk_widget_set_size_request(lbl_command,301,21);
    gtk_fixed_put(GTK_FIXED(fixed),lbl_command,7,254);
    
    entry_command = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(entry_command),command_string->str);
    gtk_widget_set_size_request(entry_command,582,31);
    gtk_fixed_put(GTK_FIXED(fixed),entry_command,5,287);
    
    int i;
    chk_E = gtk_check_button_new_with_label("Ignore Python env variables(-E)");
    gtk_widget_set_size_request(chk_E,231,26);
    gtk_fixed_put(GTK_FIXED(fixed),chk_E,4,10);
    g_signal_connect(G_OBJECT(chk_E),"toggled",G_CALLBACK(chk_E_toggled),entry_command);
    if ((i=find_string(command_string->str," -E"))!=-1)
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(chk_E),TRUE);
        
    chk_O = gtk_check_button_new_with_label("Optimize generated code slightly(-O)");
    gtk_widget_set_size_request(chk_O,281,26);
    gtk_fixed_put(GTK_FIXED(fixed),chk_O,4,40);
    g_signal_connect(G_OBJECT(chk_O),"toggled",G_CALLBACK(chk_O_toggled),entry_command);
    if ((i=find_string(command_string->str," -O"))!=-1)
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(chk_O),TRUE);
        
    chk_d = gtk_check_button_new_with_label("Debug output from parser(-d)");
    gtk_widget_set_size_request(chk_d,221,26);
    gtk_fixed_put(GTK_FIXED(fixed),chk_d,4,70);
    g_signal_connect(G_OBJECT(chk_d),"toggled",G_CALLBACK(chk_d_toggled),entry_command);
    if ((i=find_string(command_string->str," -d"))!=-1)
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(chk_d),TRUE);
        
    chk_i = gtk_check_button_new_with_label("Inspect interactively after running script(-i)");
    gtk_widget_set_size_request(chk_i,311,26);
    gtk_fixed_put(GTK_FIXED(fixed),chk_i,4,100);
    g_signal_connect(G_OBJECT(chk_i),"toggled",G_CALLBACK(chk_i_toggled),entry_command);
    if ((i=find_string(command_string->str," -i"))!=-1)
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(chk_i),TRUE);
        
    chk_s = gtk_check_button_new_with_label("Don't add user site directory to the sys.path(-s)");
    gtk_widget_set_size_request(chk_s,341,26);
    gtk_fixed_put(GTK_FIXED(fixed),chk_s,4,130);
    g_signal_connect(G_OBJECT(chk_s),"toggled",G_CALLBACK(chk_s_toggled),entry_command);
    if ((i=find_string(command_string->str," -s"))!=-1)
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(chk_s),TRUE);
        
    chk_t = gtk_check_button_new_with_label("Issue warning about inconsistent tab usage(-t)");
    gtk_widget_set_size_request(chk_t,331,26);
    gtk_fixed_put(GTK_FIXED(fixed),chk_t,4,160);
    g_signal_connect(G_OBJECT(chk_t),"toggled",G_CALLBACK(chk_t_toggled),entry_command);
    if ((i=find_string(command_string->str," -t"))!=-1)
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(chk_t),TRUE);
        
    chk_OO = gtk_check_button_new_with_label("Remove doc-strings in addition to the -O optimizations(-OO)");
    gtk_widget_set_size_request(chk_OO,424,26);
    gtk_fixed_put(GTK_FIXED(fixed),chk_OO,4,190);
    g_signal_connect(G_OBJECT(chk_OO),"toggled",G_CALLBACK(chk_OO_toggled),entry_command);
    if ((i=find_string(command_string->str," -OO"))!=-1)
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(chk_OO),TRUE);
        
    chk_W = gtk_check_button_new_with_label("Warning control(-W)");
    gtk_widget_set_size_request(chk_W,171,26);
    gtk_fixed_put(GTK_FIXED(fixed),chk_W,4,220);
    g_signal_connect(G_OBJECT(chk_W),"toggled",G_CALLBACK(chk_W_toggled),entry_command);
    
    entry_W = gtk_entry_new();
    gtk_widget_set_size_request(entry_W,131,31);
    gtk_fixed_put(GTK_FIXED(fixed),entry_W,174,218);
    
    chk_v = gtk_check_button_new_with_label("Verbose,trace import statements(-v)");
    gtk_widget_set_size_request(chk_v,282,26);
    gtk_fixed_put(GTK_FIXED(fixed),chk_v,285,10);
    g_signal_connect(G_OBJECT(chk_v),"toggled",G_CALLBACK(chk_v_toggled),entry_command);
    if ((i=find_string(command_string->str," -v"))!=-1)
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(chk_v),TRUE);
        
    chk_S = gtk_check_button_new_with_label("Don't imply 'import site' on initialization(-S)");
    gtk_widget_set_size_request(chk_S,312,26);
    gtk_fixed_put(GTK_FIXED(fixed),chk_S,285,40);
    g_signal_connect(G_OBJECT(chk_S),"toggled",G_CALLBACK(chk_S_toggled),entry_command);
    if ((i=find_string(command_string->str," -S"))!=-1)
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(chk_S),TRUE);
        
    chk_B = gtk_check_button_new_with_label("Don't write .py files on import(-B)");
    gtk_widget_set_size_request(chk_B,252,26);
    gtk_fixed_put(GTK_FIXED(fixed),chk_B,285,70);
    g_signal_connect(G_OBJECT(chk_B),"toggled",G_CALLBACK(chk_B_toggled),entry_command);
    if ((i=find_string(command_string->str," -B"))!=-1)
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(chk_B),TRUE);
        
    GtkWidget *label2 = gtk_label_new("Python Interpreter Options");
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook_options),fixed,label2);
    
    gtk_box_pack_start(GTK_BOX(vbox),notebook_options,TRUE,TRUE,2);
    gtk_box_pack_start(GTK_BOX(vbox),hbox3,FALSE,FALSE,2);
    g_signal_connect(G_OBJECT(options_window),"destroy",G_CALLBACK(options_window_destroyed),NULL);
    gtk_container_add(GTK_CONTAINER(options_window),vbox);
    gtk_widget_show_all(options_window);
    }