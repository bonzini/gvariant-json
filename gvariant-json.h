/*
 * GVariant JSON integration
 *
 * Copyright IBM, Corp. 2009
 *
 * Authors:
 *  Anthony Liguori   <aliguori@us.ibm.com>
 *
 * This work is licensed under the terms of the GNU LGPL, version 2.1 or later.
 * See the COPYING.LIB file in the top-level directory.
 *
 */

#ifndef QJSON_H
#define QJSON_H

#include <stdarg.h>
#include "gvariant-utils.h"

#define GCC_FMT_ATTR(a,b)
GVariant *g_variant_from_json(const char *string) GCC_FMT_ATTR(1, 0);
GVariant *g_variant_from_jsonf(const char *string, ...) GCC_FMT_ATTR(1, 2);
GVariant *g_variant_from_jsonv(const char *string, va_list *ap) GCC_FMT_ATTR(1, 0);

char *g_variant_to_json(GVariant *obj);
char *g_variant_to_json_pretty(GVariant *obj);

#endif /* QJSON_H */
