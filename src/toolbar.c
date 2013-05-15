#include "toolbar.h"
#include "menus.h"
#include "main.h"

void
toolbar_new_clicked (GtkWidget *widget)
{
    if (mode == GIDLE_MODE_FILE)
        toolbar_new_file_clicked (widget);
    else
        toolbar_new_proj_clicked (widget);
}

void
toolbar_new_file_clicked (GtkWidget *widget)
{
    file_new_activate (widget);
}

void
toolbar_new_proj_clicked (GtkWidget *widget)
{
    project_new_activate (widget);
}

void
toolbar_open_clicked (GtkWidget *widget)
{
    if (mode == GIDLE_MODE_FILE)
        toolbar_open_file_clicked (widget);
    else
        toolbar_open_proj_clicked (widget);
}

void
toolbar_open_file_clicked (GtkWidget *widget)
{
    file_open_activate (widget);
}

void
toolbar_open_proj_clicked (GtkWidget *widget)
{
    project_open_activate (widget);
}

void
toolbar_save_clicked (GtkWidget *widget)
{
    if (mode == GIDLE_MODE_FILE)
        toolbar_save_file_clicked (widget);
    else
        toolbar_save_project_clicked (widget);
}

void
toolbar_save_file_clicked (GtkWidget *widget)
{
    file_save_activate (widget);
}

void
toolbar_save_project_clicked (GtkWidget *widget)
{
    project_save_activate (widget);
}

void
toolbar_save_as_clicked (GtkWidget *widget)
{
    if (mode == GIDLE_MODE_FILE)
        toolbar_save_as_file_clicked (widget);
    else
        toolbar_save_as_project_clicked (widget);
}

void
toolbar_save_as_file_clicked (GtkWidget *widget)
{
    file_save_as_activate (widget);
}

void
toolbar_save_as_project_clicked (GtkWidget *widget)
{
    project_save_as_activate (widget);
}

void
toolbar_undo_clicked (GtkWidget *widget)
{
    edit_undo_activate (widget);
}

void
toolbar_redo_clicked (GtkWidget *widget)
{
    edit_redo_activate (widget);
}

void
toolbar_cut_clicked (GtkWidget *widget)
{
    edit_cut_activate (widget);
}

void
toolbar_copy_clicked (GtkWidget *widget)
{
    edit_copy_activate (widget);
}

void
toolbar_paste_clicked (GtkWidget *widget)
{
    edit_paste_activate (widget);
}

void
toolbar_delete_clicked (GtkWidget *widget)
{
    edit_delete_activate (widget);
}

void
toolbar_select_all_clicked (GtkWidget *widget)
{
    edit_select_all_activate (widget);
}

void
toolbar_find_clicked (GtkWidget *widget)
{
    search_find_activate (widget);
}

void
toolbar_find_replace_clicked (GtkWidget *widget)
{
    search_find_replace_activate (widget);
}

void
toolbar_inc_indent_clicked (GtkWidget *widget)
{
    format_inc_indent_activate (widget);
}

void
toolbar_dec_indent_clicked (GtkWidget *widget)
{
    format_dec_indent_activate (widget);
}