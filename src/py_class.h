#include "py_func.h"

#ifndef PY_CLASS_H
#define PY_CLASS_H

struct _py_class
{
    gchar *name;
    gchar **base_class_names;
    struct _py_class **base_classes;
    PyFunc **py_func_array;
    int pos;
    int indentation;
};

typedef struct _py_class PyClass;

PyClass *
py_class_new (gchar *name, gchar **base_class_name, PyClass **base_classes, PyFunc **func_array, int pos, int indentation);

PyClass *
py_class_new_from_def (gchar *def, int pos, int indentation);

void
py_class_destroy (PyClass *py_class);
#endif /*PY_CLASS_H*/