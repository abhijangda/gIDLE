#include <stdlib.h>
#include <gio/gio.h>
#include <glib.h>
#include <glib/gprintf.h>

#include "core_funcs.h"
#include "py_module.h"

/*Create a new PyModule
 */

PyModule *
py_module_new (char *name, char *path)
{
    PyModule *module = g_malloc (sizeof (PyModule));
    if (!module)
        return NULL;
    
    py_variable_init (PY_VARIABLE (module), name, MODULE);
    module->py_var.destroy = py_module_destroy;
    module->py_var.get_definition = py_module_get_definition;
    module->py_var.dup = py_module_dup;
    module->path = g_strdup (path);
    module->py_variable_array_size = 0;
    module->py_variable_array = NULL;
    module->parent_py_module = NULL;
    return module;
}

/*Duplicates the passed
 *module
 */
PyVariable *
py_module_dup (PyVariable *_module)
{
    PyModule *module = PY_MODULE (_module);
    PyVariable **py_var_array = NULL;
    int py_var_array_size = 0;
    int i;

    for (i = 0; i < module->py_variable_array_size; i++)
        py_variablev_add_variable (&py_var_array, &py_var_array_size,
            module->py_variable_array[i]->dup (module->py_variable_array[i]));
    
    PyModule *new_module = py_module_new (_module->name, module->path);
    new_module->py_variable_array = py_var_array;
    new_module->py_variable_array_size = py_var_array_size;
    py_variable_set_doc_string (PY_VARIABLE (new_module), _module->doc_string);
    return PY_VARIABLE (new_module);
}

gchar *
py_module_get_definition (PyVariable *py_var)
{
    return g_strdup (py_var->name);
}

/*Search recursively for module given
 *by name in the module.
 */
PyModule *
py_module_search_for_module_name (PyModule *module, char *name)
{
    if (module && !g_strcmp0 (PY_VARIABLE (module)->name, name))
        return module;

    int i;
    //printf ("module %s size %d\n", ((PyVariable *)module)->name, module->py_module_array_size);
    for (i = 0; i < module->py_variable_array_size; i++)
    {
        if (module->py_variable_array [i]->type != MODULE)
            continue;

        PyModule *m = py_module_search_for_module_name (PY_MODULE (module->py_variable_array [i]), name);
        if (m)
            return m;
    }
    return NULL;
}

/*Search for the class given by name
 *in the module
 */
PyVariable *
py_module_search_for_class_name (PyModule *module, char *name)
{
    int i;
    for (i = 0; i < module->py_variable_array_size; i++)
    {
        PyVariable *py_var = module->py_variable_array [i];
        if (py_var->type == MODULE)
        {
            PyVariable *var = py_module_search_for_class_name (PY_MODULE (py_var), name);
            if (var)
                return var;
        }
        else if (py_var->type == CLASS && !g_strcmp0 (py_var->name, name))
            return py_var;
    }
    return NULL;
}

void
py_modulev_add_module (PyModule ***py_modulev, int *size, PyModule *module)
{
    (*py_modulev) = g_realloc ((*py_modulev),
                                           (*size + 1) * sizeof (PyModule *));
    (*py_modulev) [*size] = module;
    (*size)++;
}

/*Destroys module
 */
void
py_module_destroy (PyVariable *_module)
{
    if (!_module)
        return;
    
    PyModule *module = PY_MODULE (_module);
    py_variable_destroy (&(module->py_var));
    int i;
    for (i = 0; i < module->py_variable_array_size; i++)
    {
        if (module->py_variable_array [i])
            module->py_variable_array [i]->destroy (module->py_variable_array [i]);
    }

    g_free (module->py_variable_array);
    module->py_variable_array = NULL;
    g_free (module->path);
    module->path = NULL;
    g_free (module);
}