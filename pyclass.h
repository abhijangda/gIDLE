//25 Lines
#ifndef __PY_CLASS_H__
#define __PY_CLASS_H__

struct _pydef
{
    char *func_name;
    int pos;
    };

struct _pyclass
{
    struct _pydef *function_array;
    int function_array_size;
    char **self_array;
    int self_array_size;
    int pos;
    int indentation;
    char *class_name;
    };

typedef struct _pyclass pyclass;
typedef struct _pydef pydef;

void pyclass_add_function(pyclass *class, char *func_name, int pos);
void pyclass_add_self(pyclass *class, char *self_name);
void pyclass_init(pyclass *class, char *class_name, int pos, int indentation);
void pyclass_remove_function(pyclass *class, char *function_name);
void pyclass_insert_function(pyclass *class, char *function_name, int pos, int index);
#endif