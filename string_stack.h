//10 Lines
#ifndef __STRING_STACK_H__
#define __STRING_STACK_H__

struct _string_stack
{
    char **text_array;
    int count;
    };

typedef struct _string_stack string_stack;

void string_stack_push(string_stack *stack,char *string);
int string_stack_pop(string_stack *stack);
#endif