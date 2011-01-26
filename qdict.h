/*
 * QDict Module
 *
 * Copyright (C) 2009 Red Hat Inc.
 *
 * Authors:
 *  Luiz Capitulino <lcapitulino@redhat.com>
 *
 * This work is licensed under the terms of the GNU LGPL, version 2.1 or later.
 * See the COPYING.LIB file in the top-level directory.
 */

#ifndef QDICT_H
#define QDICT_H

#include "qobject.h"
#include "qlist.h"
#include <stdint.h>
#include <stdbool.h>

#if GLIB_MAJOR_VERSION == 2 && GLIB_MINOR_VERSION <= 26
extern
gboolean
g_variant_lookup (GVariant    *dictionary,
                  const gchar *key,
                  const gchar *format_string,
                  ...);

extern 
GVariant *
g_variant_lookup_value (GVariant           *dictionary,
                        const gchar        *key,
                        const GVariantType *expected_type);
#endif

/* Object API */
#define qdict_size(qdict) \
        g_variant_get_n_children(qdict)

#define qdict_do_variant_lookup(qdict, key, type, c_type) ({ \
  c_type _val = (c_type) 0; g_variant_lookup(qdict, key, type, &_val); _val; })

static inline bool qdict_haskey(GVariant *qdict, const char *key)
{
    GVariant *value;
    value = g_variant_lookup_value(qdict, key, NULL);
    if (value)
        g_variant_unref(value);
    return value != NULL;
}

#define qdict_get_bool(qdict, key) \
        qdict_do_variant_lookup(qdict, key, "b", bool)
#define qdict_get_str(qdict, key) \
        qdict_do_variant_lookup(qdict, key, "s", const char *)
#define qdict_get_qlist(qdict, key) \
        qdict_do_variant_lookup(qdict, key, "av", GVariant *)

#define qdict_get_qdict(qdict, key) \
        qdict_do_variant_lookup(qdict, key, "a{sv}", GVariant *)

/* Watch out, different reference semantics!  */
#define qdict_get(qdict, key) \
        g_variant_lookup_value(qdict, key, NULL)

static inline void qdict_iter(GVariant *qdict,
                 void (*iter)(const char *key, QObject *value, void *opaque),
                 void *opaque)
{
    GVariantIter _iter;
    const char *key;
    GVariant *value;
    g_variant_iter_init(&_iter, qdict);
    while (g_variant_iter_loop (&_iter, "{sv}", &key, &value)) {
        iter (key, value, opaque);
    }
}

/* High level helpers */
static inline double qdict_get_double(GVariant *qdict, const char *key)
{
    GVariant *value = g_variant_lookup_value (qdict, key, NULL);
    double result;
    if (g_variant_is_of_type (value, G_VARIANT_TYPE_DOUBLE))
        result = g_variant_get_double (value);
    else
        result = g_variant_get_int64 (value);
    g_variant_unref(value);
    return result;
}

static inline int64_t qdict_get_try_int(GVariant *qdict, const char *key,
                                        int64_t def_value)
{
    GVariant *value = g_variant_lookup_value (qdict, key, NULL);
    int64_t result;
    if (g_variant_is_of_type (value, G_VARIANT_TYPE_INT64))
        result = g_variant_get_int64 (value);
    else
        result = def_value;

    g_variant_unref(value);
    return def_value;
}

static inline int qdict_get_try_bool(GVariant *qdict, const char *key, int def_value)
{
    GVariant *value = g_variant_lookup_value (qdict, key, NULL);
    int result;
    if (g_variant_is_of_type (value, G_VARIANT_TYPE_BOOLEAN))
        result = g_variant_get_boolean (value);
    else
        result = def_value;

    g_variant_unref(value);
    return def_value;
}

static inline
const char *qdict_get_try_str(GVariant *qdict, const char *key)
{
    GVariant *value = g_variant_lookup_value (qdict, key, NULL);
    const char *result;
    if (g_variant_is_of_type (value, G_VARIANT_TYPE_STRING))
        result = g_variant_get_string (value, NULL);
    else
        result = NULL;

    g_variant_unref(value);
    return result;
}

#endif /* QDICT_H */
