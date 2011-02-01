/*
 * signalfd GSource wrapper
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


#ifndef G_SIGNALFD_H
#define G_SIGNALFD_H

#include <glib.h>
#include <signal.h>

#ifdef _WIN32
typedef char sigset_t[4];
#endif

typedef struct _GSignalFDInfo GSignalFDInfo;

struct _GSignalFDInfo {
    guint32  gsi_signo;   /* Signal number */
    gint32   gsi_unused0;
    gint32   gsi_code;    /* Signal code */
    guint32  gsi_unused1[2];
    gint32   gsi_fd;      /* File descriptor (SIGIO) */
    guint32  gsi_unused2[4];
    gint32   gsi_status;  /* Exit status or signal (SIGCHLD) */
    gint32   gsi_unused3;
    gpointer gsi_unused4;
    gint64   gsi_unused5[2];
    gpointer gsi_addr;    /* Address that generated signal
                             (for hardware-generated signals) */
};

typedef void (*GSignalFDSourceFunc) (gpointer	          user_data,
                                     GSignalFDInfo       *info);

GSource *g_signalfd_source_new      (sigset_t		 *set);
guint    g_signalfd_add		    (sigset_t	         *set,
		                     GSignalFDSourceFunc  func,
                                     gpointer	          user_data);

guint    g_signalfd_add_full	    (gint		  priority,
				     sigset_t	         *set,
                                     GSignalFDSourceFunc  func,
                                     gpointer	          user_data,
                                     GDestroyNotify       notify);

#endif
