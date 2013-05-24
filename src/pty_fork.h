#define _XOPEN_SOURCE 600
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/select.h>
#include <unistd.h>
#include <stdio.h>
#include <glib.h>

#ifndef PTY_FORK_H
#define PTY_FORK_H

#define BUF_SIZE 256
struct child_func_data
{
    int master_fd;
    char *slave_name;
    struct termios *slave_termios;
    GPid pid;
    GIOChannel *channel;
    char **argv;
};

typedef struct child_func_data ChildProcessData;

gboolean
ptyFork (ChildProcessData *python_shell_data, GError **error);

#endif /*PTY_FORK_H*/