/*
 * QInt Module
 *
 * Copyright (C) 2009 Red Hat Inc.
 *
 * Authors:
 *  Luiz Capitulino <lcapitulino@redhat.com>
 *
 * This work is licensed under the terms of the GNU LGPL, version 2.1 or later.
 * See the COPYING.LIB file in the top-level directory.
 */

#ifndef QINT_H
#define QINT_H

#include <stdint.h>
#include "qobject.h"

typedef GVariant QInt;

#define qint_from_int(x)	g_variant_new_int64(x)
#define qint_get_int(x)		g_variant_get_int64(x)
#define qobject_to_qint(x)	(g_variant_get_int64(x), (x))

#endif /* QINT_H */
