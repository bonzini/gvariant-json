OBJS = check-qjson.o json-lexer.o json-parser.o json-streamer.o \
	qjson.o qbool.o qdict.o qfloat.o qint.o qlist.o qstring.o
LDFLAGS := -lcheck $(shell pkg-config --libs glib-2.0)
CFLAGS := -std=gnu99 -I. -g $(shell pkg-config --cflags glib-2.0)
check-qjson: $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

check-qjson.o: check-qjson.c
json-lexer.o: json-lexer.c
json-parser.o: json-parser.c
json-streamer.o: json-streamer.c
gvariant-utils.o: gvariant-utils.c
qjson.o: qjson.c
qbool.o: qbool.c
qdict.o: qdict.c
qfloat.o: qfloat.c
qint.o: qint.c
qlist.o: qlist.c
qstring.o: qstring.c
$(OBJS): $(wildcard *.h)
