//18 Lines
#include"pyclass.h"

#ifndef __PY_MODULE_H__
#define __PY_MODULE_H__
struct _pymodule
{
    char *module_name;
    char *module_path;
    pyclass *class_array;
    int class_array_size;
    };

typedef struct _pymodule pymodule;

void pymodule_init(pymodule *module,char *module_name, char *module_path);
void pymodule_add_class(pymodule *module, char *class_name, int indentation, int pos);
#endif