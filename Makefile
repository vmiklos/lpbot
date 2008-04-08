CFLAGS += $(shell pkg-config --cflags gobject-2.0) -g -Wall
LDFLAGS += $(shell pkg-config --libs gobject-2.0)
CFLAGS += $(shell pkg-config --cflags libxml-2.0)
LDFLAGS += $(shell pkg-config --libs libxml-2.0)
LDFLAGS += $(shell curl-config --libs)
LDFLAGS += $(shell pkg-config --libs mrss)

OBJS = servers.o users.o config.o db.o rss.o getdate.o remind.o lib.o

all: lpbot lpbotctl

lpbot: lpbot.o $(OBJS)

lpbotctl: lpbotctl.o $(OBJS)

-include .depend

.depend: $(wildcard *.c)
	gcc -MM *.c > .depend

doc: HEADER.html Changelog
	rm -rf apidocs
	doxygen

clean:
	rm -f *.o lpbot lpbotctl

HEADER.html: README Makefile
	ln -s README HEADER.txt
	asciidoc -a toc -a numbered -a sectids HEADER.txt
	rm HEADER.txt

Changelog: .git/refs/heads/master
	git log --no-merges |git name-rev --tags --stdin >Changelog
