CC := gcc
CFLAGS := -Wall -Werror -ggdb

EXTDIR := external
PROTODIR := ../protocol/c
COMMON := ../common
GLMATRIX := ../gl-matrix
INCDIRS := -I$(PROTODIR) -I$(COMMON) -I$(GLMATRIX) -I../ -I../module -Iexternal

CLIENTS =
CLIENTS += green_cube
CLIENTS += red_square
CLIENTS += skybox
#CLIENTS += textured_cube
CLIENTS += textured_square

OBJECTS =
OBJECTS += safemalloc.o
OBJECTS += client.o
OBJECTS += memory_layout.o
OBJECTS += geometry.o
OBJECTS += texture.o

LINKS := -lprotobuf-c -lm

all: clients

clients: $(OBJECTS) $(CLIENTS)

%: %.c $(OBJECTS)
	$(CC) $(CFLAGS) $(LINKS) $(INCDIRS) -o $@ $(OBJECTS) $(PROTODIR)/pb.o $(GLMATRIX)/gl-matrix.o $<

%.o: %.c
	$(CC) $(CFLAGS) $(LINKS) $(INCDIRS) -c -o $@ $<

clean:
	rm -f $(OBJECTS)
	rm -f $(CLIENTS)
