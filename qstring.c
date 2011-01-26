/*
 * QString Module
 *
 * Copyright (C) 2009 Red Hat Inc.
 *
 * Authors:
 *  Luiz Capitulino <lcapitulino@redhat.com>
 *
 * This work is licensed under the terms of the GNU LGPL, version 2.1 or later.
 * See the COPYING.LIB file in the top-level directory.
 */

#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include "qobject.h"
#include "qstring.h"

static void qstring_destroy_obj(QObject *obj);

static const QType qstring_type = {
    .code = QTYPE_QSTRING,
    .destroy = qstring_destroy_obj,
};

/**
 * qstring_new(): Create a new empty QString
 *
 * Return strong reference.
 */
QString *qstring_new(void)
{
    return qstring_from_str("");
}

/**
 * qstring_from_substr(): Create a new QString from a C string substring
 *
 * Return string reference
 */
QString *qstring_from_substr(const char *str, int start, int end)
{
    QString *qstring;

    qstring = malloc(sizeof(*qstring));

    qstring->length = end - start + 1;

    qstring->string = malloc(qstring->length + 1);
    memcpy(qstring->string, str + start, qstring->length);
    qstring->string[qstring->length] = 0;

    QOBJECT_INIT(qstring, &qstring_type);

    return qstring;
}

/**
 * qstring_from_str(): Create a new QString from a regular C string
 *
 * Return strong reference.
 */
QString *qstring_from_str(const char *str)
{
    return qstring_from_substr(str, 0, strlen(str) - 1);
}

/**
 * qobject_to_qstring(): Convert a QObject to a QString
 */
QString *qobject_to_qstring(const QObject *obj)
{
    if (qobject_type(obj) != QTYPE_QSTRING)
        return NULL;

    return container_of(obj, QString, base);
}

/**
 * qstring_get_str(): Return a pointer to the stored string
 *
 * NOTE: Should be used with caution, if the object is deallocated
 * this pointer becomes invalid.
 */
const char *qstring_get_str(const QString *qstring)
{
    return qstring->string;
}

/**
 * qstring_destroy_obj(): Free all memory allocated by a QString
 * object
 */
static void qstring_destroy_obj(QObject *obj)
{
    QString *qs;

    assert(obj != NULL);
    qs = qobject_to_qstring(obj);
    free(qs->string);
    free(qs);
}
