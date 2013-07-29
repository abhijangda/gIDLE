#include "class_browser.h"
#include "codewidget.h"
#include "main.h"
#include "core_funcs.h"

static GtkWidget **tree_view_array = NULL;
static GtkTreeStore **tree_store_array = NULL;
int tree_view_array_size = 0, tree_model_array_size = 0;

static GtkWidget *text_view = NULL;

static 
void insert_rows_in_tree_view (CodeWidget *codewidget, GtkTreeStore *tree_store);

static void
recursively_insert_classes (CodeWidget *codewidget, GtkTreeStore *tree_store, GtkTreeIter *parent, PyClass *klass);

static void
_tree_selection_changed (GtkTreeSelection *selection, gpointer data);

void
load_class_browser_dialog ()
{
    GtkBuilder *class_browser_builder;

    class_browser_builder = gtk_builder_new ();
    gtk_builder_add_from_file (class_browser_builder, "./ui/class_browser_dlg.glade", NULL);
    
    GObject *dialog = gtk_builder_get_object (class_browser_builder, "class_browser_dialog");
    GtkWidget *notebook = GTK_WIDGET (gtk_builder_get_object (class_browser_builder, "notebook"));
    text_view = GTK_WIDGET (gtk_builder_get_object (class_browser_builder, "textview"));
    gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (text_view),
                                  GTK_WRAP_WORD);

    g_signal_connect (gtk_builder_get_object (class_browser_builder, "cmdClose"),
                     "clicked", G_CALLBACK (class_browser_cmdCancel_clicked), (gpointer)dialog);
    
    int i;
    for (i = 0; i < get_total_pages (); i++)
    {
        if (!code_widget_array [i]->file_path)
             return;

        GtkWidget *tree_view = gtk_tree_view_new();
        tree_view_array = g_realloc (tree_view_array, 
                                      (tree_view_array_size+1)*sizeof (GtkTreeView *));
        tree_view_array [tree_view_array_size] = tree_view;
        tree_view_array_size++;
        
        GtkTreeStore *tree_store = gtk_tree_store_new (3, G_TYPE_STRING, G_TYPE_POINTER, G_TYPE_INT);

        GtkTreeSelection *tree_selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view));
        g_signal_connect (G_OBJECT (tree_selection), "changed", 
                           G_CALLBACK (_tree_selection_changed),
                           tree_store);

        GtkCellRenderer *renderer = gtk_cell_renderer_text_new ();
        GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes ("Classes",
                                                                               renderer, "text", 0, NULL);

        gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), column);
        gtk_tree_view_set_model (GTK_TREE_VIEW (tree_view),
                                  GTK_TREE_MODEL (tree_store));
        gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (tree_view),
                                            FALSE);

        insert_rows_in_tree_view (code_widget_array [i], tree_store);
        
        GtkWidget *scroll_win = gtk_scrolled_window_new (NULL, NULL);
        gtk_container_add (GTK_CONTAINER (scroll_win), tree_view);
        gtk_notebook_append_page (GTK_NOTEBOOK (notebook), scroll_win,
                                   gtk_label_new (strrchr (code_widget_array [i]->file_path, '/')+1));
    }
    gtk_widget_show_all (GTK_WIDGET (dialog));
}

void
insert_rows_in_tree_view (CodeWidget *codewidget, GtkTreeStore *tree_store)
{
    int i;
    for (i = 0; i < codewidget->py_class_array_size; i++)
    {
        GtkTreeIter iter;
        gtk_tree_store_append (tree_store, &iter, NULL);
        gtk_tree_store_set (tree_store, &iter,
                                  0, PY_VARIABLE (codewidget->py_class_array[i])->get_definition (
                                      PY_VARIABLE (codewidget->py_class_array[i])),
                                  1, PY_VARIABLE (codewidget->py_class_array[i]), -1);
        recursively_insert_classes (codewidget, tree_store, &iter,
                                     codewidget->py_class_array [i]);
    }
}

void
recursively_insert_classes (CodeWidget *codewidget, GtkTreeStore *tree_store, GtkTreeIter *parent, PyClass *klass)
{
    int i;
    for (i = 0; i < klass->nested_classes_size; i++)
    {
        GtkTreeIter iter;
        gtk_tree_store_append (tree_store, &iter, parent);
        gtk_tree_store_set (tree_store, &iter,
                                  0, PY_VARIABLE (klass->nested_classes [i])->get_definition (
                                      PY_VARIABLE (klass->nested_classes [i])),
                                  1, PY_VARIABLE (klass->nested_classes [i]), -1);
        recursively_insert_classes (codewidget, tree_store, &iter, klass->nested_classes [i]);
    }
    
    for (i = 0; i < klass->py_static_var_array_size; i++)
    {
        GtkTreeIter iter;
        gtk_tree_store_append (tree_store, &iter, parent);
        gtk_tree_store_set (tree_store, &iter,
                                  0, PY_VARIABLE (klass->py_static_var_array [i])->get_definition (
                                      PY_VARIABLE (klass->py_static_var_array [i])),
                                  1, PY_VARIABLE (klass->py_static_var_array [i]), -1);
    }

    PyFunc **py_func = klass->py_func_array;
    if (!py_func)
        return;

    while (*py_func)
    {
        GtkTreeIter iter;
        gtk_tree_store_append (tree_store, &iter, parent);
        gtk_tree_store_set (tree_store, &iter,
                                  0, PY_VARIABLE (*py_func)->get_definition (
                                      PY_VARIABLE (*py_func)),
                                  1, PY_VARIABLE (*py_func), -1);
        
        py_func++;
    }
}

static void
_tree_selection_changed (GtkTreeSelection *selection, gpointer data)
{
    GtkTreeIter tree_iter;
   
    gtk_tree_selection_get_selected (selection, NULL, &tree_iter);
    
    if (!gtk_tree_store_iter_is_valid (GTK_TREE_STORE (data),
                                         &tree_iter))
        return;

    gpointer value;
    gtk_tree_model_get (GTK_TREE_MODEL (data), &tree_iter, 1, &value, -1);
    PyVariable *py_var = PY_VARIABLE (value);
    GtkTextBuffer *buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (text_view));
    if (!py_var || !py_var->doc_string)
    {
        gtk_text_buffer_set_text (buffer, " ", -1);        
    }
    else
    {
        gtk_text_buffer_set_text (buffer, py_var->doc_string, -1);        
    }
}

void
class_browser_cmdCancel_clicked (GtkWidget *widget, gpointer dialog)
{
    gtk_widget_destroy (GTK_WIDGET (dialog));
}