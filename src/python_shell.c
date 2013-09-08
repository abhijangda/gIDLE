#include "main.h"
#include "core_funcs.h"
#include "pty_fork.h"
#include "python_shell.h"

#include <signal.h>
#include <sys/epoll.h>

#define RESTART_MSG "\n\n############Restarting Shell###############\n\n"

extern char *env_python_path;

GtkBuilder *python_shell_builder;
static gboolean python_shell_loaded = FALSE;
extern GtkWidget *status_bar;
extern ChildProcessData *python_shell_data;

extern gIDLEOptions options;

static void
show_python_shell_win ();
static void
close_python_shell_process ();

static GtkWidget *python_shell_source_view;
static GtkSourceBuffer *python_shell_source_buffer;
static gint last_pos = 0;
GtkWidget *python_shell_win;
static int py_shell_state = PY_SHELL_STATE_SHELL;
extern gboolean bash_loaded;
static gboolean going_for_restart;
static gchar *shell_str = NULL;

/* To Load Python Shell
 * window
 */
void
load_python_shell ()
{
    if (python_shell_loaded)
    {
        if (GTK_IS_WINDOW (python_shell_win))
             gtk_window_present (GTK_WINDOW (python_shell_win));

        return;
    }

    char *py_argv[] = {options.python_shell_path, NULL};
    if (!execute_python_shell (NULL, py_argv))
       return;
    
    py_shell_state = PY_SHELL_STATE_SHELL;

    show_python_shell_win ();
}

/* To run filename in
 * Python Shell
 */
void
run_file_in_python_shell (char *filename, gchar *curr_dir, gchar *argv[], gchar *env [])
{
    /* Run shell or restart it if its already loaded */
    GtkTextBuffer *buffer =  GTK_TEXT_BUFFER (python_shell_source_buffer);
    if (python_shell_loaded)
    {
        close_python_shell_process ();
        gtk_text_buffer_insert_at_cursor (buffer, RESTART_MSG, -1);
    }
    else
        show_python_shell_win ();

    /* Run all the commands in one go using "&&" */
    /*Setting Kivy environment variable
     * remove it and move it to options */
    GString *gstr_argv = g_string_new ("export PYTHONPATH=/home/abhi/kivy_repo_me/kivy && ");
 
    /* Setting curr_dir */
    if (curr_dir)
    {
        gstr_argv = g_string_append (gstr_argv, "cd ");
        gstr_argv = g_string_append (gstr_argv, curr_dir);
        gstr_argv = g_string_append (gstr_argv, " && ");
    }

    gstr_argv = g_string_append (gstr_argv, "python ");
    gstr_argv = g_string_append (gstr_argv, filename);
    
    /* Setting arguments to be passed to file */
    if (argv != NULL)
    {
        int i = -1;
        while (argv [++i])
        {
            gstr_argv = g_string_append_c (gstr_argv, ' ');
            gstr_argv = g_string_append (gstr_argv, argv [i]);
        }
    }
    
    /* Executing file */
    write (python_shell_data->master_fd, gstr_argv->str, gstr_argv->len);
    write (python_shell_data->master_fd, "\n", 1);
    
    g_string_free (gstr_argv, TRUE);
    py_shell_state = PY_SHELL_STATE_FILE;
}

/* To show Python Shell
 * Window
 */
static void
show_python_shell_win ()
{
    python_shell_builder = gtk_builder_new ();
    gtk_builder_add_from_file (python_shell_builder, 
                              "./ui/python_shell_win.glade", NULL);

    python_shell_win = GTK_WIDGET (gtk_builder_get_object (python_shell_builder,
                                                 "python_shell_win"));

    g_signal_connect (G_OBJECT (python_shell_win), "destroy",
                     G_CALLBACK (python_shell_destroy), NULL);

    python_shell_source_buffer = gtk_source_buffer_new (NULL);
    python_shell_source_view = gtk_source_view_new_with_buffer (python_shell_source_buffer);

    gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (python_shell_source_view), 
                                  GTK_WRAP_WORD);

    g_signal_connect (G_OBJECT (python_shell_source_view), "key-press-event",
                     G_CALLBACK (python_shell_text_view_key_press_event), NULL); 

    gtk_container_add (GTK_CONTAINER (gtk_builder_get_object (python_shell_builder, "scroll_win")),
                       python_shell_source_view);

    GtkSourceLanguageManager *languagemanager = gtk_source_language_manager_new();
    GtkSourceLanguage *language = gtk_source_language_manager_guess_language (languagemanager, "file.py", NULL);
    gtk_source_buffer_set_language (python_shell_source_buffer, language);

    /* Connecting activate signals to File menu items */
    g_signal_connect (gtk_builder_get_object (python_shell_builder, "file_import_module"), "activate",
                     G_CALLBACK (file_import_module_activate), NULL);
    g_signal_connect (gtk_builder_get_object (python_shell_builder, "file_save"), "activate",
                     G_CALLBACK (file_save_activate), NULL);
     /************************************/

    /*Connecting activate signals of Edit menu items */
    g_signal_connect (gtk_builder_get_object (python_shell_builder, "edit_undo"), "activate",
                     G_CALLBACK (py_shell_edit_undo_activate), NULL);
    g_signal_connect (gtk_builder_get_object (python_shell_builder, "edit_redo"), "activate",
                     G_CALLBACK (py_shell_edit_redo_activate), NULL);
    g_signal_connect (gtk_builder_get_object (python_shell_builder, "edit_cut"), "activate",
                     G_CALLBACK (py_shell_edit_cut_activate), NULL);
    g_signal_connect (gtk_builder_get_object (python_shell_builder, "edit_copy"), "activate",
                     G_CALLBACK (py_shell_edit_copy_activate), NULL);
    g_signal_connect (gtk_builder_get_object (python_shell_builder, "edit_paste"), "activate",
                     G_CALLBACK (py_shell_edit_paste_activate), NULL);
    g_signal_connect (gtk_builder_get_object (python_shell_builder, "edit_select_all"), "activate",
                     G_CALLBACK (py_shell_edit_select_all_activate), NULL);
    g_signal_connect (gtk_builder_get_object (python_shell_builder, "edit_delete"), "activate",
                     G_CALLBACK (py_shell_edit_delete_activate), NULL);
    g_signal_connect (gtk_builder_get_object (python_shell_builder, "edit_find"), "activate",
                     G_CALLBACK (py_shell_edit_find_activate), NULL);
    /************************************/
    
    /*Connecting activate signals of Shell Menu items */
    g_signal_connect (gtk_builder_get_object (python_shell_builder, "shell_restart_shell"), "activate",
                     G_CALLBACK (shell_restart_shell_activate), NULL);
    g_signal_connect (gtk_builder_get_object (python_shell_builder, "shell_view_last_restart"), "activate",
                     G_CALLBACK (shell_view_last_restart_activate), NULL);
    /************************************/

    /*Connecting activate signals of Debug Menu Items */
    g_signal_connect (gtk_builder_get_object (python_shell_builder, "debug_go_to_file_error"), "activate",
                     G_CALLBACK (debug_go_to_file_error_activate), NULL);
    /**************************************/
    
    gtk_widget_set_size_request (python_shell_win, 600, 400);
    gtk_widget_show_all (python_shell_win);
    python_shell_loaded = TRUE;
}

/* Function to read data from 
 * master_fd stream whenever
 * available
 */
gboolean
read_masterFd (GIOChannel *channel, GIOCondition condition, gpointer data)
{
    GError *error = NULL;
    gchar buffer [1024];
    gssize size_read;
    GIOStatus status;

    if (!channel->is_readable)
        return TRUE;

    memset (&buffer, 0, sizeof(buffer));
    
    //status = g_io_channel_read_chars (channel, buffer, 255, &size_read, NULL);
    //if (status != G_IO_STATUS_ERROR)
    if (read (python_shell_data->master_fd, buffer, 1024) != 0)
    {
        if (!shell_str)
           shell_str = g_strdup (buffer);

        if (!python_shell_loaded)
            return TRUE;

        if (strcmp (buffer, shell_str) != 0)
            python_shell_text_view_append_output (buffer, -1);
            
        if (g_io_channel_get_buffer_condition (channel) == G_IO_IN)
            return TRUE;

        /*To detect whether script has ended or not*/
        if (py_shell_state == PY_SHELL_STATE_FILE)
        {
            gchar *pos = g_strrstr (buffer, shell_str);
            if (pos != NULL)
            {
                /*If shell_str is at the end of buffer then restart shell */
                if (strlen (pos) == strlen (shell_str) || strlen (pos) == strlen (shell_str) + 1)
                {
                    GtkTextBuffer *textbuffer =  GTK_TEXT_BUFFER (python_shell_source_buffer);

                    gtk_text_buffer_insert_at_cursor (textbuffer, RESTART_MSG, -1);

                    while (gtk_events_pending ())
                        gtk_main_iteration_do (FALSE);

                    /* Executing new python shell process */
                    char *py_argv[] = {options.python_shell_path, NULL};
                    if (!execute_python_shell (NULL, py_argv))
                       return;

                    py_shell_state = PY_SHELL_STATE_SHELL;
                }
            }
        }
    }

    return TRUE;
}

static void 
close_python_shell_process ()
{
    g_io_channel_flush (python_shell_data->channel, NULL);

    gboolean term_success = FALSE;

    /* Terminate the process by sending SIGTERM */
    GFile *proc = g_file_new_for_path ("/proc");
    GFileEnumerator *enumerator = g_file_enumerate_children (proc, 
                                                             G_FILE_ATTRIBUTE_STANDARD_NAME,
                                                             G_FILE_QUERY_INFO_NONE,
                                                             NULL, NULL);    
    GFileInfo *folder_info;
    GError *error = NULL;
    int pid;
    while (enumerator && 
           (folder_info = g_file_enumerator_next_file (enumerator, NULL, &error)))
    {
        /* Enumerate through /proc and find the process */
        /* whose PPID is the pid of terminal */
        const gchar *name = g_file_info_get_name (folder_info);
        if (g_strtod (name, NULL) <= python_shell_data->pid)
        {
            g_object_unref (folder_info);
            continue;
        }

        GFile *proc_child = g_file_get_child (proc, name);
        GFile *status_file = g_file_get_child (proc_child, "status");
        gchar *status_path;
        gchar *contents;
        gsize readed = -1;
        if (!status_file)
        {
            g_object_unref (proc_child);
            g_object_unref (folder_info);
            continue;
        }
        
        if (g_file_load_contents (status_file, NULL, &contents, 
                                  &readed, NULL, NULL))
        {
            gchar *ppid = g_strstr_len (contents, -1, "PPid:");
            if (ppid)
            {
                GString *g_ppid = g_string_new("");
                ppid += strlen ("PPid:");
                ppid--;
                while (*(++ppid) != '\n')
                {
                    if (isdigit (*ppid))
                        g_string_append_c (g_ppid, *ppid);
                }
                
                if (g_strtod (g_ppid->str, NULL) == python_shell_data->pid)
                {
                    /* Found the process with PPID equal to PID of Terminal */
                    gchar *str_pid = g_strstr_len (contents, -1, "Pid:");
                    g_ppid = g_string_erase (g_ppid, 0, -1);
                    if (str_pid)
                    {
                        str_pid += strlen ("Pid:");
                        str_pid--;
                        while (*(++str_pid) != '\n')
                        {
                            if (isdigit (*str_pid))
                                g_string_append_c (g_ppid, *str_pid);
                        }

                    pid = g_strtod (g_ppid->str, NULL);
                    /* Kill the process */
                    kill (pid, SIGKILL);

                    /* Wait for the python to terminate */
                    int status;
                    waitpid (pid, &status, 0);
                    term_success  = TRUE;
                    }
                }
                g_string_free (g_ppid, TRUE);
            }                      
        }

        g_object_unref (status_file);
        g_object_unref (proc_child);
        g_object_unref (folder_info);
    }

    g_object_unref (proc);
    
    /* If cannot find the process, then do it the following way */
    if (!term_success)
    {
        write (python_shell_data->master_fd, "exit ()", strlen ("exit ()"));
        write (python_shell_data->master_fd, "\n", 1);
    }

    //if (python_shell_data->argv)
        //g_strfreev (python_shell_data->argv);
    
}

/* Called when Python Shell 
 * window is destroyed
 */
void
python_shell_destroy (GtkWidget *widget)
{
    close_python_shell_process ();
    python_shell_loaded = FALSE;
}

/* Execute Python Shell 
 * process with supplied arguments
 */
gboolean
execute_python_shell (gchar *curr_dir, char *argv[])
{
    if (!bash_loaded)
    {
        GError *error  = NULL;
    
        python_shell_data = g_try_malloc (sizeof (ChildProcessData));
        python_shell_data->argv = NULL;
        python_shell_data->slave_termios = g_try_malloc (sizeof (struct termios));
        python_shell_data->current_dir = NULL;
        
        /* If bash hasn't been loaded then load it first */
        if (!(bash_loaded = ptyFork (python_shell_data, &error)))
        {
            gchar *msg = g_strdup_printf ("Cannot run BASH (%s)", error->message);
            gtk_statusbar_push (GTK_STATUSBAR (status_bar), 0, msg);
            g_free (msg);
            g_error_free (error);
            return FALSE;
        }
    }

    python_shell_data->argv = g_strdupv (argv);
    python_shell_data->current_dir = curr_dir;
    
    /* Setting curr_dir */
    if (curr_dir)
    {
        GString *cd_gstring  = g_string_new ("cd ");
        cd_gstring = g_string_append (cd_gstring, curr_dir);
        write (python_shell_data->master_fd, cd_gstring->str, cd_gstring->len);
        write (python_shell_data->master_fd, "\n", 1);
        g_string_free (cd_gstring, TRUE);
    }
    
    /*Setting Kivy environment variable
     * remove it and move it to options */
    GString *str  = g_string_new ("export PYTHONPATH=/home/abhi/kivy_repo_me/kivy && ");
    str = g_string_append (str, python_shell_data->argv [0]);

    /* Running Python Shell with argv */
    if (python_shell_data->argv)
    {
        gchar **p = python_shell_data->argv;
        p++;
        while (*p)
        {
            str = g_string_append_c (str, ' ');
            str = g_string_append (str, *p);
            p++;
        }
    }

    write (python_shell_data->master_fd, str->str, str->len);
    write (python_shell_data->master_fd, "\n", 1);
    g_string_free (str, TRUE);
    return TRUE;
}

/* To append output to the 
 * shell and adjust last_pos
 */
void
python_shell_text_view_append_output (gchar *text, gssize len)
{
    GtkTextIter iter;
    GtkTextBuffer *buffer =  GTK_TEXT_BUFFER (python_shell_source_buffer);
    /*gtk_text_buffer_get_iter_at_mark (buffer, &iter,
                                     gtk_text_buffer_get_insert (buffer));*/
    gtk_text_buffer_get_end_iter (buffer, &iter);
    /*Check if text contains ...*/
    if (g_strcmp0 (text, "... ") == 0)
    {
        /* If yes, then indentation should also be inserted*/
        gchar *prev_line_text = gtk_text_buffer_get_line_text (buffer,
                                                              gtk_text_iter_get_line (&iter) - 1, TRUE);
        int indentation = get_indent_spaces_in_string (prev_line_text);
        int i;

        if (strrchr (prev_line_text, ':') != NULL)
            indentation += options.indent_width;

        gchar indent_str [indentation];
        for (i = 0; i < indentation; i++)
            indent_str [i] = ' ';
        
    gtk_text_buffer_insert (buffer, &iter, indent_str, indentation);
    gtk_text_buffer_get_iter_at_mark (buffer, &iter,
                                     gtk_text_buffer_get_insert (buffer));
    last_pos = gtk_text_iter_get_offset (&iter) - indentation;
    gtk_text_view_scroll_to_iter (GTK_TEXT_VIEW (python_shell_source_view), &iter, 0.0, FALSE, 0, 0);
    return;
    }

    gtk_text_buffer_insert (buffer, &iter, text, len);
    gtk_text_buffer_get_iter_at_mark (buffer, &iter,
                                     gtk_text_buffer_get_insert (buffer));
    last_pos = gtk_text_iter_get_offset (&iter);

    while (gtk_events_pending ())
        gtk_main_iteration_do (FALSE);
    
    gtk_text_buffer_get_end_iter (buffer, &iter);
    gtk_text_view_scroll_to_iter (GTK_TEXT_VIEW (python_shell_source_view), &iter, 0.0, FALSE, 0, 0);
}

/* Handler of "key-press-event"
 * for python_shell_source_view
 */
gboolean
python_shell_text_view_key_press_event (GtkWidget *widget, GdkEvent *event)
{
    GtkTextIter iter, new_iter, last_pos_iter;
    GtkTextBuffer *buffer =  GTK_TEXT_BUFFER (python_shell_source_buffer);
    int col, line, pos;

    gtk_text_buffer_get_iter_at_mark (buffer, &iter, gtk_text_buffer_get_insert (buffer));
    col = gtk_text_iter_get_line_offset (&iter);
    line = gtk_text_iter_get_line (&iter);
    pos = gtk_text_iter_get_offset (&iter);
    
    if (pos < last_pos)
       return TRUE;
    
    if (event->key.keyval == GDK_KEY_Return)
    {
        gtk_text_buffer_get_iter_at_offset (buffer, &last_pos_iter, last_pos);
        gchar *text = gtk_text_buffer_get_text (buffer, &last_pos_iter, &iter, TRUE);
        
          /* Check if text contains only spaces */
        if (strlen (text) == get_indent_spaces_in_string (text))
        {
            /*If yes then write only "\n"*/
            write (python_shell_data->master_fd, "\n", 1);
            return FALSE;
        }
        
        if (g_strcmp0 (text, "") == 0)
        {
            write (python_shell_data->master_fd, "\n", 1);
            return FALSE;
        }

        if (write (python_shell_data->master_fd, text, strlen (text)) > 0)
        //gssize written;
        //if (g_io_channel_write_chars (python_shell_data->channel, text, strlen (text), &written, NULL))
        {
            write (python_shell_data->master_fd, "\n", 1);
            return FALSE;
        }
        return TRUE;
    }

    return FALSE;
}

void
file_import_module_activate (GtkWidget *widget)
{
    GtkWidget *dialog = gtk_dialog_new_with_buttons ("Import Module",
                                                    GTK_WINDOW (python_shell_win),
                                                    GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                                    "Open", 1, "Cancel", 0, NULL);
    GtkWidget *label = gtk_label_new ("Enter the name of module to import");
    GtkWidget *entry = gtk_entry_new ();

    gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (dialog))),
                       label, FALSE, FALSE, 2);
    gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (dialog))),
                       entry, FALSE, FALSE, 2);
    
    gtk_widget_show_all (dialog);
    if (gtk_dialog_run (GTK_DIALOG (dialog)) == 1)
    {
        const gchar *module = gtk_entry_get_text (GTK_ENTRY (entry));
        gchar *text = g_strdup_printf ("import %s", module);
        write (python_shell_data->master_fd, text, strlen (text));
        write (python_shell_data->master_fd, "\n", 1);

        GtkTextIter iter;
        GtkTextBuffer *buffer =  GTK_TEXT_BUFFER (python_shell_source_buffer);
        gtk_text_buffer_get_iter_at_mark (buffer, &iter,
                                         gtk_text_buffer_get_insert (buffer));
        gtk_text_buffer_insert (buffer, &iter, "\n", 1);
        g_free (text);
    }
    gtk_widget_destroy (dialog);
}

void
file_save_activate (GtkWidget *);

void
py_shell_edit_undo_activate (GtkWidget *widget)
{
    if (gtk_source_buffer_can_undo (python_shell_source_buffer))
        gtk_source_buffer_undo (python_shell_source_buffer);
}

void
py_shell_edit_redo_activate (GtkWidget *widget)
{
    if (gtk_source_buffer_can_redo (python_shell_source_buffer))
        gtk_source_buffer_redo (python_shell_source_buffer);
}

void
py_shell_edit_cut_activate (GtkWidget *widget)
{
    GtkTextBuffer *buffer =  GTK_TEXT_BUFFER (python_shell_source_buffer);
    GtkClipboard *clipboard = gtk_clipboard_get (GDK_NONE);
    gtk_text_buffer_cut_clipboard (buffer, clipboard, TRUE);
}

void
py_shell_edit_copy_activate (GtkWidget *widget)
{
    GtkTextBuffer *buffer =  GTK_TEXT_BUFFER (python_shell_source_buffer);
    GtkClipboard *clipboard = gtk_clipboard_get (GDK_NONE);
    gtk_text_buffer_copy_clipboard (buffer, clipboard);
}

void
py_shell_edit_paste_activate (GtkWidget *widget)
{
    GtkTextBuffer *buffer =  GTK_TEXT_BUFFER (python_shell_source_buffer);
    GtkClipboard *clipboard = gtk_clipboard_get (GDK_NONE);
    gtk_text_buffer_paste_clipboard (buffer, clipboard, NULL, TRUE);
}

void
py_shell_edit_select_all_activate (GtkWidget *widget)
{
    GtkTextBuffer *buffer =  GTK_TEXT_BUFFER (python_shell_source_buffer);
    GtkTextIter start_iter, end_iter;
    gtk_text_buffer_get_start_iter (buffer, &start_iter);
    gtk_text_buffer_get_end_iter (buffer, &end_iter);
    gtk_text_buffer_select_range (buffer, &start_iter, &end_iter);
}

void
py_shell_edit_delete_activate (GtkWidget *widget)
{
    GtkTextBuffer *buffer =  GTK_TEXT_BUFFER (python_shell_source_buffer);
    gtk_text_buffer_delete_selection (buffer, TRUE, TRUE);
}

void
py_shell_edit_find_activate (GtkWidget *widget)
{
    load_find_dialog (python_shell_source_view);
}

static gboolean 
_shell_restart (gpointer data)
{
    GtkTextBuffer *buffer =  GTK_TEXT_BUFFER (python_shell_source_buffer);
    
    /* Check master_fd, for any readable data, restart shell 
     * only when all data has been read
     */

    int epoll_fd;
    struct epoll_event ev;
    struct epoll_event ev_arr[1];

    if ((epoll_fd = epoll_create (1)) == -1)
        return TRUE;
    
    ev.events = EPOLLIN;
    ev.data.fd = python_shell_data->master_fd;

    if (epoll_ctl (epoll_fd, EPOLL_CTL_ADD, 
        python_shell_data->master_fd, &ev) == -1)
        return TRUE;
    
    if (epoll_wait (epoll_fd, ev_arr, 1, 0))
        return TRUE;

    gtk_text_buffer_append (buffer, RESTART_MSG, -1);

    /* Executing new python shell process */
    char *py_argv[] = {options.python_shell_path, NULL};
    if (!execute_python_shell (NULL, py_argv))
       return FALSE;

    py_shell_state = PY_SHELL_STATE_SHELL;
    return FALSE;
}

void
shell_restart_shell_activate (GtkWidget *widget)
{
    if (!python_shell_loaded)
         return;

    GtkTextBuffer *buffer =  GTK_TEXT_BUFFER (python_shell_source_buffer);

    /* Closing the current open shell */
    close_python_shell_process ();
    
    g_timeout_add (10, (GSourceFunc)_shell_restart, NULL);
}

void
shell_view_last_restart_activate (GtkWidget *widget)
{
    find_previous (GTK_TEXT_VIEW (python_shell_source_view), RESTART_MSG,
                   GTK_TEXT_SEARCH_TEXT_ONLY);
}

/* To go to the line in the file
 * where error occurred
 */
void
debug_go_to_file_error_activate (GtkWidget *widget)
{
    GtkTextBuffer *buffer =  GTK_TEXT_BUFFER (python_shell_source_buffer);
    GtkTextIter iter;
    
    gtk_text_buffer_get_iter_at_mark (buffer, &iter, gtk_text_buffer_get_insert (buffer));
    
    gchar *text = gtk_text_buffer_get_line_text (buffer, gtk_text_iter_get_line (&iter), FALSE);

    GRegex *regex;
    GMatchInfo *match_info;
    gchar *file_path = NULL;
    gint line = -1;

    regex = g_regex_new ("\"+.+\"", 0, 0, NULL);
    if (g_regex_match (regex, text, 0, &match_info))
    {
        file_path = g_match_info_fetch (match_info, 0);
        file_path = remove_char (file_path, '"');
        file_path = remove_char (file_path, '"');
    }
    
    g_match_info_free (match_info);
    g_regex_unref (regex);

    gchar *line_str;
    regex = g_regex_new ("\\bline(.)+,", 0, 0, NULL);
    if (g_regex_match (regex, text, 0, &match_info))
    {
        line_str = g_match_info_fetch (match_info, 0);
        line_str = remove_char (line_str, ',');
        gchar *p = g_strstr_len (line_str, -1, "line");
        p += strlen("line");
        p = g_strstrip (p);
        
        line = g_strtod (p, NULL);
        g_free (line_str);
    }
    
    go_to_file_at_line (file_path, line);
}
