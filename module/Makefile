CC := gcc
CFLAGS := -Wall -Werror -ggdb

COMMON = ../common
PROTOCOL = ../protocol

INCD = -I../ -I$(COMMON)

OBJECTS =
OBJECTS += array_heap.o

MODULES =
MODULES += vroom_protocol.so
MODULES += input_libinput.so
MODULES += input_openhmd.so
MODULES += drm_manager.so

all: $(MODULES)

%.o: %.c %.h
	$(CC) $(CFLAGS) -c -fPIC -o $@ $<

vroom_protocol.so: vroom_protocol.c $(PROTOCOL)/c/pb_pic.o $(COMMON)/safemalloc_pic.o array_heap.o
	$(CC) $(CFLAGS) $(INCD) -I$(PROTOCOL)/c -c -fPIC -o vroom_protocol.o vroom_protocol.c
	$(CC) $(CFLAGS) -shared -o $@ -fPIC vroom_protocol.o $(PROTOCOL)/c/pb_pic.o $(COMMON)/safemalloc_pic.o array_heap.o -lrt -lev -lprotobuf-c

test_rotate.so: test_rotate.c
	$(CC) $(CFLAGS) $(INCD) -c -fPIC -o test_rotate.o test_rotate.c
	$(CC) $(CFLAGS) -shared -o $@ -fPIC test_rotate.o

input_hid.so: input_hid.c hid_monitor.o
	$(CC) $(CFLAGS) $(INCD) -c -fPIC -o input_hid.o input_hid.c
	$(CC) $(CFLAGS) -shared -o $@ -fPIC input_hid.o hid_monitor.o -ludev

input_libinput.so: input_libinput.c
	$(CC) $(CFLAGS) $(INCD) -c -fPIC -o input_libinput.o input_libinput.c
	$(CC) $(CFLAGS) -shared -o $@ -fPIC input_libinput.o -linput

input_openhmd.so: input_openhmd.c $(COMMON)/esm_pic.o $(COMMON)/safemalloc_pic.o
	$(CC) $(CFLAGS) $(INCD) -c -fPIC -o input_openhmd.o input_openhmd.c
	$(CC) $(CFLAGS) -shared -o $@ -fPIC input_openhmd.o $(COMMON)/esm_pic.o $(COMMON)/safemalloc_pic.o -lopenhmd

drm_manager.so: drm_manager.c udm.o
	$(CC) $(CFLAGS) $(INCD) -c -fPIC -o drm_manager.o drm_manager.c
	$(CC) $(CFLAGS) -shared -o $@ -fPIC drm_manager.o udm.o -ludev

clean:
	rm -f $(MODULES)
	rm -f *.o
