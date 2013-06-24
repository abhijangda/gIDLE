#include "pty_fork.h"
#include "main.h"

/*To setup child process
 * before executing python shell
 */
static void
child_func (gpointer data)
{
    ChildProcessData *cf_data = (ChildProcessData*)data;

    setsid();
    close (cf_data->master_fd);
    int slaveFd = open (cf_data->slave_name, O_RDWR | O_NONBLOCK);

    ioctl(slaveFd, TIOCSCTTY, 0);

    cf_data->slave_termios->c_lflag &= ~ECHO;
    cf_data->slave_termios->c_lflag |= ECHOE;
    cf_data->slave_termios->c_lflag |= ECHOK;
    //slave_termios->c_lflag |= ECHOKE;
    //slave_termios->c_lflag |= ECHOCTL;
    cf_data->slave_termios->c_lflag |= ICANON;        
    cf_data->slave_termios->c_lflag |= ISIG;        
    cf_data->slave_termios->c_lflag |= IEXTEN;        
    cf_data->slave_termios->c_cflag |= HUPCL;
    cf_data->slave_termios->c_iflag |= BRKINT;
    cf_data->slave_termios->c_iflag |= ICRNL;
    cf_data->slave_termios->c_iflag |= IXON;
    cf_data->slave_termios->c_iflag |= IXANY;
    cf_data->slave_termios->c_iflag |= IUTF8;
    cf_data->slave_termios->c_iflag |= IMAXBEL;
    cf_data->slave_termios->c_oflag |= OPOST;
    cf_data->slave_termios->c_oflag |= ONLCR;
    cfsetspeed (cf_data->slave_termios, B38400);        
    cf_data->slave_termios->c_cc[VMIN]=1;
    if (tcsetattr (slaveFd, TCSANOW, cf_data->slave_termios) == -1);
    
    dup2 (slaveFd, 0);        
    dup2 (slaveFd, 1);        
    dup2 (slaveFd, 2);
        
    if (slaveFd > 2)
        close (slaveFd); 
}

/* To open and start new
 * python shell process
 */
gboolean
ptyFork (ChildProcessData *python_shell_data, GError **error)
{
    int mfd, slaveFd, savedErrno;

    mfd = posix_openpt (O_RDWR | O_NOCTTY | O_NONBLOCK);     
    grantpt (mfd);
    unlockpt (mfd);

    python_shell_data->master_fd = mfd;
    python_shell_data->slave_name = g_strdup (ptsname (mfd));
    python_shell_data->sh_argv = g_malloc0 (sizeof (gchar *));
    python_shell_data->sh_argv[0] = g_strdup ("/bin/sh");

    /*if (!g_spawn_async (python_shell_data->current_dir, python_shell_data->argv, NULL, 0, 
                       child_func, (gpointer)python_shell_data, 
                       &(python_shell_data->pid), error))

        return FALSE;*/

    pid_t childPid = fork ();
    if (childPid == 0)
    {
        child_func ((gpointer)python_shell_data);
        execv (python_shell_data->sh_argv [0], python_shell_data->sh_argv);
    }
    python_shell_data->pid = childPid;
    return TRUE;
}