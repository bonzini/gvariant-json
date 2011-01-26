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

#include "json-lexer.h"
#include "json-streamer.h"

static void json_message_process_token(JSONLexer *lexer, GString *token, JSONTokenType type, int x, int y)
{
    JSONMessageParser *parser = container_of(lexer, JSONMessageParser, lexer);
    JSONToken *json_token;

    if (type == JSON_OPERATOR) {
        switch (token->str[0]) {
        case '{':
            parser->brace_count++;
            break;
        case '}':
            parser->brace_count--;
            break;
        case '[':
            parser->bracket_count++;
            break;
        case ']':
            parser->bracket_count--;
            break;
        default:
            break;
        }
    }

    json_token = g_slice_new(JSONToken);
    json_token->type = type;
    json_token->str = g_strdup (token->str);
    json_token->x = x;
    json_token->y = y;

    if (!parser->tokens) {
        parser->tokens = g_queue_new();
    }
    g_queue_push_tail (parser->tokens, json_token);
    if (parser->brace_count == 0 && parser->bracket_count == 0) {
        parser->emit(parser, parser->tokens);
        g_queue_free(parser->tokens);
        parser->tokens = NULL;
    }
}

void json_message_parser_init(JSONMessageParser *parser,
                              void (*func)(JSONMessageParser *, GQueue *))
{
    parser->emit = func;
    parser->brace_count = 0;
    parser->bracket_count = 0;
    parser->tokens = NULL;

    json_lexer_init(&parser->lexer, json_message_process_token);
}

int json_message_parser_feed(JSONMessageParser *parser,
                             const char *buffer, size_t size)
{
    return json_lexer_feed(&parser->lexer, buffer, size);
}

int json_message_parser_flush(JSONMessageParser *parser)
{
    return json_lexer_flush(&parser->lexer);
}

void json_message_parser_destroy(JSONMessageParser *parser)
{
    json_lexer_destroy(&parser->lexer);
    if (parser->tokens) {
	g_queue_free(parser->tokens);
    }
}
