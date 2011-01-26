/*
 * QFloat Module
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

#ifndef QFLOAT_H
#define QFLOAT_H

#include <stdint.h>
#include "qobject.h"

typedef GVariant QFloat;

#define qfloat_from_double(x)	g_variant_new_double(x)
#define qfloat_get_double(x)	g_variant_get_double(x)
#define qobject_to_qfloat(x)	(g_variant_get_double(x), (x))

#endif /* QFLOAT_H */
