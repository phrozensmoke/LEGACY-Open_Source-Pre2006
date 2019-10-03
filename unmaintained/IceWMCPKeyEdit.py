#! /usr/bin/env python

############################
#  IceWM Control Panel - Key Editor
#  
#  Copyright 2003 by Erica Andrews
#  PhrozenSmoke@yahoo.com
#  http://icesoundmanager.sourceforge.net
#  
#  A simple Gtk-based key configuration utility
#  for IceWM (written in 100% Python).
#  
#  This program is distributed free
#  of charge (open source) under the GNU
#  General Public License
############################

import os,GTK,gtk,sys
from gtk import *

#set translation support
from icewmcp_common import *
_=translateCP   # from icewmcp_common.py

class keypanel:
    def createPanel(self,with_logo=0):
	mainvbox=GtkVBox(0,2)
	mainvbox.set_border_width(5)
	mainvbox.set_spacing(4)
	if with_logo==1:  mainvbox.pack_start(getImage(getBaseDir()+"icewmcp.png",DIALOG_TITLE),0,0,2)
	mainvbox.pack_start(GtkLabel(_("Key Editor")),0,0,1)	
	
	self.clist=GtkCList(2,[_('Keys'),_('Command')])
	self.sc=GtkScrolledWindow()
	self.clist.set_column_width(0,150)
	self.clist.set_column_width(1,350)
	self.sc.add(self.clist)

	mainvbox.pack_start(self.sc,1,1,0)
	keybox=GtkHBox(0,0)
	keybox.set_spacing(6)
	keybox.pack_start(GtkLabel(_("Key:  ")),0,0,0)
	self.alt=GtkCheckButton("Alt")
	self.ctrl=GtkCheckButton("Ctrl")
	self.shift=GtkCheckButton("Shift")
	TIPS.set_tip(self.alt,"Alt")
	TIPS.set_tip(self.ctrl,"Ctrl")
	TIPS.set_tip(self.shift,"Shift")
	keybox.pack_start(self.alt,0,0,0)
	keybox.pack_start(self.ctrl,0,0,0)
	keybox.pack_start(self.shift,0,0,0)
	self.keyentry=GtkEntry()
	keybox.pack_start(self.keyentry,1,1,0)
	progbox=GtkHBox(0,0)
	progbox.set_spacing(4)
	progbox.pack_start(GtkLabel(_("Program:  ")),0,0,0)
	self.progentry=GtkEntry()
	progbox.pack_start(self.progentry,1,1,0)
	TIPS.set_tip(self.progentry,_("Program:  ").replace(":","").strip())
	browse=GtkButton()
	try:
			browse.add(  loadScaledImage(getPixDir()+"ism_load.png",24,24)  )
	except:
			browse.add(GtkLabel(_(" Browse... ")))
	TIPS.set_tip(browse,_(" Browse... ").replace(".","").strip())
	browse.connect("clicked",self.openProg)
	progbox.pack_start(browse,0,0,0)
	buttbox=GtkHBox(1,0)
	buttbox.set_spacing(4)
	buttbox2=GtkHBox(1,0)
	buttbox2.set_spacing(4)
	newb=GtkButton(_(" New Key "))
	setb=GtkButton(_(" Set "))
	addb=GtkButton(_(" Add "))
	delb=GtkButton(_(" Delete "))
	testb=GtkButton(_(" Test Key "))
	saveb=GtkButton(" "+_("Save")+" ")
	TIPS.set_tip(saveb,_("Save"))
	TIPS.set_tip(testb,_(" Test Key ").strip())
	TIPS.set_tip(delb,_(" Delete ").strip())
	TIPS.set_tip(addb,_(" Add ").strip())
	TIPS.set_tip(setb,_(" Set ").strip())
	TIPS.set_tip(newb,_(" New Key ").strip())
	newb.connect("clicked",self.new_key2)
	self.newb=newb
	setb.connect("clicked",self.set_key)
	addb.connect("clicked",self.add_key)
	delb.connect("clicked",self.del_key)
	testb.connect("clicked",self.test_key)
	saveb.connect("clicked",self.doSave)
	self.addb=addb
	self.setb=setb
	self.delb=delb
	self.testb=testb
	buttbox.pack_start(newb,1,1,0)
	buttbox.pack_start(setb,1,1,0)
	buttbox.pack_start(addb,1,1,0)
	buttbox2.pack_start(delb,1,1,0)
	buttbox2.pack_start(testb,1,1,0)
	buttbox2.pack_start(saveb,1,1,0)
	mainvbox.pack_start(keybox,0,0,0)	
	mainvbox.pack_start(progbox,0,0,0)	
	mainvbox.pack_start(buttbox,0,0,0)	
	mainvbox.pack_start(buttbox2,0,0,0)	
	appst=_('/File/_Apply Changes Now...')[_('/File/_Apply Changes Now...').rfind("/")+1:len(_('/File/_Apply Changes Now...'))].replace(".","").replace("_","").strip()
	appbutt=GtkButton(appst)
	TIPS.set_tip(appbutt,appst)
	appbutt.connect("clicked",self.restart_ice)
	mainvbox.pack_start(appbutt,0,0,2)		
	self.status=GtkLabel(_("Ready."))
	self.mainvbox=mainvbox
	mainvbox.show_all()
	mainvbox.pack_start(self.status,0,0,2)	
	self.clist.connect('select_row', self.clist_cb)
	self.clist.connect('unselect_row', self.new_key1)
	self.loadUp()
	return mainvbox	

    def __init__(self,noclose=0,start_prog=None) :
	self.start_comment="# This is an example for IceWM's hotkey definition file.\n#\n# Place your variants in /usr/X11R6/lib/X11/icewm or in $HOME/.icewm\n# since modifications to this file will be discarded when you\n# (re)install icewm.\n#\n# A list of all valid keyboard symbols can be found in\n# /usr/include/X11/keysym.h, keysymdefs.h,XF86keysym.h, ...\n# You'll have to omit XK_ prefixs and to replace XF86XK_ prefixes by\n# XF86. Valid modifiers are Alt, Ctrl, Shift, Meta, Super and Hyper.\n#\n#\n#\n\n"
	self.x_comment="\n\n# Multimedia key bindings for XFree86. Gather the keycodes of your\n# advanced function keys by watching the output of the xev command whilest\n# pressing those keys and map those symbols by using xmodmap.\n\n"
	self.version=this_software_version
	self.NOCLOSE=noclose
	self.preffile=""
	self.keys={}
	self.start_prog=start_prog
	try:
		self.preffile=getIceWMPrivConfigPath()+"keys"
	except:
		pass
	self.lastprog=""

    def createWindow(self):
	global WMCLASS
	WMCLASS="icewmcontrolpanelkeyedit"
	global WMNAME
	WMNAME="IceWMControlPanelKeyEdit"
	wallwin=GtkWindow(GTK.WINDOW_TOPLEVEL)
	if self.NOCLOSE==1:
		wallwin.set_data("ignore_return",1)
		wallwin.connect("key-press-event",keyPressClose)
	wallwin.set_wmclass(WMCLASS,WMNAME)
	wallwin.realize()
	wallwin.set_title(_("IceWM CP - Key Editor"))
	wallwin.set_position(GTK.WIN_POS_CENTER)
	self.wallwin=wallwin

	menu_items = [
				[_('/_File'), None, None, 0, '<Branch>'],
				[_('/File/_Open...'), '<control>O', self.openKey, 0, ''],
				[_('/File/_Save'), '<control>S', self.doSave, 0, ''],
				[_('/File/sep2'), None, None, 2, '<Separator>'],
 				[_("/_File")+"/_"+FILE_RUN,'<control>R', rundlg,421,""],
 				[_("/_File")+"/_"+_("Check for newer versions of this program..."), '<control>U', checkSoftUpdate,420,""],
				[_('/File/sep1'), None, None, 1, '<Separator>'],
				[_('/File/_Apply Changes Now...'), '<control>A', self.restart_ice, 0, ''],
				[_('/File/sep2'), None, None, 2, '<Separator>'],
				[_('/File/_Exit'), '<control>Q', self.doQuit, 0, ''],
				[_('/_Help'), None, None, 0, '<LastBranch>'],
				[_('/Help/_About...'), "F2", self.do_about, 0, ''],
  (_("/_Help")+"/_"+APP_HELP_STR, "F4", displayHelp,5002, ""),
  (_("/_Help")+"/_"+CONTRIBUTORS+"...", "F3", show_credits,913, ""),
  (_("/_Help")+"/_"+BUG_REPORT_MENU+"...", "F5", file_bug_report,5002, ""),
					]

	ag = GtkAccelGroup()
	itemf = GtkItemFactory(GtkMenuBar, '<main>', ag)
	wallwin.add_accel_group(ag)
	itemf.create_items(menu_items)
	self.menubar = itemf.get_widget('<main>')
	menubox=GtkHBox(0,0)
	menubox.pack_start(self.menubar,1,1,0)
	mainvbox1=GtkVBox(0,2)
	mainvbox1.pack_start(menubox,0,0,0)
	mainvbox1.pack_start(self.createPanel(1),1,1,2)
	wallwin.add(mainvbox1)
	wallwin.connect("destroy",self.doQuit)
	wallwin.set_default_size(-2,440)
	wallwin.show_all()

    def loadUp(self,*args) :
	self.current_row=-1
	self.current_key=None
	self.mykeys=self.get_keys()
	self.display_keys(self.mykeys)
	self.new_key()
	if not self.start_prog==None: self.progentry.set_text(str(self.start_prog).strip())
	self.setStatus(_("Ready."))

    def doQuit(self,*args) :
	if self.NOCLOSE==0: 
		gtk.mainquit()
	else: 
		self.wallwin.destroy()
		self.wallwin.hide()

    def build_key(self,*args):
	s=""
	if self.ctrl.get_active(): s="Ctrl"
	if self.alt.get_active():
		if len(s)==0: s="Alt"
		else: s=s+"+Alt"
	if self.shift.get_active():
		if len(s)==0: s="Shift"
		else: s=s+"+Shift"
	if len(self.keyentry.get_text().strip())>0:
		if len(s)==0: s=self.keyentry.get_text().strip()
		else: s=s+"+"+self.keyentry.get_text().strip()
	return s

    def set_key(self,*args):
	if self.current_key==None: return
	if not self.mykeys.has_key(self.current_key):
		msg_warn(DIALOG_TITLE,_("This key does not exist.\nPlease create it by clicking the 'New Key' button."))	
		return
	if not len(self.progentry.get_text().strip())>0:
		msg_warn(DIALOG_TITLE,_("You must specify a program for this key combination"))	
		return
	self.mykeys[self.current_key]=self.progentry.get_text().strip()
	self.display_keys(self.mykeys)
	self.new_key(1)
	self.setStatus(_("Modified."))
	try:
		l=self.mykeys.keys()
		l.sort()
		self.clist.select_row(l.index(self.current_key),0)
		self.clist.moveto(l.index(self.current_key),0)
	except:
		pass

    def add_key(self,*args):
	s=self.build_key()
	if not len(s)>0: return
	if not len(self.progentry.get_text().strip())>0:
		msg_warn(DIALOG_TITLE,_("You must specify a program for this key combination")+":\n"+s)	
		return
	if self.mykeys.has_key(s):
		msg_warn(DIALOG_TITLE,_("The key '")+s+_("' already exists and\ntriggers the program: ")+self.mykeys[s]+"\n\n"+_("You must either delete the existing key,\nor change the existing key's action using 'Set'."))
		return
	self.mykeys[s]=self.progentry.get_text().strip()
	self.display_keys(self.mykeys)
	self.setStatus(_("Modified."))
	try:
		l=self.mykeys.keys()
		l.sort()
		self.clist.select_row(l.index(s),0)
		self.clist.moveto(l.index(s),0)
	except:
		pass

    def del_key(self,*args):
	if self.current_key==None: return
	try:
		if msg_confirm(DIALOG_TITLE,_("Are you sure you want to delete this key?")+"\n\n"+self.current_key+"\n"+self.mykeys[self.current_key])==1:

			l=self.mykeys.keys()
			l.sort()
			prev_row=l.index(self.current_key)-1

			del self.mykeys[self.current_key]
			self.display_keys(self.mykeys)
			self.new_key(0)
			self.current_key=None
			self.setStatus(_("Modified."))

			self.clist.select_row(prev_row,0)
			self.clist.moveto(prev_row,0)
	except:
		pass

    def new_key1(self,*args):
	self.new_key(0)

    def new_key2(self,*args):
	self.clist.unselect_row(self.current_row,0)
	self.new_key(0)

    def new_key(self,desense=0):
	self.progentry.set_text("")
	self.alt.set_active(0)
	self.ctrl.set_active(0)
	self.shift.set_active(0)
	self.keyentry.set_text("")
	if desense==1:
		self.keyentry.set_sensitive(0)
		self.alt.set_sensitive(0)
		self.shift.set_sensitive(0)
		self.ctrl.set_sensitive(0)
		self.addb.set_sensitive(0)
		self.delb.set_sensitive(1)
		self.setb.set_sensitive(1)
		self.testb.set_sensitive(1)
	else:
		self.keyentry.set_sensitive(1)
		self.alt.set_sensitive(1)
		self.shift.set_sensitive(1)
		self.ctrl.set_sensitive(1)
		self.addb.set_sensitive(1)
		self.delb.set_sensitive(0)
		self.setb.set_sensitive(0)
		self.testb.set_sensitive(0)
		self.current_key=None

    def clist_cb(self,widget, row, col, event):
	rowlist = widget.get_row_data(row)
	self.current_row=row
	self.new_key(1)
	self.progentry.set_text(rowlist[1])
	ikeys=rowlist[0].split("+")
	self.current_key=rowlist[0]
	for ii in ikeys:
		if ii=="Ctrl": self.ctrl.set_active(1)
		elif ii=="Alt": self.alt.set_active(1)
		elif ii=="Shift": self.shift.set_active(1)
		else:
			if  len(self.keyentry.get_text())>0: self.keyentry.set_text(self.keyentry.get_text()+"+"+ii)
			else: self.keyentry.set_text(ii)
	

    def display_keys(self,key_dict):
	klist=key_dict.keys()
	klist.sort()
	self.clist.freeze()
	self.clist.clear()	
	inum=0
	for ii in klist:
		try:
			self.clist.append([ii,key_dict[ii]])
			self.clist.set_row_data(inum,[ii,key_dict[ii],inum])
			inum=inum+1
		except:
			pass
	self.clist.thaw()
		

    def parseLineList(self,string_list):
	ff=string_list
	try:
		d={}
		for ii in ff:
			if not ii.strip().startswith("key "): continue
			iline=ii[ii.find("key ")+4:len(ii)].strip()
			if iline.startswith("\""):
				ikey=iline[0:iline.rfind("\"")+1].strip()
				iprog=iline.replace(ikey,"").replace("\t","").replace("\t","").replace("\t","").strip()
				if iprog.find("#")>0: iprog=iprog[0:iprog.find("#")]
				ikey=ikey.replace("\"","").strip()
				if (len(ikey)>0) and (len(iprog)>0):
					keylist=ikey.split("+")
					arkeys=""
					if ikey.find("Ctrl")>-1: arkeys="Ctrl"
					if ikey.find("Alt")>-1:
						if len(arkeys)>0: arkeys=arkeys+"+Alt"
						else: arkeys="Alt"
					if ikey.find("Shift")>-1:
						if len(arkeys)>0: arkeys=arkeys+"+Shift"
						else: arkeys="Shift"
					for h in keylist:
						if h.strip() in ['Alt','Ctrl','Shift']: continue
						if len(arkeys)>0: arkeys=arkeys+"+"+h.strip()
						if len(arkeys)==0: arkeys=h.strip()					
					d[arkeys]=iprog
		return d
	except:
		return {}

    def get_keys(self,*args) :
	try:
		self.backup(self.preffile) # create a backup
		f=open(self.preffile)
		ff=f.read().split("\n")
		f.close()
		return self.parseLineList(ff)
	except:
		pass
	try: # this happens if we got an empty prefs file - try system prefs in a universal way, new in version 1.2
		f=open(getIceWMConfigPath()+"keys")
		ff=f.read().split("\n")
		f.close()
		return self.parseLineList(ff)
	except:
		pass
	return {}

    def openKey(self, *args):
	self.selectfile(0)

    def openProg(self, *args):
	self.selectfile(1)

    def selectfile(self, program=1):
		self.filewin = GtkFileSelection(_("Select a file..."))
		self.filewin.set_position(WIN_POS_CENTER)
		self.filewin.ok_button.connect('clicked', self.fileok)
		self.filewin.ok_button.set_data("type",program)
		self.filewin.cancel_button.connect('clicked', self.filecancel)
		if program==1: value=self.lastprog
		else: value=self.preffile
		if value != '""':
			self.filewin.set_filename(value)
		self.filewin.set_modal(TRUE)
		self.filewin.show()
		
    def fileok(self, *args):
		dirvalue = self.filewin.get_filename()
		p=args[0].get_data("type")
		if p==1:
			self.progentry.set_text(str(dirvalue))
		else:
			if os.path.exists(dirvalue):
				self.preffile=dirvalue
				self.loadUp()
			else:				
				msg_err(DIALOG_TITLE,_("No such file or directory:\n")+dirvalue)	
				return
		self.filecancel()

    def filecancel(self,*args):
		self.filewin.destroy()
		self.filewin.unmap()

    def doSave(self,*args):
	try:
		f=open(self.preffile,"w")
		flist=self.mykeys.keys()
		flist.sort()
		wrotex=0
		f.write(self.start_comment)
		for ii in flist:
			if (ii.startswith("XF86")) and (wrotex==0):
				f.write(self.x_comment+"\n")
				wrotex=1
				f.write("key \""+ii+"\"\t\t"+self.mykeys[ii]+"\n")
			else:
				f.write("key \""+ii+"\"\t\t"+self.mykeys[ii]+"\n")
		self.setStatus(_("Saved."))
		return 1
	except:
		msg_err(DIALOG_TITLE,_("ERROR saving key configuration file:\n")+self.preffile)	
		self.setStatus(_("Error saving."))
		return 0

    def restart_ice(self,*args) :
	if msg_confirm(DIALOG_TITLE,_("WARNING:\nThis feature doesn't work perfectly on all systems.\nBSD and some other systems may experience problems."))	== 1:
		i=self.doSave()
		if i==1:
			os.system('killall -HUP -q icewm &')
			os.system('killall -HUP -q icewm-gnome &')

    def setStatus(self,stattext):
	self.status.set_text(_(str(stattext)))

    def backup(self,file_name): # create backup of prefs file before editing
	try:
		f=open(file_name)
		ff=f.read()
		f.close()
		f=open(file_name+".backup-file","w")
		f.write(ff)
		f.flush()
		f.close()
	except:
		pass

    def do_about(self,*args):
	commonAbout(_("About IceWM CP - Key Editor")+" "+self.version,_("IceWM CP - Key Editor")+" "+self.version+"\n\n"+_("A simple Gtk-based key configuration utility\nfor IceWM (written in 100% Python).\n\nThis program is distributed free\nof charge (open source) under the GNU\nGeneral Public License."))	

    def test_key(self,*args):
	if self.current_key==None: return
	exec_n=self.mykeys[self.current_key]
	if not self.progentry.get_text().strip()==exec_n:  
		self.set_key()  # update the key if text entry changed
		if self.current_key==None: return
		exec_n=self.mykeys[self.current_key]
	path_find=self.is_on_path(""+exec_n)
	if path_find==1:  
		# launch the app and see if it works
		os.popen(exec_n+" &") # for to bash/bourne
	else:
		if path_find==0: # no full path given and not on system $PATH
			msg_warn(DIALOG_TITLE,_("The executable is not on your PATH")+":\n"+self.get_exec_short(""+exec_n))	
		if path_find==-1:  # full path given, but file doesnt exist	
			msg_warn(DIALOG_TITLE,_("The executable file does not exist")+":\n"+self.get_exec_short(""+exec_n))	

    def get_exec_short(self,exec_short):
	if exec_short.find(" ")>-1: exec_short=exec_short[0:exec_short.find(" ")] # "aumix -v +5" -> "aumix" is exec
	return exec_short.strip()

    def is_on_path(self,exec_name):  # new in version 1.2, check to see if an executable is on the path
	exec_name=self.get_exec_short(exec_name)

	try:
		if exec_name.find(os.sep)>-1:  # check first to see if exec_name is a FULL path
			if os.path.exists(exec_name): return 1
			else: return -1  # full path given, but not in system	
		else:
			paths=os.environ['PATH'].split(":")
			for ii in paths:
				if os.path.exists(ii+os.sep+exec_name): return 1
			return 0 # no full path given, and not on path
	except:
		pass
	return 1

def run(noclose=0,start_prog=None) :
	kp=keypanel(noclose,start_prog)
	kp.createWindow()
	hideSplash()
	if noclose==0: gtk.mainloop()

if __name__== "__main__" :
	run(0)
