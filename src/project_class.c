#include "project_class.h"
#include <glib/glib.h>

/* To create a new project
 * returns a newly created Project object
 */
Project *
project_new (gchar *name, gchar **file_paths, gchar *main_file,
             gchar *file_name, gchar *current_dir, gchar *proj_dir,
             gchar *version, gchar **argv, gchar **env_vars)
{
    Project *proj = g_try_malloc (sizeof (Project));
    if (!proj)
       return NULL;
    
    proj->name = g_strdup (name);
    proj->file_paths = g_strdupv (file_paths);
    proj->main_file = g_strdup (main_file);
    proj->current_dir = g_strdup (current_dir);
    proj->proj_dir = g_strdup (proj_dir);
    proj->version = g_strdup (version);
    proj->file_name = g_strdup (file_name);
    proj->argv = g_strdupv (argv);
    proj->env_vars = g_strdupv (env_vars);
    return proj;
}

/* To create a new project
 * from project string
 * returns a newly created Project
 */
Project *
project_new_from_string (gchar *proj_string)
{
    
}

/* To get project string
 * of a project
 * Returns a newly allocated string
 */
gchar *
project_get_string  (Project *proj)
{
    GString *proj_gstring = g_string_new ("<name>");
    
    proj_gstring = g_string_append (proj_gstring, proj->name);
    proj_gstring = g_string_append (proj_gstring, "</name>\n");
    
    proj_gstring = g_string_append (proj_gstring, "<main_file>");
    proj_gstring = g_string_append (proj_gstring, proj->main_file);
    proj_gstring = g_string_append (proj_gstring, "</main_file>\n");
    
    gchar **p = proj->file_paths;
    while (*p)
    {
        proj_gstring = g_string_append (proj_gstring, "<file>");
        proj_gstring = g_string_append (proj_gstring, *p);
        proj_gstring = g_string_append (proj_gstring, "</file>\n");
        p++;
    }
    
    proj_gstring = g_string_append (proj_string, "<current_dir>");
    proj_gstring = g_string_append (proj_string, proj->current_dir);
    proj_gstring = g_string_append (proj_string, "</current_dir>\n");
    
    proj_gstring = g_string_append (proj_string, "<version>");
    proj_gstring = g_string_append (proj_string, proj->version);
    proj_gstring = g_string_append (proj_string, "</version>\n");
    
    gchar *proj_string = proj_gstring->str;
    
    g_string_free (proj_gstring, FALSE);
    
    return proj_string;
}

/* To write current project's 
 * values to proj_file
 */
gboolean
project_write (Project *proj, GError **error)
{
    gchar *proj_str = project_get_string (proj);
    
    if (!g_file_set_contents (proj->file_name, proj_str, -1, error))
        return FALSE;
    
    return TRUE;
}

/* To destroy a 
 * project
 */
void
project_destroy (Project *proj)
{
}