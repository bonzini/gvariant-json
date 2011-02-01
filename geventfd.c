/*
 * eventfd GSource wrapper
 *
 * Copyright Red Hat, Inc. 2011
 *
 * Authors:
 *  Paolo Bonzini     <pbonzini@redhat.com>
 *
 * This work is licensed under the terms of the GNU LGPL, version 2.1 or later.
 * See the COPYING.LIB file in the top-level directory.
 *
 */


#include "geventfd.h"
#include <glib-object.h>

//#define HAVE_EVENTFD

#ifdef _WIN32
#include <windows.h>
#else
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>

#ifdef HAVE_EVENTFD
#include <sys/eventfd.h>
#endif
#endif


struct _GEventSource {
    GSource	source;
    GPollFD	poll;
#ifndef _WIN32
    int		wfd;
#endif
};

static gboolean
g_event_prepare (GSource *source,
		 gint    *timeout)
{
    *timeout = -1;

    return FALSE;
}

static gboolean
g_event_check (GSource *source)
{
    GEventSource *evsource = (GEventSource *) source;

    return (evsource->poll.revents & G_IO_IN) != 0;
}

static gboolean
g_event_dispatch (GSource	 *source,
                     GSourceFunc  callback,
                     gpointer     user_data)
{
#ifndef _WIN32
    GEventSource *evsource = (GEventSource *) source;
    gboolean fire = FALSE;
    char buf[512];
    int rc;
    do {
        rc = read(evsource->poll.fd, buf, sizeof(buf));
        fire |= (rc > 0);
    } while (rc == sizeof (buf) || (rc == -1 && errno == EINTR));
    if (rc < 0 && errno != EAGAIN) {
        g_warning ("read from eventfd returned %zd: %m\n", rc);
        return TRUE;
    }

    if (!fire)
        return FALSE;
#endif

    if (callback) {
        if (user_data == NULL)
            user_data = evsource;
        callback (user_data);
    } else {
        g_warning ("Event source dispatched without callback\n"
                   "You must call g_source_set_callback().");
    }

    return FALSE;
}

static void
g_event_finalize (GSource *source)
{
#ifndef _WIN32
    GEventSource *evsource = (GEventSource *) source;
    close (evsource->poll.fd);
#endif
}

static void
g_event_closure_callback (gpointer data)
{
    GClosure *closure = data;
    g_closure_invoke (closure, NULL, 0, NULL, NULL);
}

static GSourceFuncs event_source_funcs = {
    g_event_prepare,
    g_event_check,
    g_event_dispatch,
    g_event_finalize,
    (GSourceFunc) g_event_closure_callback,
    (gpointer) g_cclosure_marshal_VOID__VOID
};

GEventSource *
g_event_source_new (void)
{
    GEventSource *evsource;
#ifdef _WIN32
    HANDLE hEvent;

    hEvent = CreateEvent (NULL, FALSE, FALSE, NULL);
    if (hEvent == NULL) {
        return NULL;
    }

#else
    int rfd, wfd;

#if defined(HAVE_EVENTFD)
    rfd = wfd = eventfd(0, 0);
#else
    int fds[2];
    if (pipe (fds) == -1) {
        return NULL;
    }
    rfd = fds[0];
    wfd = fds[1];
    fcntl(wfd, F_SETFD, fcntl (wfd, F_GETFD) | FD_CLOEXEC);
    fcntl(wfd, F_SETFL, fcntl (wfd, F_GETFL) | O_NONBLOCK);
#endif
    if (rfd == -1) {
        return NULL;
    }

    fcntl(rfd, F_SETFL, fcntl (rfd, F_GETFL) | O_NONBLOCK);
    fcntl(rfd, F_SETFD, fcntl (rfd, F_GETFD) | FD_CLOEXEC);
#endif

    evsource = (GEventSource *) g_source_new (&event_source_funcs,
                                              sizeof (GEventSource));

#ifdef _WIN32
    evsource->poll.fd = (LONG_PTR) hEvent;
#else
    evsource->wfd = wfd;
    evsource->poll.fd = rfd;
#endif
    evsource->poll.events = G_IO_IN;
    g_source_add_poll (&evsource->source, &evsource->poll);
    return evsource;
}

void
g_event_source_notify (GEventSource *evsource)
{
#ifdef _WIN32
    SetEvent ((HANDLE) evsource->poll.fd);
#else
    guint64 buf = 1;
    write (evsource->wfd, (char *) &buf, sizeof (buf));
#endif
}

void
g_event_source_unref (GEventSource *evsource)
{
    g_source_unref (&evsource->source);
}

guint
g_event_add (GEventSource **source,
             GSourceFunc    func,
             gpointer	    user_data)
{
    return g_event_add_full (G_PRIORITY_DEFAULT, source, func, user_data, NULL);
}

guint
g_event_add_full (gint		    priority,
                  GEventSource    **source,
                  GSourceFunc       func,
                  gpointer	    user_data,
                  GDestroyNotify    notify)
{
    GEventSource *evsource;
    guint id;

    evsource = g_event_source_new ();
    if (priority != G_PRIORITY_DEFAULT)
        g_source_set_priority (&evsource->source, priority);

    g_source_set_callback (&evsource->source, (GSourceFunc) func,
                           user_data, notify);
    id = g_source_attach (&evsource->source, NULL);

    *source = evsource;
    return id;
}

#ifdef DEMO
#include <stdio.h>

gpointer wait_and_notify (gpointer user_data)
{
    GEventSource *evsource = user_data;

    int i;
    for (i = 0; i < 3; i++) {
        printf (".");
        fflush (stdout);
        g_usleep (1000000);
    }
    printf ("\n");
    fflush (stdout);
    g_event_source_notify (evsource);
    g_source_unref ((GSource *) evsource);
    return NULL;
}

int main()
{
    GEventSource *evsource;
    g_thread_init (NULL);

    GMainLoop *loop = g_main_loop_new (NULL, FALSE);
    g_event_add (&evsource, (GSourceFunc) g_main_loop_quit, loop);
    if (g_thread_create (wait_and_notify, evsource, FALSE, NULL) != NULL) {
        g_main_loop_run (loop);
    }
}
#endif
