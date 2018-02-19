CC := gcc
CFLAGS := -Wall -Werror -ggdb

SERVEROBJS := lib/vroom.pb.o lib/vrms_object.o lib/vrms_scene.o lib/vrms_server.o lib/opengl_stereo.o lib/ogl_shader_loader.o lib/esm.o lib/vrms_server_socket.o lib/array_heap.o lib/safe_malloc.o
SERVERLINKS := -lrt -lev -lprotobuf-c -lm -lpthread

CLIENTOBJS := lib/vroom.pb.o lib/vrms_client.o lib/vrms_geometry.o lib/esm.o lib/safe_malloc.o
CLIENTLINKS := -lprotobuf-c -lm
CLIENTS := bin/green_cube bin/textured_cube bin/red_square bin/textured_square bin/mixed bin/input_openhmd

TESTS := test/test_hid_device test/test_hid_monitor

EXTGL := -lGL -lGLU -lglut
INCLUDEDIRS :=
LINKDIRS :=
PREPROC :=

rpi_egl : EXTGL := -lbcm_host -lEGL -lGLESv2
rpi_egl : INCLUDEDIRS := -I/opt/vc/include
rpi_egl : LINKDIRS := -L/opt/vc/lib
rpi_egl : PREPROC := -DRASPBERRYPI

rpi_egl: pre_work egl_server client

all: pre_work server client

doc: Doxyfile
	doxygen Doxyfile

pre_work:
	mkdir -p lib/

server: src/main_glut.c $(SERVEROBJS)
	$(CC) $(CFLAGS) $(PREPROC) $(LINKDIRS) -Iinclude $(INCLUDEDIRS) $(SERVERLINKS) $(EXTGL) -o vroom-server $(SERVEROBJS) $<

egl_server: src/main_egl.c $(SERVEROBJS)
	$(CC) $(CFLAGS) $(PREPROC) $(LINKDIRS) -Iinclude $(INCLUDEDIRS) $(SERVERLINKS) $(EXTGL) -o vroom-server $(SERVEROBJS) $<

clients: $(CLIENTS)

bin/green_cube: src/client/green_cube.c $(CLIENTOBJS)
	$(CC) $(CFLAGS) $(PREPROC) $(LINKDIRS) -Iinclude $(INCLUDEDIRS) $(CLIENTLINKS) -o $@ $(CLIENTOBJS) $<

bin/textured_cube: src/client/textured_cube.c $(CLIENTOBJS)
	$(CC) $(CFLAGS) $(PREPROC) $(LINKDIRS) -Iinclude $(INCLUDEDIRS) $(CLIENTLINKS) -o $@ $(CLIENTOBJS) $<

bin/red_square: src/client/red_square.c $(CLIENTOBJS)
	$(CC) $(CFLAGS) $(PREPROC) $(LINKDIRS) -Iinclude $(INCLUDEDIRS) $(CLIENTLINKS) -o $@ $(CLIENTOBJS) $<

bin/textured_square: src/client/textured_square.c $(CLIENTOBJS)
	$(CC) $(CFLAGS) $(PREPROC) $(LINKDIRS) -Iinclude $(INCLUDEDIRS) $(CLIENTLINKS) -o $@ $(CLIENTOBJS) $<

bin/mixed: src/client/mixed.c $(CLIENTOBJS)
	$(CC) $(CFLAGS) $(PREPROC) $(LINKDIRS) -Iinclude $(INCLUDEDIRS) $(CLIENTLINKS) -o $@ $(CLIENTOBJS) $<

bin/input_openhmd: src/client/input_openhmd.c $(CLIENTOBJS)
	$(CC) $(CFLAGS) $(PREPROC) $(LINKDIRS) -Iinclude $(INCLUDEDIRS) -lopenhmd $(CLIENTLINKS) -o $@ $(CLIENTOBJS) $<

bin/input_usb: src/client/input_usb.c $(CLIENTOBJS)
	$(CC) $(CFLAGS) $(PREPROC) $(LINKDIRS) -Iinclude $(INCLUDEDIRS) -ludev -lsqlite3 $(CLIENTLINKS) -o $@ $(CLIENTOBJS) $<

#### BEGIN TESTS ####
tests: $(TESTS)

test/test_hid_device: lib/hid_device.o lib/test_harness.o src/test_hid_device.c
	$(CC) $(CFLAGS) -D_GNU_SOURCE -D_DEBUG -Iinclude -o $@ lib/hid_device.o lib/test_harness.o src/test_hid_device.c

test/test_hid_monitor: lib/hid_monitor.o src/test_hid_monitor.c
	$(CC) $(CFLAGS) -ludev -Iinclude -o $@ lib/hid_monitor.o src/test_hid_monitor.c
#### END TESTS ####

lib/vroom.pb.o: src/vroom.pb-c.c include/vroom.pb-c.h
	$(CC) $(CFLAGS) $(PREPROC) -Iinclude -c -o $@ $<

lib/hid_monitor.o: src/linux/hid_monitor.c include/hid_monitor.h
	$(CC) $(CFLAGS) $(PREPROC) -Iinclude -c -o $@ $<

lib/%.o: src/%.c include/%.h
	$(CC) $(CFLAGS) $(PREPROC) -Iinclude -c -o $@ $<

src/vroom.pb-c.c: vroom-protobuf
include/vroom.pb-c.h: vroom-protobuf
vroom-protobuf: vroom.proto
	protoc-c --c_out=. vroom.proto
	mv vroom.pb-c.c src/
	mv vroom.pb-c.h include/

clean:
	rm -f lib/*
	rm -f vroom-server
	rm -f bin/*
	rm -f test/test_*
