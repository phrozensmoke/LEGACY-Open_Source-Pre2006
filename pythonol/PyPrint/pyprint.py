#! /usr/bin/env python
# -*- coding: ISO-8859-1 -*-

########################
# PyPrint / PySpool:  PyPrint
#
# Copyright 2002-2004 
# Erica Andrews
# PhrozenSmoke ['at'] yahoo.com
# http://pythonol.sourceforge.net
#
# This software is 
# distributed under the 
# terms of the GNU 
# General Public License.
########################
########################
# A simple interface for printing 
# documents on LPRng printer 
# spools
########################

import os,copy,random,conversion,magicfile
from pyprintcommon import *

class pyprintwin:
    def __init__(self) :
	pyprintwin=Window(WINDOW_TOPLEVEL)
	self._root=pyprintwin
	set_window_icon(pyprintwin,pyprint_icon_xpm,0)
        pyprintwin.set_wmclass("PyPrintDocument","PyPrintDocument") 
	pyprintwin.set_title('PyPrint '+PYPRINT_VERSION+': Print A Document')
	pyprintwin.set_position(WIN_POS_CENTER)
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
  ("/ _File/_View Printer Spool", "<control>P", self.runPySpool, 1, "<StockItem>", STOCK_ZOOM_100),
  ("/ _File/sep5", None,  None,  44, "<Separator>"),
  ("/ _File/_Quit", "<control>Q", self.doQuit, 10, "<StockItem>", STOCK_QUIT),
  ("/_Help",  "<alt>H",  None, 16, "<LastBranch>"), 
  ("/_Help/_About PyPrint...", "F2", self.doAbout, 17, "<StockItem>", STOCK_DIALOG_INFO)


        ])
        pyprintwin.add_accel_group(ag)
	self.ag=ag
	self.itemf=itemf
        mymenubar = itemf.get_widget("<main>")
        mymenubar.show()

	mainvbox.pack_start(mymenubar, 0, 0, 0)
	pic=loadImage("pyprint_printer_big.png",pyprintwin)
	if pic: mainvbox.pack_start(pic,1,1,0)
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
	file_text.set_size_request(300,-1)
	self.file_text=file_text
	table1.attach(file_text, 1, 2, 0, 1,(EXPAND+FILL),(0), 0, 0)
	printercombo=Combo()
	self.printercombo=printercombo
	printer_entry=printercombo.entry
	printer_entry.set_editable(0)
	self.printer_entry=printer_entry
	table1.attach(printercombo, 1, 2, 1, 2,(EXPAND+FILL),(0), 0, 0)
	browsebutt=getPixmapButton(STOCK_OPEN,"Browse...",pyprintwin)
	browsebutt.connect("clicked",self.browseFile)
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
	label5=Label('Number of Copies:')
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
	self.printascombo=printascombo
	printas_entry=printascombo.entry
	printas_entry.set_editable(0)
	self.printas_entry=printas_entry
	printascombo.set_popdown_strings(['System default','Text','HTML','Image'])
	hbox5.pack_start(printascombo, 0, 0, 0)
	helpbutt=getPixmapButton(STOCK_HELP,"Help",pyprintwin)
	helpbutt.connect("clicked",self.showHelp)
	self.helpbutt=helpbutt
	hbox5.pack_start(helpbutt, 0, 0, 0)
	mainvbox.pack_start(hbox5, 0, 0, 0)
	hboxy=HBox()
	hboxy.pack_start(Label("  "),1,1,2)
	prevbutt=getPixmapButton(STOCK_ZOOM_100,"Print Preview  (in ghostview)",pyprintwin)
	prevbutt.connect("clicked",self.doPrintPreview)
	prevbutt.set_border_width(5)
	self.prevbutt=prevbutt
	hboxy.pack_start(prevbutt,0,0,1)
	hboxy.pack_start(Label("  "),1,1,2)
	mainvbox.pack_start(hboxy, 0, 0, 0)
	hbox6=HBox(1, 13)
	hbox6.set_border_width(5)
	self.hbox6=hbox6
	printbutt=getPixmapButton(STOCK_PRINT,"Print Now!",pyprintwin)
	printbutt.connect("clicked",self.doPrint)
	self.printbutt=printbutt
	hbox6.pack_start(printbutt, 0, 1, 0)
	quitbutt=getPixmapButton(STOCK_QUIT,"QUIT",pyprintwin)
	quitbutt.connect("clicked",self.doQuit)
	self.quitbutt=quitbutt
	hbox6.pack_start(quitbutt, 0, 1, 0)
	mainvbox.pack_start(hbox6, 0, 0, 0)
	pyprintwin.add(mainvbox)
	pyprintwin.connect("destroy",self.doQuit)
	global TEMP_FILES
	TEMP_FILES=[]
	text_exts=['txt','text','cnf','mf','ini','log','sgml','dtd','css','js','properties','conf','idl','diz','desktop','mime','keys','rc','inf','bat','cf','jsp','asp ','dir','scale','c','cpp','h','sql','java','csv','tsv','tcl','tk','py','wml','xml','xsl','rdf','php','wrl','php3','php4','gtkrc','pl','csh','cshrc','bashrc','sh','bash','bsh','ksh','zsh','theme','applnk','glade']
	self.text_exts=text_exts
	html_exts=['html','htm','shtml','shtm','xhtml','xhtm']
	self.html_exts=html_exts
	pyprintwin.show_all()
	self.deleteTempFiles()
	self.checkPath()
	self.loadPrinterList()

    def getRandomFileName(self,extension=".ps"):
	tmpf= "/tmp/pyprint_copy"+str(random.randrange(9999))+str(random.randrange(999))+extension
	global TEMP_FILES
	TEMP_FILES.append(tmpf)
	return tmpf

    def deleteTempFiles(self):
	global TEMP_FILES
	for ii in TEMP_FILES:
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
	if not os.path.exists(f_name):
		self.showMessage("Error: The file you have selected does not exist.")
		return
	if os.path.isdir(f_name):
		self.showMessage("Error: The file you have selected is a directory, not a file.")
		return
	if os.path.getsize(f_name)<1:
		self.showMessage("Error: The file you have selected is an empty file.")
		return
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
	if not os.path.exists(f_name):
		self.showMessage("Error: The file you have selected does not exist.")
		return
	if os.path.isdir(f_name):
		self.showMessage("Error: The file you have selected is a directory, not a file.")
		return
	if os.path.getsize(f_name)<1:
		self.showMessage("Error: The file you have selected is an empty file.")
		return
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
	global HAVE_CONVERT 
	if self.isPS(f_name): return f_name # it's already postscript, do nothing
	if self.getPrintType()=="HTML": # run it through html2ps and hope for the best
		global HAVE_HTML2PS
		if HAVE_HTML2PS==0:
			self.showMessage("Sorry, you don't have the executable 'html2ps' on your system.\nCannot print as 'HTML'.")
			return ""
		global HAVE_PDF2PS
		if HAVE_PDF2PS==0:
			self.showMessage("Sorry, you don't have the executable 'pdf2ps' on your system.\nCannot print as 'HTML'.")
			return ""
		global HAVE_PS2PDF
		if HAVE_PS2PDF==0:
			self.showMessage("Sorry, you don't have the executable 'ps2pdf' on your system.\nCannot print as 'HTML'.")
			return ""
		# copy to a .html extension to force html printing
		ff_name=f_name
		ff_name=self.getRandomFileName(".html")
		os.popen("cp -f "+f_name+" "+ff_name)
		if not os.path.exists(ff_name):
			self.showMessage("Unknown error forcing the selected file into HTML format.\nBe sure that you have write access to the /tmp/ directory.")
			return ""
		gtemp=self.getRandomFileName()
		basedir=f_name[0:f_name.rfind("/")+1]
		basedir=""+basedir

		if not basedir.endswith("/"): basedir=basedir+"/"
		#print "Base: " +basedir
		xread=readOSLines("html2ps --base "+basedir+" -o "+gtemp+self.getWantImages()+self.getWantBackground()+" "+ff_name)
		if not os.path.exists(gtemp): 
			self.showMessage("html2ps: There was an error converting the selected file to Postscript for printing.\nThe warnings and errors (if any) were as follows:"+self.formatFeedback(xread))
			return ""
		# Now html2ps creates 'funny' PS that doesnt work on all printers and Postscript viewers...fixing via 'convert'
		# convert to .pdf then back to .ps fixes this problem of non-standard postscript being produced by html2ps
		gtemp2=self.getRandomFileName(".pdf")
		"""conv_stat=conversion.doConversion(gtemp,"ps",gtemp2,"pdf",1) # convert from ps to pdf
		if not conv_stat[0]==1:
			self.showMessage("convert: There was an error converting the selected file to Postscript for printing.\nThe warnings and errors (if any) were as follows:\n\n"+str(conv_stat[1]))
			return ""  """
		gtemp2=self.getRandomFileName(".pdf") # convert back to ps from pdf
		xread=readOSLines("ps2pdf  "+gtemp+" "+gtemp2)
		if  not os.path.exists(gtemp2):
			self.showMessage("ps2pdf: There was an error converting the selected file to Postscript for printing.\nThe warnings and errors (if any) were as follows:"+self.formatFeedback(xread))
			return ""
		gtemp3=self.getRandomFileName() # convert back to ps from pdf
		xread=readOSLines("pdf2ps "+gtemp2+" "+gtemp3)
		if os.path.exists(gtemp3): return self.selectPages(gtemp3)
		self.showMessage("pdf2ps: There was an error converting the selected file to Postscript for printing.\nThe warnings and errors (if any) were as follows:"+self.formatFeedback(xread))
		return ""

	if self.getPrintType()=="Image" or ((self.getPrintType()=="Text") and (not self.isText(self.getFileExtension(f_name)))): # run it through convert and hope for the best
		if HAVE_CONVERT==0:
			self.showMessage("Sorry, you don't have the executable 'convert' on your system.\nCannot print as 'Image'.")
			return ""
		ff_name=f_name
		gtemp=self.getRandomFileName()
		imtype="text"   # for 'Text' mode printing
		if self.getPrintType()=="Image": imtype=self.getImageType(ff_name)
		conv_stat=conversion.doConversion(ff_name,imtype,gtemp,"ps",1)
		if conv_stat[0]==1: return self.selectPages(gtemp)
		self.showMessage("convert: There was an error converting the selected file to Postscript for printing.\nThe warnings and errors (if any) were as follows:\n\n"+str(conv_stat[1]))
		return ""

	# run it through a2ps and hope for the best - 'System default' or 'Text' with a known text extension
	global HAVE_A2PS
	if HAVE_A2PS==0:
		self.showMessage("Sorry, you don't have the executable 'a2ps' on your system.\nCannot print as 'System default'.")
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

    def getMagicGuess(self,f_name):
	return magicfile.file(f_name)

    def isText(self,f_ext):
	if f_ext in self.text_exts: return 1
	return 0

    def isHTML(self,f_ext):
	if f_ext in self.html_exts: return 1
	return 0

    def isPS(self,f_name):
	if self.getFileExtension(f_name)=="ps": return 1
	if self.getMagicGuess(f_name)=="ps": return 1
	return 0

    def isImage(self,f_ext):
	if conversion.isValidExtension(f_ext): return 1
	return 0

    def getImageType(self,f_name):
	ff=self.getFileExtension(f_name)
	if ff:
		if self.isImage(ff): return ff
	#try magic
	mg=self.getMagicGuess(f_name)
	if mg.startswith('image/'):	
		if self.isImage(mg.replace("image/","").strip()): return mg.replace("image/","").strip()
	# lastly ask the user
	return askFormat()		

    def guessFileType(self,f_name):
	if self.isPS(f_name): return "System default"
	ff=self.getFileExtension(f_name)
	if ff:
		if self.isText(ff): return "Text"
		if self.isImage(ff): return "Image"
		if self.isHTML(ff): return "HTML"
	mg=self.getMagicGuess(f_name)
	if mg=='txt': return "Text"
	if mg=='html': return "HTML"
	if mg.startswith('image'): return "Image"
	return "System default"

    def getWantImages(self):
	if showConfirm("Do you want the IMAGES in this html document (if any) to be printed?"): return " "
	return " -T "

    def getWantBackground(self):
	if showConfirm("Do you want the BACKGROUND COLORS in this html document (if any) to be printed?"): return " -U "
	return "  "

    def file_sel_cb(self,*args):
	self.setFile(GET_SELECTED_FILE())

    def browseFile(self,*args):
	SELECT_A_FILE(self.file_sel_cb,"PyPrint: Select A File To Print",DIALOG_WMC,DIALOG_WMN,None, "OK")

    def setFile(self,myfilename):
	if not str(myfilename).strip(): return
	try:
		if os.path.isdir(str(myfilename).strip()): return
	except:
		pass
	if not str(myfilename).strip().endswith("/"): 
		self.file_text.set_text(str(myfilename).strip())
		self.printas_entry.set_text(self.guessFileType(self.getFile()).strip())
		self.deleteTempFiles()

    def getFile(self):
	return self.escapeFile(self.file_text.get_text().strip())

    def showMessage(self,message_text):
	return showMessage(message_text)

    def showConfirm(self,message_text):
	return showConfirm(message_text)

    def doAbout(self,*args):
	showAbout("PyPrint is a simple -based document printing tool written\nin 100% Python. It is intended to be a stand-alone application for printing selected documents to LPRng and LPR-based\nprinter spools. This is ALPHA code. PyPrint has only been\ntested with LPRng 3.8.12 on local printers.\nPyPrint requires the following executables on your PATH:\nlpstat, lpr, a2ps, html2ps, convert, psselect, ps2pdf, pdf2ps, ghostview")

    def showHelp(self,*args):
	showHelp("The 'Print As' options are as follows:\n\nSystem default:  This option should be good for almost all file types.  Use this option if none of the other options match your situation or seem to be working right.  This option will simple send the file to 'a2ps', which can handle almost all file types. The file will be sent to your printer in Postscript format.\n\nImage:  Use this option if you know the file you are trying to print is an image. ImageMagick's 'convert' utility will be used to send the file to your printer in Postscript format.  This is a good option to use for lesser-known image formats which might not be familiar to 'a2ps'.\n\nHTML:  Use this option if the file you are printing is in HTML format and you want it printed as it would be seen inside a web browser. This option will use the 'html2ps' utility to prepare the document to be sent to your printer in Postscript format. If you want to print the actually HTML code, you should select 'Text' instead (see below).\n\nText:  Use this option to force a file to be printed in raw, plain text.  ImageMagick's 'convert' utility will be used to read the file as raw text. It will then be sent to your printer in Postscript format.\n\nOther Notes: Please be aware that the '# of Copies' field is not supported on all implementations of LPR/LPRng. Some systems will only allow you to print one copy at a time.  Check the documentation for your particular version of LPR/LPRng.","PyPrint: Print As HELP")

    def runPySpool(self,*args):
	launch("pyspool &")

    def doQuit(self,*args) :
	self.deleteTempFiles()
	os.popen("rm -f /tmp/pyprint* > /dev/null")  # if you are running PyPrint on a MULTI-user system, comment out this line!
	mainquit()

    def checkPath(self) :
	global HAVE_CONVERT
	global HAVE_A2PS
	global HAVE_HTML2PS
	global HAVE_LPR
	global HAVE_PSSELECT
	global HAVE_PDF2PS
	global HAVE_PS2PDF
	HAVE_LPR=1
	HAVE_CONVERT=1
	HAVE_A2PS=1
	HAVE_HTML2PS=1
	HAVE_PSSELECT=1
	HAVE_PDF2PS=1
	HAVE_PS2PDF=1
	global HAVE_GHOSTVIEW
	HAVE_GHOSTVIEW=1
	executables=['lpstat','lpr','a2ps','html2ps','convert','psselect','ps2pdf','pdf2ps','ghostview']
	for iexec in executables:
		if not isOnPath(iexec): 
			self.showMessage("The following executable does not appear to be on your PATH:\n\n"+iexec+"\n\nYou may experience problems running PyPrint properly.\nSome features may be disabled.\nPlease install '"+iexec+"' into your PATH first.")
			if iexec=="lpr": HAVE_LPR=0
			if iexec=="html2ps": HAVE_HTML2PS=0
			if iexec=="a2ps": HAVE_A2PS=0
			if iexec=="convert": HAVE_CONVERT=0
			if iexec=="psselect": HAVE_PSSELECT=0
			if iexec=="ps2pdf": HAVE_PS2PDF=0
			if iexec=="pdf2ps": HAVE_PDF2PS=0
			if iexec=="ghostview": HAVE_GHOSTVIEW=0
		else: 
			if iexec=="convert": conversion.setHaveImageMagick(1)

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

## Image Formats Dialog
class _FormatsDialog(Dialog):
	def __init__(self, formats=[]):
		Dialog.__init__(self)
		self.connect("destroy", self.quit)
		#self.connect("delete_event", self.quit)
                self.set_position (WIN_POS_CENTER)
                self.set_title("PyPrint Image Format...")
		hbox = VBox(spacing=3)
		hbox.set_border_width(5)
		hbox.pack_start(Label("WOW! I couldn't figure out the image type for this image."))
		hbox.pack_start(Label("Please tell me what kind of image this is:"))
		lcombo=Combo()
                self.lcombo=lcombo
                lcombo.entry.set_editable(0)
                ll=copy.copy(formats)
                ll.sort()
                lcombo.set_popdown_strings(ll)
                lcombo.entry.set_text("")
                lcombo.set_size_request(330,-1)
                hbox.pack_start(lcombo,1,1,2)
		self.vbox.pack_start(hbox)
		hbox.show_all()
		b = getPixmapButton (STOCK_YES, "OK" ,1)
		b.set_flags(CAN_DEFAULT)
		b.connect("clicked", self.selectFormat)
		self.action_area.pack_start(b)
		bb = getPixmapButton (STOCK_CANCEL, "Cancel" ,1)
		bb.set_flags(CAN_DEFAULT)
		bb.connect("clicked", self.quit)
		self.action_area.pack_start(bb)
		bb.show()
		self.ret = None
	def quit(self, *args):
		self.ret=None                
		self.hide()
		self.destroy()
		self.unmap()
                ll=None
		mainquit()
	def selectFormat(self,*args):
		rr =self.lcombo.entry.get_text()
		self.quit()
                self.ret=rr
                if rr=="": self.ret=None

# ask Format
def askFormat():
	win = _FormatsDialog(conversion.read_formats)
	win.show_all()
	mainloop()
	return win.ret

def run(pfile="") :
    pyprintwin().setFile(pfile)
    mainloop()

if __name__== "__main__" :
	pfile=""
	if len(sys.argv)>1:
		pfile=sys.argv[len(sys.argv)-1].strip()
	run(pfile)
