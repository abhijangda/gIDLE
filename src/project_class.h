#include <glib.h>

#ifndef PROJECT_CLASS_H
#define PROJECT_CLASS_H

struct _project
{
    gchar *name;
    gchar *main_file;
    gchar *current_dir;
    gchar *proj_dir;
    gchar *version;
    gchar *file_name;
    gchar **argv;
    gchar **env_vars;
};

typedef struct _project Project;

Project *
project_new (const gchar *name, const gchar *main_file,
             const gchar *file_name, const gchar *current_dir,
             const gchar *version, const gchar **argv, const gchar **env_vars);

Project *
project_new_from_string (gchar *proj_string, gchar *proj_file);

gchar *
project_get_string  (Project *proj);

gboolean
project_write (Project *proj, GError **error);

gboolean
project_create_files_and_dirs (Project *proj, GError **error);

void
project_destroy (Project *);

gchar *
project_get_main_file (Project *proj);

#endif /*PROJECT_CLASS_H*/