LOCALEDIR=/usr/local/share/gyach/pyvoice/locale

all: mo

#create all mo files
mo:
	if test -f  pyvoice.po; then\
		msgfmt -o `pwd`/LC_MESSAGES/pyvoice.mo pyvoice.po ; \
	fi;



#install mo files
install: mo
	if test -f `pwd`/LC_MESSAGES/pyvoice.mo; then\
		install -D --mode=644 `pwd`/LC_MESSAGES/pyvoice.mo $(LOCALEDIR)/fr/LC_MESSAGES/pyvoice.mo ; \
	fi;


clean:
	rm -f *.mo
	rm -f POTFILES.in