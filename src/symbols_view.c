#include <string.h>

#include "symbols_view.h"

#define SYMBOLS_VIEW_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), SYMBOLS_VIEW_TYPE, SymbolsViewPrivate))

struct _symbols_view_private
{
    GtkWidget *tree_view;
    GtkTreeStore *tree_store;
    GtkWidget *scrolled_win;
    GtkWidget *hbox_search, *hbox_view;
    GtkWidget *label1, *label2, *cmdSearch, *combo_search_in;
    GtkWidget *combo_search;
    GtkTreeModel *filter;
};

typedef struct _symbols_view_private SymbolsViewPrivate;

static void
symbols_view_class_init (SymbolsViewClass *klass);

static void
symbols_view_init (SymbolsView *sym_view);

static void
_symbols_view_search_in_changed (GtkComboBox *combobox, gpointer data);

static void
_search_entry_insert (GtkEntry *entry, gchar *string, gpointer  data)
{
    gtk_tree_model_filter_refilter (GTK_TREE_MODEL_FILTER (data));
}

static void
_search_entry_delete (GtkEntry *entry, GtkDeleteType type, gint count, gpointer data) 
{
    gtk_tree_model_filter_refilter (GTK_TREE_MODEL_FILTER (data));
}

static void
_symbols_view_search_in_changed (GtkComboBox *combobox, gpointer data)
{
    SymbolsView *sym_view = SYMBOLS_VIEW (data);
    SymbolsViewPrivate *priv = SYMBOLS_VIEW_GET_PRIVATE (sym_view);
    int index = gtk_combo_box_get_active (combobox);
    gtk_tree_model_filter_refilter (GTK_TREE_MODEL_FILTER (priv->filter));
}

static gboolean
_symbols_view_filter_func (GtkTreeModel *model, GtkTreeIter *iter, gpointer data)
{
    SymbolsView *sym_view = SYMBOLS_VIEW (data);
    SymbolsViewPrivate *priv = SYMBOLS_VIEW_GET_PRIVATE (sym_view);
    GtkComboBox *combobox = GTK_COMBO_BOX (priv->combo_search_in);
    int index = gtk_combo_box_get_active (combobox);
    GtkTreeIter parent;

    if (!gtk_tree_model_iter_parent (model, &parent, iter))
        return TRUE;
    
    PyVariable *py_var;
    gtk_tree_model_get (GTK_TREE_MODEL (model), iter, SYMBOLS_VIEW_COL_POINTER, &py_var, -1);
    
    if (!py_var)
        return TRUE;

    const gchar *text = gtk_entry_get_text (GTK_ENTRY (priv->combo_search));
    if (g_strcmp0 (text, ""))
    {
        if (g_strstr_len (py_var->name, -1, text))
            return TRUE;
        
        return FALSE;
    }

    if (index == SEARCH_IN_INDEX_ALL)
        return TRUE;
    
    if (index == SEARCH_IN_INDEX_MODULE)
    {
        if (py_var->type == MODULE)
            return TRUE;

        else
            return FALSE;
    }
    else if (index == SEARCH_IN_INDEX_CLASS)
    {
        if (py_var->type == MODULE )
        {
            int i;
            for (i = 0; i < PY_MODULE (py_var)->py_variable_array_size; i++)
                if (PY_MODULE (py_var)->py_variable_array[i]->type == CLASS)
                    return TRUE;

            return FALSE;
        }
        else if (py_var->type == CLASS)
            return TRUE;

        else
            return FALSE;
    }
    else if (index == SEARCH_IN_INDEX_GLOBAL_FUNC)
    {
        if (py_var->type == MODULE)
        {
            int i;
            for (i = 0; i < PY_MODULE (py_var)->py_variable_array_size; i++)
                if (PY_MODULE (py_var)->py_variable_array[i]->type == FUNC)
                    return TRUE;

            return FALSE;
        }

        else if (py_var->type == CLASS && !g_strcmp0 (py_var->name, "Global Scope"))
        {
            if (PY_CLASS (py_var)->py_func_array != NULL)
                return TRUE;

            return FALSE;
        }

        else if (py_var->type == FUNC)
             return TRUE;

        else
            return FALSE;
    }
    else if (index == SEARCH_IN_INDEX_GLOBAL_VARS)
    {
         if (py_var->type == MODULE)
        {
            int i;
            for (i = 0; i < PY_MODULE (py_var)->py_variable_array_size; i++)
                if (PY_MODULE (py_var)->py_variable_array[i]->type == STATIC_VAR)
                    return TRUE;

            return FALSE;
        }

        else if (py_var->type == CLASS && !g_strcmp0 (py_var->name, "Global Scope"))
        {
            if (PY_CLASS (py_var)->py_static_var_array_size != 0)
                return TRUE;

            return FALSE;
        }

        else if (py_var->type == STATIC_VAR)
             return TRUE;

        else
            return FALSE;
    }
    return FALSE;
}

GType
symbols_view_get_type (void)
{
    static GType scroll_win_type = 0;
    
    if (!scroll_win_type)
    {
        static const GTypeInfo symbols_view_type = 
        {
            sizeof (SymbolsViewClass),
            NULL,
            NULL,
            (GClassInitFunc) symbols_view_class_init,
            NULL,
            NULL,
            sizeof (SymbolsView),
            0,
            (GInstanceInitFunc) symbols_view_init,
        };
        scroll_win_type = g_type_register_static (GTK_TYPE_BOX, "SymbolsView", &symbols_view_type, 0);
    }
    return scroll_win_type;
}

static void
symbols_view_class_init (SymbolsViewClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    
    g_type_class_add_private (klass, sizeof (SymbolsViewPrivate));
}

static void
symbols_view_init (SymbolsView *sym_view)
{
    SymbolsViewPrivate *priv = SYMBOLS_VIEW_GET_PRIVATE (sym_view);

    priv->tree_view = gtk_tree_view_new ();
    priv->tree_store = gtk_tree_store_new (SYMBOLS_VIEW_TOTAL_COLS, GDK_TYPE_PIXBUF, G_TYPE_STRING, G_TYPE_POINTER);
    
    GtkCellRenderer *renderer = gtk_cell_renderer_pixbuf_new ();
    GtkTreeViewColumn *column = gtk_tree_view_column_new ();
    gtk_tree_view_column_pack_start (column, renderer, FALSE);
    gtk_tree_view_column_set_attributes (column, renderer, "pixbuf", SYMBOLS_VIEW_COL_PIXBUF , NULL);

    renderer = gtk_cell_renderer_text_new ();
    gtk_tree_view_column_pack_start (column, renderer, FALSE);
    gtk_tree_view_column_set_attributes (column, renderer, "text", SYMBOLS_VIEW_COL_NAME, NULL);

    gtk_tree_view_append_column (GTK_TREE_VIEW (priv->tree_view), column);
    gtk_tree_view_column_set_sizing(column, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
    
    priv->filter = gtk_tree_model_filter_new (GTK_TREE_MODEL (priv->tree_store), NULL);

    gtk_tree_view_set_model (GTK_TREE_VIEW (priv->tree_view),
                              GTK_TREE_MODEL (priv->filter));
    gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (priv->tree_view),
                                        FALSE);

    priv->scrolled_win = gtk_scrolled_window_new (NULL, NULL);
    priv->hbox_search = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 2);
    priv->hbox_view = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 2);
    priv->label1= gtk_label_new ("Search:");
    priv->label2 = gtk_label_new ("View:");
    priv->cmdSearch = gtk_button_new_with_label ("Search");
    priv->combo_search_in = gtk_combo_box_text_new ();
    gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (priv->combo_search_in), "All");
    gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (priv->combo_search_in), "Module");
    gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (priv->combo_search_in), "Class");
    gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (priv->combo_search_in), "Global Functions");
    gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (priv->combo_search_in), "Global Variables");
    
    gtk_tree_model_filter_set_visible_func (GTK_TREE_MODEL_FILTER (priv->filter), _symbols_view_filter_func, sym_view, NULL);

    gtk_combo_box_set_active (GTK_COMBO_BOX (priv->combo_search_in), SEARCH_IN_INDEX_ALL);

    g_signal_connect (G_OBJECT (priv->combo_search_in), "changed",
                     G_CALLBACK (_symbols_view_search_in_changed), sym_view);

    gtk_container_add (GTK_CONTAINER (priv->scrolled_win), priv->tree_view);

    priv->combo_search = gtk_entry_new ();
    g_signal_connect (G_OBJECT (priv->combo_search), "delete-from-cursor", G_CALLBACK (_search_entry_delete), priv->filter);
    g_signal_connect (G_OBJECT (priv->combo_search), "insert-at-cursor", G_CALLBACK (_search_entry_insert), priv->filter);

    gtk_box_pack_start (GTK_BOX (sym_view), priv->hbox_view, FALSE, FALSE, 2);
    gtk_box_pack_start (GTK_BOX (sym_view), priv->hbox_search, FALSE, FALSE, 2);
    gtk_box_pack_start (GTK_BOX (sym_view), priv->scrolled_win, TRUE, TRUE, 2);

    gtk_box_pack_start (GTK_BOX (priv->hbox_view), priv->label2, TRUE, TRUE, 2);
    gtk_box_pack_start (GTK_BOX (priv->hbox_view), priv->combo_search_in, TRUE, TRUE, 2);

    gtk_box_pack_start (GTK_BOX (priv->hbox_search), priv->label1, TRUE, TRUE, 2);
    gtk_box_pack_start (GTK_BOX (priv->hbox_search), priv->combo_search, TRUE, TRUE, 2);
}

void
_recursively_insert_py_vars (GtkTreeStore *tree_store, GtkTreeIter *parent_iter, PyVariable *py_var)
{
    if (!py_var)
        return;

    GtkTreeIter iter;
    gtk_tree_store_append (tree_store, &iter, parent_iter);
    gtk_tree_store_set (tree_store, &iter,
                              SYMBOLS_VIEW_COL_NAME,
                              py_var->name,
                              SYMBOLS_VIEW_COL_POINTER, py_var, -1);
    
    GdkPixbuf *pixbuf;

    int i;
    if (py_var->type == MODULE)
    {
        pixbuf = gdk_pixbuf_new_from_file ("./icons/module.png", NULL);
        gtk_tree_store_set (tree_store, &iter, SYMBOLS_VIEW_COL_PIXBUF, 
                                  pixbuf, -1);
        for (i = 0; i < PY_MODULE (py_var)->py_variable_array_size; i++)
            _recursively_insert_py_vars (tree_store, &iter, PY_MODULE (py_var)->py_variable_array [i]);
    }
    else if (py_var->type == CLASS)
    {
        pixbuf = gdk_pixbuf_new_from_file ("./icons/class.png", NULL);
        gtk_tree_store_set (tree_store, &iter, SYMBOLS_VIEW_COL_PIXBUF, pixbuf, -1);

        for (i = 0; i < PY_CLASS (py_var)->nested_classes_size; i++)
            _recursively_insert_py_vars (tree_store, &iter, PY_VARIABLE (PY_CLASS (py_var)->nested_classes [i]));

        for (i = 0; i < PY_CLASS (py_var)->py_static_var_array_size; i++)
            _recursively_insert_py_vars (tree_store, &iter, PY_VARIABLE (PY_CLASS (py_var)->py_static_var_array [i]));

        PyFunc **p = PY_CLASS (py_var)->py_func_array;
        if (p)
        {
            while (*p)
            {
                _recursively_insert_py_vars (tree_store, &iter, PY_VARIABLE (*p));
                p++;
            }
        }
    }
    else if (py_var->type == FUNC)
    {
        pixbuf = gdk_pixbuf_new_from_file ("./icons/func.png", NULL);
        gtk_tree_store_set (tree_store, &iter, SYMBOLS_VIEW_COL_PIXBUF, pixbuf, -1);
    }
    else if (py_var->type == STATIC_VAR)
    {
        pixbuf = gdk_pixbuf_new_from_file ("./icons/field.png", NULL);
        gtk_tree_store_set (tree_store, &iter, SYMBOLS_VIEW_COL_PIXBUF, pixbuf, -1);
    }
    g_object_unref (pixbuf);
}

void
symbols_view_fill_from_codewidget (SymbolsView *symbols_view, CodeWidget *code_widget)
{
    SymbolsViewPrivate *priv = SYMBOLS_VIEW_GET_PRIVATE (symbols_view);

    GtkTreeIter iter;
    
    if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (priv->tree_store), &iter))
    {
        do
        {
            gchar *name;
            gtk_tree_model_get (GTK_TREE_MODEL (priv->tree_store), &iter, SYMBOLS_VIEW_COL_NAME, &name, -1);
            if (!g_strcmp0 (name, strrchr (code_widget->file_path, '/')+1))
            {
                gtk_tree_store_remove (priv->tree_store, &iter);
                break;
            }
        }
        while (gtk_tree_model_iter_next (GTK_TREE_MODEL (priv->tree_store), &iter));
    }

    gtk_tree_store_append (priv->tree_store, &iter, NULL);
    gtk_tree_store_set (priv->tree_store, &iter,
                                  SYMBOLS_VIEW_COL_NAME,
                                  strrchr (code_widget->file_path, '/') + 1,
                                  SYMBOLS_VIEW_COL_POINTER, NULL, -1);
    
    GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file ("./icons/py_file.png", NULL);
    gtk_tree_store_set (priv->tree_store, &iter, SYMBOLS_VIEW_COL_PIXBUF, pixbuf, -1);
    
    g_object_unref (pixbuf);

    int i;
    for (i = 0; i < code_widget->py_variable_array_size; i++)
        _recursively_insert_py_vars (priv->tree_store, &iter, code_widget->py_variable_array [i]);
    
    for (i = 0; i < code_widget->py_class_array_size; i++)
        _recursively_insert_py_vars (priv->tree_store, &iter, PY_VARIABLE (code_widget->py_class_array [i]));
}

GtkWidget *
symbols_view_new ()
{
    GtkWidget *widget = GTK_WIDGET (g_object_new (symbols_view_get_type (), NULL));
    gtk_orientable_set_orientation (GTK_ORIENTABLE (widget), GTK_ORIENTATION_VERTICAL);
    return widget;
}