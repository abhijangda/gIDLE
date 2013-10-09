#include <gio/gio.h>

#ifndef FILE_MONITOR_H
#define FILE_MONITOR_H

struct _FileMonitor
{
    GFileMonitor **file_monitor_array;
    gchar **file_path_array;
    void (*changed_handler) (GFileMonitor *, GFile *, GFile *, GFileMonitorEvent, gpointer);
    int size;
};

typedef struct _FileMonitor FileMonitor;

FileMonitor *file_monitor_new (void (*changed_handler) (GFileMonitor *, GFile *, GFile *, GFileMonitorEvent, gpointer));
gboolean file_monitor_file (FileMonitor *, gchar *);
void file_monitor_cancel (FileMonitor *, gchar *);
void file_monitor_cancel_all (FileMonitor *);
void file_monitor_destroy (FileMonitor *);
#endif /*FILE_MONITOR_H*/