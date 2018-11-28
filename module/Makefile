CC := gcc
CFLAGS := -Wall -Werror -ggdb

MODULES =
MODULES += vroom_protocol.so
MODULES += input_libinput.so
MODULES += input_openhmd.so

all: $(MODULES)

$(OBJDIR)/vroom_pb.o: $(SRCDIR)/vroom_pb.c
	$(CC) $(CFLAGS) $(INCDIRS) -c -fPIC -o $@ $<

$(OBJDIR)/array_heap.o: $(SRCDIR)/array_heap.c
	$(CC) $(CFLAGS) $(INCDIRS) -c -fPIC -o $@ $<

$(OBJDIR)/hid_monitor.o: $(SRCDIR)/linux/hid_monitor.c
	$(CC) $(CFLAGS) $(INCDIRS) -c -fPIC -o $@ $<

$(OBJDIR)/esm.o: $(SRCDIR)/esm.c
	$(CC) $(CFLAGS) $(INCDIRS) -c -fPIC -o $@ $<

vroom_protocol.so: vroom_protocol.c vroom_pb.o array_heap.o
	$(CC) $(CFLAGS) $(INCDIRS) -c -fPIC -o vroom_protocol.o vroom_protocol.c
	$(CC) $(CFLAGS) -shared -o $@ -fPIC vroom_protocol.o vroom_pb.o array_heap.o -lrt -lev -lprotobuf-c

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
