/*
 * Auxiliary functions for GVariants, from GLib 2.28
 *
 * Copyright (C) 2011 the GLib team
 *
 * This work is licensed under the terms of the GNU LGPL, version 2.1 or later.
 * See the COPYING.LIB file in the top-level directory.
 */

#include <glib.h>
#include <string.h>
#include <stdlib.h>
#include "qint.h"
#include "qfloat.h"
#include "qdict.h"
#include "qbool.h"
#include "qstring.h"
#include "qobject.h"

#if GLIB_MAJOR_VERSION == 2 && GLIB_MINOR_VERSION <= 26
gboolean
g_variant_lookup (GVariant    *dictionary,
                  const gchar *key,
                  const gchar *format_string,
                  ...)
{
  GVariantType *type;
  GVariant *value;

  /* flatten */
  g_variant_get_data (dictionary);

  type = g_variant_type_new (format_string);
  value = g_variant_lookup_value (dictionary, key, type);
  g_variant_type_free (type);

  if (value)
    {
      va_list ap;

      va_start (ap, format_string);
      g_variant_get_va (value, format_string, NULL, &ap);
      g_variant_unref (value);
      va_end (ap);

      return TRUE;
    }

  else
    return FALSE;
}

GVariant *
g_variant_lookup_value (GVariant           *dictionary,
                        const gchar        *key,
                        const GVariantType *expected_type)
{
  GVariantIter iter;
  GVariant *entry;
  GVariant *value;

  g_return_val_if_fail (g_variant_is_of_type (dictionary,
                                              G_VARIANT_TYPE ("a{s*}")) ||
                        g_variant_is_of_type (dictionary,
                                              G_VARIANT_TYPE ("a{o*}")),
                        NULL);

  g_variant_iter_init (&iter, dictionary);

  while ((entry = g_variant_iter_next_value (&iter)))
    {
      GVariant *entry_key;
      gboolean matches;

      entry_key = g_variant_get_child_value (entry, 0);
      matches = strcmp (g_variant_get_string (entry_key, NULL), key) == 0;
      g_variant_unref (entry_key);

      if (matches)
        break;

      g_variant_unref (entry);
    }

  if (entry == NULL)
    return NULL;

  value = g_variant_get_child_value (entry, 1);
  g_variant_unref (entry);

  if (g_variant_is_of_type (value, G_VARIANT_TYPE_VARIANT))
    {
      GVariant *tmp;

      tmp = g_variant_get_variant (value);
      g_variant_unref (value);

      if (expected_type && !g_variant_is_of_type (tmp, expected_type))
        {
          g_variant_unref (tmp);
          tmp = NULL;
        }

      value = tmp;
    }

  g_return_val_if_fail (expected_type == NULL || value == NULL ||
                        g_variant_is_of_type (value, expected_type), NULL);

  return value;
}
#endif
