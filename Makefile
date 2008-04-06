CFLAGS += $(shell pkg-config --cflags gobject-2.0) -g -Wall
LDFLAGS += $(shell pkg-config --libs gobject-2.0)
CFLAGS += $(shell pkg-config --cflags libxml-2.0)
LDFLAGS += $(shell pkg-config --libs libxml-2.0)

lpbot: lpbot.o config.o

config.o: config.c lpbot.h
