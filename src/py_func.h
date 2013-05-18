#include <glib.h>
#include <glib/gprintf.h>

#ifndef PY_FUNC_H
#define PY_FUNC_H

struct _py_func
{
    gchar *name;
    gchar **argv; //NULL terminated array
    int pos;
    int indentation;
};

typedef struct _py_func PyFunc;

PyFunc *
py_func_new (gchar *name, gchar **argv, int pos, int indentation);

void
py_func_destroy (PyFunc *py_func);

PyFunc *
py_func_new_from_def (gchar *def_string, int pos, int indentation);

gchar *
py_func_get_definition (PyFunc *py_func);

PyFunc *
py_func_dup (PyFunc *py_func);

void
py_funcv_append (PyFunc ***py_funcv, PyFunc *);

PyFunc **
py_func_dupv (PyFunc **__py_func);

void
py_funcv_destroy (PyFunc **py_funcv);
#endif /*PY_FUNC_H*/