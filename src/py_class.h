#include "py_func.h"

#ifndef PY_CLASS_H
#define PY_CLASS_H

struct _py_class
{
    gchar *name;
    gchar **base_class_names;
    struct _py_class **base_classes;
    struct _py_class **nested_classes;
    int nested_classes_size;
    PyFunc **py_func_array;
    gdouble pos;
    int indentation;
};

typedef struct _py_class PyClass;

PyClass *
py_class_new (gchar *name, gchar **base_class_name, PyClass **base_classes, PyFunc **func_array, gdouble pos, int indentation);

PyClass *
py_class_new_from_def (gchar *def, gdouble pos, int indentation);

void
py_class_destroy (PyClass *py_class);

void
py_classv_add_py_class (PyClass ***py_class_array, int *size, PyClass *py_class);

void
convert_py_class_array_to_nested_class_array (PyClass ***py_classv,
                                             int *py_classvsize, PyClass **prev_py_classv,
                                             int prev_py_classv_size, int parent_class_index);
#endif /*PY_CLASS_H*/