CC := gcc
CLIENTOBJS := lib/vroom.pb.o lib/vrms_client.o lib/vrms_geometry.o lib/esm.o lib/safe_malloc.o
SERVEROBJS := lib/vroom.pb.o lib/vrms_object.o lib/vrms_scene.o lib/vrms_server.o lib/opengl_stereo.o lib/ogl_shader_loader.o lib/esm.o lib/vrms_server_socket.o lib/array_heap.o lib/safe_malloc.o
CFLAGS := -Wall -Werror -ggdb
EXTCOM := -lrt -lev -lprotobuf-c -lconfig -lm
SRVCOM := -lpthread
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
	$(CC) $(CFLAGS) $(PREPROC) $(LINKDIRS) -Iinclude $(INCLUDEDIRS) $(EXTCOM) $(SRVCOM) $(EXTGL) -o vroom-server $(SERVEROBJS) $<

egl_server: src/main_egl.c $(SERVEROBJS)
	$(CC) $(CFLAGS) $(PREPROC) $(LINKDIRS) -Iinclude $(INCLUDEDIRS) $(EXTCOM) $(SRVCOM) $(EXTGL) -o vroom-server $(SERVEROBJS) $<

clients: $(CLIENTS)

tests: $(TESTS)

test/test_hid_device: lib/hid_device.o lib/test_harness.o src/test_hid_device.c
	$(CC) $(CFLAGS) -D_GNU_SOURCE -D_DEBUG -Iinclude -o $@ lib/hid_device.o lib/test_harness.o src/test_hid_device.c

test/test_hid_monitor: src/test_hid_monitor.c lib/hid_monitor.o
	$(CC) $(CFLAGS) -ludev -Iinclude -o test/test_hid_monitor src/test_hid_monitor.c lib/hid_monitor.o

bin/green_cube: src/client/green_cube.c $(CLIENTOBJS)
	$(CC) $(CFLAGS) $(PREPROC) $(LINKDIRS) -Iinclude $(INCLUDEDIRS) $(EXTCOM) -o $@ $(CLIENTOBJS) $<

bin/textured_cube: src/client/textured_cube.c $(CLIENTOBJS)
	$(CC) $(CFLAGS) $(PREPROC) $(LINKDIRS) -Iinclude $(INCLUDEDIRS) $(EXTCOM) -o $@ $(CLIENTOBJS) $<

bin/red_square: src/client/red_square.c $(CLIENTOBJS)
	$(CC) $(CFLAGS) $(PREPROC) $(LINKDIRS) -Iinclude $(INCLUDEDIRS) $(EXTCOM) -o $@ $(CLIENTOBJS) $<

bin/textured_square: src/client/textured_square.c $(CLIENTOBJS)
	$(CC) $(CFLAGS) $(PREPROC) $(LINKDIRS) -Iinclude $(INCLUDEDIRS) $(EXTCOM) -o $@ $(CLIENTOBJS) $<

bin/mixed: src/client/mixed.c $(CLIENTOBJS)
	$(CC) $(CFLAGS) $(PREPROC) $(LINKDIRS) -Iinclude $(INCLUDEDIRS) $(EXTCOM) -o $@ $(CLIENTOBJS) $<

bin/input_openhmd: src/client/input_openhmd.c $(CLIENTOBJS)
	$(CC) $(CFLAGS) $(PREPROC) $(LINKDIRS) -Iinclude $(INCLUDEDIRS) -lopenhmd -lprotobuf-c -lm -o $@ $(CLIENTOBJS) $<

lib/vroom.pb.o: src/vroom.pb-c.c include/vroom.pb-c.h
	$(CC) $(CFLAGS) $(PREPROC) -Iinclude -c -o $@ $<

lib/vrms_server.o: src/vrms_server.c
	$(CC) $(CFLAGS) $(PREPROC) -Iinclude $(INCLUDEDIRS) -c -o $@ $<

lib/vrms_scene.o: src/vrms_scene.c
	$(CC) $(CFLAGS) $(PREPROC) -Iinclude $(INCLUDEDIRS) -c -o $@ $<

lib/vrms_object.o: src/vrms_object.c
	$(CC) $(CFLAGS) $(PREPROC) -Iinclude $(INCLUDEDIRS) -c -o $@ $<

lib/vrms_client.o: src/vrms_client.c
	$(CC) $(CFLAGS) $(PREPROC) -Iinclude $(INCLUDEDIRS) -c -o $@ $<

lib/vrms_geometry.o: src/vrms_geometry.c
	$(CC) $(CFLAGS) $(PREPROC) -Iinclude $(INCLUDEDIRS) -c -o $@ $<

lib/esm.o: src/esm.c
	$(CC) $(CFLAGS) $(PREPROC) -Iinclude $(INCLUDEDIRS) -c -o $@ $<

lib/opengl_stereo.o: src/opengl_stereo.c
	$(CC) $(CFLAGS) $(PREPROC) -Iinclude $(INCLUDEDIRS) -c -o $@ $<

lib/ogl_shader_loader.o: src/ogl_shader_loader.c
	$(CC) $(CFLAGS) $(PREPROC) -Iinclude $(INCLUDEDIRS) -c -o $@ $<

lib/safe_malloc.o: src/safe_malloc.c
	$(CC) $(CFLAGS) $(PREPROC) -Iinclude $(INCLUDEDIRS) -c -o $@ $<

lib/array_heap.o: src/array-heap.c
	$(CC) $(CFLAGS) $(PREPROC) -Iinclude $(INCLUDEDIRS) -c -o $@ $<

lib/vrms_server_socket.o: src/vrms_server_socket.c
	$(CC) $(CFLAGS) $(PREPROC) -Iinclude $(INCLUDEDIRS) -c -o $@ $<

lib/hid_device.o: src/hid_device.c
	$(CC) $(CFLAGS) $(PREPROC) -Iinclude $(INCLUDEDIRS) -c -o $@ $<

lib/test_harness.o: src/test_harness.c
	$(CC) $(CFLAGS) $(PREPROC) -Iinclude $(INCLUDEDIRS) -c -o $@ $<

lib/hid_monitor.o: src/linux/hid_monitor.c include/hid_monitor.h
	$(CC) $(CFLAGS) $(PREPROC) -Iinclude $(INCLUDEDIRS) -c -o $@ $<

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
