#   Where is IceWMCP installed on your system?
ICEWMCPDIR=/usr/share/IceWMCP

#   The translation sub-directory of the IceWMCP installation directory
#   You should leave this alone, or language translations may not work.
LOCALEDIR=$(ICEWMCPDIR)/locale

# Where do you you want the IceWMCP-TrayIcon executable installed?
BINARYDIR=/usr/X11R6/bin

# Extra configurable options, on by default...

#   Do you have the IceWMCP System Information/Hardware Plugin? 
#   If not, you can set this to 0  (0=No, 1=Yes) :
HAVE_PLUGIN_SYSTEM=1

#   Do you have the IceWMCP PyFind2 File Finder Plugin? 
#   If not, you can set this to 0  (0=No, 1=Yes) :
HAVE_PLUGIN_PYFIND=1



all: trayicon linktray striptray mo

#create all mo files
mo:
	if test -f  icewmcp-trayicon-es.po; then\
		msgfmt -o `pwd`/icewmcp-trayicon-es.mo icewmcp-trayicon-es.po ; \
	fi;
	if test -f  icewmcp-trayicon-ru.po; then\
		msgfmt -o `pwd`/icewmcp-trayicon-ru.mo icewmcp-trayicon-ru.po ; \
	fi;
	if test -f  icewmcp-trayicon-fr.po; then\
		msgfmt -o `pwd`/icewmcp-trayicon-fr.mo icewmcp-trayicon-fr.po ; \
	fi;



trayicon:
	gcc -c -DLOCALEDIR=\"$(LOCALEDIR)\" -DHAVE_PLUGIN_SYSTEM=$(HAVE_PLUGIN_SYSTEM)   -DHAVE_PLUGIN_PYFIND=$(HAVE_PLUGIN_PYFIND) -D_REENTRANT -I/usr/include/gtk-2.0 -I/usr/lib/gtk-2.0/include -I/usr/include/atk-1.0 -I/usr/include/pango-1.0 -I/usr/X11R6/include -I/usr/include/freetype2  -I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include -I.    trayicon.c -o   trayicon.o 

linktray:
	gcc trayicon.o -o   IceWMCP-TrayIcon  -Wl,--export-dynamic -lgtk-x11-2.0 -lgdk-x11-2.0 -latk-1.0 -lgdk_pixbuf-2.0 -lm -lpangoxft-1.0 -lpangox-1.0 -lpango-1.0  -lgobject-2.0 -lgmodule-2.0 -ldl  -lglib-2.0   -lfreetype

striptray:
	strip -s IceWMCP-TrayIcon


#install mo files
install: trayicon mo trayicon-install mo-install doc-install


#install trayicon
trayicon-install:
	if test -f IceWMCP-TrayIcon; then\
		install -D --mode=755 IceWMCP-TrayIcon $(BINARYDIR)/IceWMCP-TrayIcon ; \
	fi;

#create all mo files
mo-install:
	if test -f icewmcp-trayicon-es.mo; then\
		install -D --mode=644 icewmcp-trayicon-es.mo $(LOCALEDIR)/es/LC_MESSAGES/icewmcp-trayicon.mo ; \
	fi;
	if test -f icewmcp-trayicon-ru.mo; then\
		install -D --mode=644 icewmcp-trayicon-ru.mo $(LOCALEDIR)/ru/LC_MESSAGES/icewmcp-trayicon.mo ; \
	fi;
	if test -f icewmcp-trayicon-fr.mo; then\
		install -D --mode=644 icewmcp-trayicon-fr.mo $(LOCALEDIR)/fr/LC_MESSAGES/icewmcp-trayicon.mo ; \
	fi;


doc-install:
	if test -f IceWMCP-TrayIcon-LICENSE; then\
		install -D --mode=644 IceWMCP-TrayIcon-LICENSE $(ICEWMCPDIR)/doc/IceWMCP-TrayIcon-LICENSE ; \
	fi;
	if test -f IceWMCP-TrayIcon-README.txt; then\
		install -D --mode=644 IceWMCP-TrayIcon-README.txt  $(ICEWMCPDIR)/doc/IceWMCP-TrayIcon-README.txt ; \
	fi;


clean:
	rm -f *.mo
	rm -f *.o
	rm -f POTFILES.in