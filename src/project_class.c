#include "project_class.h"
#include "core_funcs.h"

/* To create a new project
 * returns a newly created Project object
 */
Project *
project_new (const gchar *name, const gchar *main_file,
             const gchar *file_name, const gchar *current_dir,
             const gchar *version, const gchar **argv, const gchar **env_vars)
{
    Project *proj = g_try_malloc (sizeof (Project));
    if (!proj)
       return NULL;
    
    proj->name = g_strdup (name);
    proj->proj_dir = g_path_get_dirname (file_name);

    gchar *s = g_strstr_len (main_file, -1, proj->proj_dir);
    gchar  *_main_file = (gchar *)main_file;

    if (s == main_file)
        _main_file = s + strlen (proj->proj_dir);

    proj->main_file = g_strdup (_main_file);
    proj->current_dir = g_strdup (current_dir);
    proj->version = g_strdup (version);
    proj->file_name = g_strdup (file_name);
    proj->argv = g_strdupv ((gchar **)argv);
    proj->env_vars = g_strdupv ((gchar **)env_vars);
    return proj;
}

gboolean
project_create_files_and_dirs (Project *proj, GError **error)
{
    GFile *file = g_file_new_for_path (proj->file_name);
    GFile *dir = g_file_new_for_path (proj->proj_dir);

    if (!g_file_make_directory_with_parents (dir, NULL, error))
    {
        if (error && !g_error_matches (*error, G_IO_ERROR, G_IO_ERROR_EXISTS))
             return FALSE;

        else if (error)
        {
            g_error_free (*error);
            error = NULL;
        }
    }

    if (!project_write (proj, error))
         return FALSE;

    gchar *main_file = project_get_main_file (proj);
    g_file_set_contents (main_file, "", -1, NULL);
    g_free (main_file);

    g_object_unref (file);
    g_object_unref (dir);
    return TRUE;
}

gchar *
project_get_main_file (Project *proj)
{
    return g_build_path ("/", proj->proj_dir, proj->main_file, NULL);
}

/* To create a new project
 * from project string
 * returns a newly created Project
 */
Project *
project_new_from_string (gchar *proj_string, gchar *proj_file)
{
    gchar *start, *end;
    
    start = g_strstr_len (proj_string, -1, "<name>") + 6; /*strlen("<name>") == 6*/
    end = g_strstr_len (proj_string, -1, "</name>");
    
    if (!start || !end)
         return NULL;
    
    end--;
    gchar *name = get_text_between_strings (proj_string, start, end);

    start = g_strstr_len (proj_string, -1, "<main_file>") + 11; /*strlen("<main_file>") == 11*/
    end = g_strstr_len (proj_string, -1, "</main_file>");
    
    if (!start || !end)
         return NULL;
    
    end--;
    gchar *main_file = get_text_between_strings (proj_string, start, end);
    
    start = g_strstr_len (proj_string, -1, "<current_dir>") + 13; /*strlen("<current_dir>") == 13*/
    end = g_strstr_len (proj_string, -1, "</current_dir>");
    
    if (!start || !end)
         return NULL;
    
    end--;
    gchar *current_dir = get_text_between_strings (proj_string, start, end);
    
    start = g_strstr_len (proj_string, -1, "<version>") + 9; /*strlen("<version>") == 9*/
    end = g_strstr_len (proj_string, -1, "</version>");
    
    if (!start || !end)
         return NULL;
    
    end--;
    gchar *version = get_text_between_strings (proj_string, start, end);

    Project *proj  = project_new (name, main_file, proj_file, current_dir,
                                   version, NULL, NULL);
    
    g_free (name);
    g_free (main_file);
    g_free (version);
    g_free (current_dir);
    
    return proj;
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
    
    proj_gstring = g_string_append (proj_gstring, "<current_dir>");
    proj_gstring = g_string_append (proj_gstring, proj->current_dir);
    proj_gstring = g_string_append (proj_gstring, "</current_dir>\n");
    
    proj_gstring = g_string_append (proj_gstring, "<version>");
    proj_gstring = g_string_append (proj_gstring, proj->version);
    proj_gstring = g_string_append (proj_gstring, "</version>\n");
    
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