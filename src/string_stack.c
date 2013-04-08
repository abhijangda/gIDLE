//33 Lines

#include"string_stack.h"
#include<stdlib.h>
#include<stdio.h>

void string_stack_push(string_stack *stack, char *string)
{   
    if(stack->text_array==NULL || stack->count==0)
    {
        stack->count=0;
        stack->text_array=(char **)malloc(1*sizeof(char *));
        }
    else
        stack->text_array=(char**)realloc(stack->text_array,(stack->count +1)*sizeof(char *));
    
    asprintf(&stack->text_array[stack->count],"%s",string);
    printf("COUNT %d\n",stack->count);
    stack->count++;    
    printf("dddddd%s\n",stack->text_array[stack->count-1]);
    }

int string_stack_pop(string_stack *stack)
{
    if(stack->count < 0)
       return 0;
   
    char *string;      
    asprintf(&string,"%s",stack->text_array[stack->count-1]);
    
    free(stack->text_array[stack->count-1]);
    stack->count--;
                        
    return 1;
    }