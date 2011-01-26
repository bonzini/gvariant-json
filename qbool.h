/*
 * QBool Module
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

#ifndef QBOOL_H
#define QBOOL_H

#include <stdint.h>
#include "qobject.h"

#define qbool_from_int(x)	g_variant_new_boolean(x)
#define qbool_get_int(x)	g_variant_get_boolean(x)

#endif /* QBOOL_H */
