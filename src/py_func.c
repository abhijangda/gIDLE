#include "py_func.h"
#include "core_funcs.h"
#include <string.h>

/* Create a new
 * PyFunc
 */
PyFunc *
py_func_new (gchar *name, gchar **argv, gdouble pos, int indentation)
{
    PyFunc *py_func = g_try_malloc (sizeof (PyFunc));
    if (!py_func)
    {
        printf ("Cannot allocate new PyFunc in py_func_new\n");
        return NULL;
    }

    py_func->name = g_strdup (name);
    py_func->argv = g_strdupv (argv);
    py_func->pos = pos;
    py_func->indentation = indentation;
    return py_func;
}

/*Create a new
 * PyFunc from definition
 * def_string must contain keyword def with a space
 */
PyFunc *
py_func_new_from_def (gchar *def_string, gdouble pos, int indentation)
{
    gchar *func = g_strstr_len (def_string, -1, "def");
    if (!func)
        return NULL;
    
    func += strlen ("def");
    gchar *open_bracket = g_strstr_len (def_string, -1, "(");

    if (!open_bracket)
        open_bracket = &def_string [strlen (def_string) - 1];
    //Copying only func_name
    gchar *name = g_try_malloc0 (sizeof (gchar) *((open_bracket - func) + 1));
    if (!name)
    {
        printf ("Cannot get name of func in py_func_new_from_def\n");
        return NULL;
    }
    
    gchar *n = name;
    gchar *i = func-1;
    while (++i != open_bracket)
         *n++ = *i;

    g_strstrip (name);

    //Stripping arguments and remove brackets
    gchar **argv = g_strsplit (open_bracket, ",", 0); 
    gchar **p = argv;
    while (*p)
    {
        g_strstrip (*p);
        *++p;
    }
    argv[0] = remove_char (argv[0], '(');    
    *--p = remove_char (*p,')');
    
    PyFunc *py_func = py_func_new (name, argv, pos, indentation);
    if (!py_func)
    {
        g_free (name);
        g_strfreev (argv);
        return NULL;
    }

    g_free (name);
    g_strfreev (argv);
    return py_func;
}

/*Get Definition of function
 * returns a newly allocated string
 */
gchar *
py_func_get_definition (PyFunc *py_func)
{
    gchar *argv_str = g_strjoinv (",", py_func->argv);
    gchar *def = g_strjoin ("", "def", py_func->name, "(", argv_str, ")",
                           NULL);
    g_free (argv_str);
    return def;
}

/*Duplicate PyFunc
 * returns a newly allocated copy
 */
PyFunc *
py_func_dup (PyFunc *__py_func)
{
    PyFunc *py_func = g_malloc (sizeof (PyFunc));
    py_func->name = g_strdup (__py_func->name);
    py_func->argv = g_strdupv (__py_func->argv);
    py_func->pos = __py_func->pos;
    return py_func;
}

/*Duplicate PyFunc array
 * returns a NULL terminated newly allocated copy
 */
PyFunc **
py_func_dupv (PyFunc **__py_funcv)
{
    if (!__py_funcv)
        return NULL;

    PyFunc **py_funcv = g_malloc (sizeof (PyFunc *));
    PyFunc **p = __py_funcv;
    int size  = 1;
    while (*p != NULL)
    {
        py_funcv [size-1] = *p;
        size++;
        py_funcv = g_realloc (py_funcv, (size)*sizeof (PyFunc *));
        p++;
    }
    py_funcv [size-1] = NULL;
    return py_funcv;
}

/* Add PyFunc to PyFunc Array
 * this function ensures that py_funcv will be NULL terminated
*/
void
py_funcv_append (PyFunc ***py_funcv, PyFunc *py_func)
{
    int size = -1;
    
    if (!(*py_funcv))
    {
        size = 1;
        (*py_funcv) = g_malloc (2*sizeof (PyFunc *));
    }
    else
    {
        /*Get size of py_funcv excluding NULL*/
        while ((*py_funcv) [++size]);
        (*py_funcv) = g_realloc (*py_funcv, (++size+1)*sizeof (PyFunc *));
    }
    (*py_funcv) [size - 1] = py_func;
    (*py_funcv) [size] = NULL;
}

/*Destroy 
 * PyFunc
 */
void
py_func_destroy (PyFunc *py_func)
{
    g_free (py_func->name);
    g_strfreev (py_func->argv);
    g_free (py_func);
}

/*Destroy
 * PyFunc array
 */
void
py_funcv_destroy (PyFunc **py_funcv)
{
    PyFunc **p = py_funcv;
    if (!p)
        return;

    int size  = 1;
    while (*p != NULL)
    {
        g_free (*p);
        p++;
    }
    g_free (py_funcv);
}