#include "py_class.h"
#include <string.h>
#include "core_funcs.h"

/*Create new PyClass
 *
*/
PyClass *
py_class_new (gchar *name, gchar **base_class_names, PyClass **base_classes,
             PyFunc **func_array, gdouble pos, int indentation)
{
    PyClass *py_class = g_malloc (sizeof (PyClass));
    py_class->name = g_strdup (name);
    py_class->base_class_names = g_strdupv (base_class_names);
    py_class->base_classes = base_classes;
    py_class->py_func_array = py_func_dupv (func_array);
    py_class->pos = pos;
    py_class->indentation = indentation;
    return py_class;
}

/*Create new PyClass
 * from definition
 */
PyClass *
py_class_new_from_def (gchar *def_string, gdouble pos,  int indentation)
{
    gchar *class = g_strstr_len (def_string, -1, "class");
    if (!class)
        return NULL;
    
    class += strlen ("class");
    gchar *open_bracket = g_strstr_len (def_string, -1, "(");
    if (!open_bracket)
        open_bracket = &def_string [strlen (def_string) - 1];

    //Copying only func_name
    int len = strlen (class) - strlen (open_bracket);

    gchar *name = g_malloc0 (sizeof (gchar) * len);
    gchar *n = name;
    gchar *i = class-1;
    while (++i != open_bracket)
         *n++ = *i;

    g_strstrip (name);
    
    //Stripping arguments and remove brackets
    gchar **base_classv = g_strsplit (open_bracket, ",", 0); 
    gchar **p = base_classv;
    while (*p)
    {
        g_strstrip (*p);
        *++p;
    }
    base_classv[0] = remove_char (base_classv[0], '(');    
    *--p = remove_char (*p,')');
    
    PyClass *py_class = py_class_new (name, base_classv, NULL, NULL, pos, indentation);
    g_free (name);
    g_strfreev (base_classv);
    return py_class;
}

/*Destroy PyClass
 *
 */
void
py_class_destroy (PyClass *py_class)
{
    g_free (py_class->name);
    g_strfreev (py_class->base_class_names);
    py_funcv_destroy (py_class->py_func_array);
    g_free (py_class);
}
