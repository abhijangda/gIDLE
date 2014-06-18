#include "file_monitor.h"

FileMonitor *file_monitor_new (void (*changed_handler) (GFileMonitor *, GFile *, GFile *, GFileMonitorEvent, gpointer))
{
    FileMonitor *fm = g_malloc0 (sizeof (FileMonitor));
    fm->file_monitor_array = NULL;
    fm->file_path_array  = NULL;
    if (!fm)
        return NULL;

    fm->size = 0;
    fm->changed_handler = changed_handler;
    return fm;
}

gboolean file_monitor_file (FileMonitor *fm, gchar *file)
{
    GFile *gfile = g_file_new_for_path (file);
    if (!gfile)
        return FALSE;
    
    fm->file_monitor_array = g_realloc (fm->file_monitor_array, sizeof (GFileMonitor*)*(fm->size + 1));
    fm->file_path_array = g_realloc (fm->file_path_array, sizeof (gchar*)*(fm->size + 1));

    GFileMonitor *gfm = g_file_monitor_file (gfile,
                                             G_FILE_MONITOR_WATCH_MOUNTS,
                                             NULL, NULL);
    if (!gfm)
        return FALSE;
    
    fm->file_monitor_array [fm->size] = gfm;
    fm->file_path_array [fm->size] = g_strdup (file);
    g_signal_connect (G_OBJECT (fm->file_monitor_array [fm->size]), "changed",
                      G_CALLBACK (fm->changed_handler), NULL);
    fm->size++;
    //g_object_unref (gfile);
    return TRUE;
}

void file_monitor_cancel (FileMonitor *fm, gchar *file)
{
    int i;
    for (i = 0; i < fm->size; i++)
        if (g_strcmp0 (file, fm->file_path_array [i]) == 0)
            break;
    
    if (i == fm->size)
        return;
    
    g_file_monitor_cancel (fm->file_monitor_array [i]);
    
    g_object_unref (fm->file_monitor_array [i]);
    g_free (fm->file_path_array [i]);
    
    int j;
    for (j = i; j < fm->size; j++)
    {
        fm->file_monitor_array [j] = fm->file_monitor_array [j+1];
        fm->file_path_array [j] = fm->file_path_array [j+1];
    }
    fm->file_monitor_array = g_realloc (fm->file_monitor_array, sizeof (GFileMonitor*) * (fm->size - 1));
    fm->file_path_array = g_realloc (fm->file_path_array, sizeof (gchar*) * (fm->size - 1));
    fm->size--;
}

void file_monitor_cancel_all (FileMonitor *fm)
{
    int i;
    for (i = 0; i < fm->size; i++)
    {
        g_file_monitor_cancel (fm->file_monitor_array [i]);
        
        g_object_unref (fm->file_monitor_array [i]);
        g_free (fm->file_path_array [i]);
    }
    
    g_free (fm->file_monitor_array);
    g_free (fm->file_path_array);
    fm->size = 0;
}

void file_monitor_destroy (FileMonitor *fm)
{
    file_monitor_cancel_all (fm);
    g_free (fm);
}