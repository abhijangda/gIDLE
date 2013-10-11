#include <gtk/gtk.h>

#include "codewidget.h"

#ifndef PATH_BROWSER_H
#define PATH_BROWSER_H

#define PATH_BROWSER_TYPE (path_browser_get_type ())
#define PATH_BROWSER(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), PATH_BROWSER_TYPE, PathBrowser))

#define PATH_BROWSER_CLASS(klass) (G_TYPE_CHECK_INSTANCE_CAST ((klass), PATH_BROWSER_TYPE, PathBrowserClass))

#define IS_PATH_BROWSER(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PATH_BROWSER_TYPE)

#define IS_PATH_BROWSER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PATH_BROWSER_TYPE))

enum
{
    PATH_BROWSER_COL_PIXBUF,
    PATH_BROWSER_COL_NAME,
    PATH_BROWSER_TOTAL_COLS,
};

struct path_browser
{
    GtkBox box;
};

struct _path_browser_class
{
    GtkBoxClass parent_class;
    
    void (*open_file) (gchar *path);
};

typedef struct path_browser PathBrowser;
typedef struct _path_browser_class PathBrowserClass;

GType path_browser_get_type (void) G_GNUC_CONST;
GtkWidget *path_browser_new (void);
#endif /*PATH_BROWSER_H*/