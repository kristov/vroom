CC := gcc
CFLAGS := -Wall -Werror -ggdb

OBJECTS =
OBJECTS += pb.o

all: protobuf $(OBJECTS)

%.o: %.c %.h
	$(CC) $(CFLAGS) -c -o $@ $<

protobuf: vroom.proto
	protoc-c --c_out=. vroom.proto
	mv vroom.pb-c.c pb.c
	mv vroom.pb-c.h pb.h
	sed -i 's/vroom.pb-c.h/pb.h/g' pb.c

clean:
	rm -f $(OBJECTS) pb.c pb.h
