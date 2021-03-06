/*
 * JSON streaming support
 *
 * Copyright IBM, Corp. 2009
 * Copyright Red Hat, Inc. 2011
 *
 * Authors:
 *  Anthony Liguori   <aliguori@us.ibm.com>
 *  Paolo Bonzini     <pbonzini@redhat.com>
 *
 * This work is licensed under the terms of the GNU LGPL, version 2.1 or later.
 * See the COPYING.LIB file in the top-level directory.
 *
 */

#ifndef QEMU_JSON_STREAMER_H
#define QEMU_JSON_STREAMER_H

#include "json-lexer.h"

typedef struct JSONMessageParser
{
    void (*emit)(struct JSONMessageParser *parser, GQueue *tokens);
    JSONLexer lexer;
    int brace_count;
    int bracket_count;
    GQueue *tokens;
} JSONMessageParser;

void json_message_parser_init(JSONMessageParser *parser,
                              void (*func)(JSONMessageParser *, GQueue *));

int json_message_parser_feed(JSONMessageParser *parser,
                             const char *buffer, size_t size);

int json_message_parser_flush(JSONMessageParser *parser);

void json_message_parser_destroy(JSONMessageParser *parser);

#endif
