CC := gcc
CFLAGS := -Wall -Werror -ggdb

DESTINATION := /usr/local/lib
PROTODIR := ../protocol/c
INCDIRS := -I$(PROTODIR)
LINKS := -lprotobuf-c -lm

all: libvroom-client.so

vroom_client.o: vroom_client.c vroom_client.h
	$(CC) $(CFLAGS) $(INCDIRS) -fPIC -c -o $@ $<

libvroom-client.so: vroom_client.o
	$(CC) $(CFLAGS) $(LINKS) -shared -fPIC -o $@ $< $(PROTODIR)/pb_pic.o

install: libvroom-client.so
	cp libvroom-client.so $(DESTINATION)/libvroom-client.so.0.0.0

clean:
	rm -f libvroom-client.so vroom_client.o
