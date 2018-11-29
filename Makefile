CC := gcc
CFLAGS := -Wall -Werror -ggdb

COMMON = common
MODULE = module
PROTOCOL = protocol

OBJECTS =
OBJECTS += gl.o
OBJECTS += object.o
OBJECTS += ogl_shader_loader.o
OBJECTS += opengl_stereo.o
OBJECTS += render_vm.o
OBJECTS += runtime.o
OBJECTS += scene.o
OBJECTS += server.o

LINKS = -ldl -lm -lpthread

EXTGL =
INCD = -I$(COMMON)
LINKD =
DEFS =
MAINSRC =

# X11 windowed client
x11-server : EXTGL = -lGL -lGLU -lglut
x11-server : DEFS = -DX11GLUT
x11-server : MAINSRC = main_glut.c

# Raspberry Pi
eglbcm-server : EXTGL := -lbcm_host -lEGL -lGLESv2
eglbcm-server : INCD += -I/opt/vc/include
eglbcm-server : LINKD := -L/opt/vc/lib
eglbcm-server : DEFS := -DRASPBERRYPI
eglbcm-server : MAINSRC := main_eglbcm.c

# Linux without X
eglkms-server : EXTGL := -lgbm -ldrm -lEGL -lGLESv2
eglkms-server : INCD += -I/usr/include/libdrm
eglkms-server : DEFS := -DEGLGBM
eglbcm-server : MAINSRC := main_eglkms.c

all: deps x11-server

x11-server: vroom-server
eglbcm-server: vroom-server
eglkms-server: vroom-server

vroom-server: $(MAINSRC) $(OBJECTS)
	$(CC) $(CFLAGS) $(DEFS) $(LINKD) $(INCD) $(LINKS) $(EXTGL) -o $@ $(OBJECTS) $(COMMON)/esm.o $(COMMON)/safemalloc.o $(MAINSRC)

%.o: %.c %.h
	$(CC) $(CFLAGS) $(DEFS) $(INCD) -c -o $@ $<

deps: commonlibs protocol-c modules

commonlibs:
	cd $(COMMON) && make

modules:
	cd $(MODULE) && make

protocol-c:
	cd $(PROTOCOL)/c && make

clean:
	rm -f *.o
	rm -f vroom-server
	cd $(COMMON) && make clean
	cd $(MODULE) && make clean
	cd $(PROTOCOL)/c && make clean

