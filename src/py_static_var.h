#include "py_variable.h"

#include <stdlib.h>
#include <gio/gio.h>
#include <glib.h>
#include <glib/gprintf.h>

#ifndef PY_STATIC_VAR_H
#define PY_STATIC_VAR_H

#define PY_STATIC_VAR(self) ((PyStaticVar *)self)

struct py_static_var
{
    PyVariable py_var;
    PyVariable *type;
    gchar *type_str;
};

typedef struct py_static_var PyStaticVar;

PyStaticVar *
py_static_var_new (gchar *name, gchar *type_str, PyVariable *type);

PyStaticVar *
py_static_var_new_from_def (gchar *def_string);

void
py_static_varv_add_py_static_var (PyStaticVar ***py_static_var_array, int *size, PyStaticVar *py_static_var);

gchar *
py_static_var_get_definition (PyVariable *var);

PyVariable *
py_static_var_dup (PyVariable *var);

void
py_static_var_destroy (PyVariable *var);

#endif /*PY_STATIC_VAR_H*/