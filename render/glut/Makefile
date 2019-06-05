CC = gcc
CFLAGS = -Wall -Werror -ggdb

render: render.c
	$(CC) $(CFLAGS) -o $@ `pkg-config --cflags --libs gtk+-3.0` $<
