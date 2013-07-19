#include <stdlib.h>
#include <glib.h>
#include <glib/gprintf.h>

#include "py_variable.h"

void
py_variable_init (PyVariable *py_var, char *name, PyType type)
{
    py_var->name = g_strdup(name);
    py_var->get_definition = NULL;
    py_var->doc_string = NULL;
    py_var->type = type;
}

void
py_variablev_add_variable (PyVariable ***py_variablev, int *size, PyVariable *variable)
{
    (*py_variablev) = g_realloc ((*py_variablev),
                                           (*size + 1) * sizeof (PyVariable *));
    (*py_variablev) [*size] = variable;
    (*size)++;
}

void
py_variable_set_doc_string (PyVariable *py_var, gchar *doc_string)
{
    if (py_var->doc_string)
        g_free (py_var->doc_string);
    
    if (!doc_string)
        return;

    gchar *pos = g_strrstr (doc_string, "'''");
    if (!pos)
        pos = g_strrstr (doc_string, "\"\"\"");
    
    if (pos)
        doc_string [pos - doc_string] = '\0';

    py_var->doc_string = g_strdup (doc_string);
}

void
py_variable_destroy (PyVariable *py_var)
{
    g_free (py_var->name);
    g_free (py_var->doc_string);
    py_var->name = NULL;
    py_var->doc_string = NULL;
}