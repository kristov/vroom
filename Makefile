CC := gcc
CFLAGS := -Wall -Werror -ggdb

SRCDIR := src
OBJDIR := lib
BINDIR := bin
TESTDIR := test
INCDIR := include
EXTDIR := external

SERVEROBJS := $(addprefix $(OBJDIR)/, vroom_pb.o vrms_object.o vrms_scene.o vrms_server.o opengl_stereo.o ogl_shader_loader.o esm.o vrms_server_socket.o array_heap.o vrms_render_vm.o)
SERVERLINKS := -lrt -lev -lprotobuf-c -lm -lpthread

CLIENTS := $(addprefix $(BINDIR)/, green_cube textured_cube red_square textured_square input_openhmd skybox input_test_rotate)
CLIENTOBJS := $(addprefix $(OBJDIR)/, vroom_pb.o vrms_client.o vrms_geometry.o esm.o)
CLIENTLINKS := -lprotobuf-c -lm

TESTS := $(addprefix $(TESTDIR)/, test_hid_device test_hid_monitor test_render_vm)

EXTGL := -lGL -lGLU -lglut
INCDIRS := -I$(INCDIR) -I$(EXTDIR)
LINKDIRS :=
PREPROC :=

eglbcm-server : EXTGL := -lbcm_host -lEGL -lGLESv2
eglbcm-server : INCDIRS := -I/opt/vc/include -I$(INCDIR)
eglbcm-server : LINKDIRS := -L/opt/vc/lib
eglbcm-server : PREPROC := -DRASPBERRYPI

eglkms-server : EXTGL := -lgbm -ldrm -lEGL -lGLESv2
eglkms-server : INCDIRS := -I/usr/include/libdrm -I$(INCDIR)
eglkms-server : PREPROC := -DEGLGBM

all: x11-server clients tests

doc: Doxyfile
	doxygen Doxyfile

$(BINDIR):
	mkdir -p $(BINDIR)

$(OBJDIR):
	mkdir -p $(OBJDIR)

x11-server: src/main_glut.c $(SERVEROBJS)
	$(CC) $(CFLAGS) $(PREPROC) $(LINKDIRS) $(INCDIRS) $(SERVERLINKS) $(EXTGL) -o vroom-server $(SERVEROBJS) $<

eglbcm-server: src/main_eglbcm.c $(SERVEROBJS)
	$(CC) $(CFLAGS) $(PREPROC) $(LINKDIRS) $(INCDIRS) $(SERVERLINKS) $(EXTGL) -o vroom-server $(SERVEROBJS) $<

eglkms-server: src/main_eglkms.c $(SERVEROBJS)
	$(CC) $(CFLAGS) $(PREPROC) $(LINKDIRS) $(INCDIRS) $(SERVERLINKS) $(EXTGL) -o vroom-server $(SERVEROBJS) $<

clients: $(CLIENTS)

$(BINDIR)/input_openhmd : EXTRALINKS := -lopenhmd
$(BINDIR)/input_usb     : EXTRALINKS := -ludev -lsqlite3 

$(BINDIR)/%: $(SRCDIR)/client/%.c $(CLIENTOBJS)
	$(CC) $(CFLAGS) $(PREPROC) $(LINKDIRS) $(INCDIRS) $(CLIENTLINKS) $(EXTRALINKS) -o $@ $(CLIENTOBJS) $<

#### BEGIN TESTS ####
tests: $(TESTS)

test/test_hid_device: lib/hid_device.o lib/test_harness.o src/test_hid_device.c
	$(CC) $(CFLAGS) -D_GNU_SOURCE -D_DEBUG $(INCDIRS) -o $@ lib/hid_device.o lib/test_harness.o src/test_hid_device.c

test/test_hid_monitor: lib/hid_monitor.o src/test_hid_monitor.c
	$(CC) $(CFLAGS) -ludev $(INCDIRS) -o $@ lib/hid_monitor.o src/test_hid_monitor.c

test/test_render_vm: lib/vrms_render_vm.o lib/test_harness.o lib/esm.o src/test_render_vm.c
	$(CC) $(CFLAGS) -lm $(INCDIRS) -o $@ lib/vrms_render_vm.o lib/test_harness.o lib/esm.o src/test_render_vm.c
#### END TESTS ####

lib/hid_monitor.o: src/linux/hid_monitor.c include/hid_monitor.h
	$(CC) $(CFLAGS) $(PREPROC) $(INCDIRS) -c -o $@ $<

$(OBJDIR)/%.o: $(SRCDIR)/%.c $(INCDIR)/%.h
	$(CC) $(CFLAGS) $(PREPROC) $(INCDIRS) -c -o $@ $<

protobuf: vroom.proto
	protoc-c --c_out=. vroom.proto
	mv vroom.pb-c.c $(SRCDIR)/vroom_pb.c
	mv vroom.pb-c.h $(INCDIR)/vroom_pb.h
	sed -i 's/vroom.pb-c.h/vroom_pb.h/g' $(SRCDIR)/vroom_pb.c

clean:
	rm -f $(OBJDIR)/*
	rm -f vroom-server
	rm -f $(BINDIR)/*
	rm -f $(TESTDIR)/test_*
