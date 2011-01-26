/*
 * JSON Parser 
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

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>

#include "qstring.h"
#include "qint.h"
#include "qdict.h"
#include "qlist.h"
#include "qfloat.h"
#include "qbool.h"
#include "json-parser.h"
#include "json-lexer.h"

typedef struct JSONParserContext
{
} JSONParserContext;

#define BUG_ON(cond) assert(!(cond))

/**
 * TODO
 *
 * 0) make errors meaningful again
 * 1) add geometry information to tokens
 * 3) should we return a parsed size?
 * 4) deal with premature EOI
 */

static QObject *parse_value(JSONParserContext *ctxt, GQueue *tokens, va_list *ap);

/**
 * Token manipulators
 *
 * tokens contain a type, a string value, and geometry information
 * about a token identified by the lexer.  These are routines that make working with
 * these objects a bit easier.
 */
static int token_is_operator(JSONToken *obj, char op)
{
    const char *val;

    if (obj->type != JSON_OPERATOR) {
        return 0;
    }

    val = obj->str;

    return (val[0] == op) && (val[1] == 0);
}

static int token_is_keyword(JSONToken *obj, const char *value)
{
    if (obj->type != JSON_KEYWORD) {
        return 0;
    }

    return strcmp(obj->str, value) == 0;
}

static int token_is_escape(JSONToken *obj, const char *value)
{
    if (obj->type != JSON_ESCAPE) {
        return 0;
    }

    return (strcmp(obj->str, value) == 0);
}

/**
 * Error handler
 */
static void parse_error(JSONParserContext *ctxt,
			JSONToken *token, const char *msg, ...)
{
    va_list ap;
    va_start(ap, msg);
    fprintf(stderr, "parse error: ");
    vfprintf(stderr, msg, ap);
    fprintf(stderr, "\n");
    va_end(ap);
}

/**
 * String helpers
 *
 * These helpers are used to unescape strings.
 */
static void wchar_to_utf8(uint16_t wchar, char *buffer, size_t buffer_length)
{
    if (wchar <= 0x007F) {
        BUG_ON(buffer_length < 2);

        buffer[0] = wchar & 0x7F;
        buffer[1] = 0;
    } else if (wchar <= 0x07FF) {
        BUG_ON(buffer_length < 3);

        buffer[0] = 0xC0 | ((wchar >> 6) & 0x1F);
        buffer[1] = 0x80 | (wchar & 0x3F);
        buffer[2] = 0;
    } else {
        BUG_ON(buffer_length < 4);

        buffer[0] = 0xE0 | ((wchar >> 12) & 0x0F);
        buffer[1] = 0x80 | ((wchar >> 6) & 0x3F);
        buffer[2] = 0x80 | (wchar & 0x3F);
        buffer[3] = 0;
    }
}

static int hex2decimal(char ch)
{
    if (ch >= '0' && ch <= '9') {
        return (ch - '0');
    } else if (ch >= 'a' && ch <= 'f') {
        return 10 + (ch - 'a');
    } else if (ch >= 'A' && ch <= 'F') {
        return 10 + (ch - 'A');
    }

    return -1;
}

/**
 * parse_string(): Parse a json string and return a QObject
 *
 *  string
 *      ""
 *      " chars "
 *  chars
 *      char
 *      char chars
 *  char
 *      any-Unicode-character-
 *          except-"-or-\-or-
 *          control-character
 *      \"
 *      \\
 *      \/
 *      \b
 *      \f
 *      \n
 *      \r
 *      \t
 *      \u four-hex-digits 
 */
static QString *qstring_from_escaped_str(JSONParserContext *ctxt, JSONToken *token)
{
    const char *ptr = token->str;
    QString *qstr = NULL;
    GString *str;
    int double_quote = 1;

    if (*ptr == '"') {
        double_quote = 1;
    } else {
        double_quote = 0;
    }
    ptr++;

    str = g_string_sized_new(10);
    while (*ptr && 
           ((double_quote && *ptr != '"') || (!double_quote && *ptr != '\''))) {
        if (*ptr == '\\') {
            ptr++;

            switch (*ptr) {
            case '"':
                g_string_append_c(str, '"');
                ptr++;
                break;
            case '\'':
                g_string_append_c(str, '\'');
                ptr++;
                break;
            case '\\':
                g_string_append_c(str, '\\');
                ptr++;
                break;
            case '/':
                g_string_append_c(str, '/');
                ptr++;
                break;
            case 'b':
                g_string_append_c(str, '\b');
                ptr++;
                break;
            case 'f':
                g_string_append_c(str, '\f');
                ptr++;
                break;
            case 'n':
                g_string_append_c(str, '\n');
                ptr++;
                break;
            case 'r':
                g_string_append_c(str, '\r');
                ptr++;
                break;
            case 't':
                g_string_append_c(str, '\t');
                ptr++;
                break;
            case 'u': {
                uint16_t unicode_char = 0;
                char utf8_char[4];
                int i = 0;

                ptr++;

                for (i = 0; i < 4; i++) {
		    int decimal = hex2decimal(*ptr);
                    if (decimal != -1) {
                        unicode_char |= decimal << ((3 - i) * 4);
                    } else {
                        parse_error(ctxt, token,
                                    "invalid hex escape sequence in string");
                        goto out;
                    }
                    ptr++;
                }

                wchar_to_utf8(unicode_char, utf8_char, sizeof(utf8_char));
                g_string_append(str, utf8_char);
            }   break;
            default:
                parse_error(ctxt, token, "invalid escape sequence in string");
                goto out;
            }
        } else {
            g_string_append_c(str, *ptr++);
        }
    }

    qstr = qstring_from_str (str->str);

out:
    g_string_free(str, TRUE);
    return qstr;
}

/**
 * Parsing rules
 */
static int parse_pair(JSONParserContext *ctxt, GVariantBuilder *builder, GQueue *tokens, va_list *ap)
{
    QObject *key, *value;
    JSONToken *peek;

    key = parse_value(ctxt, tokens, ap);
    if (!key || !g_variant_is_of_type(key, G_VARIANT_TYPE_STRING)) {
        parse_error(ctxt, peek, "key is not a string in object");
        goto out;
    }

    peek = g_queue_peek_head(tokens);
    if (!token_is_operator(peek, ':')) {
        parse_error(ctxt, peek, "missing : in object pair");
        goto out;
    }

    g_queue_pop_head(tokens);
    peek = g_queue_peek_head(tokens);
    value = parse_value(ctxt, tokens, ap);
    if (value == NULL) {
        parse_error(ctxt, peek, "Missing value in dict");
        goto out;
    }

    g_variant_builder_add (builder, "{sv}",
			   g_variant_get_string (key, NULL),
			   value);
    g_variant_unref (key);
    return 0;

out:
    if (key != NULL) {
        g_variant_unref(key);
    }
    return -1;
}

static QObject *parse_object(JSONParserContext *ctxt, GQueue *tokens, va_list *ap)
{
    static GVariantType *BOXED_DICTIONARY;
    GVariantBuilder builder;
    JSONToken *peek;

    peek = g_queue_peek_head (tokens);
    if (!token_is_operator(peek, '{')) {
        goto out_not_object;
    }

    g_queue_pop_head (tokens);
    peek = g_queue_peek_head (tokens);
    if (!BOXED_DICTIONARY) {
        BOXED_DICTIONARY = g_variant_type_new ("a{sv}");
    }
    g_variant_builder_init (&builder, BOXED_DICTIONARY);
    if (!token_is_operator(peek, '}')) {
        for (;;) {
            if (parse_pair(ctxt, &builder, tokens, ap) == -1) {
                goto out;
            }

            peek = g_queue_peek_head (tokens);
            if (token_is_operator(peek, '}')) {
                break;
            }
            if (!token_is_operator(peek, ',')) {
                parse_error(ctxt, peek, "expected separator in dict");
                goto out;
            }

            g_queue_pop_head (tokens);
        }
    }

    g_queue_pop_head (tokens);
    return g_variant_builder_end (&builder);

out:
    g_variant_builder_clear (&builder);
out_not_object:
    return NULL;
}

static QObject *parse_array(JSONParserContext *ctxt, GQueue *tokens, va_list *ap)
{
    static GVariantType *BOXED_ARRAY;
    GVariantBuilder builder;
    JSONToken *peek;

    peek = g_queue_peek_head (tokens);
    if (!token_is_operator(peek, '[')) {
        goto out_not_array;
    }

    g_queue_pop_head (tokens);
    peek = g_queue_peek_head (tokens);
    if (!BOXED_ARRAY) {
        BOXED_ARRAY = g_variant_type_new ("av");
    }
    g_variant_builder_init (&builder, BOXED_ARRAY);
    if (!token_is_operator(peek, ']')) {
        for (;;) {
	    QObject *obj = parse_value(ctxt, tokens, ap);
            if (obj == NULL) {
                goto out;
            }

            g_variant_builder_add_value (&builder, g_variant_new_variant (obj));
            peek = g_queue_peek_head (tokens);
            if (token_is_operator(peek, ']')) {
                break;
            }
            if (!token_is_operator(peek, ',')) {
                parse_error(ctxt, peek, "expected separator in array");
                goto out;
            }

            g_queue_pop_head (tokens);
        }
    }

    g_queue_pop_head (tokens);
    return g_variant_builder_end(&builder);

out:
    g_variant_builder_clear(&builder);
out_not_array:
    return NULL;
}

static QObject *parse_keyword(JSONParserContext *ctxt, GQueue *tokens)
{
    QObject *ret;
    JSONToken *token;

    token = g_queue_peek_head(tokens);
    if (token->type != JSON_KEYWORD) {
        goto out;
    }

    if (token_is_keyword(token, "true")) {
        ret = QOBJECT(qbool_from_int(TRUE));
    } else if (token_is_keyword(token, "false")) {
        ret = QOBJECT(qbool_from_int(FALSE));
    } else {
        parse_error(ctxt, token, "invalid keyword `%s'", token->str);
        goto out;
    }

    g_queue_pop_head(tokens);
    return ret;

out: 
    return NULL;
}

static QObject *parse_escape(JSONParserContext *ctxt, GQueue *tokens, va_list *ap)
{
    QObject *obj;
    JSONToken *token;

    if (ap == NULL) {
        goto out;
    }

    token = g_queue_peek_head(tokens);
    if (token_is_escape(token, "%p")) {
        obj = va_arg(*ap, QObject *);
    } else if (token_is_escape(token, "%i")) {
        obj = QOBJECT(qbool_from_int(va_arg(*ap, int)));
    } else if (token_is_escape(token, "%d")) {
        obj = QOBJECT(qint_from_int(va_arg(*ap, int)));
    } else if (token_is_escape(token, "%ld")) {
        obj = QOBJECT(qint_from_int(va_arg(*ap, long)));
    } else if (token_is_escape(token, "%lld") ||
               token_is_escape(token, "%I64d")) {
        obj = QOBJECT(qint_from_int(va_arg(*ap, long long)));
    } else if (token_is_escape(token, "%s")) {
        obj = QOBJECT(qstring_from_str(va_arg(*ap, const char *)));
    } else if (token_is_escape(token, "%f")) {
        obj = QOBJECT(qfloat_from_double(va_arg(*ap, double)));
    } else {
        goto out;
    }

    g_queue_pop_head(tokens);
    return obj;

out:
    return NULL;
}

static QObject *parse_literal(JSONParserContext *ctxt, GQueue *tokens)
{
    QObject *obj;
    JSONToken *token;

    token = g_queue_peek_head(tokens);
    switch (token->type) {
    case JSON_STRING:
        obj = QOBJECT(qstring_from_escaped_str(ctxt, token));
        break;
    case JSON_INTEGER:
        obj = QOBJECT(qint_from_int(strtoll(token->str, NULL, 10)));
        break;
    case JSON_FLOAT:
        /* FIXME dependent on locale */
        obj = QOBJECT(qfloat_from_double(strtod(token->str, NULL)));
        break;
    default:
        goto out;
    }

    g_queue_pop_head(tokens);
    return obj;

out:
    return NULL;
}

static QObject *parse_value(JSONParserContext *ctxt, GQueue *tokens, va_list *ap)
{
    QObject *obj;

    obj = parse_object(ctxt, tokens, ap);
    if (obj == NULL) {
        obj = parse_array(ctxt, tokens, ap);
    }
    if (obj == NULL) {
        obj = parse_escape(ctxt, tokens, ap);
    }
    if (obj == NULL) {
        obj = parse_keyword(ctxt, tokens);
    } 
    if (obj == NULL) {
        obj = parse_literal(ctxt, tokens);
    }

    return obj;
}

QObject *json_parser_parse(GQueue *tokens, va_list *ap)
{
    JSONParserContext ctxt = {};
    GQueue *working;
    QObject *result;

    if (!tokens)
	return NULL;

    working = g_queue_copy(tokens);
    result = parse_value(&ctxt, working, ap);
    g_queue_free(working);

    return result;
}
