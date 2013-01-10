//76 Lines
#include"pyclass.h"
#include<stdlib.h>
#include<string.h>
#include<stdio.h>

void pyclass_insert_function(pyclass *class, char *function_name, int pos, int index)
{    
    class->function_array_size++;
    class->function_array = (pydef *)realloc(class->function_array,(class->function_array_size+1)*sizeof(pydef)); 
    
    int a;   
    for(a=class->function_array_size-1;a>index;a--)                    
    {        
        asprintf(&class->function_array[a].func_name,"%s",class->function_array[a-1].func_name);
        class->function_array[a].pos = class->function_array[a-1].pos;
        }                        
    asprintf(&class->function_array[index+1],"%s",function_name);                    
    class->function_array[index+1].pos=pos;
    }        

void pyclass_init(pyclass *class, char *class_name, int pos, int indentation)
{    
    asprintf(&class->class_name,"%s",class_name);
    //printf(" ADDING WITH INDENTATION %d\n",indentation);
    class->pos = pos;
    class->indentation = indentation;
    class->function_array_size = 0;
    class->self_array_size = 0;
    class->function_array = (pydef *)malloc(1*sizeof(pydef));
    class->self_array = (char **)malloc(1*sizeof(char *));
    //printf("CLASS%sADDED\n",class_name);
    }

void pyclass_add_function(pyclass *class, char *func_name, int pos)
{
    class->function_array[class->function_array_size].pos = pos;
    asprintf(&class->function_array[class->function_array_size].func_name,"%s",func_name);
    class->function_array_size++;
    class->function_array=(pydef *)realloc(class->function_array,(class->function_array_size+1)*sizeof(pydef));    
    /*int i=-1;
    while(++i<class->function_array_size)
    {
        printf("FUN %s\n",class->function_array[i]);
        }*/
    }

void pyclass_add_self(pyclass *class, char *self_name)
{
    asprintf(&class->self_array[class->self_array_size],"%s",self_name);
    class->self_array_size++;
    class->self_array=(char **)realloc(class->self_array,(class->self_array_size+1)*sizeof(char *));
    }

void pyclass_remove_function(pyclass *class, char *function_name)
{
    int i= -1;
    while(++i<class->function_array_size)
    {        
        if(strcmp(class->function_array[i].func_name,function_name))
            break;
        }
    if(i!=class->function_array_size)
    {
        int j = i;
        while(++j<class->function_array_size)
        {            
            if (j+1<class->function_array_size)
            {
                asprintf(&class->function_array[j],"%s",class->function_array[j+1]);
                class->function_array[j].pos=class->function_array[j+1].pos;
                }
            else
                class->function_array_size--;
            }
        }
    }