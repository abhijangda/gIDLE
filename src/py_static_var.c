#include "py_static_var.h"

#include <string.h>

PyStaticVar *
py_static_var_new (gchar *name, gchar *type_str, PyVariable *type)
{
    PyStaticVar *var = g_try_malloc (sizeof (PyStaticVar));
    if (!var)
        return NULL;

    py_variable_init (PY_VARIABLE (var), name, STATIC_VAR);
    var->type_str = g_strdup (type_str);
    var->py_var.destroy = py_static_var_destroy;
    var->py_var.get_definition = py_static_var_get_definition;
    var->py_var.dup = py_static_var_dup;
    var->type = type;

    return var;
}

PyVariable *
py_static_var_dup (PyVariable *var)
{
    return PY_VARIABLE (py_static_var_new (var->name, PY_STATIC_VAR (var)->type_str, PY_STATIC_VAR (var)->type));
}

gchar *
py_static_var_get_definition (PyVariable *var)
{
    return var->name;
}

void
py_static_varv_add_py_static_var (PyStaticVar ***py_static_var_array, int *size, PyStaticVar *py_static_var)
{
    (*py_static_var_array) = g_realloc (*py_static_var_array,
                                         ((*size) + 1) * sizeof (PyStaticVar *));
    (*py_static_var_array) [*size] = py_static_var;
    (*size)++;
}

PyStaticVar *
py_static_var_new_from_def (gchar *def_string)
{
    g_strstrip (def_string);
    gchar *equal_pos = strchr (def_string, '=');
    equal_pos ++;

    gchar *name = g_strndup (def_string, equal_pos - def_string - 1);
    
    g_strstrip (name);
    
    gchar *bracket = strrchr (equal_pos, '(');

    if (!bracket)
        bracket = strrchr (equal_pos, '\0');

    gchar *type;

    if (*equal_pos == *bracket)
        type = g_strdup ("tuple");

    else
        type = g_strndup (equal_pos, bracket - equal_pos - 1);

    g_strstrip (type);

    PyStaticVar *s = py_static_var_new (name, type, NULL);
    
    g_free (name);
    g_free (type);
    return s;
}

void
py_static_var_destroy (PyVariable *var)
{
}