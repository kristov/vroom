CC := gcc
CFLAGS := -Wall -Werror -ggdb

EXTDIR := external

OBJECTS =
OBJECTS += esm.o
OBJECTS += gl.o
OBJECTS += object.o
OBJECTS += ogl_shader_loader.o
OBJECTS += opengl_stereo.o
OBJECTS += render_vm.o
OBJECTS += runtime.o
OBJECTS += safemalloc.o
OBJECTS += scene.o
OBJECTS += server.o

LINKS := -ldl -lm -lpthread

EXTGL := -lGL -lGLU -lglut
INCDIRS := -I$(EXTDIR)
LINKDIRS :=
PREPROC :=

# Raspberry Pi
eglbcm-server : EXTGL := -lbcm_host -lEGL -lGLESv2
eglbcm-server : INCDIRS := -I/opt/vc/include
eglbcm-server : LINKDIRS := -L/opt/vc/lib
eglbcm-server : PREPROC := -DRASPBERRYPI

# Linux without X
eglkms-server : EXTGL := -lgbm -ldrm -lEGL -lGLESv2
eglkms-server : INCDIRS := -I/usr/include/libdrm
eglkms-server : PREPROC := -DEGLGBM

all: x11-server

x11-server: main_glut.c $(OBJECTS)
	$(CC) $(CFLAGS) $(PREPROC) $(LINKDIRS) $(INCDIRS) $(LINKS) $(EXTGL) -o vroom-server $(OBJECTS) $<

eglbcm-server: main_eglbcm.c $(OBJECTS)
	$(CC) $(CFLAGS) $(PREPROC) $(LINKDIRS) $(INCDIRS) $(LINKS) $(EXTGL) -o vroom-server $(OBJECTS) $<

eglkms-server: main_eglkms.c $(OBJECTS)
	$(CC) $(CFLAGS) $(PREPROC) $(LINKDIRS) $(INCDIRS) $(LINKS) $(EXTGL) -o vroom-server $(OBJECTS) $<

%.o: %.c %.h
	$(CC) $(CFLAGS) $(PREPROC) $(INCDIRS) -c -o $@ $<

clean:
	rm -f *.o
	rm -f vroom-server
