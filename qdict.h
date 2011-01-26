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
#define g_variant_do_lookup(dict, key, type, c_type) ({ \
  c_type _val = (c_type) 0; g_variant_lookup(dict, key, type, &_val); _val; })

static inline bool g_variant_has_key(GVariant *dict, const char *key)
{
    GVariant *value;
    value = g_variant_lookup_value(dict, key, NULL);
    if (value)
        g_variant_unref(value);
    return value != NULL;
}

#define g_variant_lookup_boolean(dict, key) \
        g_variant_do_lookup(dict, key, "b", bool)
#define g_variant_lookup_array(dict, key) \
        g_variant_do_lookup(dict, key, "av", GVariant *)
#define g_variant_lookup_dictionary(dict, key) \
        g_variant_do_lookup(dict, key, "a{sv}", GVariant *)

static inline void g_variant_dictionary_iterate(GVariant *dict,
                 void (*iter)(const char *key, GVariant *value, void *opaque),
                 void *opaque)
{
    GVariantIter _iter;
    const char *key;
    GVariant *value;
    g_variant_iter_init(&_iter, dict);
    while (g_variant_iter_loop (&_iter, "{sv}", &key, &value)) {
        iter (key, value, opaque);
    }
}

/* High level helpers */
static inline double g_variant_lookup_double(GVariant *dict, const char *key)
{
    GVariant *value = g_variant_lookup_value (dict, key, NULL);
    double result;
    if (g_variant_is_of_type (value, G_VARIANT_TYPE_DOUBLE))
        result = g_variant_get_double (value);
    else
        result = g_variant_get_int64 (value);
    g_variant_unref(value);
    return result;
}

static inline int64_t g_variant_lookup_int64_default(GVariant *dict, const char *key,
                                        int64_t def_value)
{
    GVariant *value = g_variant_lookup_value (dict, key, NULL);
    int64_t result;
    if (g_variant_is_of_type (value, G_VARIANT_TYPE_INT64))
        result = g_variant_get_int64 (value);
    else
        result = def_value;

    g_variant_unref(value);
    return def_value;
}

static inline int g_variant_lookup_boolean_default(GVariant *dict, const char *key, int def_value)
{
    GVariant *value = g_variant_lookup_value (dict, key, NULL);
    int result;
    if (g_variant_is_of_type (value, G_VARIANT_TYPE_BOOLEAN))
        result = g_variant_get_boolean (value);
    else
        result = def_value;

    g_variant_unref(value);
    return def_value;
}

static inline
const char *g_variant_lookup_string(GVariant *dict, const char *key)
{
    GVariant *value = g_variant_lookup_value (dict, key, NULL);
    const char *result;
    if (g_variant_is_of_type (value, G_VARIANT_TYPE_STRING))
        result = g_variant_get_string (value, NULL);
    else
        result = NULL;

    g_variant_unref(value);
    return result;
}

#endif /* QDICT_H */
