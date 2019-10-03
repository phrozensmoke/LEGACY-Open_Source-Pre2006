
DATADIR = $(shell gnome-config --datadir)
LOCALEDIR = $(DATADIR)/locale
MAINDIR = pyFind
DIALOGDIR = glade

PO_DIR = po
DOMAIN = pyfind

all: mo

#create all mo files
mo:
	if test -f gtksnapshot.po; then\
		msgfmt -o gtksnapshot.mo gtksnapshot.po ; \
	fi;
	if test -f ganim8ct.po; then\
		msgfmt -o ganim8ct.mo ganim8ct.po ; \
	fi;
	if test -f ganim8.po; then\
		msgfmt -o ganim8.mo ganim8.po ; \
	fi;
	if test -f ganim8-pyprint.po; then\
		msgfmt -o ganim8-pyprint.mo ganim8-pyprint.po ; \
	fi;



#install mo files
install: mo
	if test -f gtksnapshot.mo; then\
		install -D --mode=644 gtksnapshot.mo $(LOCALEDIR)/es/LC_MESSAGES/gtksnapshot.mo ; \
	fi;
	if test -f ganim8ct.mo; then\
		install -D --mode=644 ganim8ct.mo $(LOCALEDIR)/es/LC_MESSAGES/ganim8ct.mo ; \
	fi;
	if test -f ganim8.mo; then\
		install -D --mode=644 ganim8.mo $(LOCALEDIR)/es/LC_MESSAGES/ganim8.mo ; \
	fi;
	if test -f ganim8-pyprint.mo; then\
		install -D --mode=644 ganim8-pyprint.mo $(LOCALEDIR)/es/LC_MESSAGES/ganim8-pyprint.mo ; \
	fi;

clean:
	rm -f *.mo
	rm -f POTFILES.in
