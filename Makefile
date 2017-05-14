CC := gcc
OBJS := lib/opengl_stereo.o lib/ogl_objecttree.o lib/ogl_shader_loader.o lib/esm.o
CFLAGS := -Wall -Werror -ggdb
EXTCOM := -lrt -lev -lprotobuf-c

EXTGL := -lpthread -lGL -lGLU -lglut
INCLUDEDIRS :=
LINKDIRS :=
PREPROC :=

rpi_egl : EXTGL := -lbcm_host -lEGL -lGLESv2
rpi_egl : INCLUDEDIRS := -I/opt/vc/include
rpi_egl : LINKDIRS := -L/opt/vc/lib
rpi_egl : PREPROC := -DRASPBERRYPI

all: server client

server: src/server.c lib/vroom.pb.o lib/vrms_server.o
	$(CC) $(CFLAGS) -Iinclude $(EXTCOM) -o $@ lib/vroom.pb.o lib/vrms_server.o src/array-heap.c $<

client: src/client.c lib/vroom.pb.o lib/vrms_client.o lib/vrms_geometry.o
	$(CC) $(CFLAGS) -Iinclude $(EXTCOM) -o $@ lib/vroom.pb.o lib/vrms_client.o lib/vrms_geometry.o $<

thread: src/thread.c
	$(CC) $(CFLAGS) -Iinclude -lpthread -o $@ $<

lib/vroom.pb.o: src/vroom.pb-c.c include/vroom.pb-c.h
	$(CC) $(CFLAGS) -Iinclude -c -o $@ $<

lib/vrms_server.o: src/vrms_server.c
	$(CC) $(CFLAGS) -Iinclude -c -o $@ $<

lib/vrms_client.o: src/vrms_client.c
	$(CC) $(CFLAGS) -Iinclude -c -o $@ $<

lib/vrms_geometry.o: src/vrms_geometry.c
	$(CC) $(CFLAGS) -Iinclude -c -o $@ $<

src/vroom.pb-c.c: vroom-protobuf
include/vroom.pb-c.h: vroom-protobuf
vroom-protobuf: vroom.proto
	protoc-c --c_out=. vroom.proto
	mv vroom.pb-c.c src/
	mv vroom.pb-c.h include/

#lib/opengl_stereo.o: src/opengl_stereo.c
#	$(CC) $(CFLAGS) $(PREPROC) -Iinclude $(INCLUDEDIRS) -c -o $@ $<

#lib/ogl_objecttree.o: src/ogl_objecttree.c
#	$(CC) $(CFLAGS) $(PREPROC) -Iinclude $(INCLUDEDIRS) -c -o $@ $<

#lib/ogl_shader_loader.o: src/ogl_shader_loader.c
#	$(CC) $(CFLAGS) $(PREPROC) -Iinclude $(INCLUDEDIRS) -c -o $@ $<

#lib/esm.o: src/esm.c
#	$(CC) $(CFLAGS) $(PREPROC) -Iinclude $(INCLUDEDIRS) -c -o $@ $<

#x11_glut: src/desktop_main.c $(OBJS)
#	$(CC) $(CFLAGS) $(PREPROC) $(LINKDIRS) -Iinclude $(INCLUDEDIRS) $(EXTCOM) $(EXTGL) -o $@ $(OBJS) $<

#rpi_egl: src/raspberrypi_main.c $(OBJS)
#	$(CC) $(CFLAGS) $(PREPROC) $(LINKDIRS) -Iinclude $(INCLUDEDIRS) $(EXTCOM) $(EXTGL) -o $@ $(OBJS) $<

#x11_glut_config: src/x11_glut_config.c $(OBJS)
#	$(CC) $(CFLAGS) $(PREPROC) $(LINKDIRS) -Iinclude $(INCLUDEDIRS) $(EXTCOM) $(EXTGL) -o $@ $(OBJS) $<

clean:
	rm -f lib/*
	rm -f server client
