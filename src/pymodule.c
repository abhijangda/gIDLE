//39 Lines
#include"pymodule.h"
#include<stdlib.h>
#include<stdio.h>

void pymodule_init(pymodule *module,char *module_name, char *module_path)
{
    module->class_array_size=0;
    module->class_array = NULL;
    asprintf(&module->module_name,"%s",module_name);
    asprintf(&module->module_path,"%s",module_path);
    pymodule_add_class(module,"Global Scope",0,0);
    //printf("MODULE %s initialized with class %s\n",module->module_name,module->class_array[0].class_name);
    }

void pymodule_add_class(pymodule *module, char *class_name, int indentation, int pos)
{
    if(module->class_array_size==0)
         module->class_array=(pyclass *)malloc(1*sizeof(pyclass));
    else
        module->class_array=(pyclass *)realloc(module->class_array,(module->class_array_size+1)*sizeof(pyclass));
    pyclass_init(&module->class_array[module->class_array_size],class_name,pos,indentation);
    module->class_array_size++;

    /*if(_class)
    {
        printf("Memory to class allocated\n");
        module->class_array = _class;
        }
    else
        printf("CANNOT ALLOCATE MEMORY TO CLASS ARRAY\n");*/
    }