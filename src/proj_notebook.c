#include "proj_notebook.h"
#include "main.h"

#include <string.h>

#define PROJECT_NOTEBOOK_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), PROJECT_NOTEBOOK_TYPE, ProjectNotebookPrivate))

extern GtkWidget *status_bar;

struct proj_notebook_priv
{
    GtkWidget *proj_dir_tree_view;
    GtkTreeStore *proj_dir_tree_store;
    GtkWidget *proj_dir_tree_view_scrollwin;

    GtkWidget *proj_tree_view;
    GtkTreeStore *proj_tree_store;
    GtkWidget *proj_tree_view_scrollwin;
};

typedef struct proj_notebook_priv ProjectNotebookPrivate;

static void
project_notebook_class_init (ProjectNotebookClass *klass);

static void
project_notebook_init (ProjectNotebook *proj_notebook);

static gboolean
proj_dir_tree_view_dbl_clicked (GtkWidget *widget, GdkEvent *event, gpointer data);

static gboolean
proj_tree_view_dbl_clicked (GtkWidget *widget, GdkEvent *event, gpointer data);

GType
project_notebook_get_type (void)
{
    static GType notebook_type = 0;
    
    if (!notebook_type)
    {
        static const GTypeInfo proj_notebook_type = 
        {
            sizeof (ProjectNotebookClass),
            NULL,
            NULL,
            (GClassInitFunc) project_notebook_class_init,
            NULL,
            NULL,
            sizeof (ProjectNotebook),
            0,
            (GInstanceInitFunc) project_notebook_init,
        };
        notebook_type = g_type_register_static (GTK_TYPE_NOTEBOOK, "ProjectNotebook", &proj_notebook_type, 0);
    }
    return notebook_type;
}

static void
project_notebook_class_init (ProjectNotebookClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    g_type_class_add_private (klass, sizeof (ProjectNotebookPrivate));
}

static void
project_notebook_init (ProjectNotebook *proj_notebook)
{
    ProjectNotebookPrivate *priv = PROJECT_NOTEBOOK_GET_PRIVATE (proj_notebook);

    priv->proj_dir_tree_view_scrollwin = gtk_scrolled_window_new (NULL, NULL);

    priv->proj_dir_tree_view = gtk_tree_view_new ();
    gtk_container_add (GTK_CONTAINER (priv->proj_dir_tree_view_scrollwin), priv->proj_dir_tree_view);

    g_object_ref (priv->proj_dir_tree_view_scrollwin);

    priv->proj_dir_tree_store = gtk_tree_store_new (DIR_TOTAL_COLS, GDK_TYPE_PIXBUF, G_TYPE_STRING);
    
    GtkCellRenderer *renderer = gtk_cell_renderer_pixbuf_new ();
    GtkTreeViewColumn *column = gtk_tree_view_column_new ();
    gtk_tree_view_column_pack_start (column, renderer, FALSE);
    gtk_tree_view_column_set_attributes (column,
                                                                           renderer, "pixbuf", DIR_COL_PIXBUF , NULL);

    gtk_tree_view_append_column (GTK_TREE_VIEW (priv->proj_dir_tree_view), column);

    renderer = gtk_cell_renderer_text_new ();
    gtk_tree_view_column_pack_start (column, renderer, FALSE);
    gtk_tree_view_column_add_attribute (column, renderer, "text", DIR_COL_NAME);

    gtk_tree_view_column_set_sizing(column, GTK_TREE_VIEW_COLUMN_AUTOSIZE);

    gtk_tree_view_set_model (GTK_TREE_VIEW (priv->proj_dir_tree_view),
                              GTK_TREE_MODEL (priv->proj_dir_tree_store));
    gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (priv->proj_dir_tree_view),
                                        FALSE);

    g_signal_connect (G_OBJECT (priv->proj_dir_tree_view), "button-press-event", 
                        G_CALLBACK (proj_dir_tree_view_dbl_clicked), priv);
    
    gtk_notebook_append_page (GTK_NOTEBOOK (proj_notebook), priv->proj_dir_tree_view_scrollwin, gtk_label_new ("Files"));
    /***************/

    /*Setting proj_tree_view*/
    priv->proj_tree_view_scrollwin = gtk_scrolled_window_new (NULL, NULL);

    priv->proj_tree_view = gtk_tree_view_new ();
    gtk_container_add (GTK_CONTAINER (priv->proj_tree_view_scrollwin), priv->proj_tree_view);

    g_object_ref (priv->proj_tree_view_scrollwin);

    priv->proj_tree_store = gtk_tree_store_new (PROJ_TOTAL_COLS, GDK_TYPE_PIXBUF, G_TYPE_STRING, G_TYPE_STRING);
    
    renderer = gtk_cell_renderer_pixbuf_new ();
    column = gtk_tree_view_column_new ();
    gtk_tree_view_column_pack_start (column, renderer, FALSE);
    gtk_tree_view_column_set_attributes (column,
                                         renderer, "pixbuf", PROJ_COL_PIXBUF , NULL);
 
    gtk_tree_view_append_column (GTK_TREE_VIEW (priv->proj_tree_view), column);

    renderer = gtk_cell_renderer_text_new ();
    gtk_tree_view_column_pack_start (column, renderer, FALSE);
    gtk_tree_view_column_set_attributes (column, renderer, "text", PROJ_COL_NAME, NULL);
    gtk_tree_view_column_set_sizing(column, GTK_TREE_VIEW_COLUMN_AUTOSIZE);

    gtk_tree_view_set_model (GTK_TREE_VIEW (priv->proj_tree_view),
                              GTK_TREE_MODEL (priv->proj_tree_store));
    gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (priv->proj_tree_view),
                                        FALSE);

    g_signal_connect (G_OBJECT (priv->proj_tree_view), "button-press-event", 
                        G_CALLBACK (proj_tree_view_dbl_clicked), priv);

    gtk_notebook_append_page (GTK_NOTEBOOK (proj_notebook), 
                               priv->proj_tree_view_scrollwin,
                               gtk_label_new ("Project"));
    /******************/
}

static void
_recursively_insert_into_proj_dir_tree_view (GFile *dir, ProjectNotebookPrivate *priv, GtkTreeIter *parent, GtkTreeIter *parent2)
{
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
        GtkTreeIter dir_iter, dir_iter2;
        const gchar *file_name = g_file_info_get_name (fileinfo);
        GFile *child = g_file_get_child (dir, file_name);

        gtk_tree_store_append (priv->proj_dir_tree_store, &dir_iter, parent);
        gtk_tree_store_set (priv->proj_dir_tree_store, &dir_iter,
                             DIR_COL_NAME, file_name, -1);

         if (g_file_info_get_file_type (fileinfo) == G_FILE_TYPE_DIRECTORY)
        {
            pixbuf =  gdk_pixbuf_new_from_file ("./icons/directory.png", NULL);
            gtk_tree_store_set (priv->proj_dir_tree_store, &dir_iter, DIR_COL_PIXBUF, 
                                 pixbuf, -1);
            _recursively_insert_into_proj_dir_tree_view (child, priv, &dir_iter, &dir_iter2);
        }
        else if (g_strcmp0 (strrchr (file_name, '.'), ".py") == 0)
        {
            pixbuf = gdk_pixbuf_new_from_file ("./icons/py_file.png", NULL);
            gtk_tree_store_set (priv->proj_dir_tree_store, &dir_iter, DIR_COL_PIXBUF, 
                                  pixbuf, -1);

            gtk_tree_store_append (priv->proj_tree_store, &dir_iter2, parent2);
            gtk_tree_store_set (priv->proj_tree_store, &dir_iter2,
                                 PROJ_COL_PIXBUF, 
                                 pixbuf,
                                 PROJ_COL_NAME, file_name,
                                 PROJ_COL_PATH, g_file_get_path (child),
                                 -1);
        }
        else
        {
            pixbuf = gdk_pixbuf_new_from_file ("./icons/file.png", NULL);
            gtk_tree_store_set (priv->proj_dir_tree_store, &dir_iter, DIR_COL_PIXBUF, 
                                  pixbuf, -1);

            gtk_tree_store_append (priv->proj_tree_store, &dir_iter2, parent2);
            gtk_tree_store_set (priv->proj_tree_store, &dir_iter2,
                                 PROJ_COL_PIXBUF,
                                 pixbuf,
                                 PROJ_COL_NAME, file_name,
                                 PROJ_COL_PATH, g_file_get_path (child), -1);
        }
        g_object_unref (child);
        g_object_unref (fileinfo);
        g_object_unref (pixbuf);
    }
    g_object_unref (enumerator);
}

void
project_notebook_open_project (ProjectNotebook *proj_notebook, Project *current_project)
{
    ProjectNotebookPrivate *priv = PROJECT_NOTEBOOK_GET_PRIVATE (proj_notebook);

    GtkTreeIter iter;
    gtk_tree_store_append (priv->proj_dir_tree_store, &iter, NULL);
    gtk_tree_store_set (priv->proj_dir_tree_store, &iter,
                                  DIR_COL_PIXBUF, gdk_pixbuf_new_from_file ("./icons/directory.png", NULL),
                                  DIR_COL_NAME, strrchr (current_project->proj_dir, '/')+1, -1);
    
    GtkTreeIter iter2;
    gtk_tree_store_append (priv->proj_tree_store, &iter2, NULL);
    gtk_tree_store_set (priv->proj_tree_store, &iter2,
                                  PROJ_COL_PIXBUF, gdk_pixbuf_new_from_file ("./icons/project.png", NULL),
                                  PROJ_COL_NAME, current_project->name, 
                                  PROJ_COL_PATH, NULL, -1);
    
    GFile *proj_dir = g_file_new_for_path (current_project->proj_dir);
    GFileEnumerator *enumerator;
    enumerator = g_file_enumerate_children (proj_dir,
                                            G_FILE_ATTRIBUTE_STANDARD_NAME
                                            "," 
                                            G_FILE_ATTRIBUTE_STANDARD_TYPE,
                                            G_FILE_QUERY_INFO_NONE,
                                            NULL, NULL);
    if (!enumerator)
    {
        gtk_statusbar_push (GTK_STATUSBAR (status_bar),
                             0, "Cannot Open Project.");
        return;
    }

    GFileInfo *fileinfo;

    while ((fileinfo = g_file_enumerator_next_file (enumerator, NULL, NULL)) != NULL)
    {
        GdkPixbuf *pixbuf;
        GtkTreeIter dir_iter, dir_iter2;
        const gchar *file_name = g_file_info_get_name (fileinfo);

        gtk_tree_store_append (priv->proj_dir_tree_store, &dir_iter, &iter);
        gtk_tree_store_set (priv->proj_dir_tree_store, &dir_iter,
                             DIR_COL_NAME, file_name, -1);

        GFile *file = g_file_get_child (proj_dir, file_name);

        if (g_file_info_get_file_type (fileinfo) == G_FILE_TYPE_DIRECTORY)
        {
            pixbuf = gdk_pixbuf_new_from_file ("./icons/directory.png", NULL);
            gtk_tree_store_set (priv->proj_dir_tree_store, &dir_iter, DIR_COL_PIXBUF, 
                                  pixbuf, -1);
            _recursively_insert_into_proj_dir_tree_view (file, priv, &dir_iter, &dir_iter2);
        }
        else if (g_strcmp0 (strrchr (file_name, '.'), ".py") == 0)
        {
            pixbuf = gdk_pixbuf_new_from_file ("./icons/py_file.png", NULL);
            gtk_tree_store_set (priv->proj_dir_tree_store, &dir_iter, DIR_COL_PIXBUF, 
                                  pixbuf, -1);
            
            gtk_tree_store_append (priv->proj_tree_store, &dir_iter2, &iter2);
            gtk_tree_store_set (priv->proj_tree_store, &dir_iter2,
                                 PROJ_COL_PIXBUF, 
                                 pixbuf,
                                 PROJ_COL_NAME, file_name,
                                 PROJ_COL_PATH, g_file_get_path (file),
                                 -1);
        }
        else
        {
            pixbuf = gdk_pixbuf_new_from_file ("./icons/file.png", NULL);
            gtk_tree_store_set (priv->proj_dir_tree_store, &dir_iter, DIR_COL_PIXBUF, 
                                  pixbuf, -1);

            gtk_tree_store_append (priv->proj_tree_store, &dir_iter2, &iter2);
            gtk_tree_store_set (priv->proj_tree_store, &dir_iter2,
                                 PROJ_COL_PIXBUF,
                                 pixbuf,
                                 PROJ_COL_NAME, file_name,
                                 PROJ_COL_PATH, g_file_get_path (file), -1);
        }
        g_object_unref (file);
        g_object_unref (fileinfo);
        g_object_unref (pixbuf);
    }

    g_object_unref (proj_dir);
    g_object_unref (enumerator);

    GtkTreePath *path = gtk_tree_model_get_path (GTK_TREE_MODEL (priv->proj_dir_tree_store), &iter);
    gtk_tree_view_expand_row (GTK_TREE_VIEW (priv->proj_dir_tree_view), path, FALSE);
    gtk_tree_path_free (path);
    
    path = gtk_tree_model_get_path (GTK_TREE_MODEL (priv->proj_tree_store), &iter2);
    gtk_tree_view_expand_row (GTK_TREE_VIEW (priv->proj_tree_view), path, FALSE);
    gtk_tree_path_free (path);
}

void project_notebook_clear (ProjectNotebook *proj_notebook)
{
    
}

GtkWidget *
project_notebook_new ()
{
    GtkWidget *widget = GTK_WIDGET (g_object_new (project_notebook_get_type (), NULL));
    return widget;
}

static gboolean
proj_tree_view_dbl_clicked (GtkWidget *widget, GdkEvent *_event, gpointer data)
{
    ProjectNotebookPrivate *priv = (ProjectNotebookPrivate *)data;

    GdkEventButton *event = (GdkEventButton *)_event;
    if (event->type != GDK_2BUTTON_PRESS || 
         event->window != gtk_tree_view_get_bin_window (GTK_TREE_VIEW (widget)))
        return FALSE;
    
    int tx, ty;
    GtkTreePath *path;
    GtkTreeViewColumn *column;

    if (!gtk_tree_view_get_path_at_pos (GTK_TREE_VIEW (widget), event->x, 
                                         event->y, &path, &column, NULL, NULL))
        return FALSE;
    
    GtkTreeIter iter;
    if (!gtk_tree_model_get_iter (GTK_TREE_MODEL (priv->proj_tree_store), &iter, path))
    {
        gtk_tree_path_free (path);
        return FALSE;
    }
    gchar *file_path;
    gtk_tree_model_get (GTK_TREE_MODEL (priv->proj_tree_store), &iter, PROJ_COL_PATH, &file_path, -1);
    if (file_path && !open_file_at_index (file_path, -1))
        gtk_statusbar_push (GTK_STATUSBAR (status_bar), 0, "Cannot open file.");
}

static gboolean
proj_dir_tree_view_dbl_clicked (GtkWidget *widget, GdkEvent *_event, gpointer data)
{
    ProjectNotebookPrivate *priv = (ProjectNotebookPrivate *)data;

    GdkEventButton *event = (GdkEventButton *)_event;
    if (event->type != GDK_2BUTTON_PRESS || 
         event->window != gtk_tree_view_get_bin_window (GTK_TREE_VIEW (widget)))
        return FALSE;
    
    int tx, ty;
    GtkTreePath *path;
    GtkTreeViewColumn *column;

    if (!gtk_tree_view_get_path_at_pos (GTK_TREE_VIEW (widget), event->x, 
                                         event->y, &path, &column, NULL, NULL))
        return FALSE;
    
    GtkTreeIter iter;
    if (!gtk_tree_model_get_iter (GTK_TREE_MODEL (priv->proj_dir_tree_store), &iter, path))
    {
        gtk_tree_path_free (path);
        return FALSE;
    }

    GString *file_path_str = g_string_new ("");
    while (gtk_tree_store_iter_is_valid (priv->proj_dir_tree_store, &iter))
    {
        gchar *path;
        gtk_tree_model_get (GTK_TREE_MODEL (priv->proj_dir_tree_store), &iter, DIR_COL_NAME, &path, -1);
        file_path_str = g_string_prepend (file_path_str, path);
        file_path_str = g_string_prepend_c (file_path_str, '/');
        g_free (path);

        GtkTreeIter parent_iter;
        if (!gtk_tree_model_iter_parent (GTK_TREE_MODEL (priv->proj_dir_tree_store), &parent_iter, &iter))
             break;
        
        iter = *(gtk_tree_iter_copy (&parent_iter));
    }

    file_path_str = g_string_erase (file_path_str, 0, strrchr (file_path_str->str, '/') - file_path_str->str);

    gchar *file_path = g_build_path ("/", current_project->proj_dir, file_path_str->str, NULL);

    if (!open_file_at_index (file_path, -1))
        gtk_statusbar_push (GTK_STATUSBAR (status_bar), 0, "Cannot open file.");
    
    g_free (file_path);
    g_string_free (file_path_str, TRUE);
    gtk_tree_path_free (path);

    return FALSE;
}