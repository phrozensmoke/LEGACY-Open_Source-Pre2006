NAME = _sqlite
USER = owens
GROUP = wheel
PREFIX = /usr
BINDIR = $(PREFIX)/bin
LIBDIR = $(PREFIX)/lib
INCDIR = $(PREFIX)/include/sdbc
MANDIR = $(PREFIX)/man
INSTALL = install


all:	preamble lib databases

preamble:
	@echo "SQLite Python Extension Library."
	@echo "Michael Owens"
	@date
	@echo "Century 21 Mike Bowman, Inc."

lib:	shared

shared_preamble:
	@echo
	@echo "BUILDING SHARED LIBRARY:"

shared:	shared_preamble $(LIBFILES)
	# # $(CC) -shared -o $(S_LIB) $(LIBFILES) $(LIBS) $(LDFLAGS)
	python sqlite_setup.py build
	cp -f ./build/lib*/_sqlite.so ./sqlite/
	rm -f _sqlite.o
	strip -s ./sqlite/_sqlite.so
	rm -fr ./build

db_preamble:
	@echo " "
	@echo "BUILDING DICTIONARY DATABASES"
	@echo "FOR PYTHONOL...PLEASE WAIT."
	@date
	@echo " "

databases: db_preamble
	python ./dictionary/makedb.py
	@echo " "
	@echo "DONE:  Now, type 'make install' or './PyInstallShield' to install Pythonol. "
	@echo " "

install: 
	./PyInstallShield
