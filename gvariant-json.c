/*
 * QObject JSON integration
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
#include "qint.h"
#include "qlist.h"
#include "qbool.h"
#include "qfloat.h"
#include "qdict.h"

typedef struct JSONParsingState
{
    JSONMessageParser parser;
    va_list *ap;
    QObject *result;
} JSONParsingState;

static void parse_json(JSONMessageParser *parser, GQueue *tokens)
{
    JSONParsingState *s = container_of(parser, JSONParsingState, parser);
    s->result = json_parser_parse(tokens, s->ap);
}

QObject *qobject_from_jsonv(const char *string, va_list *ap)
{
    JSONParsingState state = {};

    state.ap = ap;

    json_message_parser_init(&state.parser, parse_json);
    json_message_parser_feed(&state.parser, string, strlen(string));
    json_message_parser_flush(&state.parser);
    json_message_parser_destroy(&state.parser);

    return state.result;
}

QObject *qobject_from_json(const char *string)
{
    return qobject_from_jsonv(string, NULL);
}

/*
 * IMPORTANT: This function aborts on error, thus it must not
 * be used with untrusted arguments.
 */
QObject *qobject_from_jsonf(const char *string, ...)
{
    QObject *obj;
    va_list ap;

    va_start(ap, string);
    obj = qobject_from_jsonv(string, &ap);
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

static void to_json(QObject *obj, GString *str, int pretty, int indent);

static void to_json_dict_iter(const char *key, QObject *obj, void *opaque)
{
    ToJsonIterState *s = opaque;
    QString *qkey;
    int j;

    if (s->count)
        g_string_append(s->str, ", ");

    if (s->pretty) {
        g_string_append_c(s->str, '\n');
        for (j = 0 ; j < s->indent ; j++)
            g_string_append(s->str, "    ");
    }

    qkey = qstring_from_str(key);
    to_json(QOBJECT(qkey), s->str, s->pretty, s->indent);
    QDECREF(qkey);

    g_string_append(s->str, ": ");
    to_json(obj, s->str, s->pretty, s->indent);
    s->count++;
}

static void to_json_list_iter(QObject *obj, void *opaque)
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

static void to_json(QObject *obj, GString *str, int pretty, int indent)
{
    if (g_variant_is_of_type (obj, G_VARIANT_TYPE_INT64)) {
        QInt *val = qobject_to_qint(obj);
        g_string_append_printf(str, "%" PRId64, qint_get_int(val));
    }
    else if (g_variant_is_of_type (obj, G_VARIANT_TYPE_STRING)) {
        QString *val = qobject_to_qstring(obj);
        const char *ptr;

        ptr = qstring_get_str(val);
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
        QDict *val = qobject_to_qdict(obj);

        s.count = 0;
        s.str = str;
        s.indent = indent + 1;
        s.pretty = pretty;
        g_string_append(str, "{");
        qdict_iter(val, to_json_dict_iter, &s);
        if (pretty) {
            int j;
            g_string_append_c(str, '\n');
            for (j = 0 ; j < indent ; j++)
                g_string_append(str, "    ");
        }
        g_string_append(str, "}");
    } else if (g_variant_is_of_type (obj, G_VARIANT_TYPE_ARRAY)) {
        ToJsonIterState s;
        QList *val = qobject_to_qlist(obj);

        s.count = 0;
        s.str = str;
        s.indent = indent + 1;
        s.pretty = pretty;
        g_string_append(str, "[");
        qlist_iter(val, (void *)to_json_list_iter, &s);
        if (pretty) {
            int j;
            g_string_append_c(str, '\n');
            for (j = 0 ; j < indent ; j++)
                g_string_append(str, "    ");
        }
        g_string_append(str, "]");
    } else if (g_variant_is_of_type (obj, G_VARIANT_TYPE_DOUBLE)) {
        QFloat *val = qobject_to_qfloat(obj);
        char buffer[1024];
        int len;

        len = g_snprintf(buffer, sizeof(buffer), "%f", qfloat_get_double(val));
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
        QBool *val = qobject_to_qbool(obj);

        if (qbool_get_int(val)) {
            g_string_append(str, "true");
        } else {
            g_string_append(str, "false");
        }
    }
}

char *qobject_to_json(QObject *obj)
{
    GString *str = g_string_sized_new(30);

    to_json(obj, str, 0, 0);

    return g_string_free (str, FALSE);
}

char *qobject_to_json_pretty(QObject *obj)
{
    GString *str = g_string_sized_new(30);

    to_json(obj, str, 1, 0);

    return g_string_free (str, FALSE);
}
