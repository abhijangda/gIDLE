#include <gtk/gtk.h>

#include "codewidget.h"

#ifndef SYMBOLS_VIEW_H
#define SYMBOLS_VIEW_H

#define SYMBOLS_VIEW_TYPE (symbols_view_get_type ())
#define SYMBOLS_VIEW(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), SYMBOLS_VIEW_TYPE, SymbolsView))

#define SYMBOLS_VIEW_CLASS(klass) (G_TYPE_CHECK_INSTANCE_CAST ((klass), SYMBOLS_VIEW_TYPE, SymbolsViewClass))

#define IS_SYMBOLS_VIEW(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SYMBOLS_VIEW_TYPE)

#define IS_SYMBOLS_VIEW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), SYMBOLS_VIEW_TYPE))

enum
{
    SYMBOLS_VIEW_COL_PIXBUF,
    SYMBOLS_VIEW_COL_NAME,
    SYMBOLS_VIEW_COL_POINTER,
    SYMBOLS_VIEW_TOTAL_COLS,
};

enum
{
    SEARCH_IN_INDEX_ALL = 0,
    SEARCH_IN_INDEX_MODULE,
    SEARCH_IN_INDEX_CLASS,
    SEARCH_IN_INDEX_GLOBAL_FUNC,
    SEARCH_IN_INDEX_GLOBAL_VARS,
};

struct symbols_view
{
    GtkBox box;
};

struct _symbols_view_class
{
    GtkBoxClass parent_class;
};

typedef struct symbols_view SymbolsView;
typedef struct _symbols_view_class SymbolsViewClass;

GType symbols_view_get_type (void) G_GNUC_CONST;
GtkWidget *symbols_view_new (void);

void
symbols_view_fill_from_codewidget (SymbolsView *symbols_view, CodeWidget *code_widget);
#endif /*SYMBOLS_VIEW_H*/