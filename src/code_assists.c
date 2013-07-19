#include "code_assist.h"
#include "py_variable.h"

static void
_code_assist_tree_selection_changed (GtkTreeSelection *selection, gpointer data);

CodeAssistWidget *
code_assist_widget_new()
{
    CodeAssistWidget *_code_assist = g_malloc (sizeof (CodeAssistWidget));
    if (!_code_assist)
        return NULL;

    _code_assist->list_view = gtk_tree_view_new ();
    gtk_tree_view_set_enable_search (GTK_TREE_VIEW (_code_assist->list_view), FALSE);

    GtkTreeSelection *tree_selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (_code_assist->list_view));
    g_signal_connect (G_OBJECT (tree_selection), "changed", 
                       G_CALLBACK (_code_assist_tree_selection_changed),
                       _code_assist);

    _code_assist->list_store = gtk_list_store_new (2, G_TYPE_STRING, G_TYPE_POINTER);
    GtkCellRenderer *renderer = gtk_cell_renderer_text_new ();
    GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes ("Suggestions",
                                                                           renderer, "text", 0, NULL);

    gtk_tree_view_append_column (GTK_TREE_VIEW (_code_assist->list_view), column);
    gtk_tree_view_set_model (GTK_TREE_VIEW (_code_assist->list_view),
                              GTK_TREE_MODEL (_code_assist->list_store));
    gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (_code_assist->list_view),
                                        TRUE);

    _code_assist->scroll_win = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (_code_assist->scroll_win),
                                          GTK_SHADOW_IN);

    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (_code_assist->scroll_win), 
                                    GTK_POLICY_ALWAYS, GTK_POLICY_ALWAYS);
    gtk_container_add (GTK_CONTAINER (_code_assist->scroll_win),
                       _code_assist->list_view);
    
    gtk_widget_set_size_request (_code_assist->scroll_win, 300, 200);
    
    _code_assist->text_view = gtk_text_view_new ();
    gtk_widget_set_state_flags (_code_assist->text_view, 
                                 GTK_STATE_FLAG_INSENSITIVE, FALSE);
    gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (_code_assist->text_view),
                                  GTK_WRAP_WORD);

    _code_assist->text_scroll_win = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (_code_assist->text_scroll_win),
                                          GTK_SHADOW_IN);
    g_object_ref (_code_assist->text_scroll_win);
    
    gtk_widget_set_size_request (_code_assist->text_scroll_win, 300, 200);

    gtk_container_add (GTK_CONTAINER (_code_assist->text_scroll_win),
                        _code_assist->text_view);

    _code_assist->parent = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 2);

    //gtk_container_add (GTK_CONTAINER (_code_assist->parent),
                       //_code_assist->scroll_win);
    gtk_box_pack_start (GTK_BOX (_code_assist->parent), 
                        _code_assist->scroll_win, FALSE, FALSE, 0);

    g_object_ref (_code_assist->parent);
    
    return _code_assist;
}

static void
_code_assist_tree_selection_changed (GtkTreeSelection *selection, gpointer data)
{
    CodeAssistWidget *code_assist_widget = (CodeAssistWidget *)data;
    GtkTreeIter tree_iter;
   
    gtk_tree_selection_get_selected (selection, NULL, &tree_iter);
    
    if (!gtk_list_store_iter_is_valid (code_assist_widget->list_store,
                                         &tree_iter))
        return;

    gpointer value;
    gtk_tree_model_get (GTK_TREE_MODEL (code_assist_widget->list_store), &tree_iter, 1, &value, -1);
    PyVariable *py_var = PY_VARIABLE (value);
    if (!py_var)
        return;

    if (py_var->doc_string)
    {
        if (!gtk_widget_get_parent (code_assist_widget->text_scroll_win))
            gtk_box_pack_start (GTK_BOX (code_assist_widget->parent), 
                        code_assist_widget->text_scroll_win, FALSE, FALSE, 0);
        
        gtk_widget_show_all (code_assist_widget->text_scroll_win);
        GtkTextBuffer *buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (code_assist_widget->text_view));
        gtk_text_buffer_set_text (buffer, py_var->doc_string, -1);        
    }
    else
    {
        if (gtk_widget_get_parent (code_assist_widget->text_scroll_win))
            gtk_container_remove (GTK_CONTAINER (code_assist_widget->parent), 
                        code_assist_widget->text_scroll_win);
    }
}

void
code_assist_destroy (CodeAssistWidget *code_assist)
{
    gtk_widget_destroy (code_assist->list_view);
    gtk_widget_destroy (code_assist->scroll_win);
    gtk_widget_destroy (code_assist->parent);
    g_free (code_assist);
}