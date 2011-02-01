PROGS = check-json ghrtimer geventfd gsignalfd \
	ghrtimer-compat geventfd-compat gsignalfd-compat

JSON_OBJS = check-json.o json-lexer.o json-parser.o json-streamer.o \
	gvariant-utils.o gvariant-json.o

GLIB_CFLAGS := $(shell pkg-config --cflags glib-2.0 gobject-2.0)
GLIB_LDFLAGS := $(shell pkg-config --libs glib-2.0 gobject-2.0)
LDFLAGS = $(GLIB_LDFLAGS) $(LDFLAGS-$@)
CFLAGS = -I. $(GLIB_CFLAGS) $(CFLAGS-$@)

CFLAGS-ghrtimer.o = -DHAVE_TIMERFD -DDEMO
CFLAGS-geventfd.o = -DHAVE_EVENTFD -DDEMO
CFLAGS-gsignalfd.o = -DHAVE_SIGNALFD -DDEMO
CFLAGS-ghrtimer-compat.o = -DDEMO
CFLAGS-geventfd-compat.o = -DDEMO
CFLAGS-gsignalfd-compat.o = -DDEMO
LDFLAGS-check-json = -lcheck

.PHONY: all clean
all: $(PROGS)
clean:
	rm *.o $(PROGS)

%.o: %.c
	$(CC) -o $@ -c $< $(CFLAGS)
%-compat.o: %.c
	$(CC) -o $@ -c $< $(CFLAGS)
$(PROGS): %:
	$(CC) -o $@ $^ $(LDFLAGS)

check-json: $(JSON_OBJS)
ghrtimer: ghrtimer.o
geventfd: geventfd.o
gsignalfd: gsignalfd.o
ghrtimer-compat: ghrtimer-compat.o
geventfd-compat: geventfd-compat.o
gsignalfd-compat: gsignalfd-compat.o

ghrtimer-compat.o: ghrtimer.c
geventfd-compat.o: geventfd.c
gsignalfd-compat.o: gsignalfd.c

$(JSON_OBJS): %.o: %.c $(wildcard json-*.h) gvariant-utils.h gvariant-json.h
