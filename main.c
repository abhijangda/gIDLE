/*6999 Total Lines*/
/*856 Lines*/
#include<gdk/gdkkeysyms.h>
#include<gtksourceview/gtksourceview.h>
#include<gtksourceview/gtksourcelanguage.h>
#include<gtksourceview/gtksourcelanguagemanager.h>
#include"menus.h"
//button press and mark set not working in project

GtkSourceBuffer *buffer;
int filenotsavedarray[MAX_TABS]; 
static void destroy(GtkWidget *);
static gboolean delete_event(GtkWidget *,GdkEvent *);
void notebook_page_added(GtkWidget *,GtkWidget *, guint , GtkWidget *);
void textchanged(GtkWidget *);
static void exitdialog_cmdsave_clicked(GtkWidget *widget, GtkWidget *widgetarray[]);
gboolean txtinput_button_release_event(GtkWidget *widget,GdkEvent *event);
void cmdcancel_activated(GtkWidget *,GtkWidget *);
int filecheckbuttonsarray_size =0;
extern void options_activated(GtkWidget *widget,GtkWidget *window);
extern int find_string(char *str1, char *str2);
extern void options_window_destroyed(GtkWidget *options_window);
void func_load_settings();
void func_auto_save();
extern void new_proj_activated(GtkWidget *widget, GtkWindow *parent);
extern void new_proj_window_destroyed(GtkWidget *widget);
extern void proj_manager_destroyed(char *filearray[],GString *main_file,GString *projname);
extern void openshell_activated(GtkWidget *widget);
void toolbar_combo_class_changed(GtkComboBox *combo_box);
void toolbar_combo_func_changed(GtkComboBox *combo_box);
void path_browser_activated(GtkWidget *widget, GtkWindow *window);

int main(int argc, char *argv[])
{    
    GtkWidget *menubar, *file, *edit, *format, *tools, *navigation,*filemenu, *editmenu,*formatmenu, *toolsmenu, *runmenu,*navigationmenu,*label1,*vbox,*hbox;
    GtkWidget *filenewmenu,*filenew, *new_proj, *new_file, *fileopenmenu, *open_proj, *open_file, *open, *save, *saveas, *savecopyas, *saveall, *saveasall, *restore, *newtab,*removetab,*removealltab,*filemenu_separator,*filemenu_separator1,*exit;
    GtkWidget *addfilemenu,*addfile_existing, *addfile_new,*addfile_mainmenu,*addfile_main_existing, *addfile_main_new;
    GtkWidget *paste,*findandreplace,*find,*find_selected,*find_in_file,*reg_exp_search,*selectall,*editmenu_separator,*editmenu_separator2,*editmenu_separator3,*show_parenthesis;
    GtkWidget *indentregion,*dedentregion,*formatmenu_separator1,*commentout,*uncommentout,*formatmenu_separator2,*tabify_region,*untabify_region,*formatmenu_separator3,*uppercase_selection,*lowercase_selection,*formatmenu_separator4,*striptrailingspaces;
    GtkWidget *gnome_terminal, *options, *pdb, *file_information,*class_browser, *proj_manager,*path_browser;
    GtkWidget *back_line,*forward_line,*navigation_separator1,*beggining_line,*end_line,*navigation_separator2,*gotoline,*navigation_separator3,*add_bookmark,*navigation_separator4;
    GtkWidget *toolbar;
    GtkWidget *run,*runmodule;
    GtkToolItem *toolbar_new,*toolbar_open,*toolbar_save,*toolbar_cut,*toolbar_copy,*toolbar_paste,*toolbar_selectall,*toolbar_find,*toolbar_findandreplace,*toolbar_undo;
    GtkToolItem *toolbar_redo,*toolbar_separator,*toolbar_saveas,*toolbar_separator2,*toolbar_indent,*toolbar_dedent,*toolbar_close,*toolbar_preferences,*toolbar_separator3;
    GtkToolItem *toolbar_beginning_line, *toolbar_end_line, *toolbar_back_line, *toolbar_forward_line,*toolbar_goto_line,*toolbar_separator4,*toolbar_separator5;
    GtkToolItem *toolbar_python_shell, *toolbar_run;
    GtkToolItem *toolbar_combo_class_item,*toolbar_combo_func_item;
    GtkWidget *toolbar_new_menu,*toolbar_open_menu,*toolbar_new_file,*toolbar_new_proj,*toolbar_open_file,*toolbar_open_proj;
    GtkAccelGroup *group;
    int i;
    
    gtk_init(&argc, &argv);
    
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window),"gIDLE");
    gtk_widget_set_size_request(window,600,400);
    
    group = gtk_accel_group_new();
    menubar = gtk_menu_bar_new();
    
    /***********Initialization of all arrays and their elements ********/
    for (i=0;i<MAX_TABS;i++)
    {        
        filepathsarray[i] = "";
        int j=0;
        for(j=0;j<5;j++)
            lines_track_array[i][j] = -1;
        bookmarks_line_array[i] = -1;
        bookmarks_tabindex_array[i] = -1;
        }
    codewidget_new(&codewidget[0]);
    //buffer = GTK_SOURCE_BUFFER(gtk_source_buffer_new(NULL));
    //codewidget[i].txtinput = gtk_source_view_new_with_buffer(buffer);        
    vbox_txtinput[0] = gtk_vbox_new(FALSE,2);
    //codewidget[i]->fixed_txtinput = gtk_fixed_new();
    //codewidget[i].func_combo_cox = gtk_combo_box_text_new();
    codewidget[0].class_combo_box = gtk_combo_box_text_new();
    scrollwin[0] = gtk_scrolled_window_new(NULL,NULL);
        
    /**************************************************/
    
    notebook = gtk_notebook_new();
    
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrollwin[0]),GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(codewidget[0].txtinput),codewidget[0].fixed_txtinput);
    gtk_container_add(GTK_CONTAINER(scrollwin[0]),codewidget[0].txtinput);
    GtkWidget *hbox_combo;    
    hbox_combo = gtk_hbox_new(FALSE,2);
    gtk_box_pack_start(GTK_BOX(hbox_combo),codewidget[0].class_combo_box,FALSE,FALSE,0);    
    gtk_box_pack_start(GTK_BOX(hbox_combo),codewidget[0].func_combo_box,FALSE,FALSE,0);
    gtk_box_pack_start(GTK_BOX(vbox_txtinput[0]),hbox_combo,FALSE,FALSE,0);
    gtk_box_pack_start(GTK_BOX(vbox_txtinput[0]),scrollwin[0],TRUE,TRUE,0);
    g_signal_connect(G_OBJECT(codewidget[0].class_combo_box),"changed",G_CALLBACK(toolbar_combo_class_changed),NULL); 
    g_signal_connect(G_OBJECT(codewidget[0].func_combo_box),"changed",G_CALLBACK(toolbar_combo_func_changed),NULL);    
    
    proj_tree_view = gtk_tree_view_new();
        
    g_signal_connect(G_OBJECT(codewidget[0].txtinput),"key-press-event",G_CALLBACK(txtinput_key_press_event),NULL);
    g_signal_connect(G_OBJECT(codewidget[0].txtinput),"button-release-event",G_CALLBACK(txtinput_button_release_event),NULL);
    g_signal_connect(G_OBJECT(gtk_text_view_get_buffer(GTK_TEXT_VIEW(codewidget[0].txtinput))),"mark-set",G_CALLBACK(txtinput_mark_set),NULL);
        
    label1 = gtk_label_new("New File");
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook),vbox_txtinput[0],label1);
    
    vbox = gtk_vbox_new(FALSE,2);
    gtk_box_pack_start(GTK_BOX(vbox),menubar,FALSE,FALSE,0); /* Expand set to FALSE,Hence Fill also to FALSE*/
   
    toolbar = gtk_toolbar_new();
    gtk_box_pack_start(GTK_BOX(vbox),toolbar,FALSE,FALSE,0);
    
    hbox = gtk_hbox_new(FALSE,2);    
    gtk_box_pack_start(GTK_BOX(hbox),proj_tree_view,FALSE,FALSE,0);
    
    gtk_box_pack_start(GTK_BOX(hbox),notebook,TRUE,TRUE,0); /* Expand and Fill set to TRUE*/    
    gtk_box_pack_start(GTK_BOX(vbox),hbox,TRUE,TRUE,0);
    
    
    
    statusbar = gtk_statusbar_new();
    gtk_box_pack_start(GTK_BOX(vbox),statusbar,FALSE,FALSE,0);
    
    gtk_toolbar_set_show_arrow(GTK_TOOLBAR(toolbar),TRUE);
    gtk_toolbar_set_style(GTK_TOOLBAR(toolbar),GTK_TOOLBAR_ICONS);
    gtk_toolbar_set_tooltips(GTK_TOOLBAR(toolbar),TRUE);
    
    /*Toolbar*/
    /*toolbar_new = gtk_menu_tool_button_new_from_stock(GTK_STOCK_NEW);*/
    /*toolbar_open = gtk_menu_tool_button_new_from_stock(GTK_STOCK_OPEN);*/
    toolbar_new = gtk_tool_button_new_from_stock(GTK_STOCK_NEW);
    gtk_tool_item_set_tooltip_text(toolbar_new,"Create a new file");
    toolbar_open = gtk_tool_button_new_from_stock(GTK_STOCK_OPEN);
    gtk_tool_item_set_tooltip_text(toolbar_new,"Open a file");
    toolbar_save = gtk_tool_button_new_from_stock(GTK_STOCK_SAVE);
    gtk_tool_item_set_tooltip_text(toolbar_new,"Save current file");
    toolbar_saveas = gtk_tool_button_new_from_stock(GTK_STOCK_SAVE_AS);
    gtk_tool_item_set_tooltip_text(toolbar_new,"Save as current file");
    toolbar_separator = gtk_separator_tool_item_new();
    toolbar_undo = gtk_tool_button_new_from_stock(GTK_STOCK_UNDO);
    gtk_tool_item_set_tooltip_text(toolbar_new,"Undo last operation");
    toolbar_redo = gtk_tool_button_new_from_stock(GTK_STOCK_REDO);
    gtk_tool_item_set_tooltip_text(toolbar_new,"Redo last operation");
    toolbar_cut = gtk_tool_button_new_from_stock(GTK_STOCK_CUT);
    gtk_tool_item_set_tooltip_text(toolbar_new,"Cut the currently selected text");
    toolbar_copy = gtk_tool_button_new_from_stock(GTK_STOCK_COPY);
    gtk_tool_item_set_tooltip_text(toolbar_new,"Copy the currently selected text");
    toolbar_paste = gtk_tool_button_new_from_stock(GTK_STOCK_PASTE);
    gtk_tool_item_set_tooltip_text(toolbar_new,"Paste the text into clipboard");
    toolbar_selectall= gtk_tool_button_new_from_stock(GTK_STOCK_SELECT_ALL);
    gtk_tool_item_set_tooltip_text(toolbar_new,"Select all the text");
    toolbar_find = gtk_tool_button_new_from_stock(GTK_STOCK_FIND);
    gtk_tool_item_set_tooltip_text(toolbar_new,"Find in the text");
    toolbar_findandreplace = gtk_tool_button_new_from_stock(GTK_STOCK_FIND_AND_REPLACE);
    gtk_tool_item_set_tooltip_text(toolbar_new,"Find or Replace in the text");
    toolbar_separator2 = gtk_separator_tool_item_new();   
    toolbar_indent = gtk_tool_button_new_from_stock(GTK_STOCK_INDENT);
    gtk_tool_item_set_tooltip_text(toolbar_new,"Increase indentation of text");
    toolbar_dedent = gtk_tool_button_new_from_stock(GTK_STOCK_UNINDENT);
    gtk_tool_item_set_tooltip_text(toolbar_new,"Decrease indentation of text");
    toolbar_separator3=gtk_separator_tool_item_new();    
    toolbar_beginning_line = gtk_tool_button_new_from_stock(GTK_STOCK_GOTO_TOP);
    gtk_tool_item_set_tooltip_text(toolbar_new,"Go to beginning of the document");
    toolbar_end_line=gtk_tool_button_new_from_stock(GTK_STOCK_GOTO_BOTTOM);
    gtk_tool_item_set_tooltip_text(toolbar_new,"Go to bottom of the document");
    toolbar_back_line=gtk_tool_button_new_from_stock(GTK_STOCK_GO_BACK);
    gtk_tool_item_set_tooltip_text(toolbar_new,"Go to the last line in the history");
    toolbar_forward_line=gtk_tool_button_new_from_stock(GTK_STOCK_GO_FORWARD);
    gtk_tool_item_set_tooltip_text(toolbar_new,"Go to next line in the history");
    toolbar_goto_line = gtk_tool_button_new_from_stock(GTK_STOCK_JUMP_TO);
    gtk_tool_item_set_tooltip_text(toolbar_new,"Go to a specified line");
    toolbar_separator4=gtk_separator_tool_item_new();
    toolbar_python_shell=gtk_tool_button_new(gtk_image_new_from_pixbuf(gdk_pixbuf_new_from_file("/media/sda11/C/GTK+/gIDLE/python_shell.png",NULL)),"Open Python Shell");
    toolbar_run=gtk_tool_button_new(gtk_image_new_from_pixbuf(gdk_pixbuf_new_from_file("/media/sda11/C/GTK+/gIDLE/run.png",NULL)),"Run");
    toolbar_separator5=gtk_separator_tool_item_new();
    toolbar_preferences=gtk_tool_button_new_from_stock(GTK_STOCK_PREFERENCES);
    gtk_tool_item_set_tooltip_text(toolbar_new,"Opens Option Dialog");
    toolbar_close=gtk_tool_button_new_from_stock(GTK_STOCK_CLOSE);    
    gtk_tool_item_set_tooltip_text(toolbar_new,"Exit gIDLE");
    
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar),toolbar_new,0);
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar),toolbar_open,1);
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar),toolbar_save,2);
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar),toolbar_saveas,3);
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar),toolbar_separator,4);
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar),toolbar_undo,5);
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar),toolbar_redo,6);
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar),toolbar_cut,7);
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar),toolbar_copy,8);
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar),toolbar_paste,9); 
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar),toolbar_selectall,10); 
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar),toolbar_find,11);
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar),toolbar_findandreplace,12);
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar),toolbar_separator2,13);
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar),toolbar_indent,14);
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar),toolbar_dedent,15);    
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar),toolbar_separator3,16);
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar),toolbar_beginning_line,17);
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar),toolbar_end_line,18);
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar),toolbar_back_line,19);
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar),toolbar_forward_line,20);    
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar),toolbar_goto_line,21);
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar),toolbar_separator4,22);
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar),toolbar_run,23);
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar),toolbar_python_shell,24);
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar),toolbar_separator5,25);
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar),toolbar_preferences,26);    
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar),toolbar_close,27);    
        
    /*Create Menu Item of MenuToolButton*/
    /*Uncomment these lines for menutoolbutton*/
    /*toolbar_new_menu = gtk_menu_new();*/
    /*toolbar_open_menu = gtk_menu_new();*/
    /*toolbar_new_file = gtk_menu_item_new_with_label("Create New File");*/
    /*toolbar_new_proj = gtk_menu_item_new_with_label("Create New Project");*/
    /*toolbar_open_file = gtk_menu_item_new_with_label("Open File");*/
    /*toolbar_open_proj = gtk_menu_item_new_with_label("Open Project");*/
    /*gtk_menu_tool_button_set_menu(GTK_MENU_TOOL_BUTTON(toolbar_new),toolbar_new_menu);*/
    /*gtk_menu_tool_button_set_menu(GTK_MENU_TOOL_BUTTON(toolbar_open),toolbar_open_menu);*/
    
    /*gtk_menu_shell_append(GTK_MENU_SHELL(toolbar_new_menu),toolbar_new_file);*/
    /*gtk_menu_shell_append(GTK_MENU_SHELL(toolbar_new_menu),toolbar_new_proj);*/
    /*gtk_menu_shell_append(GTK_MENU_SHELL(toolbar_open_menu),toolbar_open_file);*/
    /*gtk_menu_shell_append(GTK_MENU_SHELL(toolbar_open_menu),toolbar_open_proj);*/
    /*g_signal_connect(G_OBJECT(toolbar_open_file),"activate",G_CALLBACK(open_file_activated),(gpointer)GTK_WINDOW(window));*/
    /*g_signal_connect(G_OBJECT(toolbar_open_proj),"activate",G_CALLBACK(open_proj_activated),(gpointer)GTK_WINDOW(window));*/
    /*g_signal_connect(G_OBJECT(toolbar_new_file),"activate",G_CALLBACK(filenew_activated),NULL);*/
    /*g_signal_connect(G_OBJECT(toolbar_new_proj),"activate",G_CALLBACK(new_proj_activated),(gpointer)GTK_WINDOW(window));*/  
    
    g_signal_connect(G_OBJECT(toolbar_open),"clicked",G_CALLBACK(open_file_activated),NULL);
    g_signal_connect(G_OBJECT(toolbar_new),"clicked",G_CALLBACK(filenew_activated),NULL);
    g_signal_connect(G_OBJECT(toolbar_undo),"clicked",G_CALLBACK(undo_activated),NULL);
    g_signal_connect(G_OBJECT(toolbar_redo),"clicked",G_CALLBACK(redo_activated),NULL);
    g_signal_connect(G_OBJECT(toolbar_cut),"clicked",G_CALLBACK(cut_activated),NULL);
    g_signal_connect(G_OBJECT(toolbar_paste),"clicked",G_CALLBACK(paste_activated),NULL);
    g_signal_connect(G_OBJECT(toolbar_copy),"clicked",G_CALLBACK(copy_activated),NULL); 
    g_signal_connect(G_OBJECT(toolbar_selectall),"clicked",G_CALLBACK(selectall_activated),NULL);
    g_signal_connect(G_OBJECT(toolbar_find),"clicked",G_CALLBACK(find_activated),NULL);
    g_signal_connect(G_OBJECT(toolbar_findandreplace),"clicked",G_CALLBACK(findandreplace_activated),NULL);
    g_signal_connect(G_OBJECT(toolbar_save),"clicked",G_CALLBACK(save_activated),(gpointer)GTK_WINDOW(window));
    g_signal_connect(G_OBJECT(toolbar_saveas),"clicked",G_CALLBACK(saveas_activated),(gpointer)GTK_WINDOW(window));
    g_signal_connect(G_OBJECT(toolbar_indent),"clicked",G_CALLBACK(indentregion_activated),(gpointer)GTK_WINDOW(window));
    g_signal_connect(G_OBJECT(toolbar_dedent),"clicked",G_CALLBACK(dedentregion_activated),(gpointer)GTK_WINDOW(window));    
    g_signal_connect(G_OBJECT(toolbar_beginning_line),"clicked",G_CALLBACK(beggining_line_activated),(gpointer)GTK_WINDOW(window));
    g_signal_connect(G_OBJECT(toolbar_end_line),"clicked",G_CALLBACK(end_line_activated),(gpointer)GTK_WINDOW(window));
    g_signal_connect(G_OBJECT(toolbar_back_line),"clicked",G_CALLBACK(back_line_activated),(gpointer)GTK_WINDOW(window));
    g_signal_connect(G_OBJECT(toolbar_forward_line),"clicked",G_CALLBACK(forward_line_activated),(gpointer)GTK_WINDOW(window));    
    g_signal_connect(G_OBJECT(toolbar_goto_line),"clicked",G_CALLBACK(gotoline_activated),(gpointer)GTK_WINDOW(window));    
    g_signal_connect(G_OBJECT(toolbar_python_shell),"clicked",G_CALLBACK(openshell_activated),(gpointer)GTK_WINDOW(window));    
    g_signal_connect(G_OBJECT(toolbar_run),"clicked",G_CALLBACK(runmodule_activated),NULL);
    g_signal_connect(G_OBJECT(toolbar_preferences),"clicked",G_CALLBACK(options_activated),(gpointer)GTK_WINDOW(window));
    g_signal_connect(G_OBJECT(toolbar_close),"clicked",G_CALLBACK(exit_activated),NULL);
    
    file = gtk_menu_item_new_with_label("File");
    edit = gtk_menu_item_new_with_label("Edit");
    format = gtk_menu_item_new_with_label("Format");
    run = gtk_menu_item_new_with_label("Run");
    tools = gtk_menu_item_new_with_label("Tools");
    navigation = gtk_menu_item_new_with_label("Navigation");
    
    formatmenu = gtk_menu_new();
    filemenu = gtk_menu_new();
    editmenu = gtk_menu_new();
    runmenu = gtk_menu_new();
    toolsmenu = gtk_menu_new();
    navigationmenu = gtk_menu_new();
    
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(file),filemenu);
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(edit),editmenu);
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(format),formatmenu);
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(run),runmenu);
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(tools),toolsmenu);
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(navigation),navigationmenu);
    
    gtk_menu_shell_append(GTK_MENU_SHELL(menubar),file);
    gtk_menu_shell_append(GTK_MENU_SHELL(menubar),edit);
    gtk_menu_shell_append(GTK_MENU_SHELL(menubar),format);
    gtk_menu_shell_append(GTK_MENU_SHELL(menubar),navigation);
    gtk_menu_shell_append(GTK_MENU_SHELL(menubar),run);
    gtk_menu_shell_append(GTK_MENU_SHELL(menubar),tools);
    
     /*Create the File menu*/
    filenew = gtk_image_menu_item_new_from_stock(GTK_STOCK_NEW,group);
    new_file = gtk_image_menu_item_new_with_label("Create New File");
    new_proj = gtk_image_menu_item_new_with_label("Create New Project");
    addfile = gtk_menu_item_new_with_label("Add File");
    addfile_new = gtk_menu_item_new_with_label("Add New File");
    addfile_existing = gtk_menu_item_new_with_label("Add Existing File");
    addfile_main = gtk_menu_item_new_with_label("Add Main File");
    addfile_main_new = gtk_menu_item_new_with_label("Add New Main File");
    addfile_main_existing = gtk_menu_item_new_with_label("Add Existing Main File");
    open = gtk_image_menu_item_new_from_stock(GTK_STOCK_OPEN,group);
    open_file = gtk_image_menu_item_new_with_label("Open File");
    open_proj = gtk_image_menu_item_new_with_label("Open Project");
    save = gtk_image_menu_item_new_from_stock(GTK_STOCK_SAVE,group);
    saveas = gtk_menu_item_new_with_label("Save As");
    saveall = gtk_menu_item_new_with_label("Save All");
    saveasall = gtk_menu_item_new_with_label("Save As All");
    savecopyas = gtk_menu_item_new_with_label("Save Copy As");
    restore = gtk_menu_item_new_with_label("Restore to previous version");
    newtab = gtk_menu_item_new_with_label("New Tab");
    removetab = gtk_menu_item_new_with_label("Remove Current Tab");
    removealltab = gtk_menu_item_new_with_label("Remove All Tabs");
    recentfiles = gtk_menu_item_new_with_label("Recent Files");
    filemenu_separator = gtk_separator_menu_item_new();
    filemenu_separator1 = gtk_separator_menu_item_new();
    exit = gtk_menu_item_new_with_label("Exit");
    
    recentfilesmenu = gtk_menu_new();
    filenewmenu = gtk_menu_new();
    fileopenmenu = gtk_menu_new();
    addfilemenu = gtk_menu_new();
    addfile_mainmenu = gtk_menu_new();
       
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(recentfiles),recentfilesmenu);
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(filenew),filenewmenu);
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(open),fileopenmenu);
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(addfile),addfilemenu);
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(addfile_main),addfile_mainmenu);
    
    gtk_menu_shell_append(GTK_MENU_SHELL(filenewmenu),new_file);
    gtk_menu_shell_append(GTK_MENU_SHELL(filenewmenu),new_proj);
    
    gtk_menu_shell_append(GTK_MENU_SHELL(fileopenmenu),open_file);
    gtk_menu_shell_append(GTK_MENU_SHELL(fileopenmenu),open_proj);    
    
    gtk_menu_shell_append(GTK_MENU_SHELL(addfilemenu),addfile_main);
    gtk_menu_shell_append(GTK_MENU_SHELL(addfilemenu),addfile_new);
    gtk_menu_shell_append(GTK_MENU_SHELL(addfilemenu),addfile_existing);
    gtk_menu_shell_append(GTK_MENU_SHELL(addfile_mainmenu),addfile_main_new);
    gtk_menu_shell_append(GTK_MENU_SHELL(addfile_mainmenu),addfile_main_existing);
    
    gtk_menu_shell_append(GTK_MENU_SHELL(filemenu),filenew);
    gtk_menu_shell_append(GTK_MENU_SHELL(filemenu),newtab);
    gtk_menu_shell_append(GTK_MENU_SHELL(filemenu),recentfiles);
    gtk_menu_shell_append(GTK_MENU_SHELL(filemenu),open);
    gtk_menu_shell_append(GTK_MENU_SHELL(filemenu),addfile);
    gtk_menu_shell_append(GTK_MENU_SHELL(filemenu),filemenu_separator);
    gtk_menu_shell_append(GTK_MENU_SHELL(filemenu),save);
    gtk_menu_shell_append(GTK_MENU_SHELL(filemenu),saveas);
    gtk_menu_shell_append(GTK_MENU_SHELL(filemenu),saveall);
    gtk_menu_shell_append(GTK_MENU_SHELL(filemenu),saveasall);
    gtk_menu_shell_append(GTK_MENU_SHELL(filemenu),savecopyas);
    gtk_menu_shell_append(GTK_MENU_SHELL(filemenu),restore);
    gtk_menu_shell_append(GTK_MENU_SHELL(filemenu),filemenu_separator1);
    gtk_menu_shell_append(GTK_MENU_SHELL(filemenu),removetab);
    gtk_menu_shell_append(GTK_MENU_SHELL(filemenu),removealltab);
    gtk_menu_shell_append(GTK_MENU_SHELL(filemenu),exit);
    
    g_signal_connect(G_OBJECT(open_file),"activate",G_CALLBACK(open_file_activated),(gpointer)GTK_WINDOW(window));
    g_signal_connect(G_OBJECT(open_proj),"activate",G_CALLBACK(open_proj_activated),(gpointer)GTK_WINDOW(window));
    g_signal_connect(G_OBJECT(new_file),"activate",G_CALLBACK(filenew_activated),NULL);
    g_signal_connect(G_OBJECT(new_proj),"activate",G_CALLBACK(new_proj_activated),(gpointer)GTK_WINDOW(window));
    g_signal_connect(G_OBJECT(addfile_new),"activate",G_CALLBACK(addfile_new_activated),(gpointer)GTK_WINDOW(window));
    g_signal_connect(G_OBJECT(addfile_existing),"activate",G_CALLBACK(addfile_existing_activated),(gpointer)GTK_WINDOW(window));
    g_signal_connect(G_OBJECT(addfile_main_new),"activate",G_CALLBACK(addfile_main_new_activated),(gpointer)GTK_WINDOW(window));
    g_signal_connect(G_OBJECT(addfile_main_existing),"activate",G_CALLBACK(addfile_main_existing_activated),(gpointer)GTK_WINDOW(window));    
    g_signal_connect(G_OBJECT(saveas),"activate",G_CALLBACK(saveas_activated),(gpointer)GTK_WINDOW(window));
    g_signal_connect(G_OBJECT(save),"activate",G_CALLBACK(save_activated),(gpointer)GTK_WINDOW(window));
    g_signal_connect(G_OBJECT(saveall),"activate",G_CALLBACK(saveall_activated),(gpointer)GTK_WINDOW(window));
    g_signal_connect(G_OBJECT(saveasall),"activate",G_CALLBACK(saveasall_activated),(gpointer)GTK_WINDOW(window));
    g_signal_connect(G_OBJECT(saveasall),"activate",G_CALLBACK(savecopyas_activated),(gpointer)GTK_WINDOW(window));
    g_signal_connect(G_OBJECT(restore),"activate",G_CALLBACK(restore_activated),NULL);
    g_signal_connect(G_OBJECT(newtab),"activate",G_CALLBACK(newtab_activated),NULL);
    g_signal_connect(G_OBJECT(removetab),"activate",G_CALLBACK(removetab_activated),NULL);
    g_signal_connect(G_OBJECT(removealltab),"activate",G_CALLBACK(removealltab_activated),NULL);
    g_signal_connect(G_OBJECT(exit),"activate",G_CALLBACK(exit_activated),NULL);     
    
    /*Create the Edit menu*/
    cut = gtk_image_menu_item_new_from_stock(GTK_STOCK_CUT,group);
    copy = gtk_image_menu_item_new_from_stock(GTK_STOCK_COPY,group);
    paste = gtk_image_menu_item_new_from_stock(GTK_STOCK_PASTE,group);
    undo = gtk_image_menu_item_new_from_stock(GTK_STOCK_UNDO,group);
    redo = gtk_image_menu_item_new_from_stock(GTK_STOCK_REDO,group);
    findandreplace = gtk_image_menu_item_new_from_stock(GTK_STOCK_FIND_AND_REPLACE,group);
    find = gtk_image_menu_item_new_from_stock(GTK_STOCK_FIND,group);
    find_selected = gtk_menu_item_new_with_label("Find Selected Text");
    find_in_file = gtk_menu_item_new_with_label("Find in File");
    reg_exp_search = gtk_menu_item_new_with_label("Regular Expression Search");
    selectall = gtk_menu_item_new_with_label("Select All");
    editmenu_separator = gtk_separator_menu_item_new();
    editmenu_separator2 = gtk_separator_menu_item_new();
    editmenu_separator3 = gtk_separator_menu_item_new();
    show_parenthesis = gtk_menu_item_new_with_label("Show surrounding parenthesis");
    
    gtk_widget_set_state(undo,GTK_STATE_INSENSITIVE);
    gtk_widget_set_state(redo,GTK_STATE_INSENSITIVE);
    gtk_widget_set_state(copy,GTK_STATE_INSENSITIVE);
    gtk_widget_set_state(cut,GTK_STATE_INSENSITIVE);
    gtk_widget_set_state(paste,GTK_STATE_INSENSITIVE);
    
    g_signal_connect(G_OBJECT(undo),"activate",G_CALLBACK(undo_activated),NULL);
    g_signal_connect(G_OBJECT(redo),"activate",G_CALLBACK(redo_activated),NULL);
    g_signal_connect(G_OBJECT(cut),"activate",G_CALLBACK(cut_activated),paste);
    g_signal_connect(G_OBJECT(paste),"activate",G_CALLBACK(paste_activated),NULL);
    g_signal_connect(G_OBJECT(copy),"activate",G_CALLBACK(copy_activated),paste);
    g_signal_connect(G_OBJECT(find),"activate",G_CALLBACK(find_activated),NULL);
    g_signal_connect(G_OBJECT(findandreplace),"activate",G_CALLBACK(findandreplace_activated),NULL);
    g_signal_connect(G_OBJECT(find_selected),"activate",G_CALLBACK(find_selected_activated),NULL);
    g_signal_connect(G_OBJECT(find_in_file),"activate",G_CALLBACK(find_in_file_activated),NULL);
    g_signal_connect(G_OBJECT(reg_exp_search),"activate",G_CALLBACK(reg_exp_search_activated),NULL);
    g_signal_connect(G_OBJECT(selectall),"activate",G_CALLBACK(selectall_activated),NULL);
    g_signal_connect(G_OBJECT(show_parenthesis),"activate",G_CALLBACK(show_parenthesis_activated),NULL);
        
    gtk_menu_shell_append(GTK_MENU_SHELL(editmenu),undo);
    gtk_menu_shell_append(GTK_MENU_SHELL(editmenu),redo);
    gtk_menu_shell_append(GTK_MENU_SHELL(editmenu),editmenu_separator3);
    gtk_menu_shell_append(GTK_MENU_SHELL(editmenu),cut);
    gtk_menu_shell_append(GTK_MENU_SHELL(editmenu),copy);
    gtk_menu_shell_append(GTK_MENU_SHELL(editmenu),paste);
    gtk_menu_shell_append(GTK_MENU_SHELL(editmenu),selectall);
    gtk_menu_shell_append(GTK_MENU_SHELL(editmenu),editmenu_separator);
    gtk_menu_shell_append(GTK_MENU_SHELL(editmenu),find);
    gtk_menu_shell_append(GTK_MENU_SHELL(editmenu),findandreplace);
    gtk_menu_shell_append(GTK_MENU_SHELL(editmenu),find_selected);
    gtk_menu_shell_append(GTK_MENU_SHELL(editmenu),find_in_file);
    gtk_menu_shell_append(GTK_MENU_SHELL(editmenu),reg_exp_search);
    gtk_menu_shell_append(GTK_MENU_SHELL(editmenu),editmenu_separator2);
    gtk_menu_shell_append(GTK_MENU_SHELL(editmenu),show_parenthesis);
    
    /*Format Menu*/
    indentregion = gtk_menu_item_new_with_label("Indent Region");
    dedentregion  = gtk_menu_item_new_with_label("Dedent Region");
    formatmenu_separator1 = gtk_separator_menu_item_new();
    commentout =gtk_menu_item_new_with_label("Comment Out Region");
    uncommentout = gtk_menu_item_new_with_label("Uncomment Out Region");
    formatmenu_separator2 = gtk_separator_menu_item_new();
    striptrailingspaces = gtk_menu_item_new_with_label("Strip Trailing Whitespaces");
    tabify_region = gtk_menu_item_new_with_label("Tabify Region");
    untabify_region = gtk_menu_item_new_with_label("Untabify Region");
    formatmenu_separator3 = gtk_separator_menu_item_new();
    lowercase_selection = gtk_menu_item_new_with_label("Lowercase Selection");
    uppercase_selection = gtk_menu_item_new_with_label("Uppercase Selection");
    formatmenu_separator4 = gtk_separator_menu_item_new();
     
    gtk_menu_shell_append(GTK_MENU_SHELL(formatmenu),indentregion);
    gtk_menu_shell_append(GTK_MENU_SHELL(formatmenu),dedentregion);
    gtk_menu_shell_append(GTK_MENU_SHELL(formatmenu),formatmenu_separator1);
    gtk_menu_shell_append(GTK_MENU_SHELL(formatmenu),commentout);
    gtk_menu_shell_append(GTK_MENU_SHELL(formatmenu),uncommentout);
    gtk_menu_shell_append(GTK_MENU_SHELL(formatmenu),formatmenu_separator2);
    gtk_menu_shell_append(GTK_MENU_SHELL(formatmenu),tabify_region);
    gtk_menu_shell_append(GTK_MENU_SHELL(formatmenu),untabify_region);
    gtk_menu_shell_append(GTK_MENU_SHELL(formatmenu),formatmenu_separator3);
    gtk_menu_shell_append(GTK_MENU_SHELL(formatmenu),uppercase_selection);
    gtk_menu_shell_append(GTK_MENU_SHELL(formatmenu),lowercase_selection);
    gtk_menu_shell_append(GTK_MENU_SHELL(formatmenu),formatmenu_separator4);
    gtk_menu_shell_append(GTK_MENU_SHELL(formatmenu),striptrailingspaces);
    
    g_signal_connect(G_OBJECT(indentregion),"activate",G_CALLBACK(indentregion_activated),NULL);
    g_signal_connect(G_OBJECT(dedentregion),"activate",G_CALLBACK(dedentregion_activated),NULL);
    g_signal_connect(G_OBJECT(commentout),"activate",G_CALLBACK(commentout_activated),NULL);
    g_signal_connect(G_OBJECT(uncommentout),"activate",G_CALLBACK(uncommentout_activated),NULL);
    g_signal_connect(G_OBJECT(striptrailingspaces),"activate",G_CALLBACK(striptrailingspaces_activated),NULL);
    g_signal_connect(G_OBJECT(tabify_region),"activate",G_CALLBACK(tabify_region_activated),NULL);
    g_signal_connect(G_OBJECT(untabify_region),"activate",G_CALLBACK(untabify_region_activated),NULL);
    g_signal_connect(G_OBJECT(uppercase_selection),"activate",G_CALLBACK(uppercase_selection_activated),NULL);
    g_signal_connect(G_OBJECT(lowercase_selection),"activate",G_CALLBACK(lowercase_selection_activated),NULL);
    
    /*Run Menu*/
    runmodule = gtk_menu_item_new_with_label("Run Module");
    openshell =  gtk_menu_item_new_with_label("Open Python Shell");
    
    gtk_menu_shell_append(GTK_MENU_SHELL(runmenu),openshell);
    gtk_menu_shell_append(GTK_MENU_SHELL(runmenu),runmodule);
    g_signal_connect(G_OBJECT(runmodule),"activate",G_CALLBACK(runmodule_activated),NULL);
    g_signal_connect(G_OBJECT(openshell),"activate",G_CALLBACK(openshell_activated),NULL);
    
    /*Tools Menu*/
    gnome_terminal = gtk_menu_item_new_with_label("Run GNOME Terminal");
    options = gtk_menu_item_new_with_label("Options");
    pdb = gtk_menu_item_new_with_label("Debug using Python Debugger (pdb)");
    file_information = gtk_menu_item_new_with_label("File Information");
    class_browser = gtk_menu_item_new_with_label("Class Browser");
    proj_manager = gtk_menu_item_new_with_label("Project Manager");
    path_browser = gtk_menu_item_new_with_label("Path Browser");
    
    gtk_menu_shell_append(GTK_MENU_SHELL(toolsmenu),gnome_terminal);
    gtk_menu_shell_append(GTK_MENU_SHELL(toolsmenu),options);
    gtk_menu_shell_append(GTK_MENU_SHELL(toolsmenu),pdb);
    gtk_menu_shell_append(GTK_MENU_SHELL(toolsmenu),file_information);
    gtk_menu_shell_append(GTK_MENU_SHELL(toolsmenu),class_browser);
    gtk_menu_shell_append(GTK_MENU_SHELL(toolsmenu),proj_manager);
    gtk_menu_shell_append(GTK_MENU_SHELL(toolsmenu),path_browser);
    
    g_signal_connect(G_OBJECT(gnome_terminal),"activate",G_CALLBACK(gnome_terminal_activated),NULL);
    g_signal_connect(G_OBJECT(pdb),"activate",G_CALLBACK(pdb_activated),NULL);
    g_signal_connect(G_OBJECT(file_information),"activate",G_CALLBACK(file_information_activated),NULL);
    g_signal_connect(G_OBJECT(options),"activate",G_CALLBACK(options_activated),window);
    g_signal_connect(G_OBJECT(class_browser),"activate",G_CALLBACK(class_browser_activated),window);
    g_signal_connect(G_OBJECT(proj_manager),"activate",G_CALLBACK(proj_manager_activated),window);
    g_signal_connect(G_OBJECT(path_browser),"activate",G_CALLBACK(path_browser_activated),window);
    
    /*Navigation Menu*/
    back_line = gtk_menu_item_new_with_label("Back");
    forward_line = gtk_menu_item_new_with_label("Forward");
    beggining_line = gtk_menu_item_new_with_label("Go to beggining of the file");
    end_line = gtk_menu_item_new_with_label("Go to end of the file");
    gotoline = gtk_menu_item_new_with_label("Go To Line");
    navigation_separator1 = gtk_separator_menu_item_new();
    navigation_separator2 = gtk_separator_menu_item_new();
    navigation_separator3 = gtk_separator_menu_item_new();
    add_bookmark = gtk_menu_item_new_with_label("Add Bookmark");
    next_bookmark = gtk_menu_item_new_with_label("Next Bookmark");
    previous_bookmark = gtk_menu_item_new_with_label("Previous Bookmark");
    clear_bookmarks = gtk_menu_item_new_with_label("Clear Bookmarks");
    navigation_separator4 = gtk_separator_menu_item_new();
        
    gtk_menu_shell_append(GTK_MENU_SHELL(navigationmenu),back_line);
    gtk_menu_shell_append(GTK_MENU_SHELL(navigationmenu),forward_line);
    gtk_menu_shell_append(GTK_MENU_SHELL(navigationmenu),navigation_separator1);
    gtk_menu_shell_append(GTK_MENU_SHELL(navigationmenu),beggining_line);
    gtk_menu_shell_append(GTK_MENU_SHELL(navigationmenu),end_line);
    gtk_menu_shell_append(GTK_MENU_SHELL(navigationmenu),navigation_separator2);
    gtk_menu_shell_append(GTK_MENU_SHELL(navigationmenu),gotoline);
    gtk_menu_shell_append(GTK_MENU_SHELL(navigationmenu),navigation_separator3);
    gtk_menu_shell_append(GTK_MENU_SHELL(navigationmenu),add_bookmark);
    gtk_menu_shell_append(GTK_MENU_SHELL(navigationmenu),next_bookmark);
    gtk_menu_shell_append(GTK_MENU_SHELL(navigationmenu),previous_bookmark);
    gtk_menu_shell_append(GTK_MENU_SHELL(navigationmenu),clear_bookmarks);
    gtk_menu_shell_append(GTK_MENU_SHELL(navigationmenu),navigation_separator4);
    
    gtk_widget_set_state(next_bookmark,GTK_STATE_INSENSITIVE);
    gtk_widget_set_state(previous_bookmark,GTK_STATE_INSENSITIVE);
    gtk_widget_set_state(clear_bookmarks,GTK_STATE_INSENSITIVE);
    
    g_signal_connect(G_OBJECT(back_line),"activate",G_CALLBACK(back_line_activated),NULL);
    g_signal_connect(G_OBJECT(forward_line),"activate",G_CALLBACK(forward_line_activated),NULL);
    g_signal_connect(G_OBJECT(beggining_line),"activate",G_CALLBACK(beggining_line_activated),NULL);
    g_signal_connect(G_OBJECT(end_line),"activate",G_CALLBACK(end_line_activated),NULL);
    g_signal_connect(G_OBJECT(gotoline),"activate",G_CALLBACK(gotoline_activated),NULL);
    g_signal_connect(G_OBJECT(add_bookmark),"activate",G_CALLBACK(add_bookmarks_activated),navigationmenu);
    g_signal_connect(G_OBJECT(next_bookmark),"activate",G_CALLBACK(next_bookmarks_activated),navigationmenu);
    g_signal_connect(G_OBJECT(previous_bookmark),"activate",G_CALLBACK(previous_bookmarks_activated),navigationmenu);
    g_signal_connect(G_OBJECT(clear_bookmarks),"activate",G_CALLBACK(clear_bookmarks_activated),navigationmenu);
        
    gtk_container_add(GTK_CONTAINER(window),vbox);
    gtk_window_add_accel_group(GTK_WINDOW(window),group);
    g_signal_connect(G_OBJECT(window),"destroy",G_CALLBACK(destroy),NULL);
    g_signal_connect(G_OBJECT(window),"delete_event",G_CALLBACK(delete_event),NULL);
    g_signal_connect(G_OBJECT(notebook),"page-added",G_CALLBACK(notebook_page_added),NULL);
    
    run_shell = g_string_new("");
    recent_files = g_string_new("");
    char *settings;
    gsize length = -1;
    if (g_file_get_contents("./settings.ini",&settings,&length,NULL) == TRUE)
    {
        int i;
        int opening_index=0,closing_index=0,value_index=0;
      
        opening_index = find_string(settings,"<run shell>") + strlen("<run shell>");
        closing_index = find_string(settings,"</run shell>");
        for(i=opening_index;i<closing_index;i++)
        {
            g_string_append_c(run_shell,settings[i]);
            }        
        if (strcmp(run_shell->str,"ENABLED") == 0)
            openshell_activated(openshell);
        opening_index = find_string(settings,"<recentfiles>") + strlen("<recentfiles>");
        closing_index = find_string(settings,"</recentfiles>");
        for(i=opening_index;i<closing_index;i++)
            g_string_append_c(recent_files,settings[i]);
        }
        
    if(strcmp(recent_files->str,"ENABLED")==0)
    {
        gtk_widget_set_sensitive(recentfiles,TRUE);
        int file_open=0,file_close=-1,j=0;
        char *string;
        gssize length = -1;
        if (g_file_get_contents("./recentfiles.ini",&string,&length,NULL) == TRUE)
        {
            while((file_open=find_strstr(string,"<file>",file_close+1))!=-1)
            {
                file_close = find_strstr(string,"</file>",file_open+1);
                GString *file;
                file = g_string_new("");
                for(i=file_open+6;i<file_close;i++)
                    g_string_append_c(file,string[i]);
                file_path_menu_itemsarray[j] = gtk_menu_item_new_with_label(file->str);            
                gtk_menu_shell_prepend(GTK_MENU_SHELL(recentfilesmenu),file_path_menu_itemsarray[j]);
                g_signal_connect(G_OBJECT(file_path_menu_itemsarray[j]),"activate",G_CALLBACK(file_path_menu_item_activated),NULL);
                gtk_widget_show(file_path_menu_itemsarray[j]);
                g_string_free(file,TRUE);
                j++;
                }
            }
        else
            gtk_widget_set_state(recentfiles,GTK_STATE_INSENSITIVE);
        }
           
    func_load_settings();
    gtk_window_maximize(GTK_WINDOW(window));
    gtk_widget_show_all(window);
    gtk_main();
    
    return 0;
    }  


extern void proj_manager_destroyed(char *filearray[],GString *main_file,GString *projname)
{
    int i=0;
    while(strcmp("",filepathsarray[i])!=0)
    {
        gtk_tree_store_remove(proj_treestore,&file_child_array[i]);
        i++;
        }
    GtkWidget *widget;
    removealltab_activated(widget);
    
    for(i=1;i<10;i++)
        asprintf(&filepathsarray[i],"%s\0",filearray[i]);    
    
    g_string_erase(mainfile,0,-1);
    g_string_erase(proj_name,0,-1);
    g_string_append(mainfile,main_file->str);
    asprintf(&filepathsarray[0],"%s\0", mainfile->str);
    g_string_append(proj_name,projname->str);
    gtk_tree_store_set(proj_treestore,&toplevel,0,proj_name->str,-1);
    gsize length = -1;
    char *filename = strrchr(mainfile->str,'/');
    filename++;
    gtk_tree_store_append(proj_treestore,&file_child_array[0],&toplevel);
    gtk_tree_store_set(proj_treestore,&file_child_array[0],0,filename,-1);
    GtkSourceLanguageManager *languagemanager = gtk_source_language_manager_new();
    GtkSourceBuffer *buffer1;
    buffer1 = GTK_SOURCE_BUFFER(gtk_source_buffer_new(NULL));
    codewidget[0].txtinput = gtk_source_view_new_with_buffer(buffer1);    
    vbox_txtinput[0] = gtk_vbox_new(FALSE,2);
    codewidget[0].func_combo_box = gtk_combo_box_text_new();
    codewidget[0].class_combo_box = gtk_combo_box_text_new();
    newtab_activated(widget);
    GtkTextBuffer *buffer;
    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(codewidget[0].txtinput));
    GtkSourceLanguage *language = gtk_source_language_manager_guess_language(languagemanager,mainfile->str,NULL);
    gtk_source_buffer_set_language(GTK_SOURCE_BUFFER(buffer),language);
    char *contents;
    g_file_get_contents(mainfile->str,&contents,&length,NULL);
    gtk_text_buffer_set_text(GTK_TEXT_BUFFER(buffer),contents,-1);
    g_free(contents);
    gtk_notebook_set_tab_label_text(GTK_NOTEBOOK(notebook),vbox_txtinput[0],filename);
    i=1;
    while(strcmp("",filepathsarray[i])!=0)
    {
        char *filename = strrchr(filepathsarray[i],'/');
        filename++;
        gtk_tree_store_append(proj_treestore,&file_child_array[i],&toplevel);
        gtk_tree_store_set(proj_treestore,&file_child_array[i],0,filename,-1);
        GtkSourceBuffer *buffer1;
        buffer1 = GTK_SOURCE_BUFFER(gtk_source_buffer_new(NULL));
        codewidget[i].txtinput = gtk_source_view_new_with_buffer(buffer1);    
        GtkWidget *scrollwin = gtk_scrolled_window_new(NULL,NULL);
        gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrollwin),GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC);
        codewidget[i].func_combo_box = gtk_combo_box_text_new();
        codewidget[i].class_combo_box = gtk_combo_box_text_new();
        newtab_activated(widget);
        buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(codewidget[i].txtinput));
        GtkSourceLanguage *language = gtk_source_language_manager_guess_language(languagemanager,filepathsarray[i],NULL);
        gtk_source_buffer_set_language(GTK_SOURCE_BUFFER(buffer),language);
        char *contents;
        g_file_get_contents(filepathsarray[i],&contents,&length,NULL);     
        gtk_text_buffer_set_text(GTK_TEXT_BUFFER(buffer),contents,-1);
        g_free(contents);
        gtk_notebook_set_tab_label_text(GTK_NOTEBOOK(notebook),vbox_txtinput[i],filename);
        i++;
        }
    }

extern void new_proj_window_destroyed(GtkWidget *widget)
{
    proj_treestore = gtk_tree_store_new(1,G_TYPE_STRING);
    gtk_tree_store_append(proj_treestore,&toplevel,NULL);
    gtk_tree_view_set_model(GTK_TREE_VIEW(proj_tree_view),GTK_TREE_MODEL(proj_treestore));
    proj_col = gtk_tree_view_column_new();
    proj_renderer = gtk_cell_renderer_text_new();
    gtk_tree_view_column_pack_start(proj_col,proj_renderer,TRUE);
    gtk_tree_view_column_add_attribute(proj_col,proj_renderer,"text",0);
    gtk_tree_view_append_column(GTK_TREE_VIEW(proj_tree_view),proj_col);
    gtk_tree_view_column_set_title(proj_col,"Project");
    gtk_tree_store_set(proj_treestore,&toplevel,0,proj_name->str,-1);
    int w,h;
    gtk_window_get_size(GTK_WINDOW(window),&w,&h);
    gtk_widget_set_size_request(proj_tree_view,200,h);
    int i;    
    for (i=gtk_notebook_get_n_pages(GTK_NOTEBOOK(notebook));i>=0;i--)
    {
        if(strcmp(filepathsarray[i],"")!=0)
            free(filepathsarray[i]);
        codewidget_destroy(&codewidget[i]);
        gtk_notebook_remove_page(GTK_NOTEBOOK(notebook),i);
        }
    mainfile=g_string_erase(mainfile,0,-1);
    }

extern void options_window_destroyed(GtkWidget *options_window)
{
    func_load_settings();
    }

void func_load_settings()
{
    char *settings;
    gsize length = -1;
        
    indentation = g_string_new("");
    indent_width = g_string_new("");
    auto_save = g_string_new("");
    auto_save_time = g_string_new("");
    show_line_numbers = g_string_new("");
    highlight_current_line = g_string_new("");
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
        
        opening_index = find_string(settings,"<recentfiles>") + strlen("<recentfiles>");
        closing_index = find_string(settings,"</recentfiles>");
        for(i=opening_index;i<closing_index;i++)
            g_string_append_c(recent_files,settings[i]);
        
        opening_index = find_string(settings,"<tabwidth>") + strlen("<tabwidth>");
        closing_index = find_string(settings,"</tabwidth>");
        for(i=opening_index;i<closing_index;i++)
            g_string_append_c(tabwidth,settings[i]);
        }
    if(strcmp(recent_files->str,"ENABLED")==0)
        gtk_widget_set_sensitive(recentfiles,TRUE);
    else
        gtk_widget_set_state(recentfiles,GTK_STATE_INSENSITIVE);
    int i;
    GtkSourceView *sourceview;
    for (i =0; i<gtk_notebook_get_n_pages(GTK_NOTEBOOK(notebook));i++)
    {
        sourceview = GTK_SOURCE_VIEW(codewidget[i].txtinput);
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
    
    if (strcmp(auto_save->str,"ENABLED") == 0)
    {        
        int interval =  (int)(strtod(auto_save_time->str,NULL)*60);
        g_timeout_add_seconds(interval,(GSourceFunc)auto_save_func,NULL);
        }    
    }

void cmdcancel_activated(GtkWidget *widget,GtkWidget *dialog)
{
    gtk_widget_destroy(dialog);
    }
   
void notebook_page_added(GtkWidget *widget,GtkWidget *page,guint index,GtkWidget *widget2)
{
    g_signal_connect(G_OBJECT(codewidget[index].txtinput),"key-press-event",G_CALLBACK(txtinput_key_press_event),NULL);   
    }
   
static void destroy(GtkWidget *window)
{
    //g_free(class_array);
    //g_free(func_array);
    //g_free(class_pos_array);
    //g_free(class_indentation_array);
    gtk_main_quit();
    }

static void exitdialog_cmdsave_clicked(GtkWidget *widget, GtkWidget *widgetarray[])
{
    int i = 0;
    gssize length = -1;
    while (i<filecheckbuttonsarray_size)
    {
        if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widgetarray[i])) == TRUE)
        {
            if (strcmp(filepathsarray[filenotsavedarray[i]],"") !=0)
            {
                GtkTextIter start_iter, end_iter;
                GtkTextBuffer *buffer;
                buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(codewidget[filenotsavedarray[i]].txtinput));
                gtk_text_buffer_get_end_iter(buffer,&end_iter);
                gtk_text_buffer_get_start_iter(buffer,&start_iter);
                g_file_set_contents(filepathsarray[filenotsavedarray[i]],gtk_text_buffer_get_text(buffer,&start_iter,&end_iter,FALSE),length,NULL);
                }
            else
            {
                gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook),filenotsavedarray[i]);
                saveas_activated(widget,GTK_WINDOW(window));
                }
            }
        else
        {
            break;
            }
         i++;
        }
    gtk_widget_destroy(exit_dialog);
    gtk_widget_destroy(window);
    }

static gboolean delete_event(GtkWidget *window,GdkEvent *event)
{
    int i=0,j = -1;
   
    for (i = 0; i<10;i++)
    {
        filenotsavedarray[i] = -1;
        }
    
    for (i = 0; i <gtk_notebook_get_n_pages(GTK_NOTEBOOK(notebook)); i++)
    {
        GtkTextBuffer *buffer;
        GtkTextIter start_iter, end_iter;
        buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(codewidget[i].txtinput));
        gtk_text_buffer_get_start_iter(buffer, &start_iter);
        gtk_text_buffer_get_end_iter(buffer, &end_iter);
        char *text;
        text = gtk_text_buffer_get_text(buffer,&start_iter,&end_iter,FALSE);
        if (function_filechanged(filepathsarray[i],text) == TRUE)
        {
            if (j==-1)
            {
                j=0;
                }
            filenotsavedarray[j] = i;
            j++;
            }
        g_free(text);
        }
    
    if (j != -1)
    {
        GtkWidget *label,*cmdclosewithoutsave,*cmdcancel,*cmdsave;
        filecheckbuttonsarray_size = i;
        GtkWidget *filecheckbuttonsarray[i];
        
        exit_dialog = gtk_dialog_new();
        
        label = gtk_label_new("Following files are not saved. Please select the files you want to save before closing.");
        cmdclosewithoutsave = gtk_button_new_with_mnemonic("_Close without saving");
        cmdsave = gtk_button_new_with_mnemonic("_Save");
        cmdcancel = gtk_button_new_with_mnemonic("_Cancel");
        gtk_window_set_title(GTK_WINDOW(exit_dialog),"gIDLE");
        
        gtk_box_pack_start(GTK_BOX(GTK_DIALOG(exit_dialog)->vbox),label,FALSE,FALSE,0);
        for (i =0; i <j; i++)
        {
            filecheckbuttonsarray[i] = gtk_check_button_new_with_label(gtk_notebook_get_tab_label_text(GTK_NOTEBOOK(notebook),vbox_txtinput[filenotsavedarray[i]]));
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(filecheckbuttonsarray[i]),TRUE);
            gtk_box_pack_start(GTK_BOX(GTK_DIALOG(exit_dialog)->vbox),filecheckbuttonsarray[i],FALSE,FALSE,0);
            }
        gtk_container_add(GTK_CONTAINER(GTK_DIALOG(exit_dialog)->action_area),cmdclosewithoutsave);
        gtk_container_add(GTK_CONTAINER(GTK_DIALOG(exit_dialog)->action_area),cmdcancel);
        gtk_container_add(GTK_CONTAINER(GTK_DIALOG(exit_dialog)->action_area),cmdsave);
        g_signal_connect(G_OBJECT(cmdclosewithoutsave),"clicked",G_CALLBACK(exit_activated),exit_dialog);
        g_signal_connect(G_OBJECT(cmdsave),"clicked",G_CALLBACK(exitdialog_cmdsave_clicked),filecheckbuttonsarray);
        g_signal_connect(G_OBJECT(cmdcancel),"clicked",G_CALLBACK(cmdcancel_activated),exit_dialog);
        gtk_window_set_modal(GTK_WINDOW(exit_dialog),FALSE);
        gtk_widget_show_all(exit_dialog);
        gtk_dialog_run(GTK_DIALOG(exit_dialog));        
        return TRUE;
        }
    else    
        return FALSE;     
    }
