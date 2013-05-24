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
    py_class->nested_classes = NULL;
    py_class->nested_classes_size = 0;
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

    //Copying only class_name
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

/* Add py_class to 
 * py_class_array
 */
void
py_classv_add_py_class (PyClass ***py_class_array, int *size, PyClass *py_class)
{
    (*py_class_array) = g_realloc (*py_class_array,
                                           ((*size) + 1) * sizeof (PyClass *));
    (*py_class_array) [*size] = py_class;
    (*size)++;
}

/* Add nested classes to their parent classes
 * new array is returned to through argument
 */
void
convert_py_class_array_to_nested_class_array (PyClass ***py_classv,
                                             int *py_classvsize, PyClass **prev_py_classv,
                                             int prev_py_classv_size, int parent_class_index)
{
    int i, first_more_indent = -1;
    if (parent_class_index == prev_py_classv_size)
        return;

    /*If current class is at indentation 0, then add it to py_classv*/
    if (prev_py_classv [parent_class_index]->indentation == 0)
    {
        py_classv_add_py_class (py_classv, py_classvsize,
                               prev_py_classv [parent_class_index]);
    }

   /*Check if next class has indentation greater than current_class*/
   if (parent_class_index + 1 !=prev_py_classv_size &&
      prev_py_classv [parent_class_index + 1]->indentation > prev_py_classv [parent_class_index]->indentation)
    {
        first_more_indent = prev_py_classv [parent_class_index + 1]->indentation;
        py_classv_add_py_class (&(prev_py_classv [parent_class_index]->nested_classes), 
                               &(prev_py_classv [parent_class_index]->nested_classes_size),
                               prev_py_classv [parent_class_index + 1]);
    }
    
    /*If no class with greater indentation is found then call this recursively*/
    if (first_more_indent == -1)
    {
        convert_py_class_array_to_nested_class_array (py_classv, py_classvsize,
                                                      prev_py_classv,
                                                      prev_py_classv_size,
                                                      parent_class_index + 1);
        return;
    }

    /*Now add all the classes which are at the same indentation of first_more_indent
     *break when first class is found which at less indentation than first_more_indent
     */
    for (i = parent_class_index + 2; i < prev_py_classv_size; i++)
    {
        if (prev_py_classv [i]->indentation == first_more_indent)
        {
            py_classv_add_py_class (&(prev_py_classv [parent_class_index]->nested_classes), 
                                   &(prev_py_classv [parent_class_index]->nested_classes_size),
                                   prev_py_classv [i]);
        }
        else if (prev_py_classv [i]->indentation > first_more_indent)
            continue;
        else
            break;
    }

    /*Now call this function recursively for next class in array*/
    convert_py_class_array_to_nested_class_array (py_classv, py_classvsize,
                                                      prev_py_classv,
                                                      prev_py_classv_size,
                                                      parent_class_index + 1);
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
    g_free (py_class->nested_classes);
    g_free (py_class);
}
