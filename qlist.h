/*
 * QList Module
 *
 * Copyright (C) 2009 Red Hat Inc.
 *
 * Authors:
 *  Luiz Capitulino <lcapitulino@redhat.com>
 *
 * This work is licensed under the terms of the GNU LGPL, version 2.1 or later.
 * See the COPYING.LIB file in the top-level directory.
 */

#ifndef QLIST_H
#define QLIST_H

#include "qobject.h"

#define qlist_empty(qlist)	(g_variant_n_children(qlist) == 0)

static inline void qlist_iter(GVariant *qlist,
                 void (*iter)(GVariant *obj, void *opaque), void *opaque)
{
    GVariant *var;
    GVariantIter _iter;

    g_variant_iter_init(&_iter, qlist);
    while (g_variant_iter_loop (&_iter, "v", &var))
      iter (var, opaque);
}

#endif /* QLIST_H */
