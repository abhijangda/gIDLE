#include <gtk/gtk.h>

#ifndef RUN_SCRIPT_DIALOG_H
#define RUN_SCRIPT_DIALOG_H

void
load_run_script_dialog (char *filename);

void
run_script_dialog_run_clicked (GtkWidget *);

void
run_script_dialog_cancel_clicked (GtkWidget *, gpointer dialog);

#endif /*RUN_SCRIPT_DIALOG_H*/