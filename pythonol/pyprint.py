# -*- coding: ISO-8859-1 -*-

######################
# PyPrint 
#
# Copyright (c) 2002-2004 
# Erica Andrews
# PhrozenSmoke ['at'] yahoo.com
# http://pythonol.sourceforge.net/
#
# This software is 
# distributed under the 
# terms of the GNU 
# General Public License.
######################

import copy,random
from pyfilter import *
from pyprintcommon import *


icondict={
	"/ _File/_Close": STOCK_QUIT,
	"/_Help/_About PyPrint...":STOCK_DIALOG_INFO,
	     }

def setup_menu_images(itemf):
	for ii in icondict.keys():
		try:
			men=itemf.get_widget(ii.replace("_",""))
			meni=Image()
			meni.set_from_stock(icondict[ii],ICON_SIZE_MENU)
			meni.show()
			men.set_image(meni)
		except:
			pass

class pyprintwin:
    def __init__(self) :
	pyprintwin=Window(WINDOW_TOPLEVEL)
	pyprintwin.set_wmclass(WINDOW_WMC,WINDOW_WMN)
	set_window_icon(pyprintwin, STOCK_PRINT,1)
	pyprintwin.set_title('PyPrint '+PYPRINT_VERSION+': Print A Document')
	pyprintwin.set_position(WIN_POS_CENTER)
	self._root=pyprintwin
	pyprintwin.realize()
	self.pyprintwin=pyprintwin
	mainvbox=VBox(0, 9)
	self.mainvbox=mainvbox
	mymenubar=MenuBar()
	self.mymenubar=mymenubar

        ag = AccelGroup()
        itemf = ItemFactory(MenuBar, "<main>", ag)
        itemf.create_items([
            # path              key           callback    action#  type
  ("/ _File",  "<alt>F",  None,  0, "<Branch>"),
  ("/ _File/_Close", "<control>Q", self.doQuit, 10, "<ImageItem>", STOCK_NO),
  ("/_Help",  "<alt>H",  None, 16, "<LastBranch>"), 
  ("/_Help/_About PyPrint...", "F2", self.doAbout, 17, "<ImageItem>", STOCK_NO)
        ])
        pyprintwin.add_accel_group(ag)
        mymenubar = itemf.get_widget("<main>")
	self.ag=ag
	self.itemf=itemf
	setup_menu_images(itemf)
        mymenubar.show()

	mainvbox.pack_start(mymenubar, 0, 0, 0)
	pic=loadImage(getPixDir()+"pyprint_printer_big.gif",pyprintwin)
	if pic: mainvbox.pack_start(pic,0,0,0)
	table1=Table(2, 3, 0)
	table1.set_row_spacings(8)
	table1.set_col_spacings(10)
	table1.set_border_width(5)
	self.table1=table1
	label7=Label('File:')
	label7.set_alignment(0, 0.5)
	self.label7=label7
	table1.attach(label7, 0, 1, 0, 1,(FILL),(0), 0, 0)
	label8=Label('Printer:')
	label8.set_alignment(0, 0.5)
	self.label8=label8
	table1.attach(label8, 0, 1, 1, 2,(FILL),(0), 0, 0)
	file_text=Entry()
	self.file_text=file_text
	file_text.set_editable(0)
	table1.attach(file_text, 1, 2, 0, 1,(EXPAND+FILL),(0), 0, 0)
	printercombo=Combo()
	self.printercombo=printercombo
	printer_entry=printercombo.entry
	printer_entry.set_editable(0)
	self.printer_entry=printer_entry
	table1.attach(printercombo, 1, 2, 1, 2,(EXPAND+FILL),(0), 0, 0)
	browsebutt=getPixmapButton(STOCK_OPEN,"Browse...",pyprintwin)
	browsebutt.connect("clicked",self.browseFile)
	browsebutt.set_sensitive(0)
	self.browsebutt=browsebutt
	table1.attach(browsebutt, 2, 3, 0, 1,(FILL),(0), 0, 0)
	refreshbutton=getPixmapButton(STOCK_REFRESH,"Refresh Printer List",pyprintwin)
	refreshbutton.connect("clicked",self.loadPrinterList)
	self.refreshbutton=refreshbutton
	table1.attach(refreshbutton, 2, 3, 1, 2,(FILL),(0), 0, 0)
	mainvbox.pack_start(table1, 0, 0, 0)
	hseparator1=HSeparator()
	self.hseparator1=hseparator1
	mainvbox.pack_start(hseparator1, 0, 0, 0)
	hbox3=HBox(0, 2)
	hbox3.set_border_width(5)
	self.hbox3=hbox3
	label3=Label('Pages:    ')
	self.label3=label3
	hbox3.pack_start(label3, 0, 0, 0)
	allradio=RadioButton(None, 'All pages')
	self.allradio=allradio
	hbox3.pack_start(allradio, 0, 0, 0)
	label4=Label('       ')
	self.label4=label4
	hbox3.pack_start(label4, 1, 1, 0)
	selectedradio=RadioButton(self.allradio, 'Selected pages:')
	self.selectedradio=selectedradio
	hbox3.pack_start(selectedradio, 0, 0, 0)
	selpages_text=Entry()
	self.selpages_text=selpages_text
	selpages_text.set_text("1-5,8,9,23")
	#selpages_text.set_editable(0)
	hbox3.pack_start(selpages_text, 1, 1, 0)
	mainvbox.pack_start(hbox3, 0, 0, 0)
	hbox4=HBox(0, 10)
	hbox4.set_border_width(5)
	self.hbox4=hbox4
	label5=Label('# of Copies:')
	self.label5=label5
	hbox4.pack_start(label5, 0, 0, 0)
	copies_text=Entry()
	copies_text.set_size_request(40, -1)
	self.copies_text=copies_text
	copies_text.set_text("1")
	hbox4.pack_start(copies_text, 0, 0, 0)
	mainvbox.pack_start(hbox4, 0, 0, 0)
	hbox5=HBox(0, 10)
	hbox5.set_border_width(5)
	self.hbox5=hbox5
	label6=Label('Print As:')
	self.label6=label6
	hbox5.pack_start(label6, 0, 0, 0)
	printascombo=Combo()
	printascombo.set_sensitive(0)
	self.printascombo=printascombo
	printas_entry=printascombo.entry
	printas_entry.set_editable(0)
	self.printas_entry=printas_entry
	printascombo.set_popdown_strings(['System default','Text','HTML','Image'])
	hbox5.pack_start(printascombo, 0, 0, 0)
	helpbutt=getPixmapButton(STOCK_DIALOG_QUESTION,"Help",pyprintwin)
	helpbutt.connect("clicked",self.showHelp)
	self.helpbutt=helpbutt
	hbox5.pack_start(helpbutt, 0, 0, 0)
	mainvbox.pack_start(hbox5, 0, 0, 0)
	hboxy=HBox()
	hboxy.pack_start(Label("  "),1,1,2)
	prevbutt=getPixmapButton(STOCK_FIND,"Print Preview  (in ghostview)",pyprintwin)
	prevbutt.connect("clicked",self.doPrintPreview)
	prevbutt.set_border_width(5)
	self.prevbutt=prevbutt
	hboxy.pack_start(prevbutt,0,0,1)
	hboxy.pack_start(Label("  "),1,1,2)
	mainvbox.pack_start(hboxy, 0, 0, 0)
	hbox6=HBox(1, 13)
	hbox6.set_border_width(5)
	self.hbox6=hbox6
	printbutt=getPixmapButton(STOCK_PRINT," Print Now!",pyprintwin)
	printbutt.connect("clicked",self.doPrint)
	self.printbutt=printbutt
	hbox6.pack_start(printbutt, 0, 1, 0)
	quitbutt=getPixmapButton(STOCK_CANCEL,"CANCEL",pyprintwin)
	quitbutt.connect("clicked",self.doQuit)
	self.quitbutt=quitbutt
	hbox6.pack_start(quitbutt, 0, 1, 0)
	mainvbox.pack_start(hbox6, 0, 0, 0)
	mainvbox.pack_start(Label("  EXPERIMENTAL: PyPrint supports only local LPRng/LPR-based printers.  "), 0, 0, 2)
	pyprintwin.add(mainvbox)
	pyprintwin.connect("destroy",self.doQuit)	
	#pyprintwin.set_modal(1)
	self.TEMP_FILES=[]
	text_exts=['txt','text','cnf','mf','ini','log','sgml','dtd','css','js','properties','conf','idl','diz','desktop','mime','keys','rc','inf','bat','cf','jsp','asp ','dir','scale','c','cpp','h','sql','java','csv','tsv','tcl','tk','py','wml','xml','xsl','rdf','php','wrl','php3','php4','gtkrc','pl','csh','cshrc','bashrc','sh','bash','bsh','ksh','zsh','theme','applnk','glade']
	self.text_exts=text_exts
	pyprintwin.show_all()
	self.deleteTempFiles()
	self.checkPath()
	self.loadPrinterList()

    def getRandomFileName(self,extension=".ps"):
	tmpf= "/tmp/pyprint_copy"+str(random.randrange(9999))+str(random.randrange(999))+extension	
	self.TEMP_FILES.append(tmpf)
	return tmpf

    def deleteTempFiles(self):	
	for ii in self.TEMP_FILES:
		os.popen("rm -f "+ii+" > /dev/null")

    def escapeFile(self,f_name):
	ff=f_name
	escape=[" "]
	for ii in escape:
		ff=ff.replace(ii,"\\"+ii)
	return ff

    def getFileExtension(self,f_name):
	if f_name.rfind(".") > -1:
		exten=f_name[f_name.rfind("."):len(f_name)].strip().replace(".","")
		if exten.find("/") > -1: exten=""
		return exten.lower()
	return ""

    def doPrintPreview(self,*args):
	f_name=self.getFile()
	if not f_name: return
	global HAVE_GHOSTVIEW
	if HAVE_GHOSTVIEW=="0":
		self.showMessgage("Sorry, you don't have the 'ghostview' executable on your PATH.\nYou need to either install ghostview or\nedit the pyprint.py script to use another Postscript-capable viewer (ggv, kghostview, etc).")
		return
	pages=self.getPages()
	if not len(pages): return
	ghosty=self.convertToPostscript(f_name)
	# add psselect stuff here
	if len(ghosty): # lauch ghostview
		os.popen("ghostview "+ghosty+" &")

    def doPrint(self,*args):
	f_name=self.getFile()
	if not f_name: return
	global HAVE_LPR
	if HAVE_LPR==0:
		self.showMessage("Sorry, you cannot print with PyPrint because the executable\n'lpr' is not on your PATH.")
		return
	pages=self.getPages()
	if not len(pages): return
	printer=self.getPrinter()
	if not printer: return
	if printer=="NO PRINTERS FOUND":
		self.showMessage("No printers available for printing!\nPrinters must be registed with lpr/lpd to be usable by PyPrint.")
		return
	copies=self.getCopies()
	if not copies > 0: return
	ghosty=self.convertToPostscript(f_name)
	if len(ghosty): # lauch ghostview
		xread=readOSLines("lpr -Zcopies="+str(copies).strip()+" -P "+printer+" "+ghosty)
		self.showMessage("The file '"+f_name+"' was sent to printer '"+printer+"'\nwith the following feedback:"+self.formatFeedback(xread))

    def selectPages(self,f_name):
	pages=self.getPages()
	#print "pages: "+pages
	if pages.lower()=="all": return f_name
	ff_name=self.getRandomFileName()
	xread=readOSLines("psselect -p"+pages+" "+f_name+" "+ff_name)
	if not os.path.exists(ff_name): 
		self.showMessage("psselect: There was an error extracting the 'Selected Pages' from this file.\nThe warnings and errors (if any) were as follows:"+self.formatFeedback(xread))
		return ""
	return ff_name

    def convertToPostscript(self,f_name):
	if self.isPS(f_name): return f_name # it's already postscript, do nothing
	# run it through a2ps and hope for the best - 'System default' or 'Text' with a known text extension
	global HAVE_A2PS
	if HAVE_A2PS==0:
		self.showMessage("Sorry, you don't have the executable 'a2ps' on your system.\nCannot print or preview as 'Text'.")
		return ""
	# if 'Text', copy to a .txt extension to force text printing
	ff_name=f_name
	gtemp=self.getRandomFileName()
	xread=readOSLines("a2ps -o "+gtemp+" "+ff_name)
	if os.path.exists(gtemp): return self.selectPages(gtemp)
	self.showMessage("a2ps: There was an error converting the selected file to Postscript for printing.\nThe warnings and errors (if any) were as follows:"+self.formatFeedback(xread))
	return ""

    def formatFeedback(self,xread):
	sread="\n\n"
	for ii in xread:
		sread=sread+ii+"\n"
	return sread

    def isText(self,f_ext):
	if f_ext in self.text_exts: return 1
	return 0

    def isPS(self,f_name):
	if self.getFileExtension(f_name)=="ps": return 1
	return 0

    def browseFile(self,*args):
	SELECT_A_FILE(self.browseFileCB,FILE_SELECTOR_TITLE,DIALOG_WMC,DIALOG_WMN,None,"OK","Cancel")

    def browseFileCB(self,*args):
	f=GET_SELECTED_FILE()
	if f: self.setFile(f)

    def setFile(self,myfilename):
	if not str(myfilename).strip(): return
	if not str(myfilename).strip().endswith("/"): 
		self.file_text.set_text(str(myfilename).strip())
		self.printas_entry.set_text('Text')

    def getFile(self):
	return self.escapeFile(self.file_text.get_text().strip())

    def showMessage(self,message_text):
	return showMessage(message_text)

    def showConfirm(self,message_text):
	return showConfirm(message_text)

    def doAbout(self,*args):
	showAbout("PyPrint is a simple Gtk-based document printing tool written in 100% Python. It is intended to be a stand-alone application for printing selected documents to LPRng and LPR-based printer spools. This is ALPHA code. PyPrint has only been tested with LPRng 3.8.12 on local printers. PyPrint requires the following executables on your PATH: lpstat, lpr, a2ps, psselect, ghostview  This is special, modified version of PyPrint for Pythoñol.")

    def showHelp(self,*args):
	showHelp("PyPrint only works with LPR/LPRng-based local printers.  PyPrint requires LPRng/LPR (with the 'lpd' daemon), ghostscript & ghostview, and a2ps (any2ps). It is known to work very well with Apsfilter.  Please be aware that the '# of Copies' field is not supported on all implementations of LPR/LPRng. Some systems will only allow you to print one copy at a time.  Check the documentation for your particular version of LPR/LPRng.  PyPrint is a simple Gtk-based document printing tool written in 100% Python. It is intended to be a stand-alone application for printing selected documents to LPRng and LPR-based printer spools. This is ALPHA code. PyPrint has only been tested with LPRng 3.8.12 on local printers. PyPrint requires the following executables on your PATH: lpstat, lpr, a2ps, psselect, ghostview.  This is a special, modified version of PyPrint for Pythoñol, configured for text-only printing.","PyPrint: Print As HELP")

    def doQuit(self,*args) :
	self.deleteTempFiles()
	#os.popen("rm -f /tmp/pyprint* > /dev/null")  # if you are running PyPrint on a MULTI-user system, comment out this line!
	os.popen("rm -f "+self.getFile())
	self.pyprintwin.hide()
	self.pyprintwin.destroy()
	self.pyprintwin.unmap()

    def checkPath(self) :
	global HAVE_A2PS
	global HAVE_LPR
	global HAVE_PSSELECT
	global HAVE_GHOSTVIEW
	HAVE_LPR=1
	HAVE_A2PS=1
	HAVE_PSSELECT=1
	HAVE_GHOSTVIEW=1
	#executables=['lpstat','lpr','a2ps','html2ps','convert','psselect','ps2pdf','pdf2ps','ghostview']
	executables=['lpstat','lpr','a2ps','psselect','ghostview']
	for iexec in executables:
		if not isOnPath(iexec): 
			self.showMessage("The following executable does not appear to be on your PATH:\n\n"+iexec+"\n\nYou may experience problems running PyPrint properly.\nSome features may be disabled.\nPlease install '"+iexec+"' into your PATH first.")
			if iexec=="lpr": HAVE_LPR=0
			if iexec=="a2ps": HAVE_A2PS=0
			if iexec=="psselect": HAVE_PSSELECT=0
			if iexec=="ghostview": HAVE_GHOSTVIEW=0

    def loadPrinterList(self,*args) :
	printl=getPrinterNames()
	if not len(printl): printl=['NO PRINTERS FOUND']
	self.printercombo.set_popdown_strings(printl)
	self.printer_entry.set_text(printl[0])

    def getPrinter(self) :
	return self.printer_entry.get_text().strip()

    def getPrintType(self) :
	return self.printas_entry.get_text().strip()

    def getCopies(self) :
	try:
		ii=int(str(self.copies_text.get_text()).strip())
		if ii > 0: return ii
		self.showMessage("The # of Copies field must be at least 1.")
		return -1		
	except:
		self.showMessage("The # of Copies field must contain a NUMBER (1 or above).")
		return -1

    def getPages(self) :
	try:
		if self.allradio.get_active(): return "ALL"
		global HAVE_PSSELECT
		if HAVE_PSSELECT==0:
			self.showMessage("Sorry, you do not have the 'psselect' executable in your PATH.\nWithout 'psselect', you may only print ALL pages (the entire document).")
			return ""
		pages=self.selpages_text.get_text().replace(" ","").replace(" ","").replace(" ","").replace(" ","").strip().lower()
		acceptable=['0','1','2','3','4','5','6','7','8','9','-',',']
		for ii in pages:
			if not ii in acceptable: 
				self.showMessage("The 'Selected Pages' field may only contain numbers, commas, and hypens.")
				return ""
		if pages.find("0-") > -1:  # a page range starts with zero
			self.showMessage("Invalid value for 'Selected Pages'.\nPage ranges cannot start with 0, but can start with 1.")
			return ""
		if pages.startswith("-") or pages.endswith("-"):  # a page range starts with zero
			self.showMessage("Invalid value for 'Selected Pages'.\nThis value may not START or END with a hypen.")
			return ""
		if pages.startswith(",") or pages.endswith(","):  # a page range starts with zero
			self.showMessage("Invalid value for 'Selected Pages'.\nThis value may not START or END with a comma.")
			return ""
		return pages
	except:
		self.showMessage("Invalid value for 'Pages'.")
		return ""


def makeTempFile(temp_data):
	tempf=tmpf= "/tmp/pyprint_copy"+str(random.randrange(9999))+str(random.randrange(999))+".txt"
	try:
		f=open(tempf,"w")
		f.write(unfilter(temp_data))
		f.flush()
		f.close()
		return tempf
	except:
		return None
