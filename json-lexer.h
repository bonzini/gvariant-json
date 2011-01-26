/*
 * JSON lexer
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

#ifndef QEMU_JSON_LEXER_H
#define QEMU_JSON_LEXER_H

#include <glib.h>
#include <errno.h>
#include <stddef.h>

#ifndef offsetof
#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *) 0)->MEMBER)
#endif

#ifndef container_of
#define container_of(ptr, type, member) ({                      \
        const typeof(((type *) 0)->member) *__mptr = (ptr);     \
        (type *) ((char *) __mptr - offsetof(type, member));})
#endif

typedef enum json_token_type {
    JSON_OPERATOR = 100,
    JSON_INTEGER,
    JSON_FLOAT,
    JSON_KEYWORD,
    JSON_STRING,
    JSON_ESCAPE,
    JSON_SKIP,
} JSONTokenType;

typedef struct JSONToken
{
    int type;
    char *str;
    int x;
    int y;
} JSONToken;

typedef struct JSONLexer JSONLexer;

typedef void (JSONLexerEmitter)(JSONLexer *, GString *, JSONTokenType, int x, int y);

struct JSONLexer
{
    JSONLexerEmitter *emit;
    int state;
    GString *token;
    int x, y;
};

void json_lexer_init(JSONLexer *lexer, JSONLexerEmitter func);

int json_lexer_feed(JSONLexer *lexer, const char *buffer, size_t size);

int json_lexer_flush(JSONLexer *lexer);

void json_lexer_destroy(JSONLexer *lexer);

#endif
