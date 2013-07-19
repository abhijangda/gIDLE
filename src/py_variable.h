/* An abstract class for Python Variable's including
 * Variables, Functions, Classes, Modules. All classes should
 * derive from this class. 
 * It provides "get_definition" function, which should be overriden 
 * and a member "name"
 */

#ifndef PY_VARIABLE_H
#define PY_VARIABLE_H

#define PY_VARIABLE(self) ((PyVariable *)self)

enum _py_type
{
    FUNC,
    CLASS,
    MODULE,
    STATIC_VAR,
};

typedef enum _py_type PyType;

struct py_variable
{
    char *name;
    char *doc_string;
    char *(*get_definition)(struct py_variable *);
    void (*destroy) (struct py_variable  *);
    struct py_variable *(*dup) (struct py_variable *);
    PyType type;
};

typedef struct py_variable PyVariable;

void
py_variable_init (PyVariable *py_var, char *name, PyType type);

void
py_variable_set_doc_string (PyVariable *py_var, char *doc_string);

void
py_variablev_add_variable (PyVariable ***py_variablev, int *size, PyVariable *variable);

void
py_variable_destroy (PyVariable *py_variable);

#endif /* PY_VARIABLE_H */