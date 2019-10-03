SHAREDDIR=/usr/local/share/gyach
BINDIR=/usr/local/bin
WIN32DIR=/usr/lib/win32

echo "Installing GYach Enhanced"
cp -f  ./gyach/src/gyach  ${BINDIR}/gyach
chmod 755  ${BINDIR}/gyach
rm -fr  ${SHAREDDIR}
mkdir  ${SHAREDDIR}/ &> /dev/null
mkdir  ${SHAREDDIR}/doc/ &> /dev/null
mkdir  ${SHAREDDIR}/locale/ &> /dev/null
mkdir  ${SHAREDDIR}/locale/es/ &> /dev/null
mkdir  ${SHAREDDIR}/locale/es/LC_MESSAGES/ &> /dev/null
mkdir  ${SHAREDDIR}/locale/fr/ &> /dev/null
mkdir  ${SHAREDDIR}/locale/fr/LC_MESSAGES/ &> /dev/null
echo "Installing GYach Enhanced Icons"
cp -f  ./gyach/pixmaps/gyach-icon.xpm  ${SHAREDDIR}/pixmaps/
cp -f  ./gyach/pixmaps/gyach-icon_48.png  ${SHAREDDIR}/pixmaps/
cp -f  ./gyach/pixmaps/gyach-icon_32.png  ${SHAREDDIR}/pixmaps/
echo "Installing GYach Enhanced Smileys"
cp -fr  ./gyach/smileys  ${SHAREDDIR}/
echo "Installing GYach Enhanced Sounds"
cp -fr  ./gyach/sounds  ${SHAREDDIR}/
#  echo "Installing GYach Enhanced TUXVironments"
#  cp -fr  ./gyach/tuxvironments  ${SHAREDDIR}/
#  echo "Installing GYach Enhanced Audibles"
#  cp -fr  ./gyach/audibles  ${SHAREDDIR}/
echo "Installing GYach Enhanced Locale Support"
cp -f  ./gyach/locale/es/LC_MESSAGES/gyach.mo  ${SHAREDDIR}/locale/es/LC_MESSAGES/gyach.mo
cp -f  ./gyach/locale/fr/LC_MESSAGES/gyach.mo  ${SHAREDDIR}/locale/fr/LC_MESSAGES/gyach.mo
echo "Installing GYach Enhanced Documentation"
cp -f  ./gyach/COPYING  ${SHAREDDIR}/doc/LICENSE
cp -f  ./gyach/BUGS  ${SHAREDDIR}/doc/
cp -f  ./gyach/PHROZEN_SMOKE_README.txt  ${SHAREDDIR}/doc/
cp -f  ./gyach/README  ${SHAREDDIR}/doc/
cp -f  ./gyach/AUTHORS  ${SHAREDDIR}/doc/
cp -f  ./gyach/ChangeLog  ${SHAREDDIR}/doc/
cp -f  ./gyach/gyache-help.html  ${SHAREDDIR}/doc/
cp -f  ./gyach/gyache-help-short.txt  ${SHAREDDIR}/doc/
cp -f  ./gyach/gyache-ylinks.html  ${SHAREDDIR}/doc/
cp -f  ./gyach/sample.gyachrc  ${SHAREDDIR}/doc/
echo "Installing TrueSpeech Codec"
mkdir  ${WIN32DIR}/ &> /dev/null
cp -f ./pytsp/codec/tssoft32.acm ${WIN32DIR}/
cp -f ./pytsp/codec/tsd32.dll ${WIN32DIR}/
echo "Installing Py! Voice Chat for GYach Enhanced"
mkdir  ${SHAREDDIR}/pyvoice/ &> /dev/null
mkdir  ${SHAREDDIR}/pyvoice/pixmaps/ &> /dev/null
mkdir  ${SHAREDDIR}/pyvoice/doc/ &> /dev/null
mkdir  ${SHAREDDIR}/pyvoice/locale/ &> /dev/null
mkdir  ${SHAREDDIR}/pyvoice/locale/es/ &> /dev/null
mkdir  ${SHAREDDIR}/pyvoice/locale/es/LC_MESSAGES/ &> /dev/null
mkdir  ${SHAREDDIR}/pyvoice/locale/fr/ &> /dev/null
mkdir  ${SHAREDDIR}/pyvoice/locale/fr/LC_MESSAGES/ &> /dev/null
cp -f ./pyvoice/pyvoice.py  ${SHAREDDIR}/pyvoice/
cp -f ./pyvoice/pyvoiceui.py  ${SHAREDDIR}/pyvoice/
cp -f ./pyvoice/pyvoiceui-Gtk1.py  ${SHAREDDIR}/pyvoice/
chmod 755  ${SHAREDDIR}/pyvoice/pyvoice.py
chmod 755  ${SHAREDDIR}/pyvoice/pyvoiceui.py
chmod 755  ${SHAREDDIR}/pyvoice/pyvoiceui-Gtk1.py
cp -f ./pyvoice/pixmaps/*  ${SHAREDDIR}/pyvoice/pixmaps/
cp -f ./pyvoice/pytsp.py  ${SHAREDDIR}/pyvoice/
cp -f ./pyvoice/_pytsp.so  ${SHAREDDIR}/pyvoice/
cp -f ./pyvoice/tsp.header  ${SHAREDDIR}/pyvoice/
echo "Installing Py! Voice Chat Documentation"
cp -f ./pyvoice/pyvoice.html  ${SHAREDDIR}/pyvoice/doc/
cp -f ./pyvoice/pyvoiceui.html  ${SHAREDDIR}/pyvoice/doc/
cp -f ./pytsp/pytsp.html  ${SHAREDDIR}/pyvoice/doc/
cp -f ./pytsp/pyesd.html  ${SHAREDDIR}/pyvoice/doc/
cp -f ./pytsp/README.pyesd  ${SHAREDDIR}/pyvoice/doc/
cp -f ./pytsp/README.pytsp  ${SHAREDDIR}/pyvoice/doc/
cp -f ./pyvoice/AUTHORS  ${SHAREDDIR}/pyvoice/doc/
cp -f ./pyvoice/README  ${SHAREDDIR}/pyvoice/doc/README.pyvoice
cp -f ./pyvoice/LICENSE  ${SHAREDDIR}/pyvoice/doc/
cp -f ./pyvoice/locale/es/LC_MESSAGES/pyvoice.mo ${SHAREDDIR}/pyvoice/locale/es/LC_MESSAGES/pyvoice.mo
cp -f ./pyvoice/locale/fr/LC_MESSAGES/pyvoice.mo ${SHAREDDIR}/pyvoice/locale/fr/LC_MESSAGES/pyvoice.mo

echo "Installation complete."







