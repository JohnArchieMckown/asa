CFLAGS = -O2 -g
VERSION = 1.2
BINDIR = $(DESTDIR)/usr/bin
MANDIR = $(DESTDIR)/usr/man/man1
DOCDIR = $(DESTDIR)/usr/doc/asa
PAGER = less

DIST= asa.c asa.1 Makefile COPYING asa-example.doc README

all: asa

install: all
	install -d -m 755 $(DOCDIR)
	install -m 644 asa-example.doc $(DOCDIR)
	install -s -m 755 asa $(BINDIR)
	install -m 644 asa.1 $(MANDIR)

demo: all
	./asa asa-example.doc | $(PAGER)

asa.o: asa.c

asa: asa.o

checkin: $(DIST)
	ci -l $(DIST)

clean:
	rm -f asa *.o core *~ *.bak

dist: $(DIST)
	(cd ..; tar cf - `for a in $(DIST); do echo asa-$(VERSION)/$$a; done`) |\
	gzip -9 > ../asa-$(VERSION).tar.gz
