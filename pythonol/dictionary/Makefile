NAME = _sqlite
USER = owens
GROUP = wheel
PREFIX = /usr
BINDIR = $(PREFIX)/bin
LIBDIR = $(PREFIX)/lib
INCDIR = $(PREFIX)/include/sdbc
MANDIR = $(PREFIX)/man
INSTALL = install


all:	preamble lib

preamble:
	@echo "DICTIONARY DATABASES"
	@echo "FOR PYTHONOL"
	@date
	@echo " "

lib:	shared

shared_preamble:
	@echo
	@echo "BUILDING DICTIONARY DATABASES...PLEASE WAIT:"

shared:	shared_preamble $(LIBFILES)
	# # $(CC) -shared -o $(S_LIB) $(LIBFILES) $(LIBS) $(LDFLAGS)	
	python ./makedb.py

	

