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
 *
 * QObject Reference Counts Terminology
 * ------------------------------------
 *
 *  - Returning references: A function that returns an object may
 *  return it as either a weak or a strong reference.  If the reference
 *  is strong, you are responsible for calling QDECREF() on the reference
 *  when you are done.
 *
 *  If the reference is weak, the owner of the reference may free it at
 *  any time in the future.  Before storing the reference anywhere, you
 *  should call QINCREF() to make the reference strong.
 *
 *  - Transferring ownership: when you transfer ownership of a reference
 *  by calling a function, you are no longer responsible for calling
 *  QDECREF() when the reference is no longer needed.  In other words,
 *  when the function returns you must behave as if the reference to the
 *  passed object was weak.
 */
#ifndef QOBJECT_H
#define QOBJECT_H

#include <stddef.h>
#include <assert.h>
#include <glib.h>

typedef GVariant QObject;

/* Get the 'base' part of an object */
#define QOBJECT(obj) (obj)

#define qobject_incref(obj)      g_variant_ref(obj)
#define qobject_decref(obj)      g_variant_unref(obj)

/* High-level interface for qobject_incref() */
#define QINCREF(obj)      g_variant_ref(obj)

/* High-level interface for qobject_decref() */
#define QDECREF(obj)      g_variant_unref(obj)

#endif /* QOBJECT_H */
