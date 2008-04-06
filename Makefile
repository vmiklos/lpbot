CFLAGS += $(shell pkg-config --cflags gobject-2.0) -g -Wall
LDFLAGS += $(shell pkg-config --libs gobject-2.0)
CFLAGS += $(shell pkg-config --cflags libxml-2.0)
LDFLAGS += $(shell pkg-config --libs libxml-2.0)

lpbot: lpbot.o config.o

config.o: config.c lpbot.h

doc: HEADER.html Changelog

HEADER.html: README Makefile
	ln -s README HEADER.txt
	asciidoc -a toc -a numbered -a sectids -a encoding=iso-8859-2 -a lang=hu HEADER.txt
	rm HEADER.txt

Changelog: .git/refs/heads/master
	git log --no-merges |git name-rev --tags --stdin >Changelog
