#ifndef PROJECT_CLASS_H
#define PROJECT_CLASS_H

struct _project
{
    gchar *name;
    gchar **file_paths; /* This array should be NULL terminated */
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
project_new (gchar *name, gchar **file_paths, gchar *main_file,
             gchar *file_name, gchar *current_dir, gchar *proj_dir,
             gchar *version, gchar **argv, gchar **env_vars);

Project *
project_new_from_string (gchar *proj_string);

gchar *
project_get_string  (Project *proj);

gboolean
project_write (Project *proj, GError *error);

void
project_destroy (Project *);

#endif /*PROJECT_CLASS_H*/