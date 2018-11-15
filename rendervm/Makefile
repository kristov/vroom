CC := gcc
CFLAGS := -Wall -Werror -ggdb

OBJECTS := rendervm.o test_harness.o
TESTS := test.t

all: $(OBJECTS) tests

tests: $(TESTS) rendervm.o rendervm.c

%.o: %.c %.h
	$(CC) $(CFLAGS) -c -o $@ $<

%.t: %.c
	$(CC) $(CFLAGS) -o $@ rendervm.o test_harness.o $<

clean:
	rm -rf *.t
	rm -rf *.o
