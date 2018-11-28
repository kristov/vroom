CC := gcc
CFLAGS := -Wall -Werror -ggdb

EXTDIR := external
INCDIRS := -I../protocol -I../ -Iexternal

CLIENTS =
CLIENTS += green_cube
CLIENTS += red_square
CLIENTS += skybox
CLIENTS += textured_cube
CLIENTS += textured_square

OBJECTS =
OBJECTS += safemalloc.o
OBJECTS += vrms_client.o
OBJECTS += vrms_geometry.o

LINKS := -lprotobuf-c -lm

all: clients

clients: $(OBJECTS) $(CLIENTS)

%: %.c $(OBJECTS)
	$(CC) $(CFLAGS) $(LINKS) $(INCDIRS) -o $@ $(OBJECTS) ../protocol/pb.o $<

%.o: %.c
	$(CC) $(CFLAGS) $(LINKS) $(INCDIRS) -c -o $@ $<

clean:
	rm -f $(OBJECTS)
	rm -f $(CLIENTS)
