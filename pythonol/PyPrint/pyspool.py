#! /usr/bin/env python
# -*- coding: ISO-8859-1 -*-

########################
# PyPrint / PySpool:  PySpool
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

#############################################
#  PySpool  
#  
#  PySpool is a simple -based spool monitor and 
#  manager written in 100% Python. It is intended to 
#  monitor LPRng and LPR-based printer spools. 
#  This is ALPHA code. PySpool has only been tested 
#  with LPRng 3.8.12 on local printers. PySpool 
#  requires the following executables on your PATH: 
#  lpstat, cancel, checkpc
#  
#  PySpool is open source under the General Public 
#  License (GPL). PySpool is part of the PyPrint
#  utility package (same author).
#  
#  NO technical support will be provided for this 
#  application. 
#  Enjoy!
#############################################

import copy
from pyprintcommon import *

class pyspoolwin :
    def __init__(self) :
	self.last_printers=[]
	self.last_readit=""
	self.last_status=""
	pyspoolwin=Window(WINDOW_TOPLEVEL)
	set_window_icon(pyspoolwin,pyspool_icon_xpm,0)
        pyspoolwin.set_wmclass("PySpoolQueue","PySpool") 
	pyspoolwin.set_title('PySpool: Printer Queue version '+PYPRINT_VERSION)	
	pyspoolwin.set_position(WIN_POS_CENTER)
	self._root=pyspoolwin
	pyspoolwin.realize()
	self.pyspoolwin=pyspoolwin
	mainvbox=VBox(0,0)
	self.mainvbox=mainvbox
	mymenu=MenuBar()
	self.mymenu=mymenu

        ag = AccelGroup()
        itemf = ItemFactory(MenuBar, "<main>", ag)
        itemf.create_items([
            # path              key           callback    action#  type
  ("/ _File",  "<alt>F",  None,  0, "<Branch>"),
  ("/ _File/_Print A Document...", "<control>P", self.runPyPrint, 1, "<StockItem>", STOCK_PRINT),
  ("/ _File/sep5", None,  None,  44, "<Separator>"),
  ("/ _File/_Quit", "<control>Q", self.doQuit, 10, "<StockItem>", STOCK_QUIT),
  ("/_Help",  "<alt>H",  None, 16, "<LastBranch>"), 
  ("/_Help/_About PySpool...", "F2", self.doAbout, 17, "<StockItem>", STOCK_DIALOG_INFO)


        ])
        pyspoolwin.add_accel_group(ag)
	self.ag=ag
	self.itemf=itemf
        mymenu = itemf.get_widget("<main>")
        mymenu.show()

	mainvbox.pack_start(mymenu,0,0,0)
	hbox1=HBox(0,12)
	hbox1.set_border_width(8)
	self.hbox1=hbox1
	label8=Label('Select Printer:')
	self.label8=label8
	#pyspoolwin.set_size_request(640,-1)
	hbox1.pack_start(label8,0,0,0)
	printcombo=Combo()
	printcombo.set_size_request(200,-1)
	self.printcombo=printcombo
	combo_entry=printcombo.entry
	combo_entry.set_editable(0)
	self.combo_entry=combo_entry
	hbox1.pack_start(printcombo,0,0,0)
	viewbutt=getPixmapButton(STOCK_ZOOM_100, 'View')
	viewbutt.connect("clicked",self.changeView)
	self.viewbutt=viewbutt
	hbox1.pack_start(viewbutt,0,0,0)
	hbox1.pack_start(Label(" "),1,1,0)
	mainvbox.pack_start(hbox1,0,0,0)
	pic=loadImage("pyprint_printer_big.png",pyspoolwin)
	if pic: 	hbox1.pack_start(pic,0,0,0)
	hbox1.pack_start(Label(" "),1,1,0)
	hbox3=HBox(0,8)
	hbox3.set_border_width( 4)
	self.hbox3=hbox3
	mytable=Table(5,2,0)
	mytable.set_row_spacings(3)
	mytable.set_col_spacings(7)
	self.mytable=mytable
	label15=Label('Description:')
	label15.set_alignment(0,0.5)
	self.label15=label15
	mytable.attach(label15,0,1,0,1,(FILL),(0),0,0)
	label16=Label('Queue:')
	label16.set_alignment( 0,0.5)
	self.label16=label16
	mytable.attach(label16,0,1,1,2,(FILL),(0),0,0)
	label17=Label('Server:')
	label17.set_alignment(0,0.5)
	self.label17=label17
	mytable.attach(label17,0,1,2,3,(FILL),(0),0,0)
	label18=Label('Status:')
	label18.set_alignment( 0,0.5)
	self.label18=label18
	mytable.attach(label18,0,1,3,4,(FILL),(0),0,0)
	label19=Label('Info:')
	label19.set_alignment( 0,0.5)
	self.label19=label19
	mytable.attach(label19,0,1,4,5,(FILL),(0),0,0)
	desc_text=Entry()
	desc_text.set_size_request(375,-1)
	self.desc_text=desc_text
	desc_text.set_editable(0)
	mytable.attach(desc_text,1,2,0,1,(EXPAND+FILL),(0),0,0)
	q_text=Entry()
	self.q_text=q_text
	q_text.set_editable(0)
	mytable.attach(q_text,1,2,1,2,(EXPAND+FILL),(0),0,0)
	server_text=Entry()
	self.server_text=server_text
	server_text.set_editable(0)
	mytable.attach(server_text,1,2,2,3,(EXPAND+FILL),(0),0,0)
	status_text=Entry()
	self.status_text=status_text
	status_text.set_editable(0)
	mytable.attach(status_text,1,2,3,4,(EXPAND+FILL),(0),0,0)
	info_text=Entry()
	info_text.set_editable(0)
	self.info_text=info_text
	mytable.attach(info_text,1,2,4,5,(EXPAND+FILL),(0),0,0)
	hbox3.pack_start( mytable,1,1,0)
	vbox3=VBox(1,7)
	self.vbox3=vbox3
	refreshbutt=getPixmapButton(STOCK_REFRESH,"Refresh Queue Details",pyspoolwin)
	refreshbutt.connect("clicked",self.loadDetailsR)
	self.refreshbutt=refreshbutt
	vbox3.pack_start( refreshbutt,0,0,0)
	cancelbutt=getPixmapButton(STOCK_CANCEL,"Cancel Selected Print Job",pyspoolwin)
	cancelbutt.connect("clicked",self.cancelJob)
	self.cancelbutt=cancelbutt
	vbox3.pack_start( cancelbutt,0,0,0)
	hbox3.pack_start( vbox3,0,0,0)
	mainvbox.pack_start(hbox3,0,0,4)
	scrollwin=ScrolledWindow()
	scrollwin.set_border_width(3)
	scrollwin.set_size_request(-1,170)
	self.scrollwin=scrollwin
	myclist=CList(7)
	myclist.connect("unselect_row",self.startUpdates)
	myclist.connect("select_row",self.makeSelection)
	myclist.column_titles_show()
	myclist.set_column_width(0,70)
	myclist.set_column_width(1,53)
	myclist.set_column_width(2,71)
	myclist.set_column_width(3,140)
	myclist.set_column_width(4,117)
	myclist.set_column_width(5,78)
	myclist.set_column_width(6,400)
	self.myclist=myclist
	label1=Label('State')
	self.label1=label1
	myclist.set_column_widget(0,label1)
	label2=Label('Job ID')
	self.label2=label2
	myclist.set_column_widget(1,label2)
	label3=Label('Size')
	self.label3=label3
	myclist.set_column_widget(2,label3)
	label4=Label('Files')
	self.label4=label4
	myclist.set_column_widget(3,label4)
	label5=Label('Owner/ID')
	self.label5=label5
	myclist.set_column_widget(4,label5)
	label6=Label('Time')
	self.label6=label6
	myclist.set_column_widget(5,label6)
	label7=Label('Class')
	self.label7=label7
	myclist.set_column_widget(6,label7)
	scrollwin.add(myclist)
	mainvbox.pack_start(scrollwin,1,1,0)
	mystatusbar=Statusbar()
	self.mystatusbar=mystatusbar
	statid=mystatusbar.get_context_id("spooler")
	self.statid=statid
	mainvbox.pack_start(mystatusbar,0,0,0)
	pyspoolwin.add( mainvbox)
	pyspoolwin.connect("destroy",self.doQuit)
	global SELECTED_PRINTER
	SELECTED_PRINTER=None
	global SELECTING
	SELECTING=0
	global JOB_ID
	JOB_ID=-1
	pyspoolwin.show_all()
	self.checkPath() # check for needed executables
	# correct any problems with the printer queue directories
	launch("checkpc -f > /dev/null")
	try:
		self.loadDetails()
	except:
		pass
	gtk.timeout_add(1700,self.autoLoad) # reload the details every 1.5 seconds

    def changeView(self,*args) :
	if not self.combo_entry.get_text().strip()=="":
		global SELECTED_PRINTER
		SELECTED_PRINTER=self.combo_entry.get_text().strip()
		self.loadDetails()
		self.startUpdates()

    def startUpdates(self,*args) :	
	global JOB_ID
	JOB_ID=-1
	global SELECTING
	SELECTING=0

    def stopUpdates(self,*args) :	
	global SELECTING
	SELECTING=1

    def makeSelection(self,clist=None,crow=-1,*args) :
	global JOB_ID
	if crow>-1: 
		JOB_ID=self.myclist.get_text(crow,1).strip()
		#self.showMessage("Job ID is:  "+str(JOB_ID))
	self.stopUpdates()

    def cancelJob(self,*args) :
	self.stopUpdates()
	global JOB_ID
	if str(JOB_ID)=="-1": 
		self.startUpdates()
		return
	global SELECTED_PRINTER
	if SELECTED_PRINTER=="NONE FOUND": 
		self.startUpdates()
		return
	if self.showConfirm("Are you SURE you want to cancel print job "+str(JOB_ID)+" on printer '"+str(SELECTED_PRINTER)+"' ?")==1:  
		fflines=readOSLines("cancel -P "+str(SELECTED_PRINTER)+" "+str(JOB_ID))
		ss="Cancel Print Job "+str(JOB_ID)+" - Result:\n\n"
		for ii in fflines: 
			ss=ss+ii+"\n"
		self.showMessage(ss)
	self.loadDetailsR()	

    def autoLoad(self,*args) :	
	global SELECTING
	if SELECTING==1: return TRUE
	try:
		self.loadDetails()
	except:
		pass
	return TRUE

    def loadDetailsR(self,*args) :		
	self.loadDetails()
	self.startUpdates()

    def loadDetails(self,*args) :
	self.loadPrinterNames()
	readit=self.readPrinterInfo()
	if readit==self.last_readit: return
	self.last_readit=readit
	self.loadQueue(copy.copy(readit))
	self.loadInfo(copy.copy(readit))
	global PRINTER_COUNT
	global SELECTED_PRINTER
	pstatus=" Available printers: "+str(PRINTER_COUNT)+"        Current printer: "+str(SELECTED_PRINTER)
	if pstatus==self.last_status: return
	self.last_status=pstatus
	self.mystatusbar.push(self.statid, pstatus)

    def loadInfoDefaults(self) :
	self.desc_text.set_text("description unavailable")
	self.q_text.set_text("queue status unavailable")
	self.server_text.set_text("server status unavailable")
	self.status_text.set_text("status unavailable")
	self.info_text.set_text("info unavailable")

    def clearInfo(self) :
	self.desc_text.set_text("")
	self.q_text.set_text("")
	self.server_text.set_text("")
	self.status_text.set_text("")
	self.info_text.set_text("")

    def loadInfo(self,readit) :
	self.info_text
	global SELECTED_PRINTER
	if not SELECTED_PRINTER: 
		self.loadInfoDefaults()
		return
	if SELECTED_PRINTER.strip()=="": 
		self.loadInfoDefaults()
		return
	if not len(readit):
		self.loadInfoDefaults()
		return
	self.clearInfo()
	try:
		for iinfo in readit:
			infoline=iinfo.strip()
			if infoline.lower().startswith("printer"): 
				self.info_text.set_text(self.info_text.get_text()+infoline.replace("printer","").replace("Printer:","").strip()+"  ")
			if infoline.startswith("Status:"): self.status_text.set_text(infoline.replace("Status:","").strip())
			if infoline.startswith("Server:"): 
				self.server_text.set_text(infoline.replace("Server:","").strip())
				if infoline.lower().find("no server active") > -1: self.server_text.set_text(self.server_text.get_text()+"    (No clients currently printing)")
			if infoline.startswith("Description:"): self.desc_text.set_text(infoline.replace("Description:","").strip())
			if infoline.startswith("Queue:"): self.q_text.set_text(infoline.replace("Queue:","").strip())
	except:
		self.loadInfoDefaults()				

    def loadPrinterNames(self) :
	global SELECTED_PRINTER
	global PRINTER_COUNT
	global JOB_ID
	PRINTER_COUNT="0"
	if not self.combo_entry.get_text().strip()=="": SELECTED_PRINTER=self.combo_entry.get_text().strip()
	try:
		printers=getPrinterNames()
		PRINTER_COUNT=str(len(printers))
		if printers==self.last_printers: return
		if len(printers)==0: 
			self.printcombo.set_popdown_strings(['NONE FOUND'])
			self.combo_entry.set_text('NONE FOUND')
			JOB_ID=-1 # if there are no printers all old job ids are invalid
			self.last_printers=printers
		else: 
			printers.sort()
			if printers==self.last_printers: return
			self.last_printers=printers
			self.printcombo.set_popdown_strings(printers)
			self.printcombo.show_all()
			if not SELECTED_PRINTER: SELECTED_PRINTER=printers[0].strip()
			if SELECTED_PRINTER=="": SELECTED_PRINTER=printers[0].strip()
			self.combo_entry.set_text(SELECTED_PRINTER.strip())
			PRINTER_COUNT=str(len(printers))
	except:
		self.printcombo.set_popdown_strings(['NONE FOUND'])
		self.combo_entry.set_text('NONE FOUND')
		JOB_ID=-1

    def readPrinterInfo(self) :
	global SELECTED_PRINTER
	if not SELECTED_PRINTER: return []
	if SELECTED_PRINTER.strip()=="": return []
	if SELECTED_PRINTER=="NONE FOUND": return []
	return readOSLines("lpstat -D -p "+SELECTED_PRINTER.strip())

    def loadQueue(self,xreadlines_list) :	
	self.myclist.freeze()
	self.myclist.clear()
	try:
		foundQ=0
		for ii in xreadlines_list:
			if ii.lower().strip().startswith("rank"): 
				foundQ=1
				continue
			if foundQ==1:
				qline=ii.replace("  "," ").replace("  "," ").replace("  "," ").replace("  "," ").replace("  "," ").strip().split(" ")
				qinfo=[qline[0],qline[3],qline[5],qline[4],qline[1],qline[6],qline[2]]
				self.myclist.append(qinfo)
	except:
		self.myclist.append(["Error","Error","Error","Error","Error","Error","Error"])
	self.myclist.thaw()

    def showMessage(self,message_text):
	return showMessage(message_text,"PySpool")

    def showConfirm(self,message_text):
	return showConfirm(message_text,"PySpool")

    def doAbout(self,*args):
	showAbout("PySpool is a simple -based spool monitor and manager written\nin 100% Python. It is intended to monitor LPRng and LPR-based\nprinter spools. This is ALPHA code. PySpool has only been\ntested with LPRng 3.8.12 on local printers.\nPySpool requires the following executables on your PATH:\nlpstat, cancel, checkpc","PySpool")

    def runPyPrint(self,*args):
	launch("pyprint &")

    def doQuit(self,*args) :
	gtk.mainquit()

    def checkPath(self) :
	executables=['lpstat','cancel','checkpc']
	for iexec in executables:
		if not isOnPath(iexec): self.showMessage("The following executable does not appear to be on your PATH:\n\n"+iexec+"\n\nYou may experience problems running PySpool properly.\nPlease install '"+iexec+"' into your PATH first.")

def run() :
    pyspoolwin()
    gtk.mainloop()

if __name__== "__main__" :
    run()
