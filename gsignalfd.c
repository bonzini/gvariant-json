/*
 * signalfd GSource wrapper
 *
 * Copyright IBM, Corp. 2011
 * Copyright Red Hat, Inc. 2011
 *
 * Authors:
 *  Anthony Liguori   <aliguori@us.ibm.com>
 *  Paolo Bonzini     <pbonzini@redhat.com>
 *
 * This work is licensed under the terms of the GNU LGPL, version 2.1 or later.
 * See the COPYING.LIB file in the top-level directory.
 *
 */


#include "gsignalfd.h"
#include <glib-object.h>

//#define HAVE_SIGNALFD

#ifndef _WIN32
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <syscall.h>
#include <fcntl.h>
#include <pthread.h>

#ifdef HAVE_SIGNALFD
struct my_signalfd_siginfo {
    guint32 ssi_signo;   /* Signal number */
    gint32  ssi_errno;   /* Error number (unused) */
    gint32  ssi_code;    /* Signal code */
    guint32 ssi_pid;     /* PID of sender */
    guint32 ssi_uid;     /* Real UID of sender */
    gint32  ssi_fd;      /* File descriptor (SIGIO) */
    guint32 ssi_tid;     /* Kernel timer ID (POSIX timers) */
    guint32 ssi_band;    /* Band event (SIGIO) */
    guint32 ssi_overrun; /* POSIX timer overrun count */
    guint32 ssi_trapno;  /* Trap number that caused signal */
    gint32  ssi_status;  /* Exit status or signal (SIGCHLD) */
    gint32  ssi_int;     /* Integer sent by sigqueue(2) */
    guint64 ssi_ptr;     /* Pointer sent by sigqueue(2) */
    guint64 ssi_utime;   /* User CPU time consumed (SIGCHLD) */
    guint64 ssi_stime;   /* System CPU time consumed (SIGCHLD) */
    guint64 ssi_addr;    /* Address that generated signal
                            (for hardware-generated signals) */
    guint8  pad[48];     /* Pad size to 128 bytes (allow for
                            additional fields in the future) */
};

static void
ssiginfo_to_siginfo (siginfo_t *info, struct my_signalfd_siginfo *ssinfo)
{
    info->si_signo = ssinfo->ssi_signo;
    info->si_errno = 0;
    info->si_code = ssinfo->ssi_code;
    info->si_fd = ssinfo->ssi_fd;
    info->si_status = ssinfo->ssi_status;
    info->si_addr = (gpointer) (uintptr_t) ssinfo->ssi_addr;
}
#else
struct my_signalfd_siginfo {
    siginfo_t info;
};

#define ssi_signo	info.si_signo
#define ssi_code	info.si_code
#define ssi_fd		info.si_fd
#define ssi_status	info.si_status
#define ssi_addr	info.si_addr

static void
ssiginfo_to_siginfo (siginfo_t *info, struct my_signalfd_siginfo *ssinfo)
{
    *info = ssinfo->info;
}

struct sigfd_compat_info
{
    sigset_t mask;
    int fd;
};

static void *sigwait_compat(void *opaque)
{
    struct sigfd_compat_info *info = opaque;
    sigset_t mask = info->mask;
    int fd = info->fd;
    int rc;

    g_free (info);

    do {
        struct my_signalfd_siginfo ssinfo;
	size_t ofs = 0;

        do {
            rc = sigwaitinfo(&mask, &ssinfo.info);
        } while (rc == -1 && errno == EINTR);

        if (rc >= 0) {
            char *buf = (char *) &ssinfo;
            while (ofs < sizeof(ssinfo)) {
                do {
                    rc = write(fd, buf + ofs, sizeof(ssinfo) - ofs);
                } while (rc == -1 && errno == EINTR);
                if (rc == -1) {
                    break;
                }
                ofs += rc;
            }
        }
    } while (rc >= 0);

    close (fd);
    return NULL;
}

static int signalfd_compat(const sigset_t *mask)
{
    pthread_attr_t attr;
    pthread_t tid;
    struct sigfd_compat_info *info;
    int fds[2];
    sigset_t all, old;

    info = g_malloc(sizeof(*info));
    if (info == NULL) {
        errno = ENOMEM;
        return -1;
    }

    if (pipe(fds) == -1) {
        g_free(info);
        return -1;
    }

    memcpy(&info->mask, mask, sizeof(*mask));
    info->fd = fds[1];
    fcntl(fds[0], F_SETFD, fcntl (fds[0], F_GETFD) | FD_CLOEXEC);
    fcntl(fds[1], F_SETFD, fcntl (fds[1], F_GETFD) | FD_CLOEXEC);

    sigfillset(&all);
    sigprocmask(SIG_BLOCK, &all, &old);
    g_thread_create(sigwait_compat, info, FALSE, NULL);
    sigprocmask(SIG_SETMASK, &old, NULL);
    return fds[0];
}
#endif

static void
ssiginfo_to_gsiginfo (GSignalFDInfo *gsinfo, struct my_signalfd_siginfo *ssinfo)
{
    gsinfo->gsi_signo = ssinfo->ssi_signo;
    gsinfo->gsi_code = ssinfo->ssi_code;
    gsinfo->gsi_fd = ssinfo->ssi_fd;
    gsinfo->gsi_status = ssinfo->ssi_status;
    gsinfo->gsi_addr = (gpointer) (uintptr_t) ssinfo->ssi_addr;
}
#endif


struct _GSignalFDSource {
    GSource	source;
    GPollFD	poll;
};

typedef struct _GSignalFDSource GSignalFDSource;

static gboolean
g_signalfd_prepare (GSource *source,
                    gint    *timeout)
{
    *timeout = -1;

    return FALSE;
}

static gboolean
g_signalfd_check (GSource *source)
{
    GSignalFDSource *sigfd = (GSignalFDSource *) source;

    return (sigfd->poll.revents & G_IO_IN) != 0;
}

static gboolean
g_signalfd_dispatch (GSource	 *source,
                     GSourceFunc  callback,
                     gpointer     user_data)
{
#ifndef _WIN32
    GSignalFDSource *sigfd = (GSignalFDSource *) source;

    while (1) {
        struct my_signalfd_siginfo ssinfo;
        struct sigaction action;

	char *buf = (char *) &ssinfo;
        size_t ofs = 0;
        gint rc;
        while (ofs < sizeof(ssinfo)) {
            rc = read(sigfd->poll.fd, buf + ofs, sizeof(ssinfo) - ofs);
            if (rc <= 0) {
                if (errno == EINTR) {
                    continue;
                }
                if (errno == EAGAIN) {
                    break;
                }
                g_critical ("read from signalfd returned %zd: %m\n", rc);
                return FALSE;
            }
            ofs += rc;
        }
	if (ofs == 0) {
	    break;
	}
        if (ofs != sizeof(ssinfo)) {
	    g_critical ("truncated read from signalfd\n");
            return FALSE;
        }

        sigaction(ssinfo.ssi_signo, NULL, &action);
        if (action.sa_handler == SIG_IGN) {
            continue;
        }

        if (action.sa_handler == SIG_DFL) {
            if (callback) {
                GSignalFDInfo gsinfo;
                ssiginfo_to_gsiginfo (&gsinfo, &ssinfo);
                ((GSignalFDSourceFunc) callback) (user_data, &gsinfo);
            } else {
                g_warning ("Signal source dispatched without callback\n"
                           "You must call g_source_set_callback().");
            }
            continue;
        }

        if (action.sa_sigaction) {
            siginfo_t info;
            ssiginfo_to_siginfo (&info, &ssinfo);
            action.sa_sigaction(ssinfo.ssi_signo, &info, NULL);
        } else {
            action.sa_handler(ssinfo.ssi_signo);
        }
    }
#endif

    return TRUE;
}

static void
g_signalfd_finalize (GSource *source)
{
#ifndef _WIN32
    GSignalFDSource *sigfd = (GSignalFDSource *) source;
    close (sigfd->poll.fd);
#endif
}

static void
g_signalfd_closure_callback (gpointer data, GSignalFDInfo *gsi)
{
    GClosure *closure = data;
    GValue param = { 0, };

    g_value_init (&param, G_TYPE_BOXED);
    g_value_set_boxed (&param, gsi);
    g_closure_invoke (closure, NULL, 1, &param, NULL);
}

static GSourceFuncs signalfd_source_funcs = {
    g_signalfd_prepare,
    g_signalfd_check,
    g_signalfd_dispatch,
    g_signalfd_finalize,
    (GSourceFunc) g_signalfd_closure_callback,
    (gpointer) g_cclosure_marshal_VOID__VOID
};

GSource *
g_signalfd_source_new (sigset_t *set)
{
    GSignalFDSource *sigfd;
#ifndef _WIN32
    sigset_t oldmask;
    int fd;

    pthread_sigmask (SIG_BLOCK, set, &oldmask);
#if defined(HAVE_SIGNALFD)
    fd = syscall(SYS_signalfd, -1, set, _NSIG / 8);
    if (fd != -1) {
	fcntl(fd, F_SETFD, fcntl (fd, F_GETFD) | FD_CLOEXEC);
    }
#else
    fd = signalfd_compat(set);
#endif
    if (fd == -1) {
        pthread_sigmask (SIG_SETMASK, &oldmask, NULL);
        return NULL;
    }

    fcntl(fd, F_SETFL, fcntl (fd, F_GETFL) | O_NONBLOCK);
#endif

    sigfd = (GSignalFDSource *) g_source_new (&signalfd_source_funcs,
                                              sizeof (GSignalFDSource));

#ifndef _WIN32
    sigfd->poll.fd = fd;
    sigfd->poll.events = G_IO_IN;
    g_source_add_poll (&sigfd->source, &sigfd->poll);
#endif
    g_source_set_priority (&sigfd->source, G_PRIORITY_HIGH);
    return &sigfd->source;
}

guint
g_signalfd_add (sigset_t	    *set,
                GSignalFDSourceFunc  func,
                gpointer	     user_data)
{
    return g_signalfd_add_full (G_PRIORITY_HIGH, set, func, user_data, NULL);
}

guint
g_signalfd_add_full (gint		  priority,
                     sigset_t	         *set,
                     GSignalFDSourceFunc  func,
                     gpointer	          user_data,
                     GDestroyNotify       notify)
{
    GSource *source;
    guint id;

    source = g_signalfd_source_new (set);
    if (priority != G_PRIORITY_HIGH)
        g_source_set_priority (source, priority);

    g_source_set_callback (source, (GSourceFunc) func, user_data, notify);
    id = g_source_attach (source, NULL);
    g_source_unref(source);

    return id;
}

#ifdef DEMO
void callback (gpointer user_data, GSignalFDInfo *info)
{
    g_printf ("%s\n", strsignal (info->gsi_signo));
    if (info->gsi_signo == SIGTERM) {
        g_main_loop_quit ((GMainLoop *) user_data);
    }
}

int main()
{
    g_thread_init (NULL);
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGINT);
    sigaddset(&mask, SIGTERM);

    g_printf ("My PID is %d\n", getpid ());
    GMainLoop *loop = g_main_loop_new (NULL, FALSE);
    g_signalfd_add (&mask, callback, loop);
    g_main_loop_run (loop);
}
#endif
