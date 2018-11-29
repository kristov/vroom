CC := gcc
CFLAGS := -Wall -Werror -ggdb

COMMON = common

OBJECTS =
OBJECTS += gl.o
OBJECTS += object.o
OBJECTS += ogl_shader_loader.o
OBJECTS += opengl_stereo.o
OBJECTS += render_vm.o
OBJECTS += runtime.o
OBJECTS += scene.o
OBJECTS += server.o

LINKS := -ldl -lm -lpthread

EXTGL := -lGL -lGLU -lglut
INCD := -I$(COMMON)
LINKD :=
DEFS :=
MAINSRC := main_glut.c

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

all: x11-server

x11-server: vroom-server
eglbcm-server: vroom-server
eglkms-server: vroom-server

commonlibs:
	cd $(COMMON) && make

vroom-server: $(MAINSRC) commonlibs $(OBJECTS)
	$(CC) $(CFLAGS) $(DEFS) $(LINKD) $(INCD) $(LINKS) $(EXTGL) -o $@ $(OBJECTS) $(COMMON)/esm.o $(COMMON)/safemalloc.o $<

%.o: %.c %.h
	$(CC) $(CFLAGS) $(DEFS) $(INCD) -c -o $@ $<

clean:
	rm -f *.o
	rm -f vroom-server
	cd $(COMMON) && make clean

