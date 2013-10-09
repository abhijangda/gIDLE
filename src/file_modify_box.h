#include <gtk/gtk.h>

#include "codewidget.h"

#ifndef FILE_MODIFY_BOX_H
#define FILE_MODIFY_BOX_H

#define FILE_MODIFY_BOX_TYPE (file_modify_box_get_type ())
#define FILE_MODIFY_BOX(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), FILE_MODIFY_BOX_TYPE, FileModifyBox))

#define FILE_MODIFY_BOX_CLASS(klass) (G_TYPE_CHECK_INSTANCE_CAST ((klass), FILE_MODIFY_BOX_TYPE, FileModifyBoxClass))

#define IS_FILE_MODIFY_BOX(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), FILE_MODIFY_BOX_TYPE)

#define IS_FILE_MODIFY_BOX_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), FILE_MODIFY_BOX_TYPE))

enum _FileModifyBoxResponse
{
    FILE_MODIFY_BOX_RESPONSE_YES,
    FILE_MODIFY_BOX_RESPONSE_NO
};

typedef enum _FileModifyBoxResponse FileModifyBoxResponse;

struct file_modify_box
{
    GtkBox box;
};

struct _file_modify_box_class
{
    GtkBoxClass parent_class;
    void (*clicked) (GtkWidget *, FileModifyBoxResponse response, gpointer data);
};

typedef struct file_modify_box FileModifyBox;
typedef struct _file_modify_box_class FileModifyBoxClass;

GType  file_modify_box_get_type (void) G_GNUC_CONST;
GtkWidget * file_modify_box_new (void);
#endif /*FILE_MODIFY_BOX_H*/