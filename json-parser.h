/*
 * JSON Parser 
 *
 * Copyright IBM, Corp. 2009
 * Copyright Red Hat, Inc. 2011
 *
 * Authors:
 *  Anthony Liguori   <aliguori@us.ibm.com>
 *  Paolo Bonizni     <pbonzini@redhat.com>
 *
 * This work is licensed under the terms of the GNU LGPL, version 2.1 or later.
 * See the COPYING.LIB file in the top-level directory.
 *
 */

#ifndef QEMU_JSON_PARSER_H
#define QEMU_JSON_PARSER_H

#include <glib.h>

GVariant *json_parser_parse(GQueue *tokens, va_list *ap);

#endif
