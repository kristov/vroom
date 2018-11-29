CC := gcc
CFLAGS := -Wall -Werror -ggdb

OBJECTS =
OBJECTS += esm.o
OBJECTS += esm_pic.o
OBJECTS += safemalloc.o
OBJECTS += safemalloc_pic.o

all: $(OBJECTS)

%.o: %.c %.h
	$(CC) $(CFLAGS) -c -o $@ $<

esm_pic.o: esm.c esm.h
	$(CC) $(CFLAGS) -c -fPIC -o $@ $<

safemalloc_pic.o: safemalloc.c safemalloc.h
	$(CC) $(CFLAGS) -c -fPIC -o $@ $<

clean:
	rm -f $(OBJECTS)
