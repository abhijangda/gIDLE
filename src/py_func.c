#include "py_func.h"
#include "core_funcs.h"
#include <string.h>

extern PyVariable destroyed_variable;

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

    py_variable_init (PY_VARIABLE (py_func), name, FUNC);  
    py_func->py_var.destroy = py_func_destroy;
    py_func->py_var.get_definition = py_func_get_definition;
    py_func->py_var.dup = py_func_dup;
    py_func->arg_array = NULL;
    py_func->arg_array_size = 0;
    if (argv)
    {
        gchar **arg = argv;
        while (*arg)
        {
            if (!g_strcmp0 (*arg, ""))
            {
                arg++;
                continue;
            }
            PyStaticVar *var = NULL;
            if (strrchr (*arg, '='))
                var = py_static_var_new_from_def (*arg);

            else
                var = py_static_var_new (*arg, "", NULL);

            py_static_varv_add_py_static_var (&(py_func->arg_array), 
                                               &(py_func->arg_array_size), var);
            arg++;
        }
    }
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
py_func_get_definition (PyVariable *py_var)
{
    PyFunc *py_func = (PyFunc *)py_var;
    int i;
    GString *argv_str = g_string_new("");
    
    for (i = 0; i < py_func->arg_array_size; i++)
    {
        argv_str = g_string_append (argv_str,
                                    py_static_var_get_definition (
                                        PY_VARIABLE (py_func->arg_array [i])));
        if (i >= 1)
            argv_str = g_string_append_c (argv_str, ',');
    }
    gchar *def = g_strjoin ("", "def ", ((PyVariable *)py_func)->name, "(", argv_str->str, ")",
                           NULL);
    g_string_free (argv_str, TRUE);
    return def;
}

/*Duplicate PyFunc
 * returns a newly allocated copy
 */
PyVariable *
py_func_dup (PyVariable *__py_func)
{
    PyFunc *func = py_func_new (__py_func->name, NULL,
                                  PY_FUNC (__py_func)->pos, 
                                  PY_FUNC (__py_func)->indentation);
    func->arg_array = PY_FUNC (__py_func)->arg_array;
    func->arg_array_size = PY_FUNC (__py_func)->arg_array_size;
    py_variable_set_doc_string (PY_VARIABLE (func), __py_func->doc_string);
    return PY_VARIABLE (func);
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
        py_funcv [size-1] = PY_FUNC (py_func_dup (PY_VARIABLE (*p)));
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
py_func_destroy (PyVariable *py_func)
{
    if (!py_func)
        return;

    PyFunc *_func = PY_FUNC (py_func);
    int i;
    for (i = 0; i < _func->arg_array_size; i++)
         py_static_var_destroy (PY_VARIABLE (_func->arg_array [i]));

    py_variable_destroy (&(_func->py_var));
    g_free (_func);
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
        PyVariable *py_var  = PY_VARIABLE (*p);
        py_func_destroy (py_var);
        p++;
    }

    g_free (py_funcv);
}