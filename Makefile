CC?=gcc
CFLAGS?=--debug
LDFLAGS=-L/usr/local/lib -ljabra
INCLUDES=-Iinclude/

SRC = $(wildcard src/*.c)
OBJS = $(patsubst %.c, %.o, $(SRC))


all: jabra-timesync

jabra-timesync: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<
