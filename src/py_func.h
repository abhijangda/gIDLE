#include <glib.h>
#include <glib/gprintf.h>

#include "py_variable.h"
#include "py_static_var.h"

#ifndef PY_FUNC_H
#define PY_FUNC_H

#define PY_FUNC(self) ((PyFunc *)self)

struct _py_func
{
    PyVariable py_var;
    gdouble pos;
    int indentation;
    PyStaticVar **arg_array;
    int arg_array_size;
};

typedef struct _py_func PyFunc;

PyFunc *
py_func_new (gchar *name, gchar **argv, gdouble pos, int indentation);

void
py_func_destroy (PyVariable *py_func);

PyFunc *
py_func_new_from_def (gchar *def_string, gdouble pos, int indentation);

gchar *
py_func_get_definition (PyVariable *py_var);

PyVariable *
py_func_dup (PyVariable *py_func);

void
py_funcv_append (PyFunc ***py_funcv, PyFunc *);

PyFunc **
py_func_dupv (PyFunc **__py_func);

void
py_funcv_destroy (PyFunc **py_funcv);
#endif /*PY_FUNC_H*/