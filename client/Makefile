CC := gcc
CFLAGS := -Wall -Werror -ggdb

EXTDIR := external
COMMON := ../common
GLMATRIX := ../gl-matrix
INCDIRS := -I$(COMMON) -I$(GLMATRIX) -I../ -I../module -Iexternal

CLIENTS =
CLIENTS += green_cube
CLIENTS += red_square
CLIENTS += skybox
#CLIENTS += textured_cube
CLIENTS += textured_square

OBJECTS =
OBJECTS += safemalloc.o
OBJECTS += memory_layout.o
OBJECTS += geometry.o
OBJECTS += texture.o

LINKS := -lm -lvroom-client

all: clients

clients: $(OBJECTS) $(CLIENTS)

%: %.c $(OBJECTS)
	$(CC) $(CFLAGS) $(LINKS) $(INCDIRS) -o $@ $(OBJECTS) $(GLMATRIX)/gl-matrix.o $<

%.o: %.c
	$(CC) $(CFLAGS) $(LINKS) $(INCDIRS) -c -o $@ $<

clean:
	rm -f $(OBJECTS)
	rm -f $(CLIENTS)
