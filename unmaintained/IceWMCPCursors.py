#! /usr/bin/env python

##############################
#  IceWMCP - Cursor Editor
#  
#  Copyright 2003 by Erica Andrews
#  PhrozenSmoke@yahoo.com
#  http://icesoundmanager.sourceforge.net
#  
#  A simple Gtk-based cursor configuration utility
#  for IceWM (written in 100% Python).
#  
#  This program is distributed free
#  of charge (open source) under the GNU
#  General Public License
##############################

import os,GTK,gtk,sys
from gtk import *

#set translation support
from icewmcp_common import *
_=translateCP   # from icewmcp_common.py

import icewmcp_common
icewmcp_common.NOSPLASH=1  # disable splash window from icepref_td

import icepref_td
from icepref_td import *

class cursors:
    def __init__(self):
	self.version=this_software_version
	cwin=GtkWindow()
	tips=GtkTooltips()
	cwin=GtkWindow(GTK.WINDOW_TOPLEVEL)
	cwin.set_wmclass("icewmcp-cedit","icewmcp-cedit")
	cwin.realize()
	cwin.set_title(_("IceWM CP - Cursor Editor")+" "+self.version)
	cwin.set_position(GTK.WIN_POS_CENTER)
	self.cwin=cwin
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

	menu_items = [
				[_('/_File'), None, None, 0, '<Branch>'],
				[_('/File/_Apply Changes Now...'), "<control>A", self.restart_ice, 0, ''],
				[_('/File/sep1'), None, None, 1, '<Separator>'],
 				[_("/_File")+"/_"+FILE_RUN,"<control>R", rundlg,421,""],
 				[_("/_File")+"/_"+_("Check for newer versions of this program..."), "<control>U", checkSoftUpdate,420,""],
				[_('/File/sep1'), None, None, 1, '<Separator>'],
				[_('/File/_Exit'), '<control>Q', self.doQuit, 0, ''],
				[_('/_Help'), None, None, 0, '<LastBranch>'],
				[_('/Help/_About...'), "F2", self.do_about, 0, ''],
  (_("/_Help")+"/_"+APP_HELP_STR, "F4", displayHelp,5001, ""),
  (_("/_Help")+"/_"+CONTRIBUTORS+"...", "F3", show_credits,913, ""),
  (_("/_Help")+"/_"+BUG_REPORT_MENU+"...", "F5", file_bug_report,5001, ""),
					]

	ag = GtkAccelGroup()
	itemf = GtkItemFactory(GtkMenuBar, '<main>', ag)
	cwin.add_accel_group(ag)
	itemf.create_items(menu_items)
	self.menubar = itemf.get_widget('<main>')	
	vbox=self.vbox
	vbox.pack_start(self.menubar,0,0,0)
	vbox.pack_start(getImage(getBaseDir()+"icewmcp.png",DIALOG_TITLE),0,0,2)
	vbox.pack_start(GtkLabel(_("Cursor Editor")),0,0,2)
	sc=GtkScrolledWindow()
	slay=GtkLayout()
	self.slay=slay
	sc.add(slay)
	vbox.pack_start(sc,1,1,6)
	myy=3
	for ii in self.cursors:
		hb=GtkHBox(0,0)
		hb.set_border_width(2)
		cwid=ImageWidget(ii, "\""+ii+"\"",ImagePreviewer(0))
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
	cwin.set_default_size(420,470)
	actstr=_('/File/_Apply Changes Now...')[_('/File/_Apply Changes Now...').rfind("/")+1:len(_('/File/_Apply Changes Now...'))].replace("_","").replace(".","").strip()
	abutt=GtkButton(" "+actstr+" ")
	TIPS.set_tip(abutt,actstr)
	vbox1=GtkVBox(0,0)
	vbox1.set_border_width(5)
	vbox1.pack_start(abutt,0,0,0)
	vbox.pack_start(vbox1,0,0,2)
	cwin.add(vbox)
	cwin.connect("destroy",self.doQuit)
	cwin.show_all()

    def doQuit(self,*args):
	mainquit()

    def do_about(self,*args):
	commonAbout(_("IceWM CP - Cursor Editor"),_("IceWM CP - Cursor Editor")+" "+self.version+"\n\n"+_("A Gtk-based cursor configuration utility")+" "+_("for IceWM (written in 100% Python)."))

    def restart_ice(self,*args) :
	if msg_confirm(DIALOG_TITLE,_("WARNING:\nThis feature doesn't work perfectly on all systems.\nBSD and some other systems may experience problems."))	== 1:
		os.system('killall -HUP -q icewm &')
		os.system('killall -HUP -q icewm-gnome &')

def run(*args):
	cursors()
	hideSplash()
	mainloop()
if __name__== "__main__" :
	run()
	