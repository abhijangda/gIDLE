#include "file_modify_box.h"

#define FILE_MODIFY_BOX_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), FILE_MODIFY_BOX_TYPE, FileModifyBoxPrivate))

struct _file_modify_box_private
{
    GtkWidget *label;
    GtkWidget *cmdYes;
    GtkWidget *cmdNo;
    GtkWidget *hbox;
};

enum SIGNALS
{
    CLICKED_SIGNAL,
    TOTAL_SIGNALS
};

static int file_modify_box_signals [TOTAL_SIGNALS] = {0};

typedef struct _file_modify_box_private FileModifyBoxPrivate;

static void
file_modify_box_class_init (FileModifyBoxClass *klass);

static void
file_modify_box_init (FileModifyBox *);

static void
cmdyes_clicked (GtkWidget *widget, gpointer data)
{
    g_signal_emit_by_name (data, "clicked", FILE_MODIFY_BOX_RESPONSE_YES);
}

static void
cmdno_clicked (GtkWidget *widget, gpointer data)
{
    g_signal_emit_by_name (data, "clicked", FILE_MODIFY_BOX_RESPONSE_NO);
}


GType
file_modify_box_get_type (void)
{
    static GType box_type = 0;
    
    if (!box_type)
    {
        static const GTypeInfo file_modify_box_type = 
        {
            sizeof (FileModifyBoxClass),
            NULL,
            NULL,
            (GClassInitFunc) file_modify_box_class_init,
            NULL,
            NULL,
            sizeof (FileModifyBox),
            0,
            (GInstanceInitFunc) file_modify_box_init,
        };
        box_type = g_type_register_static (GTK_TYPE_BOX, "FileModifyBox", &file_modify_box_type, 0);
    }
    return box_type;
}

static void
file_modify_box_class_init (FileModifyBoxClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    g_type_class_add_private (klass, sizeof (FileModifyBoxPrivate));
    
    file_modify_box_signals[CLICKED_SIGNAL] =
                g_signal_new ("clicked", G_TYPE_FROM_CLASS (klass),
                              G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
                              G_STRUCT_OFFSET (FileModifyBoxClass, clicked),
                              NULL, NULL, g_cclosure_marshal_VOID__INT, 
                              G_TYPE_NONE, 1, G_TYPE_INT);

}

static void
file_modify_box_init (FileModifyBox *fm_box)
{
    FileModifyBoxPrivate *priv = FILE_MODIFY_BOX_GET_PRIVATE (fm_box);
    priv->label = gtk_label_new ("Current file has been modified outside gIDLE. Do you want to reload file?");
    priv->cmdYes = gtk_button_new_with_label ("Yes");
    priv->cmdNo = gtk_button_new_with_label ("No");
    g_signal_connect (G_OBJECT (priv->cmdYes), "clicked",
                      G_CALLBACK (cmdyes_clicked), fm_box);
    g_signal_connect (G_OBJECT (priv->cmdNo), "clicked",
                      G_CALLBACK (cmdno_clicked), fm_box);

    priv->hbox  = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 2);
    gtk_box_pack_end (GTK_BOX (priv->hbox), priv->cmdNo, FALSE, FALSE, 2);
    gtk_box_pack_end (GTK_BOX (priv->hbox), priv->cmdYes, FALSE, FALSE, 2);
    
    gtk_box_pack_start (GTK_BOX (fm_box), priv->label, FALSE, FALSE, 2);
    gtk_box_pack_start (GTK_BOX (fm_box), priv->hbox, FALSE, FALSE, 2);
}

GtkWidget *
file_modify_box_new ()
{
    GtkWidget *widget = GTK_WIDGET (g_object_new (file_modify_box_get_type (), NULL));
    gtk_orientable_set_orientation (GTK_ORIENTABLE (widget), GTK_ORIENTATION_HORIZONTAL);
    return widget;
}
