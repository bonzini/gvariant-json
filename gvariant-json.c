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

#include <glib.h>
#include <inttypes.h>
#include <string.h>
#include "json-lexer.h"
#include "json-parser.h"
#include "json-streamer.h"
#include "gvariant-json.h"
#include "gvariant-utils.h"

typedef struct JSONParsingState
{
    JSONMessageParser parser;
    va_list *ap;
    GVariant *result;
} JSONParsingState;

static void parse_json(JSONMessageParser *parser, GQueue *tokens)
{
    JSONParsingState *s = container_of(parser, JSONParsingState, parser);
    s->result = json_parser_parse(tokens, s->ap);
}

GVariant *g_variant_from_jsonv(const char *string, va_list *ap)
{
    JSONParsingState state = {};

    state.ap = ap;

    json_message_parser_init(&state.parser, parse_json);
    json_message_parser_feed(&state.parser, string, strlen(string));
    json_message_parser_flush(&state.parser);
    json_message_parser_destroy(&state.parser);

    return state.result;
}

GVariant *g_variant_from_json(const char *string)
{
    return g_variant_from_jsonv(string, NULL);
}

/*
 * IMPORTANT: This function aborts on error, thus it must not
 * be used with untrusted arguments.
 */
GVariant *g_variant_from_jsonf(const char *string, ...)
{
    GVariant *obj;
    va_list ap;

    va_start(ap, string);
    obj = g_variant_from_jsonv(string, &ap);
    va_end(ap);

    assert(obj != NULL);
    return obj;
}

typedef struct ToJsonIterState
{
    int indent;
    int pretty;
    int count;
    GString *str;
} ToJsonIterState;

static void to_json(GVariant *obj, GString *str, int pretty, int indent);

static void to_json_dict_iter(const char *key, GVariant *obj, void *opaque)
{
    ToJsonIterState *s = opaque;
    GVariant *qkey;
    int j;

    if (s->count)
        g_string_append(s->str, ", ");

    if (s->pretty) {
        g_string_append_c(s->str, '\n');
        for (j = 0 ; j < s->indent ; j++)
            g_string_append(s->str, "    ");
    }

    qkey = g_variant_new_string(key);
    to_json(qkey, s->str, s->pretty, s->indent);
    g_variant_unref(qkey);

    g_string_append(s->str, ": ");
    to_json(obj, s->str, s->pretty, s->indent);
    s->count++;
}

static void to_json_list_iter(GVariant *obj, void *opaque)
{
    ToJsonIterState *s = opaque;
    int j;

    if (s->count)
        g_string_append(s->str, ", ");

    if (s->pretty) {
        g_string_append_c(s->str, '\n');
        for (j = 0 ; j < s->indent ; j++)
            g_string_append(s->str, "    ");
    }

    to_json(obj, s->str, s->pretty, s->indent);
    s->count++;
}

static void to_json(GVariant *obj, GString *str, int pretty, int indent)
{
    if (g_variant_is_of_type (obj, G_VARIANT_TYPE_INT64)) {
        g_string_append_printf(str, "%" PRId64, g_variant_get_int64(obj));
    }
    else if (g_variant_is_of_type (obj, G_VARIANT_TYPE_STRING)) {
        const char *ptr;

        ptr = g_variant_get_string(obj, NULL);
        g_string_append_c(str, '\"');
        while (*ptr) {
            if ((ptr[0] & 0xE0) == 0xE0 &&
                (ptr[1] & 0x80) && (ptr[2] & 0x80)) {
                uint16_t wchar;

                wchar  = (ptr[0] & 0x0F) << 12;
                wchar |= (ptr[1] & 0x3F) << 6;
                wchar |= (ptr[2] & 0x3F);
                ptr += 2;

                g_string_append_printf(str, "\\u%04X", wchar);
            } else if ((ptr[0] & 0xE0) == 0xC0 && (ptr[1] & 0x80)) {
                uint16_t wchar;

                wchar  = (ptr[0] & 0x1F) << 6;
                wchar |= (ptr[1] & 0x3F);
                ptr++;

                g_string_append_printf(str, "\\u%04X", wchar);
            } else switch (ptr[0]) {
                case '\"':
                    g_string_append(str, "\\\"");
                    break;
                case '\\':
                    g_string_append(str, "\\\\");
                    break;
                case '\b':
                    g_string_append(str, "\\b");
                    break;
                case '\f':
                    g_string_append(str, "\\f");
                    break;
                case '\n':
                    g_string_append(str, "\\n");
                    break;
                case '\r':
                    g_string_append(str, "\\r");
                    break;
                case '\t':
                    g_string_append(str, "\\t");
                    break;
                default: {
                    if (ptr[0] <= 0x1F) {
                        g_string_append_printf(str, "\\u%04X", ptr[0]);
                    } else {
                        g_string_append_c(str, ptr[0]);
                    }
                    break;
                }
                }
            ptr++;
        }
        g_string_append_c(str, '\"');
    } else if (g_variant_is_of_type (obj, G_VARIANT_TYPE_DICTIONARY)) {
        ToJsonIterState s;

        s.count = 0;
        s.str = str;
        s.indent = indent + 1;
        s.pretty = pretty;
        g_string_append(str, "{");
        g_variant_dictionary_iterate(obj, to_json_dict_iter, &s);
        if (pretty) {
            int j;
            g_string_append_c(str, '\n');
            for (j = 0 ; j < indent ; j++)
                g_string_append(str, "    ");
        }
        g_string_append(str, "}");
    } else if (g_variant_is_of_type (obj, G_VARIANT_TYPE_ARRAY)) {
        ToJsonIterState s;

        s.count = 0;
        s.str = str;
        s.indent = indent + 1;
        s.pretty = pretty;
        g_string_append(str, "[");
        g_variant_array_iterate(obj, (void *)to_json_list_iter, &s);
        if (pretty) {
            int j;
            g_string_append_c(str, '\n');
            for (j = 0 ; j < indent ; j++)
                g_string_append(str, "    ");
        }
        g_string_append(str, "]");
    } else if (g_variant_is_of_type (obj, G_VARIANT_TYPE_DOUBLE)) {
        char buffer[1024];
        int len;

        len = g_snprintf(buffer, sizeof(buffer), "%f", g_variant_get_double(obj));
        while (len > 0 && buffer[len - 1] == '0') {
            len--;
        }

        if (len && buffer[len - 1] == '.') {
            buffer[len - 1] = 0;
        } else {
            buffer[len] = 0;
        }
        
        g_string_append(str, buffer);
    } else if (g_variant_is_of_type (obj, G_VARIANT_TYPE_BOOLEAN)) {

        if (g_variant_get_boolean(obj)) {
            g_string_append(str, "true");
        } else {
            g_string_append(str, "false");
        }
    }
}

char *g_variant_to_json(GVariant *obj)
{
    GString *str = g_string_sized_new(30);

    to_json(obj, str, 0, 0);

    return g_string_free (str, FALSE);
}

char *g_variant_to_json_pretty(GVariant *obj)
{
    GString *str = g_string_sized_new(30);

    to_json(obj, str, 1, 0);

    return g_string_free (str, FALSE);
}
