/*
 * Copyright IBM, Corp. 2009
 *
 * Authors:
 *  Anthony Liguori   <aliguori@us.ibm.com>
 *
 * This work is licensed under the terms of the GNU LGPL, version 2.1 or later.
 * See the COPYING.LIB file in the top-level directory.
 *
 */
#include <check.h>
#include <inttypes.h>
#include <stdlib.h>
#include <stdbool.h>

#include "gvariant-utils.h"
#include "gvariant-json.h"

START_TEST(escaped_string)
{
    int i;
    struct {
        const char *encoded;
        const char *decoded;
        int skip;
    } test_cases[] = {
        { "\"\\b\"", "\b" },
        { "\"\\f\"", "\f" },
        { "\"\\n\"", "\n" },
        { "\"\\r\"", "\r" },
        { "\"\\t\"", "\t" },
        { "\"\\/\"", "/", .skip = 1 },
        { "\"\\\\\"", "\\" },
        { "\"\\\"\"", "\"" },
        { "\"hello world \\\"embedded string\\\"\"",
          "hello world \"embedded string\"" },
        { "\"hello world\\nwith new line\"", "hello world\nwith new line" },
        { "\"single byte utf-8 \\u0020\"", "single byte utf-8  ", .skip = 1 },
        { "\"double byte utf-8 \\u00A2\"", "double byte utf-8 \xc2\xa2" },
        { "\"triple byte utf-8 \\u20AC\"", "triple byte utf-8 \xe2\x82\xac" },
        {}
    };

    for (i = 0; test_cases[i].encoded; i++) {
        GVariant *obj;
        char *str;

        obj = g_variant_from_json(test_cases[i].encoded);

        fail_unless(obj != NULL);
        fail_unless(g_variant_is_of_type(obj, G_VARIANT_TYPE_STRING));
        
        fail_unless(strcmp(g_variant_get_string(obj, NULL), test_cases[i].decoded) == 0,
                    "%s != %s\n", g_variant_get_string(obj, NULL), test_cases[i].decoded);

        if (test_cases[i].skip == 0) {
            str = g_variant_to_json(obj);
            fail_unless(strcmp(str,test_cases[i].encoded) == 0,
                        "%s != %s\n", str, test_cases[i].encoded);

            free(str);
        }
        g_variant_unref(obj);
    }
}
END_TEST

START_TEST(simple_string)
{
    int i;
    struct {
        const char *encoded;
        const char *decoded;
    } test_cases[] = {
        { "\"hello world\"", "hello world" },
        { "\"the quick brown fox jumped over the fence\"",
          "the quick brown fox jumped over the fence" },
        {}
    };

    for (i = 0; test_cases[i].encoded; i++) {
        GVariant *obj;
        char *str;

        obj = g_variant_from_json(test_cases[i].encoded);

        fail_unless(obj != NULL);
        fail_unless(g_variant_is_of_type(obj, G_VARIANT_TYPE_STRING));
        
        fail_unless(strcmp(g_variant_get_string(obj, NULL), test_cases[i].decoded) == 0);

        str = g_variant_to_json(obj);
        fail_unless(strcmp(str, test_cases[i].encoded) == 0);

        g_variant_unref(obj);
        
        free(str);
    }
}
END_TEST

START_TEST(single_quote_string)
{
    int i;
    struct {
        const char *encoded;
        const char *decoded;
    } test_cases[] = {
        { "'hello world'", "hello world" },
        { "'the quick brown fox \\' jumped over the fence'",
          "the quick brown fox ' jumped over the fence" },
        {}
    };

    for (i = 0; test_cases[i].encoded; i++) {
        GVariant *obj;

        obj = g_variant_from_json(test_cases[i].encoded);

        fail_unless(obj != NULL);
        fail_unless(g_variant_is_of_type(obj, G_VARIANT_TYPE_STRING));
        
        fail_unless(strcmp(g_variant_get_string(obj, NULL), test_cases[i].decoded) == 0);

        g_variant_unref(obj);
    }
}
END_TEST

START_TEST(vararg_string)
{
    int i;
    struct {
        const char *decoded;
    } test_cases[] = {
        { "hello world" },
        { "the quick brown fox jumped over the fence" },
        {}
    };

    for (i = 0; test_cases[i].decoded; i++) {
        GVariant *obj;

        obj = g_variant_from_jsonf("%s", test_cases[i].decoded);

        fail_unless(obj != NULL);
        fail_unless(g_variant_is_of_type(obj, G_VARIANT_TYPE_STRING));
        
        fail_unless(strcmp(g_variant_get_string(obj, NULL), test_cases[i].decoded) == 0);

        g_variant_unref(obj);
    }
}
END_TEST

START_TEST(simple_number)
{
    int i;
    struct {
        const char *encoded;
        int64_t decoded;
        int skip;
    } test_cases[] = {
        { "0", 0 },
        { "1234", 1234 },
        { "1", 1 },
        { "-32", -32 },
        { "-0", 0, .skip = 1 },
        { },
    };

    for (i = 0; test_cases[i].encoded; i++) {
        GVariant *obj;

        obj = g_variant_from_json(test_cases[i].encoded);
        fail_unless(obj != NULL);
        fail_unless(g_variant_is_of_type(obj, G_VARIANT_TYPE_INT64));

        fail_unless(g_variant_get_int64(obj) == test_cases[i].decoded);
        if (test_cases[i].skip == 0) {
            char *str;

            str = g_variant_to_json(obj);
            fail_unless(strcmp(str, test_cases[i].encoded) == 0);
            free(str);
        }

        g_variant_unref(obj);
    }
}
END_TEST

START_TEST(float_number)
{
    int i;
    struct {
        const char *encoded;
        double decoded;
        int skip;
    } test_cases[] = {
        { "32.43", 32.43 },
        { "0.222", 0.222 },
        { "-32.12313", -32.12313 },
        { "-32.20e-10", -32.20e-10, .skip = 1 },
        { },
    };

    for (i = 0; test_cases[i].encoded; i++) {
        GVariant *obj;

        obj = g_variant_from_json(test_cases[i].encoded);
        fail_unless(obj != NULL);
        fail_unless(g_variant_is_of_type(obj, G_VARIANT_TYPE_DOUBLE));

        fail_unless(g_variant_get_double(obj) == test_cases[i].decoded);

        if (test_cases[i].skip == 0) {
            char *str;

            str = g_variant_to_json(obj);
            fail_unless(strcmp(str, test_cases[i].encoded) == 0);
            free(str);
        }

        g_variant_unref(obj);
    }
}
END_TEST

START_TEST(vararg_number)
{
    GVariant *obj;
    int value = 0x2342;
    int64_t value64 = 0x2342342343LL;
    double valuef = 2.323423423;

    obj = g_variant_from_jsonf("%d", value);
    fail_unless(obj != NULL);
    fail_unless(g_variant_is_of_type(obj, G_VARIANT_TYPE_INT64));

    fail_unless(g_variant_get_int64(obj) == value);

    g_variant_unref(obj);

    obj = g_variant_from_jsonf("%" PRId64, value64);
    fail_unless(obj != NULL);
    fail_unless(g_variant_is_of_type(obj, G_VARIANT_TYPE_INT64));

    fail_unless(g_variant_get_int64(obj) == value64);

    g_variant_unref(obj);

    obj = g_variant_from_jsonf("%f", valuef);
    fail_unless(obj != NULL);
    fail_unless(g_variant_is_of_type(obj, G_VARIANT_TYPE_DOUBLE));

    fail_unless(g_variant_get_double(obj) == valuef);

    g_variant_unref(obj);
}
END_TEST

START_TEST(keyword_literal)
{
    GVariant *obj;
    char *str;

    obj = g_variant_from_json("true");
    fail_unless(obj != NULL);
    fail_unless(g_variant_is_of_type(obj, G_VARIANT_TYPE_BOOLEAN));

    fail_unless(g_variant_get_boolean(obj) != 0);

    str = g_variant_to_json(obj);
    fail_unless(strcmp(str, "true") == 0);
    free(str);

    g_variant_unref(obj);

    obj = g_variant_from_json("false");
    fail_unless(obj != NULL);
    fail_unless(g_variant_is_of_type(obj, G_VARIANT_TYPE_BOOLEAN));

    fail_unless(g_variant_get_boolean(obj) == 0);

    str = g_variant_to_json(obj);
    fail_unless(strcmp(str, "false") == 0);
    free(str);

    g_variant_unref(obj);

    obj = g_variant_from_jsonf("%i", false);
    fail_unless(obj != NULL);
    fail_unless(g_variant_is_of_type(obj, G_VARIANT_TYPE_BOOLEAN));

    fail_unless(g_variant_get_boolean(obj) == 0);

    g_variant_unref(obj);
    
    obj = g_variant_from_jsonf("%i", true);
    fail_unless(obj != NULL);
    fail_unless(g_variant_is_of_type(obj, G_VARIANT_TYPE_BOOLEAN));

    fail_unless(g_variant_get_boolean(obj) != 0);

    g_variant_unref(obj);
}
END_TEST

typedef struct LiteralQDictEntry LiteralQDictEntry;
typedef struct LiteralGVariant LiteralGVariant;

enum {
    QTYPE_NONE,
    QTYPE_QINT,
    QTYPE_QSTRING,
    QTYPE_QDICT,
    QTYPE_QLIST
};

struct LiteralGVariant
{
    int type;
    union {
        int64_t qint;
        const char *qstr;
        LiteralQDictEntry *qdict;
        LiteralGVariant *qlist;
    } value;
};

struct LiteralQDictEntry
{
    const char *key;
    LiteralGVariant value;
};

#define QLIT_QINT(val) (LiteralGVariant){.type = QTYPE_QINT, .value.qint = (val)}
#define QLIT_QSTR(val) (LiteralGVariant){.type = QTYPE_QSTRING, .value.qstr = (val)}
#define QLIT_QDICT(val) (LiteralGVariant){.type = QTYPE_QDICT, .value.qdict = (val)}
#define QLIT_QLIST(val) (LiteralGVariant){.type = QTYPE_QLIST, .value.qlist = (val)}

typedef struct QListCompareHelper
{
    int index;
    LiteralGVariant *objs;
    int result;
} QListCompareHelper;

static int compare_litqobj_to_qobj(LiteralGVariant *lhs, GVariant *rhs);

static void compare_helper(GVariant *obj, void *opaque)
{
    QListCompareHelper *helper = opaque;

    if (helper->result == 0) {
        return;
    }

    if (helper->objs[helper->index].type == QTYPE_NONE) {
        helper->result = 0;
        return;
    }

    helper->result = compare_litqobj_to_qobj(&helper->objs[helper->index++], obj);
}

static int compare_litqobj_to_qobj(LiteralGVariant *lhs, GVariant *rhs)
{
    static const GVariantType *types[] = {
        [QTYPE_QINT] = G_VARIANT_TYPE_INT64,
        [QTYPE_QSTRING] = G_VARIANT_TYPE_STRING,
        [QTYPE_QDICT] = G_VARIANT_TYPE_DICTIONARY,
        [QTYPE_QLIST] = G_VARIANT_TYPE_ARRAY
    };
    if (!g_variant_is_of_type (rhs, types[lhs->type])) {
        return 0;
    }

    switch (lhs->type) {
    case QTYPE_QINT:
        return lhs->value.qint == g_variant_get_int64(rhs);
    case QTYPE_QSTRING:
        return (strcmp(lhs->value.qstr, g_variant_get_string(rhs, NULL)) == 0);
    case QTYPE_QDICT: {
        int i;

        for (i = 0; lhs->value.qdict[i].key; i++) {
            GVariant *obj = g_variant_lookup_value(rhs, lhs->value.qdict[i].key,
						   NULL);

            if (!compare_litqobj_to_qobj(&lhs->value.qdict[i].value, obj)) {
	        g_variant_unref (obj);
                return 0;
            }
	    g_variant_unref (obj);
        }

        return 1;
    }
    case QTYPE_QLIST: {
        QListCompareHelper helper;

        helper.index = 0;
        helper.objs = lhs->value.qlist;
        helper.result = 1;
        
        g_variant_array_iterate(rhs, compare_helper, &helper);

        return helper.result;
    }
    default:
        break;
    }

    return 0;
}

START_TEST(simple_dict)
{
    int i;
    struct {
        const char *encoded;
        LiteralGVariant decoded;
    } test_cases[] = {
        {
            .encoded = "{\"foo\": 42, \"bar\": \"hello world\"}",
            .decoded = QLIT_QDICT(((LiteralQDictEntry[]){
                        { "foo", QLIT_QINT(42) },
                        { "bar", QLIT_QSTR("hello world") },
                        { }
                    })),
        }, {
            .encoded = "{}",
            .decoded = QLIT_QDICT(((LiteralQDictEntry[]){
                        { }
                    })),
        }, {
            .encoded = "{\"foo\": 43}",
            .decoded = QLIT_QDICT(((LiteralQDictEntry[]){
                        { "foo", QLIT_QINT(43) },
                        { }
                    })),
        },
        { }
    };

    for (i = 0; test_cases[i].encoded; i++) {
        GVariant *obj;
        char *str;

        obj = g_variant_from_json(test_cases[i].encoded);
        fail_unless(obj != NULL);
        fail_unless(g_variant_is_of_type(obj, G_VARIANT_TYPE_DICTIONARY));

        fail_unless(compare_litqobj_to_qobj(&test_cases[i].decoded, obj) == 1);

        str = g_variant_to_json(obj);
        g_variant_unref(obj);

        obj = g_variant_from_json(str);
        fail_unless(obj != NULL);
        fail_unless(g_variant_is_of_type(obj, G_VARIANT_TYPE_DICTIONARY));

        fail_unless(compare_litqobj_to_qobj(&test_cases[i].decoded, obj) == 1);
        g_variant_unref(obj);
        free(str);
    }
}
END_TEST

START_TEST(simple_list)
{
    int i;
    struct {
        const char *encoded;
        LiteralGVariant decoded;
    } test_cases[] = {
        {
            .encoded = "[43,42]",
            .decoded = QLIT_QLIST(((LiteralGVariant[]){
                        QLIT_QINT(43),
                        QLIT_QINT(42),
                        { }
                    })),
        },
        {
            .encoded = "[43]",
            .decoded = QLIT_QLIST(((LiteralGVariant[]){
                        QLIT_QINT(43),
                        { }
                    })),
        },
        {
            .encoded = "[]",
            .decoded = QLIT_QLIST(((LiteralGVariant[]){
                        { }
                    })),
        },
        {
            .encoded = "[{}]",
            .decoded = QLIT_QLIST(((LiteralGVariant[]){
                        QLIT_QDICT(((LiteralQDictEntry[]){
                                    {},
                                        })),
                        {},
                            })),
        },
        { }
    };

    for (i = 0; test_cases[i].encoded; i++) {
        GVariant *obj;
        char *str;

        obj = g_variant_from_json(test_cases[i].encoded);
        fail_unless(obj != NULL);
        fail_unless(g_variant_is_of_type(obj, G_VARIANT_TYPE_ARRAY));

        fail_unless(compare_litqobj_to_qobj(&test_cases[i].decoded, obj) == 1);

        str = g_variant_to_json(obj);
        g_variant_unref(obj);

        obj = g_variant_from_json(str);
        fail_unless(obj != NULL);
        fail_unless(g_variant_is_of_type(obj, G_VARIANT_TYPE_ARRAY));

        fail_unless(compare_litqobj_to_qobj(&test_cases[i].decoded, obj) == 1);
        g_variant_unref(obj);
        free(str);
    }
}
END_TEST

START_TEST(simple_whitespace)
{
    int i;
    struct {
        const char *encoded;
        LiteralGVariant decoded;
    } test_cases[] = {
        {
            .encoded = " [ 43 , 42 ]",
            .decoded = QLIT_QLIST(((LiteralGVariant[]){
                        QLIT_QINT(43),
                        QLIT_QINT(42),
                        { }
                    })),
        },
        {
            .encoded = " [ 43 , { 'h' : 'b' }, [ ], 42 ]",
            .decoded = QLIT_QLIST(((LiteralGVariant[]){
                        QLIT_QINT(43),
                        QLIT_QDICT(((LiteralQDictEntry[]){
                                    { "h", QLIT_QSTR("b") },
                                    { }})),
                        QLIT_QLIST(((LiteralGVariant[]){
                                    { }})),
                        QLIT_QINT(42),
                        { }
                    })),
        },
        {
            .encoded = " [ 43 , { 'h' : 'b' , 'a' : 32 }, [ ], 42 ]",
            .decoded = QLIT_QLIST(((LiteralGVariant[]){
                        QLIT_QINT(43),
                        QLIT_QDICT(((LiteralQDictEntry[]){
                                    { "h", QLIT_QSTR("b") },
                                    { "a", QLIT_QINT(32) },
                                    { }})),
                        QLIT_QLIST(((LiteralGVariant[]){
                                    { }})),
                        QLIT_QINT(42),
                        { }
                    })),
        },
        { }
    };

    for (i = 0; test_cases[i].encoded; i++) {
        GVariant *obj;
        char *str;

        obj = g_variant_from_json(test_cases[i].encoded);
        fail_unless(obj != NULL);
        fail_unless(g_variant_is_of_type(obj, G_VARIANT_TYPE_ARRAY));

        fail_unless(compare_litqobj_to_qobj(&test_cases[i].decoded, obj) == 1);

        str = g_variant_to_json(obj);
        g_variant_unref(obj);

        obj = g_variant_from_json(str);
        fail_unless(obj != NULL);
        fail_unless(g_variant_is_of_type(obj, G_VARIANT_TYPE_ARRAY));

        fail_unless(compare_litqobj_to_qobj(&test_cases[i].decoded, obj) == 1);

        g_variant_unref(obj);
        free(str);
    }
}
END_TEST

START_TEST(simple_varargs)
{
    GVariant *embedded_obj;
    GVariant *obj;
    LiteralGVariant decoded = QLIT_QLIST(((LiteralGVariant[]){
            QLIT_QINT(1),
            QLIT_QINT(2),
            QLIT_QLIST(((LiteralGVariant[]){
                        QLIT_QINT(32),
                        QLIT_QINT(42),
                        {}})),
            {}}));

    embedded_obj = g_variant_from_json("[32, 42]");
    fail_unless(embedded_obj != NULL);

    obj = g_variant_from_jsonf("[%d, 2, %p]", 1, embedded_obj);
    fail_unless(obj != NULL);

    fail_unless(compare_litqobj_to_qobj(&decoded, obj) == 1);

    g_variant_unref(obj);
}
END_TEST

START_TEST(empty_input)
{
    const char *empty = "";

    GVariant *obj = g_variant_from_json(empty);
    fail_unless(obj == NULL);
}
END_TEST

START_TEST(unterminated_string)
{
    GVariant *obj = g_variant_from_json("\"abc");
    fail_unless(obj == NULL);
}
END_TEST

START_TEST(unterminated_sq_string)
{
    GVariant *obj = g_variant_from_json("'abc");
    fail_unless(obj == NULL);
}
END_TEST

START_TEST(unterminated_escape)
{
    GVariant *obj = g_variant_from_json("\"abc\\\"");
    fail_unless(obj == NULL);
}
END_TEST

START_TEST(unterminated_array)
{
    GVariant *obj = g_variant_from_json("[32");
    fail_unless(obj == NULL);
}
END_TEST

START_TEST(unterminated_array_comma)
{
    GVariant *obj = g_variant_from_json("[32,");
    fail_unless(obj == NULL);
}
END_TEST

START_TEST(invalid_array_comma)
{
    GVariant *obj = g_variant_from_json("[32,}");
    fail_unless(obj == NULL);
}
END_TEST

START_TEST(unterminated_dict)
{
    GVariant *obj = g_variant_from_json("{'abc':32");
    fail_unless(obj == NULL);
}
END_TEST

START_TEST(unterminated_dict_comma)
{
    GVariant *obj = g_variant_from_json("{'abc':32,");
    fail_unless(obj == NULL);
}
END_TEST

#if 0
START_TEST(invalid_dict_comma)
{
    GVariant *obj = g_variant_from_json("{'abc':32,}");
    fail_unless(obj == NULL);
}
END_TEST

START_TEST(unterminated_literal)
{
    GVariant *obj = g_variant_from_json("nul");
    fail_unless(obj == NULL);
}
END_TEST
#endif

static Suite *gvariant_json_suite(void)
{
    Suite *suite;
    TCase *string_literals, *number_literals, *keyword_literals;
    TCase *dicts, *lists, *whitespace, *varargs, *errors;

    string_literals = tcase_create("String Literals");
    tcase_add_test(string_literals, simple_string);
    tcase_add_test(string_literals, escaped_string);
    tcase_add_test(string_literals, single_quote_string);
    tcase_add_test(string_literals, vararg_string);

    number_literals = tcase_create("Number Literals");
    tcase_add_test(number_literals, simple_number);
    tcase_add_test(number_literals, float_number);
    tcase_add_test(number_literals, vararg_number);

    keyword_literals = tcase_create("Keywords");
    tcase_add_test(keyword_literals, keyword_literal);
    dicts = tcase_create("Objects");
    tcase_add_test(dicts, simple_dict);
    lists = tcase_create("Lists");
    tcase_add_test(lists, simple_list);

    whitespace = tcase_create("Whitespace");
    tcase_add_test(whitespace, simple_whitespace);

    varargs = tcase_create("Varargs");
    tcase_add_test(varargs, simple_varargs);

    errors = tcase_create("Invalid JSON");
    tcase_add_test(errors, empty_input);
    tcase_add_test(errors, unterminated_string);
    tcase_add_test(errors, unterminated_escape);
    tcase_add_test(errors, unterminated_sq_string);
    tcase_add_test(errors, unterminated_array);
    tcase_add_test(errors, unterminated_array_comma);
    tcase_add_test(errors, invalid_array_comma);
    tcase_add_test(errors, unterminated_dict);
    tcase_add_test(errors, unterminated_dict_comma);
#if 0
    /* FIXME: this print parse error messages on stderr.  */
    tcase_add_test(errors, invalid_dict_comma);
    tcase_add_test(errors, unterminated_literal);
#endif

    suite = suite_create("QJSON test-suite");
    suite_add_tcase(suite, string_literals);
    suite_add_tcase(suite, number_literals);
    suite_add_tcase(suite, keyword_literals);
    suite_add_tcase(suite, dicts);
    suite_add_tcase(suite, lists);
    suite_add_tcase(suite, whitespace);
    suite_add_tcase(suite, varargs);
    suite_add_tcase(suite, errors);

    return suite;
}

int main(void)
{
    int nf;
    Suite *s;
    SRunner *sr;

    s = gvariant_json_suite();
    sr = srunner_create(s);
        
    if (!getenv("FORK")) srunner_set_fork_status (sr, CK_NOFORK);
    srunner_run_all(sr, CK_NORMAL);
    nf = srunner_ntests_failed(sr);
    srunner_free(sr);
    
    return (nf == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
