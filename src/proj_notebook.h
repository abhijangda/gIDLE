#include <gtk/gtk.h>

#include "project_class.h"

#ifndef PROJ_NOTEBOOK_H
#define PROJ_NOTEBOOK_H

#define PROJECT_NOTEBOOK_TYPE (project_notebook_get_type ())
#define PROJECT_NOTEBOOK(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), PROJECT_NOTEBOOK_TYPE, ProjectNotebook))

#define PROJECT_NOTEBOOK_CLASS(klass) (G_TYPE_CHECK_INSTANCE_CAST ((klass), PROJECT_NOTEBOOK_TYPE, ProjectNotebook))

#define IS_PROJECT_NOTEBOOK(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PROJECT_NOTEBOOK_TYPE)

#define IS_PROJECT_NOTEBOOK_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PROJECT_NOTEBOOK_TYPE))

enum
{
    DIR_COL_PIXBUF = 0,
    DIR_COL_NAME,
    DIR_TOTAL_COLS,
    
    PROJ_COL_PIXBUF=0,
    PROJ_COL_NAME,
    PROJ_COL_PATH,
    PROJ_TOTAL_COLS,
};

struct _proj_notebook
{
    GtkNotebook notebook;
};

struct _proj_notebook_class
{
    GtkNotebookClass parent;
};

typedef struct _proj_notebook ProjectNotebook;
typedef struct _proj_notebook_class ProjectNotebookClass;

GType project_notebook_get_type (void) G_GNUC_CONST;
GtkWidget *project_notebook_new (void);
void project_notebook_open_project (ProjectNotebook *proj_notebook, Project *project);
void project_notebook_clear (ProjectNotebook *proj_notebook);
void project_notebook_show_tree_views (ProjectNotebook *proj_notebook, gboolean show);
#endif /*PROJ_NOTEBOOK_H*/