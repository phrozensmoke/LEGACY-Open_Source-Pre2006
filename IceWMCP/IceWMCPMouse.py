#! /usr/bin/env python

######################################
# IceWM Control Panel Mouse Settings
# 
# Copyright 2003 by Erica Andrews 
# (PhrozenSmoke@yahoo.com)
# http://icesoundmanager.sourceforge.net
#
#  This program incorporates 3 tools:
#
#  A simple Gtk-based cursor configuration utility
#  for IceWM (written in 100% Python).  The old 
#  'IceWMCPCursors.py' module
# 
# A simple Gtk-based utility for setting the mouse speed.
# It's a simple frontend for 'xset'.  
#
#  An embedded module from IcePref2 for configuring 
#  mouse behavior under IceWM.
# 
# This program is distributed under the GNU General
# Public License (open source).
#######################################

import os,GTK,gtk,sys
from gtk import *

#set translation support
import icewmcp_common
from icewmcp_common import *
_=translateCP   # from icewmcp_common.py
icewmcp_common.NOSPLASH=1  # disable splash window from icepref_td,icepref

import icepref_td
import icepref

global ICE_TAB
ICE_TAB=None
global BSD_WARN
BSD_WARN=0
global DO_QUIT
DO_QUIT=1
global CWIN
CWIN=None

# shared methods

def restart_ice(*args) :
	global BSD_WARN
	if not BSD_WARN==1:
		if not msg_confirm(DIALOG_TITLE,_("WARNING:\nThis feature doesn't work perfectly on all systems.\nBSD and some other systems may experience problems."))==1:
			return
	BSD_WARN=1
	global ICE_TAB
	ICE_TAB.save_current_settings("nowarn")
	os.system('killall -HUP -q icewm &')
	os.system('killall -HUP -q icewm-gnome &')

def doQuit(*args):
	global DO_QUIT
	global CWIN
	if DO_QUIT==1:	mainquit()
	else: 
		CWIN.hide()
		CWIN.destroy()
		CWIN.unmap()


# mouse speed configuration tab, from the old IceWMCPMouse.py
class mousetab:
    def __init__(self) :
	mainvbox=GtkVBox(0,1)
	mainvbox.set_border_width(3)
	mainvbox.set_spacing(3)
	try:
		mainvbox.pack_start(loadScaledImage(getPixDir()+"ps2-mouse.png",56,65),0,0,6)
	except:
		pass
	mainframe=GtkFrame()
	mainframe.set_label(_(" Select your desired mouse speed: "))
	self.adj=GtkAdjustment(4,1,75,1,1,1)
	c=GtkHScale(self.adj)
	TIPS.set_tip(c,_(" Select your desired mouse speed: ").replace(":","").strip())
	c.set_digits(0)
	c.set_draw_value(1)
	c.set_update_policy(UPDATE_DISCONTINUOUS)
	c.set_value_pos(POS_TOP)
	cbox=GtkHBox(0,0)	
	cbox.set_border_width(10)
	cbox.pack_start(c,1,1,0)
	cbox.show_all()
	mainframe.add(cbox)
	mainvbox.pack_start(mainframe,1,1,5)
	abutton=GtkButton(_("Apply"))
	TIPS.set_tip(abutton,_("Apply"))
	rbutton=GtkButton(_("Reset"))
	TIPS.set_tip(rbutton,_("Reset"))
	cbutton=GtkButton(_("Close"))
	TIPS.set_tip(cbutton,_("Close"))
	hb=GtkHBox(0,0)
	hb.pack_start(abutton,1,1,0)
	hb.pack_start(rbutton,1,1,0)
	hb.pack_start(cbutton,1,1,0)
	hb.set_spacing(4)
	mainvbox.pack_start(GtkLabel(""),1,1,0)
	mainvbox.pack_start(hb,0,0,1)
	abutton.connect("clicked",self.doApply)
	rbutton.connect("clicked",self.doReset)
	cbutton.connect("clicked",doQuit)
	self.mainvbox=mainvbox
	mainvbox.show_all()

    def doReset(self,*args) : # reset to a reasonable speed
	os.popen("xset m 4 &> /dev/null &")
	self.adj.set_value(4)

    def doApply(self,*args) : # set to desired speed
	self.swin=None
	os.popen("xset m "+str(int(self.adj.value))+" &> /dev/null &")
	self.doSafe()

    def doSafe(self,*args):
	global WMCLASS
	global WMNAME
	swin=GtkWindow(GTK.WINDOW_TOPLEVEL)
	swin.set_wmclass("icewmcp-mouse","icewmcp-Mouse")
	swin.realize()
	swin.set_title(_("IceWM CP - Mouse Settings"))
	swin.set_position(GTK.WIN_POS_CENTER)
	mainv=GtkVBox(0,0)
	mainv.set_spacing(3)
	mainv.set_border_width(4)
	mainv.pack_start(GtkLabel(_("Your mouse speed has been set to '")+str(int(self.adj.value))+_("'.\nIf you are not happy with this mouse speed,\nwait 7 seconds and the mouse speed will be\nreset to a reasonable speed.\n")),0,0,2)
	hb=GtkVBox(0,0)
	hb.set_spacing(7)
	b1=GtkButton(_("CANCEL change and RESET the mouse"))
	b2=GtkButton(_("KEEP the new mouse speed"))
	b1.connect("clicked",self.resetSafe)
	b2.connect("clicked",self.closeSafe)
	b2.set_data("window",swin)
	b1.set_data("window",swin)
	hb.pack_start(b1,1,1,0)
	hb.pack_start(b2,1,1,0)
	mainv.pack_start(hb,1,1,2)
	swin.add(mainv)
	self.swin=swin
	swin.show_all()
	gtk.timeout_add(7100,self.checkSafe)

    def resetSafe(self,*args):
	self.doReset()
	args[0].get_data("window").destroy()
	args[0].get_data("window").unmap()
	self.swin=None

    def closeSafe(self,*args):
	args[0].get_data("window").destroy()
	args[0].get_data("window").unmap()
	self.swin=None

    def checkSafe(self,*args):
	if not self.swin==None:
		self.swin.set_data("window",self.swin)
		self.resetSafe(self.swin)
	return FALSE


# cursor panel, from the old IceWMCPCursors tool

class cursortab:
    def __init__(self,cwin):
	self.version=this_software_version
	global CONFIG_THEME_PATH
	CONFIG_THEME_PATH=getIceWMPrivConfigPath()+"cursors"+os.sep
	icepref_td.CONFIG_THEME_PATH=getIceWMPrivConfigPath()+"cursors"+os.sep
	icepref_td.APP_WINDOW=cwin
	try:
		os.makedirs(CONFIG_THEME_PATH)
	except:
		pass
	self.cursors=['left.xpm','move.xpm','right.xpm','sizeB.xpm','sizeBL.xpm','sizeBR.xpm','sizeL.xpm','sizeR.xpm','sizeT.xpm','sizeTL.xpm','sizeTR.xpm']
	self.widgets=[]
	self.vbox=GtkVBox(0,0)

	vbox=self.vbox
	vbox.pack_start(GtkLabel(_("Cursor Editor")),0,0,3)
	sc=GtkScrolledWindow()
	slay=GtkLayout()
	self.slay=slay
	sc.add(slay)
	vbox.pack_start(sc,1,1,6)
	myy=3
	for ii in self.cursors:
		hb=GtkHBox(0,0)
		hb.set_border_width(2)
		cwid=icepref_td.ImageWidget(ii, "\""+ii+"\"",icepref_td.ImagePreviewer(0))
		cwid.entry.set_editable(0)
		cwid.set_data("my_image",ii)
		self.widgets.append(cwid)
		hb.pack_start(cwid,1,1,0)
		hb.show_all()
		f=GtkFrame()
		f.add(hb)
		f.show_all()
		self.slay.put(f,3,myy)
		myy=myy+f.size_request()[1]+4
	for ii in self.widgets:
		ii.PREV.update_image(CONFIG_THEME_PATH+ii.get_data("my_image"))
	slay.set_size(400,myy+50)
	vbox.set_border_width(0)
	#cwin.set_default_size(420,470)
	actstr=_('/File/_Apply Changes Now...')[_('/File/_Apply Changes Now...').rfind("/")+1:len(_('/File/_Apply Changes Now...'))].replace("_","").replace(".","").strip()
	abutt=GtkButton(" "+actstr+" ")
	abutt.connect("clicked",restart_ice)  # bug fix, 5.5.2003, left this out of vers. 2.2
	TIPS.set_tip(abutt,actstr)
	cbutton=GtkButton(_("Close"))
	TIPS.set_tip(cbutton,_("Close"))
	cbutton.connect("clicked",doQuit)
	hbox1=GtkHBox(1,0)
	hbox1.set_border_width(3)
	hbox1.set_spacing(5)
	hbox1.pack_start(abutt,1,1,0)
	hbox1.pack_start(cbutton,1,1,0)
	vbox.pack_start(hbox1,0,0,1)
	self.mainvbox=vbox
	vbox.show_all()

    def run_as_root(self,root_bool):
	# added 6.22.2003 - Run As Root functionality
	global CONFIG_THEME_PATH
	if root_bool==1:
		CONFIG_THEME_PATH=getIceWMConfigPath()+"cursors"+os.sep
		icepref_td.CONFIG_THEME_PATH=getIceWMConfigPath()+"cursors"+os.sep
	else:
		CONFIG_THEME_PATH=getIceWMPrivConfigPath()+"cursors"+os.sep
		icepref_td.CONFIG_THEME_PATH=getIceWMPrivConfigPath()+"cursors"+os.sep
	try:
		os.makedirs(CONFIG_THEME_PATH)
	except:
		pass
	for ii in self.widgets:
		ii.PREV.update_image(CONFIG_THEME_PATH+ii.get_data("my_image"))


# main class
class mousewin:
    def __init__(self) :
	self.version=this_software_version
	cwin=GtkWindow()
	cwin=GtkWindow(GTK.WINDOW_TOPLEVEL)
	cwin.set_wmclass("icewmcp-mouse","icewmcp-Mouse")
	cwin.realize()
	cwin.set_title(_("IceWM CP - Mouse Settings")+" "+self.version)
	cwin.set_position(GTK.WIN_POS_CENTER)
	self.cwin=cwin

	self.vbox=GtkVBox(0,0)

	menu_items = [
				[_('/_File'), None, None, 0, '<Branch>'],
				[_('/File/_Apply Changes Now...'), "<control>A", restart_ice, 0, ''],
				[_('/File/sep1'), None, None, 1, '<Separator>'],
 				[_("/_File")+"/_"+FILE_RUN,"<control>R", rundlg,421,""],
 				[_("/_File")+"/_"+_("Check for newer versions of this program..."), "<control>U", checkSoftUpdate,420,""],
				[_('/File/sep1'), None, None, 1, '<Separator>'],
				[_('/File/_Exit'), '<control>Q', doQuit, 0, ''],
				[_('/_Help'), None, None, 0, '<LastBranch>'],
				[_('/Help/_About...'), "F2", self.do_about, 0, ''],
  (_("/_Help")+"/_"+APP_HELP_STR, "F4", displayHelp,5012, ""),
  (_("/_Help")+"/_"+CONTRIBUTORS+"...", "F3", show_credits,913, ""),
  (_("/_Help")+"/_"+BUG_REPORT_MENU+"...", "F5", file_bug_report,5012, ""),
					]

	ag = GtkAccelGroup()
	itemf = GtkItemFactory(GtkMenuBar, '<main>', ag)
	cwin.add_accel_group(ag)
	itemf.create_items(menu_items)
	self.menubar = itemf.get_widget('<main>')	

	# added 6.21.2003 - "run as root" menu selector
	self.leftmenu=self.menubar.children()[0].get_submenu()
	self.run_root_button=GtkCheckButton(" "+RUN_AS_ROOT)
	self.run_root_menu=GtkMenuItem()
	self.run_root_menu.add(self.run_root_button)
	self.leftmenu.prepend(self.run_root_menu)
	self.run_root_button.connect("clicked",self.run_as_root)

	vbox1=GtkVBox(0,0)
	vbox=self.vbox
	vbox.set_border_width(4)
	vbox1.pack_start(self.menubar,0,0,0)
	vbox1.pack_start(vbox,1,1,0)
	vbox.pack_start(getImage(getBaseDir()+"icewmcp.png",DIALOG_TITLE),0,0,1)
	self.title_lab=GtkLabel(translateP("Mouse"))
	vbox.pack_start(self.title_lab,0,0,1)
	vbox.set_spacing(2)
	notebook = GtkNotebook()
	notebook.set_tab_pos(POS_TOP)
	notebook.set_scrollable(TRUE)
	vbox.pack_start(notebook,1,1,2)
	mousepan=mousetab()
	curspan=cursortab(cwin)
	self.curspan=curspan
	global ICE_TAB
	ICE_TAB=icepref.PullTab("Mouse")
	icetab=ICE_TAB.get_tab()
	icepan=GtkVBox(0,0)
	icepan.set_spacing(3)
	icepan.pack_start(GtkLabel(_("Mouse Behavior")),0,0,3)
	icepan.pack_start(icetab,1,1,0)
	actstr=_('/File/_Apply Changes Now...')[_('/File/_Apply Changes Now...').rfind("/")+1:len(_('/File/_Apply Changes Now...'))].replace("_","").replace(".","").strip()
	abutt=GtkButton(" "+actstr+" ")
	abutt.connect("clicked",restart_ice)
	TIPS.set_tip(abutt,actstr)
	cbutton=GtkButton(_("Close"))
	TIPS.set_tip(cbutton,_("Close"))
	cbutton.connect("clicked",doQuit)
	hbox1=GtkHBox(1,0)
	hbox1.set_border_width(3)
	hbox1.set_spacing(5)
	hbox1.pack_start(abutt,1,1,0)
	hbox1.pack_start(cbutton,1,1,0)
	icepan.pack_start(hbox1,0,0,2)

	notebook.append_page(mousepan.mainvbox, GtkLabel(_("Speed")))
	notebook.append_page(curspan.mainvbox, GtkLabel(translateP("Cursors")))
	notebook.append_page(icepan, GtkLabel(_("Behavior")))

	cwin.set_default_size(410,480)
	cwin.add(vbox1)
	cwin.connect("destroy",doQuit)
	global CWIN
	CWIN=cwin
	cwin.show_all()

    def do_about(self,*args):
	commonAbout(_("IceWM CP - Mouse Settings"),_("IceWM CP - Mouse Settings")+" "+self.version+"\n\n"+_("IceWM CP - Cursor Editor")+" "+self.version+"\n\n"+_("A Gtk-based mouse configuration utility")+" "+_("for IceWM (written in 100% Python)."))

    def run_as_root(self,*args):
	# added 6.22.2003 - Run As Root functionality
	global ICE_TAB
	if self.run_root_button.get_active():
		ICE_TAB.run_as_root(1)
		self.curspan.run_as_root(1)
		self.cwin.set_title(_("IceWM CP - Mouse Settings")+" "+self.version+"   [ROOT]")
		self.title_lab.set_text(translateP("Mouse")+"   [ROOT]")
	else:
		ICE_TAB.run_as_root(0)
		self.curspan.run_as_root(0)
		self.cwin.set_title(_("IceWM CP - Mouse Settings")+" "+self.version)
		self.title_lab.set_text(translateP("Mouse"))
	self.menubar.deactivate()


# main methods
def run(doquit=1) :
	global DO_QUIT
	DO_QUIT=doquit	
	mousewin()
	hideSplash()
	if doquit==1: gtk.mainloop()

if __name__== "__main__" :
	run()
