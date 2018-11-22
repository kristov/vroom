CC := gcc
CFLAGS := -Wall -Werror -ggdb

OBJECTS := mat2.o mat4.o mat3.o vec3.o vec2.o vec4.o quat.o
HEADERS := $(OBJECTS:.o=.h)

all: $(OBJECTS) gl-matrix.o gl-matrix.h

%.o: %.c %.h
	$(CC) $(CFLAGS) -c -o $@ $<

gl-matrix.o:
	ld -r $(OBJECTS) -o gl-matrix.o

gl-matrix.h: $(OBJECTS)
	cat $(HEADERS) > $@

clean:
	rm -rf *.o
	rm gl-matrix.h
