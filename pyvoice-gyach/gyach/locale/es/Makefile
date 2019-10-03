LOCALEDIR=/usr/local/share/gyach/locale

all: mo

#create all mo files
mo:
	if test -f  gyach.po; then\
		msgfmt -o `pwd`/LC_MESSAGES/gyach.mo gyach.po ; \
	fi;



#install mo files
install: mo
	if test -f  `pwd`/LC_MESSAGES/gyach.mo; then\
		install -D --mode=644 `pwd`/LC_MESSAGES/gyach.mo $(LOCALEDIR)/es/LC_MESSAGES/gyach.mo ; \
	fi;


clean:
	rm -f *.mo
	rm -f POTFILES.in