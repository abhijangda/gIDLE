#include "py_func.h"
#include "py_class.h"

#ifndef PY_MODULE_H
#define PY_MODULE_H

#define PY_MODULE(self) ((PyModule *)self)

struct py_module
{
    PyVariable py_var;
    char *path;
    PyVariable **py_variable_array;
    int py_variable_array_size;
    struct py_module *parent_py_module;
};

typedef struct py_module PyModule;

PyModule *
py_module_new (char *name, char *path);

gchar *
py_module_get_definition (PyVariable *py_var);

PyModule *
py_module_search_for_module_name (PyModule *module, char *name);

PyVariable *
py_module_search_for_class_name (PyModule *module, char *name);

void
py_modulev_add_module (PyModule ***py_module, int *size, PyModule *module);

PyVariable *
py_module_dup (PyVariable *module);

void
py_module_destroy (PyVariable *module);

#endif /*PY_MODULE_H*/