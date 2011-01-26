/*
 * QString Module
 *
 * Copyright (C) 2009 Red Hat Inc.
 *
 * Authors:
 *  Luiz Capitulino <lcapitulino@redhat.com>
 *
 * This work is licensed under the terms of the GNU LGPL, version 2.1 or later.
 * See the COPYING.LIB file in the top-level directory.
 */

#ifndef QSTRING_H
#define QSTRING_H

#include <stdint.h>
#include "qobject.h"

typedef GVariant QString;

#define qstring_from_str(str)	  g_variant_new_string(str)
#define qstring_get_str(str)	  g_variant_get_string(str, NULL)
#define qobject_to_qstring(str)	  (g_variant_get_string(str, NULL), (str))

#endif /* QSTRING_H */
