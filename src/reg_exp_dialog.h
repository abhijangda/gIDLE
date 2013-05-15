#include <gtk/gtk.h>

#ifndef REG_EXP_DIALOG_H
#define REG_EXP_DIALOG_H
void
load_reg_exp_dialog (GtkTextView *text_view);

void
reg_exp_dialog_cmdClose_clicked (GtkWidget *widget, gpointer dialog);

void
reg_exp_dialog_cmdSearch_clicked (GtkWidget *widget);
#endif //REG_EXP_DIALOG_H