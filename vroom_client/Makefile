CC := gcc
CFLAGS := -Wall -Werror -ggdb

PROTODIR := ../protocol/c
INCDIRS := -I$(PROTODIR)
LINKS := -lprotobuf-c -lm

all: vroom-client.so

vroom_client.o: vroom_client.c vroom_client.h
	$(CC) $(CFLAGS) $(INCDIRS) -fPIC -c -o $@ $<

vroom-client.so: vroom_client.o
	$(CC) $(CFLAGS) $(LINKS) -shared -fPIC -o $@ $< $(PROTODIR)/pb_pic.o

clean:
	rm -f vroom-client.so vroom_client.o
