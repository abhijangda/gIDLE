#include <string.h>

#include "path_browser.h"

#define PATH_BROWSER_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), PATH_BROWSER_TYPE, PathBrowserPrivate))

struct _path_browser_private
{
    GtkWidget *tree_view;
    GtkTreeStore *tree_store;
    GtkWidget *scrollwin;
    
    GtkWidget *popup_menu;
};

typedef struct _path_browser_private PathBrowserPrivate;

static void
path_browser_class_init (PathBrowserClass *klass);

static void
path_browser_init (PathBrowser *sym_view);

static void
_row_expanded (GtkTreeView *tree_view, GtkTreeIter *iter, GtkTreePath *path, gpointer data);

static void
_row_collapsed (GtkTreeView *tree_view, GtkTreeIter *iter, GtkTreePath *path, gpointer data);

static void
_path_browser_add_for_path (PathBrowser *path_browser, GFile *dir, GtkTreeIter *parent);

GType
path_browser_get_type (void)
{
    static GType box_type = 0;
    
    if (!box_type)
    {
        static const GTypeInfo path_browser_type = 
        {
            sizeof (PathBrowserClass),
            NULL,
            NULL,
            (GClassInitFunc) path_browser_class_init,
            NULL,
            NULL,
            sizeof (PathBrowser),
            0,
            (GInstanceInitFunc) path_browser_init,
        };
        box_type = g_type_register_static (GTK_TYPE_BOX, "PathBrowser", &path_browser_type, 0);
    }
    return box_type;
}

static void
path_browser_class_init (PathBrowserClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    
    g_type_class_add_private (klass, sizeof (PathBrowserPrivate));
}

static void
path_browser_init (PathBrowser *sym_view)
{
    PathBrowserPrivate *priv = PATH_BROWSER_GET_PRIVATE (sym_view);
    
    priv->scrollwin = gtk_scrolled_window_new (NULL, NULL);
    
    gtk_box_pack_start (GTK_BOX (sym_view), priv->scrollwin, TRUE, TRUE, 2);

    priv->tree_view = gtk_tree_view_new ();
    gtk_container_add (GTK_CONTAINER (priv->scrollwin), priv->tree_view);

    g_object_ref (priv->scrollwin);

    priv->tree_store = gtk_tree_store_new (PATH_BROWSER_TOTAL_COLS,
                                                      GDK_TYPE_PIXBUF,
                                                      G_TYPE_STRING);
    
    GtkCellRenderer *renderer = gtk_cell_renderer_pixbuf_new ();
    GtkTreeViewColumn *column = gtk_tree_view_column_new ();
    gtk_tree_view_column_pack_start (column, renderer, FALSE);
    gtk_tree_view_column_set_attributes (column,
                                                                           renderer, "pixbuf", PATH_BROWSER_COL_PIXBUF , NULL);

    gtk_tree_view_append_column (GTK_TREE_VIEW (priv->tree_view), column);

    renderer = gtk_cell_renderer_text_new ();
    gtk_tree_view_column_pack_start (column, renderer, FALSE);
    gtk_tree_view_column_add_attribute (column, renderer, "text", PATH_BROWSER_COL_NAME);

    gtk_tree_view_column_set_sizing(column, GTK_TREE_VIEW_COLUMN_AUTOSIZE);

    gtk_tree_view_set_model (GTK_TREE_VIEW (priv->tree_view),
                              GTK_TREE_MODEL (priv->tree_store));
    gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (priv->tree_view),
                                        FALSE);
    
    g_signal_connect (G_OBJECT (priv->tree_view), "row-expanded", G_CALLBACK (_row_expanded), sym_view);
    g_signal_connect (G_OBJECT (priv->tree_view), "row-collapsed", G_CALLBACK (_row_collapsed), sym_view);
    /***************/
}

GtkWidget *
path_browser_new ()
{
    GtkWidget *widget = GTK_WIDGET (g_object_new (path_browser_get_type (), NULL));
    gtk_orientable_set_orientation (GTK_ORIENTABLE (widget), GTK_ORIENTATION_VERTICAL);
    return widget;
}

static void
_row_expanded (GtkTreeView *tree_view, GtkTreeIter *iter, GtkTreePath *path, gpointer data)
{
    PathBrowserPrivate *priv = PATH_BROWSER_GET_PRIVATE (data);
    
    GtkTreeIter iter1 = *(gtk_tree_iter_copy (iter));

    GString *file_path_str = g_string_new ("");
    while (gtk_tree_store_iter_is_valid (priv->tree_store, &iter1))
    {
        gchar *fpath;
        gtk_tree_model_get (GTK_TREE_MODEL (priv->tree_store), &iter1, PATH_BROWSER_COL_NAME, &fpath, -1);
        printf ("%s\n", fpath);
        file_path_str = g_string_prepend (file_path_str, fpath);
        file_path_str = g_string_prepend_c (file_path_str, '/');
        g_free (fpath);
        
        GtkTreeIter parent_iter;
        if (!gtk_tree_model_iter_parent (GTK_TREE_MODEL (priv->tree_store), &parent_iter, &iter1))
             break;
        
        iter1 = *(gtk_tree_iter_copy (&parent_iter));
    }
    if (file_path_str->str [0] == file_path_str->str [1] == '/')
        g_string_erase (file_path_str, 0, 1);
    
    GtkTreeIter child;
    
    while (gtk_events_pending ())
        gtk_main_iteration_do (FALSE);

    GFile *dir = g_file_new_for_path (file_path_str->str);
    _path_browser_add_for_path (PATH_BROWSER (data), dir, iter);
    g_object_unref (dir);
    g_string_free (file_path_str, TRUE);
    
    if (gtk_tree_model_iter_children (GTK_TREE_MODEL (priv->tree_store), &child, iter))
        gtk_tree_store_remove (priv->tree_store, &child);
}

static void
_row_collapsed (GtkTreeView *tree_view, GtkTreeIter *iter, GtkTreePath *path, gpointer data)
{
    PathBrowserPrivate *priv = PATH_BROWSER_GET_PRIVATE (data);

    GtkTreeIter child;
    while (gtk_tree_model_iter_children (GTK_TREE_MODEL (priv->tree_store), &child, iter))
        gtk_tree_store_remove (priv->tree_store, &child);
    
    gtk_tree_store_append (priv->tree_store, &child, iter);
}

static void
_path_browser_add_for_path (PathBrowser *path_browser, GFile *dir, GtkTreeIter *parent)
{
    PathBrowserPrivate *priv = PATH_BROWSER_GET_PRIVATE (path_browser);
    GFileInfo *dirinfo;
    
    GFileEnumerator *enumerator;
    enumerator = g_file_enumerate_children (dir,
                                            G_FILE_ATTRIBUTE_STANDARD_NAME
                                            "," 
                                            G_FILE_ATTRIBUTE_STANDARD_TYPE,
                                            G_FILE_QUERY_INFO_NONE,
                                            NULL, NULL);
    if (!enumerator)
        return;

    GFileInfo *fileinfo;

    while ((fileinfo = g_file_enumerator_next_file (enumerator, NULL, NULL)) != NULL)
    {
        GdkPixbuf *pixbuf;
        GtkTreeIter dir_iter;
        const gchar *file_name = g_file_info_get_name (fileinfo);
        GFile *child = g_file_get_child (dir, file_name);

         if (g_file_info_get_file_type (fileinfo) == G_FILE_TYPE_DIRECTORY)
        {
            gtk_tree_store_insert (priv->tree_store, &dir_iter, parent, 1);
            gtk_tree_store_set (priv->tree_store, &dir_iter,
                             PATH_BROWSER_COL_NAME, file_name, -1);

            pixbuf =  gdk_pixbuf_new_from_file ("./icons/directory.png", NULL);
            gtk_tree_store_set (priv->tree_store, &dir_iter, PATH_BROWSER_COL_PIXBUF, 
                                 pixbuf, -1);
            GtkTreeIter iter2;
            gtk_tree_store_append (priv->tree_store, &iter2, &dir_iter);
            g_object_unref (pixbuf);
        }
        else if (g_strcmp0 (strrchr (file_name, '.'), ".py") == 0)
        {
            gtk_tree_store_append (priv->tree_store, &dir_iter, parent);
            gtk_tree_store_set (priv->tree_store, &dir_iter,
                             PATH_BROWSER_COL_NAME, file_name, -1);

            pixbuf = gdk_pixbuf_new_from_file ("./icons/py_file.png", NULL);
            gtk_tree_store_set (priv->tree_store, &dir_iter, PATH_BROWSER_COL_PIXBUF, 
                                  pixbuf, -1);
            g_object_unref (pixbuf);
        }
        g_object_unref (child);
        g_object_unref (fileinfo);
    }
    g_object_unref (enumerator);
}

void
path_browser_show_paths (PathBrowser *path_browser)
{
    PathBrowserPrivate *priv = PATH_BROWSER_GET_PRIVATE (path_browser);
    GtkTreeIter iter;
    if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (priv->tree_store), &iter))
        return;

    extern char *sys_path_string;
    char **sys_pathv = g_strsplit (sys_path_string, "\n", 0);
    char **path = sys_pathv;
    while (*path)
    {
        GFile *file = g_file_new_for_path (*path);
        GFileInfo *fileinfo = g_file_query_info (file, G_FILE_ATTRIBUTE_STANDARD_TYPE, G_FILE_QUERY_INFO_NONE, NULL, NULL);

        if (fileinfo && g_file_info_get_file_type (fileinfo) == G_FILE_TYPE_DIRECTORY)
        {
            GtkTreeIter dir_iter;

            gtk_tree_store_append (priv->tree_store, &dir_iter, NULL);
            gtk_tree_store_set (priv->tree_store, &dir_iter,
                                 PATH_BROWSER_COL_NAME, *path, -1);
    
             if (g_file_info_get_file_type (fileinfo) == G_FILE_TYPE_DIRECTORY)
            {
                GdkPixbuf *pixbuf =  gdk_pixbuf_new_from_file ("./icons/directory.png", NULL);
                gtk_tree_store_set (priv->tree_store, &dir_iter, PATH_BROWSER_COL_PIXBUF, 
                                     pixbuf, -1);
                g_object_unref (pixbuf);
                GtkTreeIter iter;
                gtk_tree_store_append (priv->tree_store, &iter, &dir_iter);
            }
            g_object_unref (fileinfo);
        }

        g_object_unref (file);
        path++;
    }
    g_strfreev (sys_pathv);
}