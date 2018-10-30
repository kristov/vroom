CC := gcc
CFLAGS := -Wall -Werror -ggdb

SRCDIR := src
OBJDIR := lib
BINDIR := bin
MODULEDIR := module
TESTDIR := test
INCDIR := include
EXTDIR := external

SERVEROBJS := $(addprefix $(OBJDIR)/, vrms_object.o vrms_scene.o vrms_server.o opengl_stereo.o ogl_shader_loader.o esm.o vrms_runtime.o vrms_render_vm.o)
SERVERLINKS := -ldl -lm -lpthread

CLIENTS := $(addprefix $(BINDIR)/, green_cube textured_cube red_square textured_square skybox)

MODULES := $(addprefix $(MODULEDIR)/, vroom_protocol.so input_libinput.so input_openhmd.so)

CLIENTOBJS := $(addprefix $(OBJDIR)/, vroom_pb.o vrms_client.o vrms_geometry.o esm.o)
CLIENTLINKS := -lprotobuf-c -lm

TESTS := $(addprefix $(TESTDIR)/, test_hid_device test_hid_monitor test_render_vm)

EXTGL := -lGL -lGLU -lglut
INCDIRS := -I$(INCDIR) -I$(EXTDIR)
LINKDIRS :=
PREPROC :=

# Raspberry Pi
eglbcm-server : EXTGL := -lbcm_host -lEGL -lGLESv2
eglbcm-server : INCDIRS := -I/opt/vc/include -I$(INCDIR)
eglbcm-server : LINKDIRS := -L/opt/vc/lib
eglbcm-server : PREPROC := -DRASPBERRYPI

# Linux without X
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

modules: $(MODULES)

$(BINDIR)/%: $(SRCDIR)/client/%.c $(CLIENTOBJS)
	$(CC) $(CFLAGS) $(PREPROC) $(LINKDIRS) $(INCDIRS) $(CLIENTLINKS) $(EXTRALINKS) -o $@ $(CLIENTOBJS) $<

$(OBJDIR)/vroom_pb.o: $(SRCDIR)/vroom_pb.c
	$(CC) $(CFLAGS) $(INCDIRS) -c -fPIC -o $@ $<

$(OBJDIR)/array_heap.o: $(SRCDIR)/array_heap.c
	$(CC) $(CFLAGS) $(INCDIRS) -c -fPIC -o $@ $<

$(OBJDIR)/hid_monitor.o: $(SRCDIR)/linux/hid_monitor.c
	$(CC) $(CFLAGS) $(INCDIRS) -c -fPIC -o $@ $<

$(OBJDIR)/esm.o: $(SRCDIR)/esm.c
	$(CC) $(CFLAGS) $(INCDIRS) -c -fPIC -o $@ $<

$(MODULEDIR)/vroom_protocol.so: $(SRCDIR)/module/vroom_protocol.c $(OBJDIR)/vroom_pb.o $(OBJDIR)/array_heap.o
	$(CC) $(CFLAGS) $(INCDIRS) -c -fPIC -o $(OBJDIR)/vroom_protocol.o $(SRCDIR)/module/vroom_protocol.c
	$(CC) $(CFLAGS) -shared -o $@ -fPIC $(OBJDIR)/vroom_protocol.o $(OBJDIR)/vroom_pb.o $(OBJDIR)/array_heap.o -lrt -lev -lprotobuf-c

$(MODULEDIR)/test_rotate.so: $(SRCDIR)/module/test_rotate.c
	$(CC) $(CFLAGS) $(INCDIRS) -c -fPIC -o $(OBJDIR)/test_rotate.o $(SRCDIR)/module/test_rotate.c
	$(CC) $(CFLAGS) -shared -o $@ -fPIC $(OBJDIR)/test_rotate.o

$(MODULEDIR)/input_hid.so: $(SRCDIR)/module/input_hid.c $(OBJDIR)/hid_monitor.o
	$(CC) $(CFLAGS) $(INCDIRS) -c -fPIC -o $(OBJDIR)/input_hid.o $(SRCDIR)/module/input_hid.c
	$(CC) $(CFLAGS) -shared -o $@ -fPIC $(OBJDIR)/input_hid.o $(OBJDIR)/hid_monitor.o -ludev

$(MODULEDIR)/input_libinput.so: $(SRCDIR)/module/input_libinput.c
	$(CC) $(CFLAGS) $(INCDIRS) -c -fPIC -o $(OBJDIR)/input_libinput.o $(SRCDIR)/module/input_libinput.c
	$(CC) $(CFLAGS) -shared -o $@ -fPIC $(OBJDIR)/input_libinput.o -linput

$(MODULEDIR)/input_openhmd.so : EXTRALINKS := -lopenhmd

$(MODULEDIR)/input_openhmd.so: $(SRCDIR)/module/input_openhmd.c $(OBJDIR)/esm.o
	$(CC) $(CFLAGS) $(INCDIRS) -c -fPIC -o $(OBJDIR)/input_openhmd.o $(SRCDIR)/module/input_openhmd.c
	$(CC) $(CFLAGS) -shared -o $@ -fPIC $(OBJDIR)/input_openhmd.o $(OBJDIR)/esm.o $(EXTRALINKS)

#### BEGIN TESTS ####
tests: $(TESTS)

test/test_hid_device: lib/hid_device.o lib/test_harness.o src/test_hid_device.c
	$(CC) $(CFLAGS) -D_GNU_SOURCE -D_DEBUG $(INCDIRS) -o $@ lib/hid_device.o lib/test_harness.o src/test_hid_device.c

test/test_hid_monitor: lib/hid_monitor.o src/test_hid_monitor.c
	$(CC) $(CFLAGS) -ludev $(INCDIRS) -o $@ lib/hid_monitor.o src/test_hid_monitor.c

test/test_render_vm: lib/vrms_render_vm.o lib/test_harness.o lib/esm.o src/test_render_vm.c
	$(CC) $(CFLAGS) -lm $(INCDIRS) -o $@ lib/vrms_render_vm.o lib/test_harness.o lib/esm.o src/test_render_vm.c
#### END TESTS ####

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
	rm -f $(MODULEDIR)/*.so
