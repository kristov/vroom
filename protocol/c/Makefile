CC := gcc
CFLAGS := -Wall -Werror -ggdb

OBJECTS =
OBJECTS += pb.o
OBJECTS += pb_pic.o

all: $(OBJECTS)

%.o: %.c %.h
	$(CC) $(CFLAGS) -c -o $@ $<

pb_pic.o: pb.c pb.h
	$(CC) $(CFLAGS) -c -fPIC -o $@ $<

protobuf: vroom.proto
	protoc-c --c_out=. vroom.proto
	mv vroom.pb-c.c pb.c
	mv vroom.pb-c.h pb.h
	sed -i 's/vroom.pb-c.h/pb.h/g' pb.c

vroom.proto:
	cp ../vroom.proto ./

clean:
	rm -f $(OBJECTS)
	rm -f vroom.proto
