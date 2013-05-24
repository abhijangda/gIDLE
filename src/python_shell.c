#include "main.h"
#include "core_funcs.h"
#include "pty_fork.h"
#include "python_shell.h"

#define RESTART_MSG "\n\n############Restarting Shell###############\n\n"

GtkBuilder *python_shell_builder;
static gboolean python_shell_loaded = FALSE;
extern GtkWidget *status_bar;
extern ChildProcessData *python_shell_data;

static void show_python_shell_win ();
static gboolean read_masterFd (GIOChannel *channel, GIOCondition condition, gpointer data);
static void close_python_shell_process ();

static GtkWidget *python_shell_source_view;
static GtkSourceBuffer *python_shell_source_buffer;
static gint last_pos = 0;
GtkWidget *python_shell_win;

/* To Load Python Shell
 * window
 */
void
load_python_shell ()
{
    if (python_shell_loaded)
        return;

    char *py_argv[] = {python_shell_path, NULL};
    if (!execute_python_shell (py_argv))
       return;

    python_shell_data->channel = g_io_channel_unix_new (python_shell_data->master_fd);
    g_io_add_watch (python_shell_data->channel, G_IO_IN,
                   (GIOFunc)read_masterFd, &(python_shell_data->master_fd));
    
    show_python_shell_win ();
}

/* To run filename in
 * Python Shell
 */
void
run_file_in_python_shell (char *filename)
{
    if (python_shell_loaded)
        close_python_shell_process ();
    else
        show_python_shell_win ();
    
    char *py_argv[] = {python_shell_path, filename, NULL};
    if (!execute_python_shell (py_argv))
       return;

    python_shell_data->channel = g_io_channel_unix_new (python_shell_data->master_fd);
    g_io_add_watch (python_shell_data->channel, G_IO_IN,
                   (GIOFunc)read_masterFd, &(python_shell_data->master_fd));
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
static gboolean
read_masterFd (GIOChannel *channel, GIOCondition condition, gpointer data)
{
    GError *error = NULL;
    gchar buffer [256];
    gssize size_read;
    GIOStatus status;

    if (!channel->is_readable)
        return TRUE;

    memset (&buffer, 0, sizeof(buffer));
    
    //status = g_io_channel_read_chars (channel, buffer, 255, &size_read, NULL);
    //if (status != G_IO_STATUS_ERROR)
    if (read (python_shell_data->master_fd, buffer, 255) != 0)
        python_shell_text_view_append_output (buffer, -1);

    return TRUE;
}

static void 
close_python_shell_process ()
{
    g_io_channel_unref (python_shell_data->channel);
    //g_io_channel_shutdown (python_shell_data->channel, TRUE, NULL);
    close (python_shell_data->master_fd);
    g_spawn_close_pid (python_shell_data->pid);
    
    g_strfreev (python_shell_data->argv);
    g_free (python_shell_data->slave_termios);
    g_free (python_shell_data);
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
execute_python_shell (char *argv[])
{
    GError *error = NULL;

    python_shell_data = g_try_malloc (sizeof (ChildProcessData));
    python_shell_data->argv = g_strdupv (argv);
    python_shell_data->slave_termios = g_try_malloc (sizeof (struct termios));

    tcgetattr(0, python_shell_data->slave_termios);

    if (!ptyFork (python_shell_data, &error))
    {
        gchar *msg = g_strdup_printf ("Cannot run Python Shell (%s)", error->message);
        gtk_statusbar_push (GTK_STATUSBAR (status_bar), 0, msg);
        g_free (msg);
        g_error_free (error);
        return FALSE;
    }
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
    gtk_text_buffer_get_iter_at_mark (buffer, &iter,
                                     gtk_text_buffer_get_insert (buffer));

    /*Check if text contains ...*/
    if (g_strcmp0 (text, "... ") == 0)
    {
        /* If yes, then indentation should also be inserted*/
        gchar *prev_line_text = gtk_text_buffer_get_line_text (buffer,
                                                              gtk_text_iter_get_line (&iter) - 1);
        int indentation = get_indent_spaces_in_string (prev_line_text);
        int i;

        if (strrchr (prev_line_text, ':') != NULL)
            indentation += indent_width;

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

void
shell_restart_shell_activate (GtkWidget *widget)
{
    if (!python_shell_loaded)
         return;

    GtkTextBuffer *buffer =  GTK_TEXT_BUFFER (python_shell_source_buffer);

    /* Closing the current open shell */
    close_python_shell_process ();
    
    gtk_text_buffer_insert_at_cursor (buffer,
                                      RESTART_MSG,
                                      -1);

    /* Executing new python shell process */
    char *py_argv[] = {python_shell_path, NULL};
    if (!execute_python_shell (py_argv))
       return;
    
    python_shell_data->channel = g_io_channel_unix_new (python_shell_data->master_fd);
    g_io_add_watch (python_shell_data->channel, G_IO_IN,
                   (GIOFunc)read_masterFd, &(python_shell_data->master_fd));
}

void
shell_view_last_restart_activate (GtkWidget *widget)
{
    find_previous (GTK_TEXT_VIEW (python_shell_source_view), RESTART_MSG,
                   GTK_TEXT_SEARCH_TEXT_ONLY);
}

void
debug_go_to_file_error_activate (GtkWidget *widget)
{
    
}