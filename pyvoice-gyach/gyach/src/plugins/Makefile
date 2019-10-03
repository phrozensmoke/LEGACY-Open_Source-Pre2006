PLUGDIR=/usr/local/share/gyach/plugins


MY_INCLUDES=-I/usr/include/gtk-2.0 -I/usr/lib/gtk-2.0/include -I/usr/include/atk-1.0 -I/usr/include/pango-1.0 -I/usr/X11R6/include -I/usr/include/freetype2 -I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include 

MY_LINKS=-Xlinker -Bdynamic -Wl,--export-dynamic -lgtk-x11-2.0 -lgdk-x11-2.0 -latk-1.0 -lgdk_pixbuf-2.0 -lm -lpangoxft-1.0 -lpangox-1.0 -lpango-1.0  -lgobject-2.0 -lgmodule-2.0 -ldl -lgthread-2.0 -lpthread -lglib-2.0   -lfreetype

all: photos xmms blowfish mcrypt gpgme

blowfish:
	gcc --shared  -g -O3  gyach-blowfish.c -I. -I.. -I../.. ${MY_INCLUDES}  -o gyach-blowfish.so
	strip -s gyach-blowfish.so

mcrypt:
	gcc --shared -g -O3  gyach-mcrypt.c -I. -I.. -I../.. ${MY_INCLUDES}  -o gyach-mcrypt.so -lmcrypt
	strip -s gyach-mcrypt.so

gpgme:
	gcc --shared  -g -O3  gyach-gpgme.c -I. -I.. -I../.. ${MY_INCLUDES}  -o gyach-gpgme.so -lgpgme
	strip -s gyach-gpgme.so


xmms:
	gcc --shared -g -O3  gyach-xmms.c -I. -I.. -I../..  -I/usr/include/gtk-2.0 -I/usr/lib/gtk-2.0/include -I/usr/include/atk-1.0 -I/usr/include/pango-1.0 -I/usr/X11R6/include -I/usr/include/freetype2 -I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include -I/usr/X11R6/include/xmms  -o gyach-xmms.so -L/usr/X11R6/lib -L/usr/lib -L/usr/X11R6/lib  ${MY_LINKS}  -lXi -lXext -lX11 -lm -lxmms
	strip -s gyach-xmms.so

photos:
	gcc --shared  -g -O3  gyach-photos.c -I. -I.. -I../.. ${MY_INCLUDES}  -o gyach-photos.so
	strip -s gyach-photos.so


install: 
	if test -f  `pwd`/gyach-blowfish.so; then\
		install -D --mode=755 `pwd`/gyach-blowfish.so $(PLUGDIR)/gyach-blowfish.so ; \
	fi;
	if test -f  `pwd`/gyach-mcrypt.so; then\
		install -D --mode=755 `pwd`/gyach-mcrypt.so $(PLUGDIR)/gyach-mcrypt.so ; \
	fi;
	if test -f  `pwd`/gyach-gpgme.so; then\
		install -D --mode=755 `pwd`/gyach-gpgme.so $(PLUGDIR)/gyach-gpgme.so ; \
	fi;
	if test -f  `pwd`/gyach-xmms.so; then\
		install -D --mode=755 `pwd`/gyach-xmms.so $(PLUGDIR)/gyach-xmms.so ; \
	fi;
	if test -f  `pwd`/gyach-photos.so; then\
		install -D --mode=755 `pwd`/gyach-photos.so $(PLUGDIR)/gyach-photos.so ; \
	fi;

