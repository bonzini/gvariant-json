/*
 * QEMU Object Model.
 *
 * Based on ideas by Avi Kivity <avi@redhat.com>
 *
 * Copyright (C) 2009 Red Hat Inc.
 *
 * Authors:
 *  Luiz Capitulino <lcapitulino@redhat.com>
 *
 * This work is licensed under the terms of the GNU LGPL, version 2.1 or later.
 * See the COPYING.LIB file in the top-level directory.
 */
#ifndef QOBJECT_H
#define QOBJECT_H

#include <stddef.h>
#include <assert.h>
#include <glib.h>

typedef GVariant QObject;

#define qobject_incref(obj)      g_variant_ref(obj)
#define qobject_decref(obj)      g_variant_unref(obj)

#endif /* QOBJECT_H */
