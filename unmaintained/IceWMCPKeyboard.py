#! /usr/bin/env python

######################################
# IceWM Keyboard Settings Settings 
# 
# Copyright 2003 by Erica Andrews 
# (PhrozenSmoke@yahoo.com)
# http://icesoundmanager.sourceforge.net
# 
# A simple Gtk-based utility for setting the keyboard repeat 
# rate and delay.  It's a simple frontend for 'xset'.
# 
# This program is distributed under the GNU General
# Public License (open source).
#######################################

import os,GTK,gtk,sys
from gtk import *

#set translation support
from icewmcp_common import *
_=translateCP   # from icewmcp_common.py

class keywin:
    def __init__(self) :
	global WMCLASS
	WMCLASS="icewmcontrolpanel"
	global WMNAME
	WMNAME="IceWMControlPanel"
	keywin=GtkWindow(GTK.WINDOW_TOPLEVEL)
	keywin.set_wmclass(WMCLASS,WMNAME)
	keywin.realize()
	keywin.set_title(_("IceWM CP - Keyboard Settings")+" v."+this_software_version)
	keywin.set_position(GTK.WIN_POS_CENTER)
	self.keywin=keywin
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
  (_("/_Help")+"/_"+APP_HELP_STR, "F4", displayHelp,5013, ""),
  (_("/_Help")+"/_"+CONTRIBUTORS+"...", "F3", show_credits,913, ""),
  (_("/_Help")+"/_"+BUG_REPORT_MENU+"...", "F5", file_bug_report,5013, ""),
        ])

        keywin.add_accel_group(ag)
        mymenubar=itemf.get_widget("<main>")
        mymenubar.show()
	mainvbox1.pack_start(mymenubar,0,0,0)
	mainvbox=GtkVBox(0,1)
	mainvbox1.pack_start(mainvbox,1,1,0)
	mainvbox.set_border_width(4)
	mainvbox.set_spacing(4)
	mainvbox.pack_start(getImage(getBaseDir()+"icewmcp.png",_("IceWM Control Panel: Keyboard")),0,0,2)
	mainvbox.pack_start(GtkLabel(_(" Select your desired keyboard repeat rate: ")),0,0,5)	
	hbox=GtkHBox(0,0)
	c=GtkCombo()
	c2=GtkCombo()
	mlist=[]
	for ii in range(10,100):
		if not ii==0:
			mlist.append(str(ii))
	c2.set_popdown_strings(mlist)
	self.c2_entry=c2.entry
	self.c2_entry.set_text("20")	
	self.c2_entry.set_editable(0)
	c.set_popdown_strings(["25","50","100","150","200","225","250","275","300","325","350","375","400","425","450","475","500","550","600","650","700","750","800","850","900","950","1000","1500","2000"])
	hbox.pack_start(GtkLabel(_("Delay: ")),0,0,2)
	self.c_entry=c.entry
	self.c_entry.set_text("400")	
	self.c_entry.set_editable(0)
	TIPS.set_tip(c.entry,_("Delay: ").replace(":","").strip())
	hbox.pack_start(c,0,0,2)
	hbox.pack_start(GtkLabel("    "),1,1,2)
	hbox.pack_start(GtkLabel(_("Rate: ")),0,0,2)
	hbox.pack_start(c2,0,0,2)
	TIPS.set_tip(c2.entry,_("Rate: ").replace(":","").strip())
	mainvbox.pack_start(hbox,1,1,0)
	hbox1=GtkHBox(0,0)
	hbox1.set_spacing(6)
	hbox1.pack_start(GtkLabel(_("Test: ")),0,0,0)
	self.entry=GtkEntry()
	TIPS.set_tip(self.entry,_("Test: ").replace(":","").strip())
	hbox1.pack_start(self.entry,1,1,0)	
	mainvbox.pack_start(hbox1,0,0,0)
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
	mainvbox.pack_start(hb,1,1,4)
	keywin.add(mainvbox1)
	abutton.connect("clicked",self.doApply)
	rbutton.connect("clicked",self.doReset)
	cbutton.connect("clicked",self.doQuit)
	keywin.connect("destroy",self.doQuit)
	keywin.show_all()

    def doQuit(self,*args) :
	gtk.mainquit()

    def doReset(self,*args) : # reset to a reasonable speed
	os.popen("xset r rate 400 20 &> /dev/null &")
	self.c_entry.set_text("400")
	self.c2_entry.set_text("20")
	self.entry.set_text("")

    def doApply(self,*args) : # set to desired speed
	os.popen("xset r rate "+self.c_entry.get_text()+" "+self.c2_entry.get_text()+" &> /dev/null &")
	self.entry.set_text("")

def run() :
	keywin()
	hideSplash()
	gtk.mainloop()

if __name__== "__main__" :
	run()
