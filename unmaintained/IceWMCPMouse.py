#! /usr/bin/env python

######################################
# IceWM Mouse Settings Settings 2.1
# 
# Copyright 2003 by Erica Andrews 
# (PhrozenSmoke@yahoo.com)
# http://icesoundmanager.sourceforge.net
# 
# A simple Gtk-based utility for setting the mouse speed.
# It's a simple frontend for 'xset'.
# 
# This program is distributed under the GNU General
# Public License (open source).
#######################################

import os,GTK,gtk,sys
from gtk import *

#set translation support
from icewmcp_common import *
_=translateCP   # from icewmcp_common.py

class mousewin:
    def __init__(self) :
	global WMCLASS
	WMCLASS="icewmcontrolpanel"
	global WMNAME
	WMNAME="IceWMControlPanel"
	mousewin=GtkWindow(GTK.WINDOW_TOPLEVEL)
	mousewin.set_wmclass(WMCLASS,WMNAME)
	mousewin.realize()
	mousewin.set_title(_("IceWM CP - Mouse Settings")+" v."+this_software_version)
	mousewin.set_position(GTK.WIN_POS_CENTER)
	self.mousewin=mousewin
	mainvbox1=GtkVBox(0,1)
	mainvbox1.set_spacing(2)
	mymenubar=GtkMenuBar()
	self.mymenubar=mymenubar

        ag=GtkAccelGroup()
        itemf=GtkItemFactory(GtkMenuBar, "<main>", ag)
        itemf.create_items([
            # path              key           callback    action#  type
  (_("/ _File"),  "<alt>F",  None,  0,"<Branch>"),
 				[_("/ _File")+"/_"+FILE_RUN,"<control>R", rundlg,421,""],
  (_("/ _File")+"/_"+_("Check for newer versions of this program..."), "<control>U", checkSoftUpdate,420,""),
  (_("/ _File/sep4"), None,  None,  49, "<Separator>"),
  (_("/ _File/_Quit"), "<control>Q", self.doQuit,10,""),
  (_("/_Help"),  "<alt>H",  None,16, "<LastBranch>"), 
  (_("/_Help")+"/_"+APP_HELP_STR, "F4", displayHelp,5012, ""),
  (_("/_Help")+"/_"+CONTRIBUTORS+"...", "F3", show_credits,913, ""),
  (_("/_Help")+"/_"+BUG_REPORT_MENU+"...", "F5", file_bug_report,5012, ""),
        ])

        mousewin.add_accel_group(ag)
        mymenubar=itemf.get_widget("<main>")
        mymenubar.show()
	mainvbox1.pack_start(mymenubar,0,0,0)
	mainvbox=GtkVBox(0,1)
	mainvbox1.pack_start(mainvbox,1,1,0)
	mainvbox.set_border_width(4)
	mainvbox.set_spacing(4)
	mainvbox.pack_start(getImage(getBaseDir()+"icewmcp.png",_("IceWM Control Panel: Mouse")),0,0,2)
	mainvbox.pack_start(GtkLabel(_(" Select your desired mouse speed: ")),0,0,3)	
	c=GtkCombo()
	mlist=[]
	for ii in range(36):
		if not ii==0:
			mlist.append(str(ii))
	c.set_popdown_strings(mlist)
	TIPS.set_tip(c.entry,_(" Select your desired mouse speed: ").replace(":","").strip())
	self.c_entry=c.entry
	self.c_entry.set_text("4")	
	self.c_entry.set_editable(0)
	mainvbox.pack_start(c,0,0,2)
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
	mainvbox.pack_start(hb,1,1,2)
	mousewin.add(mainvbox1)
	abutton.connect("clicked",self.doApply)
	rbutton.connect("clicked",self.doReset)
	cbutton.connect("clicked",self.doQuit)
	mousewin.connect("destroy",self.doQuit)
	mousewin.show_all()

    def doQuit(self,*args) :
	gtk.mainquit()

    def doReset(self,*args) : # reset to a reasonable speed
	os.popen("xset m 4 &> /dev/null &")
	self.c_entry.set_text("4")

    def doApply(self,*args) : # set to desired speed
	self.swin=None
	os.popen("xset m "+self.c_entry.get_text()+" &> /dev/null &")
	self.doSafe()

    def doSafe(self,*args):
	global WMCLASS
	global WMNAME
	swin=GtkWindow(GTK.WINDOW_TOPLEVEL)
	swin.set_wmclass(WMCLASS,WMNAME)
	swin.realize()
	swin.set_title("IceWM CP - Mouse Settings")
	swin.set_position(GTK.WIN_POS_CENTER)
	mainv=GtkVBox(0,0)
	mainv.set_spacing(3)
	mainv.set_border_width(4)
	mainv.pack_start(GtkLabel(_("Your mouse speed has been set to '")+self.c_entry.get_text()+_("'.\nIf you are not happy with this mouse speed,\nwait 7 seconds and the mouse speed will be\nreset to a reasonable speed.\n")),0,0,2)
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

def run() :
	mousewin()
	hideSplash()
	gtk.mainloop()

if __name__== "__main__" :
	run()
