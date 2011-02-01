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


#ifndef G_EVENTFD_H
#define G_EVENTFD_H

#include <glib.h>

struct _GEventSource;
typedef struct _GEventSource GEventSource;

GEventSource *g_event_source_new      (void);

void     g_event_source_notify   (GEventSource  *source);
void     g_event_source_unref    (GEventSource  *source);

guint    g_event_add		 (GEventSource **source,
		                  GSourceFunc    func,
                                  gpointer	 user_data);

guint    g_event_add_full	 (gint		  priority,
				  GEventSource **source,
		                  GSourceFunc    func,
                                  gpointer	 user_data,
                                  GDestroyNotify notify);

#endif
