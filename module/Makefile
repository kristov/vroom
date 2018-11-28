CC := gcc
CFLAGS := -Wall -Werror -ggdb

OBJECTS =
OBJECTS += array_heap.o

MODULES =
MODULES += vroom_protocol.so
MODULES += input_libinput.so
MODULES += input_openhmd.so

all: $(MODULES)

pb.o: pb.c
	$(CC) $(CFLAGS) -c -fPIC -o $@ $<

%.o: %.c %.h
	$(CC) $(CFLAGS) -c -fPIC -o $@ $<

vroom_protocol.so: vroom_protocol.c pb.o array_heap.o
	$(CC) $(CFLAGS) $(INCDIRS) -c -fPIC -o vroom_protocol.o vroom_protocol.c
	$(CC) $(CFLAGS) -shared -o $@ -fPIC vroom_protocol.o pb.o array_heap.o -lrt -lev -lprotobuf-c

test_rotate.so: test_rotate.c
	$(CC) $(CFLAGS) $(INCDIRS) -c -fPIC -o test_rotate.o test_rotate.c
	$(CC) $(CFLAGS) -shared -o $@ -fPIC test_rotate.o

input_hid.so: input_hid.c hid_monitor.o
	$(CC) $(CFLAGS) $(INCDIRS) -c -fPIC -o input_hid.o input_hid.c
	$(CC) $(CFLAGS) -shared -o $@ -fPIC input_hid.o hid_monitor.o -ludev

input_libinput.so: input_libinput.c
	$(CC) $(CFLAGS) $(INCDIRS) -c -fPIC -o input_libinput.o input_libinput.c
	$(CC) $(CFLAGS) -shared -o $@ -fPIC input_libinput.o -linput

input_openhmd.so: input_openhmd.c esm.o
	$(CC) $(CFLAGS) $(INCDIRS) -c -fPIC -o input_openhmd.o input_openhmd.c
	$(CC) $(CFLAGS) -shared -o $@ -fPIC input_openhmd.o esm.o -lopenhmd

clean:
	rm -f $(MODULES)
	rm -f *.o
