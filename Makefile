OBJS = check-json.o json-lexer.o json-parser.o json-streamer.o \
	gvariant-utils.o gvariant-json.o
LDFLAGS := -lcheck $(shell pkg-config --libs glib-2.0)
CFLAGS := -I. -g $(shell pkg-config --cflags glib-2.0)
check-json: $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

check-json.o: check-json.c
json-lexer.o: json-lexer.c
json-parser.o: json-parser.c
json-streamer.o: json-streamer.c
gvariant-utils.o: gvariant-utils.c
gvariant-json.o: gvariant-json.c
$(OBJS): $(wildcard *.h)
