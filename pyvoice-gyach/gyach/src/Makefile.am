## Process this file with automake to produce Makefile.in

SUBDIRS = src

EXTRA_DIST = gyach.glade sample.gyachrc sample.ignore.list \
	sample.ignore.regex BUGS depcomp scripts/my-gtk-config \
	README.FreeBSD README.NetBSD README.MacOSX README.Win32 \
	src/interface.c.gtk2.patch

install-data-local:
	@$(NORMAL_INSTALL)
	if test -d $(srcdir)/pixmaps; then \
	  $(mkinstalldirs) $(DESTDIR)$(pkgdatadir)/pixmaps; \
	  for pixmap in $(srcdir)/pixmaps/*; do \
	    if test -f $$pixmap; then \
	      $(INSTALL_DATA) $$pixmap $(DESTDIR)$(pkgdatadir)/pixmaps; \
	    fi \
	  done \
	fi
	if test -d $(srcdir)/smileys; then \
	  $(mkinstalldirs) $(DESTDIR)$(pkgdatadir)/smileys; \
	  for smiley in $(srcdir)/smileys/*; do \
	    if test -f $$smiley; then \
	      $(INSTALL_DATA) $$smiley $(DESTDIR)$(pkgdatadir)/smileys; \
	    fi \
	  done \
	fi

uninstall-local:
	rm -rf $(DESTDIR)$(pkgdatadir)

dist-hook:
	if test -d pixmaps; then \
	  mkdir $(distdir)/pixmaps; \
	  for pixmap in pixmaps/*; do \
	    if test -f $$pixmap; then \
	      cp -p $$pixmap $(distdir)/pixmaps; \
	    fi \
	  done \
	fi
	if test -d smileys; then \
	  mkdir $(distdir)/smileys; \
	  for smiley in smileys/*; do \
	    if test -f $$smiley; then \
	      cp -p $$smiley $(distdir)/smileys; \
	    fi \
	  done \
	fi

