#include <gtk/gtk.h>

#ifndef TOOLBAR_H
#define TOOLBAR_H

void
toolbar_new_clicked (GtkWidget *widget);

void
toolbar_new_file_clicked (GtkWidget *widget);

void
toolbar_new_proj_clicked (GtkWidget *widget);

void
toolbar_open_clicked (GtkWidget *widget);

void
toolbar_open_file_clicked (GtkWidget *widget);

void
toolbar_open_proj_clicked (GtkWidget *widget);

void
toolbar_save_clicked (GtkWidget *widget);

void
toolbar_save_file_clicked (GtkWidget *widget);

void
toolbar_save_project_clicked (GtkWidget *widget);

void
toolbar_save_as_clicked (GtkWidget *widget);

void
toolbar_save_as_file_clicked (GtkWidget *widget);

void
toolbar_save_as_project_clicked (GtkWidget *widget);

void
toolbar_undo_clicked (GtkWidget *widget);

void
toolbar_redo_clicked (GtkWidget *widget);

void
toolbar_cut_clicked (GtkWidget *widget);

void
toolbar_copy_clicked (GtkWidget *widget);

void
toolbar_paste_clicked (GtkWidget *widget);

void
toolbar_delete_clicked (GtkWidget *widget);

void
toolbar_select_all_clicked (GtkWidget *widget);

void
toolbar_find_clicked (GtkWidget *widget);

void
toolbar_find_replace_clicked (GtkWidget *widget);

void
toolbar_inc_indent_clicked (GtkWidget *widget);

void
toolbar_dec_indent_clicked (GtkWidget *widget);

#endif /*TOOLBAR_H*/