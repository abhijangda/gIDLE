#include <stdlib.h>
#include <gio/gio.h>
#include <glib.h>
#include <glib/gprintf.h>
#include <string.h>

#include "codewidget.h"
#include "core_funcs.h"

#define LEVEL 2

extern GAsyncQueue *async_queue;

extern GRegex *regex_import_as, *regex_import;
extern GRegex *regex_from_import, *regex_from_import_as;
extern GRegex *regex_class, *regex_func;
extern GRegex *regex_global_var, *regex_static_var;
extern GRegex *regex_self_var;

extern char *sys_path_string;

static void
_codewidget_parser_parse_lines (CodeWidget *codewidget, GFileInputStream **distream, PyModule **_module, PyClass **klass);

static PyModule *
_codewidget_parser_recursively_create_modules (CodeWidget*, GFile *dir, PyModule **parent);

char *
_py_module_get_doc_string_for_py_var (GFileInputStream **distream);

/*This function will search for module given by name
 *inside dir recursively. It will return TRUE if module 
 *is found and module_path will be assigned to 
 *the path of module. If not found it will return FALSE
 *and module_path will be NULL.
 */
static gboolean
_search_for_module_recursively (char *name, GFile *dir, char **module_path)
{
    gchar *dir_basename = g_file_get_basename (dir);
    if (!g_strcmp0 (name, dir_basename))
    {
        GFile *file = g_file_get_child (dir, "__init__.py");
        if (g_file_query_exists (file, NULL))
        {
            g_free (dir_basename);
            g_object_unref (file);
            *module_path = g_file_get_path (dir);
            return TRUE;
        }
        g_object_unref (file);
    }

    g_free (dir_basename);

    GFileEnumerator *enumerator;
    enumerator = g_file_enumerate_children (dir,
                                            G_FILE_ATTRIBUTE_STANDARD_NAME
                                            "," 
                                            G_FILE_ATTRIBUTE_STANDARD_TYPE,
                                            G_FILE_QUERY_INFO_NONE,
                                            NULL, NULL);
    if (!enumerator)
    {
        module_path = NULL;
        return FALSE;
    }

    GFileInfo *fileinfo;
    while ((fileinfo = g_file_enumerator_next_file (enumerator, NULL, NULL)) != NULL)
    {
        const gchar *file_name = g_file_info_get_name (fileinfo);
        if (g_file_info_get_file_type (fileinfo) == G_FILE_TYPE_DIRECTORY)
        {
            GFile *file = g_file_get_child (dir, file_name);
            if (_search_for_module_recursively (name, file, module_path))
                return TRUE;

            g_object_unref (file);
        }
        else
        {
            gchar *full_name = g_strconcat (name, ".py", NULL);
            if (!g_strcmp0 (full_name, file_name))
            {
                GFile *file = g_file_get_child (dir, file_name);
                *module_path = g_file_get_path (file);
                g_object_unref (file);
                return TRUE;
            }
        }
        g_object_unref (fileinfo);
    }

    g_object_unref (enumerator);
    return FALSE;
}

/*This function will search for module in dir.
 *It will search for module in this directory
 *only by visiting dir's child. Return same 
 *values as above function.
 */
static gboolean
_search_for_module_in_dir (char *name, GFile *dir, char **module_path)
{
    GFileEnumerator *enumerator;
    enumerator = g_file_enumerate_children (dir,
                                            G_FILE_ATTRIBUTE_STANDARD_NAME
                                            "," 
                                            G_FILE_ATTRIBUTE_STANDARD_TYPE,
                                            G_FILE_QUERY_INFO_NONE,
                                            NULL, NULL);
    if (!enumerator)
    {
        module_path = NULL;
        return FALSE;
    }

    GFileInfo *fileinfo;
    while ((fileinfo = g_file_enumerator_next_file (enumerator, NULL, NULL)) != NULL)
    {
        const gchar *file_name = g_file_info_get_name (fileinfo);
        if (g_file_info_get_file_type (fileinfo) == G_FILE_TYPE_DIRECTORY)
        {
            GFile *_dir = g_file_get_child (dir, file_name);
            gchar *dir_basename = g_file_get_basename (_dir);
            if (!g_strcmp0 (name, dir_basename))
            {
                g_free (dir_basename);
                GFile *file = g_file_get_child (_dir, "__init__.py");
                if (g_file_query_exists (file, NULL));
                {
                    g_object_unref (file);
                    *module_path = g_file_get_path (_dir);
                    return TRUE;
                }
                g_object_unref (file);
            }

            g_free (dir_basename);
            g_object_unref (_dir);
        }
        else
        {
            gchar *full_name = g_strconcat (name, ".py", NULL);
            if (!g_strcmp0 (full_name, file_name))
            {
                GFile *file = g_file_get_child (dir, file_name);
                *module_path = g_file_get_path (file);
                g_object_unref (file);
                return TRUE;
            }
        }
        g_object_unref (fileinfo);
    }

    g_object_unref (enumerator);
    return FALSE;
}

/*This function will search for module
 *given by name in directories of sys_pathv
 *recursively.
 */
static char *
_get_module_path_from_name (char *name, char **sys_pathv)
{
    char **path = sys_pathv;

    while (*path)
    {
        char *module_path = NULL;
        GFile *file = g_file_new_for_path (*path);
        if (_search_for_module_recursively (name, file, &module_path))
        {
            g_object_unref (file);
            return module_path;
        }
        g_object_unref (file);
        path++;
    }
    return NULL;
}

/*This function checks for module
 *given be name in the codewidget's
 *py_variable_array and return it 
 *if found.
 */
PyModule *
codewidget_parser_check_module_for_name (CodeWidget *codewidget, char *name)
{
    int i;

    for (i = 0; i <codewidget->py_variable_array_size; i++)
    {
        if (codewidget->py_variable_array[i]->type != MODULE)
            continue;

        PyModule *module = NULL;
        module = py_module_search_for_module_name (PY_MODULE (codewidget->py_variable_array[i]), name);
        if (module)
            return module;
    }

    return NULL;
}

/*This function will check for import statements in line
 *It will import any module and add to codewidget or 
 *parent_module if found.
 */
void
check_for_import_module (CodeWidget *codewidget, PyModule **_parent_module, char *line)
{
    PyModule *parent_module = NULL;
    if (_parent_module)
        parent_module = *_parent_module;

    GMatchInfo *match_info;
    /* First check if statement is like 
     * from <module> import <submodule> as <name>
     */
    
    /* Then check if it is like 
     * from <module> import <subthing>
     */
    int level = 0;
    PyModule *m = parent_module;
    while (m)
    {
        m = m->parent_py_module;
        level++;
    }

    if (level >= LEVEL -1)
        return;

    if (g_regex_match (regex_from_import_as, line, 0, &match_info))
    {
    }
    else if (g_regex_match (regex_from_import, line, 0, &match_info))
    {
        /*If statement is from <module>.<module> import <class/func/module*/
        g_strstrip (line);
        gchar *line2 = g_strstr_len (line, -1, "from") + 4; /*strlen("from")=4*/
        g_strstrip (line2);
        gchar **str_array = g_strsplit (line2, " ", 0);
        gchar *parent_name = str_array [0];
        gchar *line3 = g_strstr_len (line2, -1, "import") + 6; /*strlen("import") = 6*/
        g_strstrip (line3);
        gchar **str_array2 = g_strsplit (line3, ",", 0);

        gchar **str_split = str_array2;

        while (*str_split)
        {
            /*Combine parent_name and name to be imported from str_array2*/
            g_strstrip (*str_split);
            gchar *name = g_strconcat (parent_name, ".", *str_split, NULL);
            PyModule *module  = NULL;
            /*Import this name*/
            module = codewidget_parser_get_module_from_name (codewidget, name, _parent_module);
            if (module)
            {
                /*<module>.<module>.<class/func/module> is successfully loaded*/
                gchar *basename = g_path_get_basename (module->path);
                gchar *_name = g_strconcat (*str_split, ".py", NULL);
                if (g_strcmp0 (_name, basename))
                {
                    /*If basename and _name are not equal then name to be
                     *imported is not module.
                     */
                   
                    /*Search for *str_split in module*/
                    int i;
                    for (i = 0; i < module->py_variable_array_size; i++)
                         if (!g_strcmp0 (module->py_variable_array [i]->name, *str_split))
                            break;

                    if (i < module->py_variable_array_size)
                    {
                        /*If founded (which it should be), then add it to
                         *parent_module's py_variable_array
                         */
                        gchar *pname = g_strdup (PY_VARIABLE (module)->name);
                        gchar *dot_pos = strrchr (pname, '.');
                        PY_VARIABLE (module)->name [dot_pos - pname] = '\0';
                        g_free (pname);
                        if (parent_module)
                        {
                                PyVariable *new_var = module->py_variable_array [i]->dup (module->py_variable_array [i]);
                                g_free (new_var->name);
                                new_var->name = g_strdup (*str_split);
                                /*Add it to parent module*/
                                py_variablev_add_variable (&(parent_module->py_variable_array),
                                                            &(parent_module->py_variable_array_size),
                                                            new_var);
                        }
                        else
                        {                          
                                PyVariable *new_var = module->py_variable_array [i]->dup (module->py_variable_array [i]);
                                g_free (new_var->name);
                                new_var->name = g_strdup (*str_split);
                                /*Add it to parent module*/
                                py_variablev_add_variable (&(codewidget->py_variable_array),
                                                            &(codewidget->py_variable_array_size),
                                                            new_var);
                        }
                    }
                }
                else
                {
                     /*If basename and _name are equal then name to be
                     *imported is a module. So, change name of imported module
                     */
                    g_free (((PyVariable *)module)->name);
                    ((PyVariable *)module)->name = g_strdup (*str_split);
                }

                g_free (basename);
            }
            *str_split++;
            g_free (name);
        }
            
        g_strfreev (str_array);
        g_strfreev (str_array2);
        g_match_info_free (match_info);
    }
    else if (g_regex_match (regex_import_as, line, 0, &match_info))
    {
        /* if statement is like import <module> as <name> */
        g_strstrip (line);
        gchar **str_array = g_strsplit (line," ", 0);

        gchar *name = str_array [1];
        gchar *alias = str_array [3];

        PyModule *module  = NULL;
        module = codewidget_parser_get_module_from_name (codewidget, name, _parent_module);
        if (module)
        {
            if (g_strstr_len (name, -1, "."))
            {
                /*if statement is like import <module>.<module> as <name>*/
                /*In this case first check if the basename of module path is
                 *same as the string after the last dot
                 *If yes, then it is a module and simply change its name to
                 * alias
                 *Otherwise, it is a class or a function, add it to the
                 *corresponding imported array
                 */
            }
            else
            {
                /* Change the name to alias */
                g_free (PY_VARIABLE (module)->name);
                PY_VARIABLE (module)->name = g_strdup (alias);
            }
        }

        g_strfreev (str_array);
        g_match_info_free (match_info);
    }
    else if (g_regex_match (regex_import, line, 0, &match_info))
    {
        /*If not then is it like import <module>, <module>*/
        gchar *line2 = g_strstr_len (line, -1, "import") + 6; /*strlen("import") = 6*/
        g_strstrip (line2);
        gchar **str_array = g_strsplit (line2,",", 0);

        gchar **name = str_array;

        PyModule *module = NULL;
        while (*name)
        {
            g_strstrip(*name);
            module = codewidget_parser_get_module_from_name (codewidget, *name, _parent_module);
            name++;
        }
        g_strfreev (str_array);
        g_match_info_free (match_info);
    }
}

/*This function is the main called function
 *to import any module, it will call all other
 *functions. If module has already been imported
 *then it will return that module otherwise a new
 *module.
 */
PyModule * 
codewidget_parser_get_module_from_name (CodeWidget *codewidget, char *name, PyModule **parent)
{
    if (!g_strcmp0 (name, "sys"))
        return NULL;

    PyModule *module = codewidget_parser_check_module_for_name (codewidget, name);
    if (module)
    {
        /*If module has already been imported then add only to code widget 
         * if required, because adding to parent module may make a circular
         * list type thing, because of which above function may produce
         * infinite recursion
         */

        if (!parent)
        {
            /*If module is in codewidget then dont't add it*/
            int i;
            for (i = 0; i < codewidget->py_variable_array_size; i++)
                if (!g_strcmp0 (codewidget->py_variable_array [i]->name, name))
                    break;

            if (i == codewidget->py_variable_array_size)
            {
                py_variablev_add_variable (&(codewidget->py_variable_array), 
                                        &(codewidget->py_variable_array_size), PY_VARIABLE (module)->dup (PY_VARIABLE (module)));
                module->parent_py_module = NULL;
            }
        }
    return module;
    }

    char *dir = g_path_get_dirname (codewidget->file_path);
    char *full_sys_path_str = g_strconcat (dir, "\n", sys_path_string, NULL);
    g_free (dir);

    char **sys_pathv = g_strsplit (full_sys_path_str, "\n", 0);

    /*Get path of module*/
    char *path;
    /*If name is of type <module>.<module> or <module>.<class>
     * i.e. name involves "."
     */

    if (g_strstr_len (name, -1, "."))
    {
        gchar **path_split = g_strsplit (name, ".", 0);
        /*String before first "." means module*/
        path = _get_module_path_from_name (path_split [0], sys_pathv);
        if (!path)
            return NULL;

        /*If module is a file then import it*/

        if (!g_strstr_len (path, -1, ".py"))
        {
            /*If module is a folder then go deep untill we 
             * found as much as we can*/
            gchar **_split = path_split;
            _split++;
            while (*_split)
            {
                GFile *dir = g_file_new_for_path (path);
                g_free (path);
                path = NULL;
                gboolean ans = _search_for_module_in_dir (*_split, dir, &path);

                if (ans)
                {
                    /*If found then check if it is a py file*/
                    if (g_strstr_len (path, -1, ".py"))
                    {
                        /*If it is then break*/
                        g_object_unref (dir);
                        break;
                    }
                }
                else
                    /*Cannot find the path so return*/
                    goto end;

                _split++;
                g_object_unref (dir);
            }
        }
        g_strfreev (path_split);
    }

    else
        path = _get_module_path_from_name (name, sys_pathv);

    if (!path)
        return NULL;

    module = py_module_new (name, path);
    if (!module)
        goto end;
    
    /*Sends status bar message to main loop using GAsynQueues*/
    gchar *status_text = g_strdup_printf ("Parsing file %s", path);
    g_async_queue_push (async_queue, (gpointer) status_text);

    if (parent)
    {
        py_variablev_add_variable (&((*parent)->py_variable_array), 
                                &((*parent)->py_variable_array_size), PY_VARIABLE (module));
        module->parent_py_module = *parent;
    }

    else
    {
        py_variablev_add_variable (&(codewidget->py_variable_array), 
                                &(codewidget->py_variable_array_size), PY_VARIABLE (module));
        module->parent_py_module = NULL;
    }

    /*Open module*/
    if (g_strstr_len (path, -1, ".py"))
    {
        int len = strlen (path);
        if (path [len - 1] != 'y' || path [len - 2] != 'p' || path [len - 3] != '.')
            goto end;

        /*If module is a file*/
        /*Open it and parse each line*/
        if (!module)
            goto end;

        GFile *file = g_file_new_for_path (path);
        GFileInputStream *istream = g_file_read (file, NULL, NULL);
        if (!istream)
        {
            PyVariable *py_var = PY_VARIABLE (module);
            PY_VARIABLE (module)->destroy (py_var);
            g_object_unref (file);
            g_free (path);
            g_strfreev (sys_pathv);
            return NULL;
        }

        /*Find Module's doc string*/
        gchar *doc_str = _py_module_get_doc_string_for_py_var (&istream);
        if (doc_str)
        {
            py_variable_set_doc_string ((PyVariable*)module, doc_str);
            g_free (doc_str);
        }
        _codewidget_parser_parse_lines (codewidget, &istream, &module, NULL);

        g_object_unref (istream);
        g_object_unref (file);
    }
    else
    {
        /*If module is a folder*/
        /*Recursively create modules of files and folders and parse files*/
        GFile *dir = g_file_new_for_path (path);
        _codewidget_parser_recursively_create_modules (codewidget, dir, &module);
        g_object_unref (dir);
    }

end:
    g_free (path);
    g_strfreev (sys_pathv);

    return module;
}

/*This function is called to import a directory module 
 *it will recursively create directories child module.
 *and returns the directory module.
 */
PyModule *
_codewidget_parser_recursively_create_modules (CodeWidget *codewidget, GFile *dir, PyModule **parent)
{
    GFileEnumerator *enumerator;
    enumerator = g_file_enumerate_children (dir,
                                            G_FILE_ATTRIBUTE_STANDARD_NAME
                                            "," 
                                            G_FILE_ATTRIBUTE_STANDARD_TYPE,
                                            G_FILE_QUERY_INFO_NONE,
                                            NULL, NULL);

    if (!enumerator)
        return NULL;

    GFileInfo *fileinfo;
    PyModule *new_module = NULL;
    while ((fileinfo = g_file_enumerator_next_file (enumerator, NULL, NULL)) != NULL)
    {
        const gchar *file_name = g_file_info_get_name (fileinfo);
        GFile *file = g_file_get_child (dir, file_name);
        gchar *path = g_file_get_path (file);
        if (g_file_info_get_file_type (fileinfo) == G_FILE_TYPE_DIRECTORY)
        {
            new_module = py_module_new ((gchar *)file_name, path);
            py_variablev_add_variable (&((*parent)->py_variable_array), 
                                         &((*parent)->py_variable_array_size),
                                         PY_VARIABLE (new_module));
            new_module->parent_py_module = *parent;
            _codewidget_parser_recursively_create_modules (codewidget, file, &new_module);
        }

        else
        {
            if (!g_strcmp0 (file_name, "__init__.py"))
            {
                g_object_unref (fileinfo);
                g_object_unref (file);
                continue;
            }

            int len = strlen (file_name);
            if (file_name [len - 1] != 'y' || file_name [len - 2] != 'p' || file_name [len - 3] != '.')
            {
                g_object_unref (fileinfo);
                g_object_unref (file);
                continue;
            }

            GFileInputStream *istream = g_file_read (file, NULL, NULL);
            if (!istream)
            {
                g_object_unref (file);
                g_object_unref (fileinfo);
                continue;
            }

            gchar *py_pos = g_strstr_len (file_name, -1, ".py");
            gchar name [py_pos - file_name + 1];
            gchar *p = (gchar *)file_name, *q = name;
            while (p != py_pos)
                *q++ = *p++;

            *q = '\0';

            new_module = py_module_new (name, path);
            py_variablev_add_variable (&((*parent)->py_variable_array),
                                        &((*parent)->py_variable_array_size),
                                        PY_VARIABLE (new_module));
            new_module->parent_py_module = NULL;

            /*Find Module's doc string*/
            gchar *doc_str = _py_module_get_doc_string_for_py_var (&istream);
            if (doc_str)
            {
                py_variable_set_doc_string (PY_VARIABLE (new_module), doc_str);
                g_free (doc_str);
            }
            _codewidget_parser_parse_lines (codewidget, &istream, &new_module, NULL);

            g_object_unref (istream);
        }
        
        g_free (path);
        g_object_unref (file);
        g_object_unref (fileinfo);
    }
    return new_module;
}

/*Given the current GFileInputStream, this function will
 *get the doc string defined at the current line. It will
 *return the doc_string if found otherwise return NULL.
 */
char *
_py_module_get_doc_string_for_py_var (GFileInputStream **distream)
{
    gchar *line;
    gsize line_size;
    GFileInputStream *_distream = *distream;
    goffset curr_seek = g_seekable_tell (G_SEEKABLE (_distream));
    GString *doc_str = g_string_new ("");

    /*Remove comment lines*/
    while ((line = g_file_input_stream_read_line (*distream)))
    {
        char *hash_text = g_strrstr (line, "#");
        if (hash_text)
            line[hash_text - line] = '\0';
        
        g_strstrip(line);
        if (g_strcmp0 (line, ""))
            break;

        g_free (line);
    }

    if (!line)
    {
        g_seekable_seek (G_SEEKABLE (_distream), curr_seek, G_SEEK_SET, 
                          NULL, NULL);
        return NULL;
    }

    /* First Line should contain Triple or Single Quotes */
    gchar *quotes_pos = g_strstr_len (line, -1, "'''");
    if (!quotes_pos)
         quotes_pos = g_strstr_len (line, -1, "\"\"\"");
    
    if (!quotes_pos)
    {
        g_seekable_seek (G_SEEKABLE (_distream), curr_seek, G_SEEK_SET, 
                        NULL, NULL);
        return NULL;
    }
    quotes_pos += 3;
    doc_str = g_string_append (doc_str, quotes_pos);
    
    /*May be single line contains both starting and ending triple quotes*/
    gchar *second_quotes_pos = g_strstr_len (quotes_pos, -1, "'''");
    if (!second_quotes_pos)
        second_quotes_pos = g_strstr_len (quotes_pos, -1, "\"\"\"");
    
    if(second_quotes_pos)
    {
        line [second_quotes_pos - line] = '\0';
        goto end;
    }

    while ((line = g_file_input_stream_read_line (*distream)))
    {
        /* Append to doc_str untill triple quotes are again found */
        quotes_pos = g_strstr_len (line, -1, "'''");
        if (!quotes_pos)
            quotes_pos = g_strstr_len (line, -1, "\"\"\"");
        
        if (quotes_pos)
            line [quotes_pos - line] = '\0';

        g_strstrip (line);
        doc_str = g_string_append_c (doc_str, ' ');
        doc_str = g_string_append (doc_str, line);

        if (quotes_pos)
        {
            /* cannot add below line above "if (quotes_pos)" because 
             * quotes_pos is dependent on line
             */
            g_free (line);
            break;
        }

        g_free (line);
    }

end:
    /* Line NULL means there is error in reading or EOF */
    if (line)
    {
        gchar *str = doc_str->str;
        g_string_free (doc_str, FALSE);
        return str;
    }
    else
    {
        g_seekable_seek (G_SEEKABLE (_distream), curr_seek, G_SEEK_SET, 
                          NULL, NULL);
        g_string_free (doc_str, TRUE);
        return NULL;
    }
}

/*This function parses file given by GFileInputStream, line by line
 *creating any class and their function any importing any required
 *module.
 */
void
_codewidget_parser_parse_lines (CodeWidget *codewidget, GFileInputStream **distream, PyModule **_module, PyClass **klass)
{
    PyModule *module = *_module;
    gchar *line;
    gsize line_size;
    int indentation;
    gboolean can_find_static = TRUE;
    GMatchInfo *match_info_class, *match_info_func, *match_info;

    while ((line = g_file_input_stream_read_line (*distream)))
    {
        /*Remove Comments*/
        char *hash_text = g_strrstr (line, "#");
        if (hash_text)
            line[hash_text - line] = '\0';

        if (g_regex_match (regex_class, line, 0, &match_info_class))
        {
            gchar *class_def_string = g_match_info_fetch (match_info_class,0);
            class_def_string = remove_char (class_def_string, ':');
    
            indentation = get_indent_spaces_in_string (class_def_string) / indent_width;
            if (indentation > 0 && !klass)
            {
                g_free (class_def_string);
                continue;
            }

            PyClass *new_class = py_class_new_from_def (class_def_string, 
                                                        0, indentation);

            py_variablev_add_variable (&(module->py_variable_array), 
                                     &(module->py_variable_array_size), PY_VARIABLE (new_class));

            g_free (class_def_string);

            if (indentation > 0)
            {
                py_classv_add_py_class (&((*klass)->nested_classes), &((*klass)->nested_classes_size), new_class);
            }

            /*Detecting for class doc strings*/
            gchar *doc_str = _py_module_get_doc_string_for_py_var (distream);
            if (doc_str)
            {
                py_variable_set_doc_string ((PyVariable*)new_class, doc_str);
                g_free (doc_str);
            }

            _codewidget_parser_parse_lines (codewidget, distream, _module, &new_class);

            /******************/
            g_match_info_free (match_info_class);
        }
        else if (g_regex_match (regex_func, line ,0, &match_info_func))
        {
            gchar *func_def_string = g_match_info_fetch(match_info_func,0);
            can_find_static = FALSE;
            func_def_string = remove_char (func_def_string, ':');
            indentation = get_indent_spaces_in_string (func_def_string) / indent_width;
            PyFunc *py_func = py_func_new_from_def (func_def_string, 0,
                                                   indentation);

            if (!py_func)
            {
                g_free (func_def_string);
                continue;
            }
            if (klass)
                py_funcv_append (&((*klass)->py_func_array), py_func);
            else
                py_variablev_add_variable (&(module->py_variable_array), 
                                            &(module->py_variable_array_size), PY_VARIABLE (py_func));

            /*************************/
            g_free (func_def_string);
            g_match_info_free (match_info_func);

             /*Detecting for function's doc string*/
            gchar *doc_str = _py_module_get_doc_string_for_py_var (distream);
            if (doc_str)
            {
                py_variable_set_doc_string (PY_VARIABLE (py_func), doc_str);
                g_free (doc_str);
            }
            
            if (klass && py_func && !g_strcmp0 (PY_VARIABLE (py_func)->name, "__init__"))
            {
                /*Detect __init__ and parse it. As __init__ function is always
                 *executed when object is created, so all instance variable 
                 *declared in __init__ will be added
                 */
                while ((line = g_file_input_stream_read_line (*distream)))
                {
                    int indentation2 = get_indent_spaces_in_string (line) / indent_width;
                    if (indentation2 == indentation)
                    {
                        g_seekable_seek (G_SEEKABLE (*distream),
                             g_seekable_tell (G_SEEKABLE (*distream)) - strlen (line) + 1, G_SEEK_SET,
                             NULL, NULL);
                        break;
                    }

                    if (g_regex_match (regex_self_var, line ,0, &match_info_class))
                    {
                        gchar *self_dot_pos = strchr (line, '.');
                        self_dot_pos++;
                        PyStaticVar *static_var = py_static_var_new_from_def (self_dot_pos);
                        if (static_var)
                        {
                            /*Detecting for static_var's doc string. All though Python
                              *doesn't support it but static variables has their doc string
                              *set by user
                              */
                            int i;
                            for (i = 0; i < (*klass)->py_static_var_array_size; i++)
                            {
                                if (!g_strcmp0 (PY_VARIABLE (static_var)->name, PY_VARIABLE ((*klass)->py_static_var_array[i])->name))
                                    break;
                            }
                            
                            if (i == (*klass)->py_static_var_array_size)
                            {
                                py_static_varv_add_py_static_var (&((*klass)->py_static_var_array),
                                                            &((*klass)->py_static_var_array_size), static_var);
                                gchar *doc_str = _py_module_get_doc_string_for_py_var (distream);
                                if (doc_str)
                                {
                                    py_variable_set_doc_string (PY_VARIABLE (static_var), doc_str);
                                    g_free (doc_str);
                                }
                            }
                            else
                                py_static_var_destroy (PY_VARIABLE (static_var));
                        }
                        g_match_info_free (match_info_class);
                    }
                g_free (line);
                line = NULL;
                }                
            }
        }
        else if (klass && can_find_static && 
                 g_regex_match (regex_static_var, line ,0, &match_info_class))
        {
            PyStaticVar *static_var = py_static_var_new_from_def (line);
            if (static_var)
            {
                py_static_varv_add_py_static_var (&((*klass)->py_static_var_array),
                                                &((*klass)->py_static_var_array_size), static_var);
                
                 /*Detecting for static_var's doc string. All though Python
                  *doesn't support it but static variables has their doc string
                  *set by user
                  */
               
                gchar *doc_str = _py_module_get_doc_string_for_py_var (distream);
                if (doc_str)
                {
                    py_variable_set_doc_string (PY_VARIABLE (static_var), doc_str);
                    g_free (doc_str);
                }
            }
            g_match_info_free (match_info_class);
        }
        
        /*If indentation of current line is less than current class
         * indentation then go to previous class
         */   
        indentation = get_indent_spaces_in_string (line) / indent_width;
        if (klass && indentation < (*klass)->indentation)
            return;

        check_for_import_module (codewidget, _module, line);
        if (g_regex_match (regex_global_var, line ,0, &match_info))
        {
            PyStaticVar *static_var = py_static_var_new_from_def (line);
            if (!klass)
                py_variablev_add_variable (&(module->py_variable_array),
                                            &(module->py_variable_array_size), PY_VARIABLE (static_var));
        }

        g_free (line);
        line = NULL;
    }
}