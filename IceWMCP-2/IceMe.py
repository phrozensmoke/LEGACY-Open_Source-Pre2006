#!/usr/bin/env python
# -*- coding: ISO-8859-1 -*-

################################################
## IceMe
# =====
#
# Copyright 2000-2002, Dirk Moebius <dmoebius@gmx.net> 
# and Mike Hostetler <thehaas@binary.net>
#
# This work comes with no warranty regarding its suitability for any purpose.
# The author is not responsible for any damages caused by the use of this
# program.
#
# This software is distributed under the GNU General Public License
#################################################
#################################
#  With Modifications by Erica Andrews 
#  (PhrozenSmoke ['at'] yahoo.com)
#  February 2003-February 2004
#  
#  This is a modified version of IceMe 
#  1.0.0 ("IceWMCP Edition"), optimized for 
#  IceWM ControlPanel.
#  
#  Copyright (c) 2003-2004
#  Erica Andrews
#  PhrozenSmoke ['at'] yahoo.com
#  http://icesoundmanager.sourceforge.net
#################################
#############################################
#	This program is free software; you can redistribute
#	it and/or modify it under the terms of the GNU 
#	General Public License as published by the 
#	Free Software Foundation; either version 2 of the
#	License, or (at your option) any later version.
#
#	This program is distributed in the hope that it will 
#	be useful, but WITHOUT ANY WARRANTY; 
#	without even the implied warranty of 
#	MERCHANTABILITY or FITNESS FOR A 
#	PARTICULAR PURPOSE.
#
#	You should have received a copy of the GNU 
#	General Public License along with this program; 
#	if not, write to the Free Software Foundation, Inc., 
#	59 Temple Place - Suite 330, Boston, MA 
#	02111-1307, USA.
#############################################
#############################
#  PyGtk-2 Port Started By: 
#  	David Moore (djm6202@yahoo.co.nz)
#	March 2003
#############################
#############################
#  PyGtk-2 Port Continued By: 
#	Erica Andrews
#  	PhrozenSmoke ['at'] yahoo.com
#	October/November 2003
#############################

import glob,stat
from popen2 import popen2

import icewmcp_common
# disable splash for internal IcePref stuff coming from IceWMCPKeyboard
icewmcp_common.NOSPLASH=1

import icepref  # added 12.20.2003 for menu behavior tab, Erica Andrews

#set translation support
from icewmcp_common import *

def _(somestr):
	return to_utf8(translateME(somestr))

def _CP(somestr): # added 12.20.2003 for menu behavior tab, Erica Andrews
	return to_utf8(translateCP(somestr))  # from icewmcp_common.py

from constants import *
from Preferences import Preferences
from IceMenuTree import IceMenuTree
from MenuParser import MenuParser
#from IconSelectionDialog import IconSelectionDialog     # disabled, 4/25/2003 - Erica Andrews
import IconSelectionDialog     # import module, instead of class, 4/25/2003 - Erica Andrews
from PreviewWindow import PreviewWindow
from icewmcp_dnd import *  # 5.16.2003 - drag-n-drop support


VERSION = this_software_version+"/IceWMCP Edition"


class IceMe(Window):

    ########################################################################
    # init GUI
    ########################################################################

    def __init__(self, preferences):
	self.last_exec=''   # added 4.3.2003, Erica Andrews, file selection 'memory'
	self.i_am_root=0  # added 6.22.2003 - are we running as root?
	self.diable_file_overrides() # added 12.21.2003, Erica Andrews

        Window.__init__(self, WINDOW_TOPLEVEL)
	self.set_title("IceMe - IceWMCP Edition")
	self.set_position(WIN_POS_CENTER)
	set_basic_window_icon(self)
	self.set_wmclass("iceme","IceMe")  # added 4.4.2003, Erica Andrews
        self.set_default_size(GDK.screen_width()-140, GDK.screen_height()-175)
        self.connect("delete_event", mainquit)

        self.preferences       = preferences
        self.iconsel_dlg       = None
        self.do_update_widgets = 0
        self.key_editor_dlg    = None

        self.__initIconCache(preferences.getIconPaths())
        menu = self.__initMenu()
        tb = self.__initToolbar()

        self.tree = IceMenuTree(self)
        self.tree.connect("tree-select-row", self.on_tree_select_row)
        self.tree.connect_after("drag-begin", self.on_tree_drag_begin)
        self.tree.connect_after("drag-end", self.on_tree_drag_end)
        self.tree.show()

        scr = ScrolledWindow()
        scr.set_policy(POLICY_AUTOMATIC, POLICY_AUTOMATIC)
        scr.add(self.tree)
        scr.show()

        name_label = Label(_("Name:"))
        name_label.set_alignment(1.0, 0.5)
        name_label.show()

        self.name_entry = Entry()
        self.name_entry.connect("changed", self.on_name_changed)
        self.name_entry.show()

        self.command_label = Label(_("Command:"))
        self.command_label.set_alignment(1.0, 0.5)
        self.command_label.show()

        self.command = Entry()
        self.command.connect("changed", self.on_command_changed)
	self.command.set_data("sensitive",0)
	self.command.set_data("is_command",1)
	addDragSupport(self.command,self.setDrag)  # added 12.21.2003, Erica Andrews DnD support
        self.command.show()

        cmd_icon =getImage(getPixDir()+"iceme_open.png",_CP("Browse..."))
        cmd_icon.show()
        self.command_button = Button()
        self.command_button.add(cmd_icon)
        self.command_button.connect("clicked", self.on_command_button_clicked)
	self.command_button.set_data("is_command",1)
	addDragSupport(self.command_button,self.setDrag)  # added 12.21.2003, Erica Andrews DnD support
        self.command_button.show()

        command_hbox = HBox(FALSE, 5)
        command_hbox.pack_start(self.command)
        command_hbox.pack_start(self.command_button, expand=FALSE, fill=FALSE)
        command_hbox.show()

        icon_label = Label(_("Icon:"))
        icon_label.set_alignment(1.0, 0.5)
        icon_label.show()

        self.icon_name = Entry()
	self.icon_name.set_data("sensitive",0)
	addDragSupport(self.icon_name,self.setDrag)  # 2.24.2003 - drag-n-drop support, drop icons on text entry
        self.icon_name.connect("focus-out-event", self.on_icon_name_focus_out_event)
        self.icon_name.show()

        dummy, pix, mask = self.getCachedIcon("-")
	self.icon=Image()
        self.icon.set_from_pixmap(pix, mask)
        self.icon.show()

        self.icon_button = Button(None)
        self.icon_button.add(self.icon)
	addDragSupport(self.icon_button,self.setDrag)   # 2.24.2003 - drag-n-drop support, drop icons on button
        self.icon_button.connect("clicked", self.on_icon_button_clicked)
        self.icon_button.show()

        icon_hbox = HBox(FALSE, 5)
        icon_hbox.pack_start(self.icon_name)
        icon_hbox.pack_start(self.icon_button, expand=FALSE, fill=FALSE)
        icon_hbox.show()

        sep = HSeparator()
        sep.show()

        self.is_restart = CheckButton(_("Start as a new window manager"))
        self.is_restart.connect("toggled", self.on_is_restart_toggled)
        self.is_restart.show()

        self.add_shortcut = getPixmapButton(None, STOCK_ADD ,_("Add shortcut..."))
        self.add_shortcut.connect("clicked", self.icewmcpKeyEdit) # changed 2.21.2003 - run IceWMCP-KeyEdit
        self.add_shortcut.show()

        table = Table(2, 6, FALSE)
        table.attach(name_label,         0, 1, 0, 1, 0)
        table.attach(self.name_entry,          1, 2, 0, 1, (EXPAND+FILL))
        table.attach(self.command_label, 0, 1, 1, 2, 0)
        table.attach(command_hbox,       1, 2, 1, 2, (EXPAND+FILL))
        table.attach(icon_label,         0, 1, 2, 3, 0)
        table.attach(icon_hbox,          1, 2, 2, 3, (EXPAND+FILL))
        table.attach(sep,                0, 2, 3, 4, (EXPAND+FILL))
        table.attach(self.is_restart,    1, 2, 4, 5, (EXPAND+FILL))
        table.attach(self.add_shortcut,  1, 2, 5, 6, (EXPAND+FILL))
        table.set_border_width(10)
        table.set_row_spacings(5)
        table.set_col_spacings(5)
        table.show()

        self.rightframe = Frame(_("Menu entry"))
	self.table_box=VBox(0,0)
	self.table_box.pack_start(table,0,0,0)
	self.table_box.pack_start(Label(" "),1,1,0)
	try:
		self.table_box.pack_start(loadImage(getBaseDir()+"icewmcp_short.png"),0,0,2)
		self.table_box.pack_start(Label(" "),1,1,0)
	except:
		pass
        self.rightframe.set_border_width(10)
        self.rightframe.add(self.table_box)
        self.rightframe.show_all()

        hpaned = HPaned()
        hpaned.pack1(scr, TRUE, TRUE)
        hpaned.pack2(self.rightframe, FALSE, FALSE)
        hpaned.set_position(350)  # changed 12.20.2003, Erica Andrews, more space
        hpaned.show()

        self.statusbar = Statusbar()
        self.statusbar_context = self.statusbar.get_context_id("main")
        self.setStatus("IceMe v" + VERSION)
        self.statusbar.show()

	# added 6.21.2003 - "run as root" menu selector
	self.leftmenu=self.menubar.get_children()[0].get_submenu()
	self.run_root_button=CheckMenuItem(" "+RUN_AS_ROOT)
	self.leftmenu.prepend(self.run_root_button)
	self.run_root_button.connect("toggled",self.run_as_root)
	self.leftmenu.show_all()


	# Feature enhancement: 12.20.2003, Erica Andrews, added a notebook setup:
	# on the first tab - the standard IceMe menu editor, on the second tab -
	# Menu behavior management from IcePref2's embedded Menu tab
	self.BSD_WARN=0
        vbox_iceme = VBox(0,0)
        vbox_iceme.pack_start(tb, FALSE)
        vbox_iceme.pack_start(hpaned, 1,1, 0)
        vbox_iceme.show()

	ICE_TAB=icepref.PullTab("Menus")
	self.ICE_TAB=ICE_TAB
	icetab=ICE_TAB.get_tab()
	icepan=VBox(0,0)
	icepan.set_spacing(3)
	icepan.pack_start(getImage(getBaseDir()+"icepref2.png","ICEPREF2"),0,0,1)
	icepan.pack_start(Label(_("Menu")),0,0,3)
	icepan.pack_start(icetab,1,1,0)
	actstr=_CP('/File/_Apply Changes Now...')[_CP('/File/_Apply Changes Now...').rfind("/")+1:len(_CP('/File/_Apply Changes Now...'))].replace("_","").replace(".","").strip()
	abutt=getPixmapButton(None, STOCK_APPLY ,actstr)
	abutt.connect("clicked",self.apply_icepref_settings)
	TIPS.set_tip(abutt,actstr)
	hbox1=HBox(1,0)
	hbox1.set_border_width(3)
	hbox1.set_spacing(5)
	hbox1.pack_start(Label(" "),1,1,0)
	hbox1.pack_start(abutt,0,0,0)
	hbox1.pack_start(Label(" "),1,1,0)
	icepan.pack_start(hbox1,0,0,2)
	icepan.show_all()

	notebox=VBox(0,0)
	notebook = Notebook()
	self.notebook=notebook
	notebook.set_tab_pos(POS_TOP)
	notebook.set_scrollable(TRUE)
	notebox.pack_start(notebook,1,1,2)
	notebook.append_page(vbox_iceme, Label(_("Menu")))
	notebook.append_page(icepan, Label(_CP("Behavior")))
	notebook.show()
	notebox.show()

        vbox = VBox(0,0)
        vbox.pack_start(menu, FALSE)
        vbox.pack_start(notebox, 1,1, 0)
        vbox.pack_start(self.statusbar, FALSE)
        vbox.show()
        self.add(vbox)

	# added 4/27/2003 - Erica Andrews, new prettier start-up process, more informative with Splash screen
	# code comes from 'icewmcp_common.py'
	setSplash(self.initAll, "IceMe:  "+_("Loading menus and icons...please wait."))
	showSplash()

    def apply_icepref_settings(self, *args) :
	# added 12.20.2003 for menu behavior tab, Erica Andrews
	if not self.BSD_WARN==1:
		if not msg_confirm(DIALOG_TITLE,_CP("WARNING:\nThis feature doesn't work perfectly on all systems.\nBSD and some other systems may experience problems."))==1:
			return
	self.BSD_WARN=1
	self.ICE_TAB.save_current_settings("nowarn")
	#   changed 12.20.2003 - use common Bash shell probing
	#   to fix BUG NUMBER: 1523884
	#   Reported By: david ['-at-'] jetnet.co.uk
	#   Reported At: Fri Oct 31 23:47:12 2003
	fork_process("killall -HUP -q icewm")
	fork_process("killall -HUP -q icewm-gnome")


    # added 6.18.2003, Erica Andews - allow loading of menu,program, and toolbar 
    # files from arbitrary directories, not just the home directory...will be used to
    # to support 'run as root' fuctionality

    def initCustomDir(self,path_name):
	self.preferences.IGNORE_HOME=1
	self.preferences.GLOBAL_ICEWM_DIR=path_name

    # added 6.18.2003, Erica Andews - 'Run as Root' functionality, for system admins
    def run_as_root(self,*args):
	if self.run_root_button.get_active():
		self.i_am_root=1
		self.diable_file_overrides()
		self.initCustomDir(getIceWMConfigPath() )  # set path
		# reload everything
		self.ICE_TAB.run_as_root(1)  # added 12.20.2003, Erica Andrews
		setSplash(self.initAll, "IceMe [ROOT]: "+_("Loading menus and icons...please wait."))
		showSplash()
		self.setStatus("IceMe v" + VERSION+"    [ROOT]")
		self.set_title("IceMe - IceWMCP Edition    [ROOT]")
	else:  # turn 'root' OFF
		self.i_am_root=0
		self.diable_file_overrides()
		self.initCustomDir(getIceWMPrivConfigPath() )  # set path
		# reload everything
		self.ICE_TAB.run_as_root(0) # added 12.20.2003, Erica Andrews
		setSplash(self.initAll, "IceMe: "+_("Loading menus and icons...please wait."))
		showSplash()
		self.setStatus("IceMe v" + VERSION)
		self.set_title("IceMe - IceWMCP Edition")
	self.menubar.deactivate()


    def diable_file_overrides(self, *args):
	#    added 12.21.2003, Erica Andrews: allow opening of abitrary
	#    menu, programs, and toolbar files for editing
	self.override_menu_file=None
	self.override_programs_file=None
	self.override_toolbar_file=None

    def menu_selection_dialog_ok(self, *args):
	#    added 12.21.2003, Erica Andrews: allow opening of abitrary
	#    menu, programs, and toolbar files for editing
	try:
		entries=[args[0].get_data("menu-entry"), 
			args[0].get_data("prog-entry"),
			args[0].get_data("tool-entry")]
		for ii in entries:
			if ii.get_text().strip()=="": 
				return
			if ii.get_text().find(os.sep)==-1: 
				return
			if ii.get_text().strip().endswith(os.sep): 
				return
		self.override_menu_file=entries[0].get_text().strip()
		self.override_programs_file=entries[1].get_text().strip()
		self.override_toolbar_file=entries[2].get_text().strip()
		sel_file=self.override_menu_file
		sel_dir=sel_file[0:sel_file.rfind(os.sep)+1]
		self.initCustomDir(sel_dir)  # set path
		# reload everything
		setSplash(self.initAll, "IceMe : "+_("Loading menus and icons...please wait."))
		showSplash()
		self.setStatus("IceMe v" + VERSION+"    ["+sel_file+"]")
		self.set_title("IceMe - IceWMCP Edition")
	except:
		return
	try:
		args[0].get_data("selwin").hide()
		args[0].get_data("selwin").destroy()
		args[0].get_data("selwin").unmap()
	except:
		pass

    def menu_selection_dialog_close(self, *args):
	#    added 12.21.2003, Erica Andrews: allow opening of abitrary
	#    menu, programs, and toolbar files for editing
	try:
		args[0].get_data("selwin").hide()
		args[0].get_data("selwin").destroy()
		args[0].get_data("selwin").unmap()
	except:
		pass

    def menu_sel_file_box_cb(self, *args):
	#    added 12.21.2003, Erica Andrews: allow opening of abitrary
	#    menu, programs, and toolbar files for editing
	try:
		ff=GET_SELECTED_FILE()
		if not ff: return
		if ff=="": return
		if ff.endswith(os.sep): return
		self.file_entry_to_update.set_text(ff)
	except:
		pass

    def menu_sel_file_box(self, *args):
	#    added 12.21.2003, Erica Andrews: allow opening of abitrary
	#    menu, programs, and toolbar files for editing
	import icewmcp_common
	need_menu=0
	if icewmcp_common.ICEWMCP_LAST_FILE==getIceWMPrivConfigPath(): need_menu=1
	if icewmcp_common.ICEWMCP_LAST_FILE==None: need_menu=1
	if icewmcp_common.ICEWMCP_LAST_FILE=='': need_menu=1
	if need_menu==1:
		icewmcp_common.ICEWMCP_LAST_FILE=self.preferences.getIceWMFile("menu")
	try:
		self.file_entry_to_update=args[0].get_data("myentry")
		SELECT_A_FILE(self.menu_sel_file_box_cb,_CP("Select a file..."),"iceme","IceMe")
	except:
		pass

    def open_menu_selection_dialog(self, *args):
	#    added 12.21.2003, Erica Andrews: allow opening of abitrary
	#    menu, programs, and toolbar files for editing
	selwin=Window( WINDOW_TOPLEVEL)
	selwin.set_title(_("Open IceWM Menu File")+"...")
	set_basic_window_icon(selwin)
	selwin.set_position(WIN_POS_CENTER)
	selwin.set_wmclass("iceme","IceMe")
	selwin.set_data("selwin",selwin)
        selwin.connect("delete_event", self.menu_selection_dialog_close)
	selbox=VBox(0,0)
	selbox.set_border_width(4)
	selbox.set_spacing(3)
	selbox.pack_start(Label(_("Open IceWM Menu File")),0,0,2)

        table = Table(2, 3, FALSE)
	entries=[]
        table.attach(Label(_("Menu")+":"), 0, 1, 0, 1, 0)
	menubox=HBox(0,0)
	menubox.set_spacing(3)
	menentry=Entry()
	menentry.set_size_request(280,-1)
	menentry.set_text(self.preferences.getIceWMFile("menu"))
	entries.append(menentry)
	menubox.pack_start(menentry, 1,1,0)
        selbutton1 = Button()
        selbutton1.add(getImage(getPixDir()+"iceme_open.png",_CP("Browse...")))
        selbutton1.connect("clicked", self.menu_sel_file_box)
	selbutton1.set_data("myentry",menentry)
	menubox.pack_start(selbutton1, 0,0,0)
        table.attach(menubox,  1, 2, 0, 1, (EXPAND+FILL))

        table.attach(Label(_("Programs")+":"), 0, 1, 1, 2, 0)
	menubox=HBox(0,0)
	menubox.set_spacing(3)
	menentry=Entry()
	menentry.set_text(self.preferences.getIceWMFile("programs"))
	entries.append(menentry)
	menubox.pack_start(menentry, 1,1,0)
        selbutton1 = Button()
        selbutton1.add(getImage(getPixDir()+"iceme_open.png",_CP("Browse...")))
        selbutton1.connect("clicked", self.menu_sel_file_box)
	selbutton1.set_data("myentry",menentry)
	menubox.pack_start(selbutton1, 0,0,0)
        table.attach(menubox, 1, 2, 1, 2, (EXPAND+FILL))

        table.attach(Label(_("Toolbar")+":"),  0, 1, 2, 3, 0)
	menubox=HBox(0,0)
	menubox.set_spacing(3)
	menentry=Entry()
	menentry.set_text(self.preferences.getIceWMFile("toolbar"))
	entries.append(menentry)
	menubox.pack_start(menentry, 1,1,0)
        selbutton1 = Button()
        selbutton1.add(getImage(getPixDir()+"iceme_open.png",_CP("Browse...")))
        selbutton1.connect("clicked", self.menu_sel_file_box)
	selbutton1.set_data("myentry",menentry)
	menubox.pack_start(selbutton1, 0,0,0)
        table.attach(menubox,  1, 2, 2, 3, (EXPAND+FILL))
        table.set_row_spacings(3)
        table.set_col_spacings(3)

	selh=HBox(0,0)
	okbutt=getPixmapButton(None, STOCK_APPLY ,DIALOG_OK)
	canbutt=getPixmapButton(None, STOCK_CANCEL ,DIALOG_CANCEL)
	TIPS.set_tip(canbutt,DIALOG_CANCEL)
	TIPS.set_tip(okbutt,DIALOG_OK)
	canbutt.set_data("selwin",selwin)
	canbutt.connect("clicked", self.menu_selection_dialog_close)
	okbutt.set_data("selwin",selwin)
	okbutt.set_data("menu-entry",entries[0])
	okbutt.set_data("prog-entry",entries[1])
	okbutt.set_data("tool-entry",entries[2])
	okbutt.connect("clicked", self.menu_selection_dialog_ok)
	selh.pack_start(okbutt,0,0,0)
	selh.pack_start(Label(""),1,1,0)
	selh.pack_start(canbutt,0,0,0)
	selbox.pack_start(table,0,0,0)
	selbox.pack_start(Label(""),1,1,0)
	selbox.pack_start(selh,0,0,0)
	selwin.add(selbox)
	selwin.set_modal(1)
	selwin.show_all()


    def initAll(self,*args):
	# added 4/25/2003 - Erica Andrews, new prettier start-up process, more informative
	# doesn't show ugly 'stalled' screen while icons and menus are loading, show only after everything is ready
	# added lots of error catching to help ensure start-up, regardless of errors
	try:
        	self.__initToolTips()
	except:
		pass
	try:
        	self.__initTree()
	except:
		pass
	try:
        	self.__initIcePref()
	except:
		pass
        self.show()  # do not use "show_all"
	hideSplash()
	import IceWMCPKeyboard
	self.IceWMCPKeyboard=IceWMCPKeyboard
	hideSplash()  # just to be extra sure
	return 0


    def __initToolTips(self):
	# changed 4/27/2003 - Erica Andrews, use 'TIPS' from icewmcp_common.py
        TIPS.set_tip(self.command_button,
            _("Click here to select an executable via a file selection box."))
        TIPS.set_tip(self.icon_button,
            _("Click here to select another icon"))
        TIPS.set_tip(self.is_restart,
            _("If selected, this entry will cause IceWM to quit and start the command as new window manager."))


    def __initTree(self):
        self.do_update_widgets = 0
	#   changed 12.21.2003, Erica Andrews: allow overriding of file locations
	#   to permit loading of arbitrary menu, programs, and toolbar files
        if self.override_menu_file==None: 
		menu_file = self.preferences.getIceWMFile("menu")
	else: 
		menu_file=self.override_menu_file
	if self.override_programs_file==None:
        	programs_file = self.preferences.getIceWMFile("programs")
	else:
		programs_file=self.override_programs_file
	if self.override_toolbar_file==None:
        	toolbar_file = self.preferences.getIceWMFile("toolbar")
	else:
		toolbar_file=self.override_toolbar_file
        rootnode = self.tree.init(menu_file, programs_file, toolbar_file)
	#print "FILES:  "+menu_file+" "+programs_file+"  "+toolbar_file
        self.do_update_widgets = 1
        self.tree.select(rootnode)


    def __initMenu(self):
        ag = AccelGroup()
        itemf = ItemFactory(MenuBar, "<main>", ag)
	self.ag=ag
	self.itemf=itemf
        itemf.create_items([
            # path              key           callback    action#  type
            (_("/_File"),          None,         None,             0, "<Branch>"),  

	    # added 6.19.2003 - Erica Andrews, support for opening random 'menu' files
            (_("/_File")+"/"+_("Open IceWM Menu File")+"...",   "<control>O", self.open_menu_selection_dialog,  112, ""),

            (_("/_File/_Save"),    "<control>S", self.on_save,     1, ""),
            (_("/_File/_Revert"),  "<control>T", self.on_revert,   2, ""),
            (_("/_File/_Preview"), "<control>P", self.on_preview,  3, ""),
	    (_('/_File/sep1'), None, None, 2, '<Separator>'),
 	    (_("/_File")+"/_"+FILE_RUN,"<control>R", rundlg,421,""), # added 4.2.2003
 	    (_("/_File")+"/_"+_("Check for newer versions of this program..."), "<control>U", checkSoftUpdate,420,""),
            (_("/_File/sep1"),     None,         None,             4, "<Separator>"),
            (_("/_File/_Exit"),    "<control>Q", mainquit,         5, ""),
            (_("/_Edit"),          None,         None,             6, "<Branch>"),
            (_("/_Edit/_Cut"),     "<control><shift>X", self.on_cut,      7, ""),
            (_("/_Edit/C_opy"),    "<control><shift>C", self.on_copy,     8, ""),
            (_("/_Edit/_Paste"),   "<control><shift>V", self.on_paste,    9, ""),
            (_("/_Edit/sep2"),     None,         None,            10, "<Separator>"),
            (_("/_Edit/_Delete"),  "<control>D",         self.on_delete,  11, ""),  # changed 4.27.2003, added keyboard shortcut
            (_("/_Edit/sep3"),     None,         None,            12, "<Separator>"),
            (_("/_Edit/_IceWM preferences"), "F9", self.icewmcpIcePref2, 13, ""),  # changed 2.21.2003
            (_("/_Edit/_Shortcuts"), "F10",       self.icewmcpKeyEdit, 14, ""), # changed 2.21.2003
            (_("/_Help"),          None,         None,            15, "<LastBranch>"),
            (_("/_Help/_About..."), "F2",        self.on_about,   16, ""),
	  (_("/_Help")+"/_"+APP_HELP_STR, "F4", displayHelp,5009, ""),  # added 4.26.2003
  (_("/_Help")+"/_"+CONTRIBUTORS+"...", "F3", show_credits,913, ""), # added 4.2.2003
  (_("/_Help")+"/_"+BUG_REPORT_MENU+"...", "F5", file_bug_report,5009, ""), # added 4.2.2003
        ])
        self.add_accel_group(ag)
        menu = itemf.get_widget("<main>")
	self.menubar=menu  # added 6.21.2003, Erica Andrews
        menu.show()

        # remember some menu entries that may be disabled later:
        self.menu_cut     = itemf.get_widget_by_action(7)
        self.menu_copy    = itemf.get_widget_by_action(8)
        self.menu_paste   = itemf.get_widget_by_action(9)
        self.menu_delete  = itemf.get_widget_by_action(11)
        self.menu_icepref = itemf.get_widget_by_action(13)

        hdlbox = HandleBox()
        hdlbox.add(menu)
        hdlbox.show()
        return hdlbox


    def __getTbButton(self, name, pic, callback, tooltip):
	# method changed 5.6.2003, Erica Andrews - if a toolbar image fails to load
	# don't just dump an 'exception' the console and quit.  Instead, return a regular 
	# Button - it's not pretty, but the program is still usable. An absent pixmap is not 
	# a reason for a program not to start!
	try:
		# changed here - use the 'loadScaledImage' method for loading toolbar images
		# so we can load something besides just .xpm, comes from 'icewmcp_common' -
		# also, with the 'loadScaledImage' we can force all images to the same size, 
		# regardless of the the real image size is
		# 5.6.2003 - Erica Andrews

		tb_obj=self.tb.append_item(name, tooltip, None,
            loadScaledImage(getPixDir()+pic,24,24),
            callback)
	except:		
		b=Button(name)
		b.show_all()
		return self.tb.append_widget(b,tooltip,name)
	tb_obj.set_border_width(3)
	return tb_obj


    def __initToolbar(self):
        self.tb = Toolbar()
        self.tb.set_orientation(gtk.ORIENTATION_HORIZONTAL)
        self.tb.set_style(gtk.TOOLBAR_ICONS)

        self.tb_save = self.__getTbButton(
            _("Save"), "iceme_save.png", self.on_save,
            _("Save the menu"))
        self.tb_revert = self.__getTbButton(
            _("Revert"), "iceme_revert.png", self.on_revert,
            _("Revert to last saved version"))
        self.tb_preview = self.__getTbButton(
            _("Preview"), "iceme_preview.png", self.on_preview,
            _("Preview the menu as IceWM would display it"))

        self.tb.append_space()

        self.tb_new_entry = self.__getTbButton(
            _("New Entry"), "iceme_new_entry.png", self.on_new_entry,
            _("Create a new normal entry"))
        self.tb_new_sep = self.__getTbButton(
           _( "New Sep."), "iceme_new_sep.png", self.on_new_sep,
            _("Create a new separator"))
        self.tb_new_menu = self.__getTbButton(
            _("New Menu"), "iceme_new_menu.png", self.on_new_submenu,
            _("Create a new submenu"))

	#     added 12.20.2003, Erica Andrews, support for Gnome1/Gnome2 menus
	#     Fixes Bug Report/Feature Request #488846
	#     Received from: klaumikli ['at'] gmx.de, at: Fri Oct 17 14:46:01 2003

        self.tb_new_gnome1 = self.__getTbButton(
            "Gnome 1", "iceme_gnome1.png", self.on_new_gnome1,
            _("Create a new Gnome 1 menu"))
        self.tb_new_gnome2 = self.__getTbButton(
            "Gnome 2", "iceme_gnome2.png", self.on_new_gnome2,
            _("Create a new Gnome 2 menu"))

        self.tb.append_space()

        self.tb_cut = self.__getTbButton(
            _("Cut"), "iceme_edit_cut.png", self.on_cut,
           _( "Cut selected entry to buffer"))
        self.tb_copy = self.__getTbButton(
           _( "Copy"), "iceme_edit_copy.png", self.on_copy,
            _("Copy selected entry to buffer"))
        self.tb_paste = self.__getTbButton(
            _("Paste"), "iceme_edit_paste.png", self.on_paste,
            _("Paste buffer under current selection"))
        self.tb_delete = self.__getTbButton(
           _( "Delete"), "iceme_edit_delete.png", self.on_delete,
           _( "Delete selected entry"))

        self.tb.append_space()

        self.tb_up = self.__getTbButton(
            _("Up"), "iceme_up.png", self.on_move_up,
            _("Move selected entry up"))
        self.tb_down = self.__getTbButton(
            _("Down"), "iceme_down.png", self.on_move_down,
            _("Move selected entry down"))

        self.tb.append_space()

        self.tb_icepref = self.__getTbButton(
            _("Preferences"), "iceme_icepref.png", self.icewmcpIcePref2,
            _("Edit the IceWM preferences with IcePref2"))
        self.tb_shortcuts = self.__getTbButton(
            _("Shortcuts"), "iceme_keys.png", self.icewmcpKeyEdit,
            _("Edit shortcuts")) ## changed to launch IceWMCP-KeyEdit, default editor crashed on startup, 2/21/2003

	#self.tb.set_button_relief(2) # added 5.7.2003, Erica Andrews
        self.tb.show()
        hdlbox = HandleBox()
        hdlbox.add(self.tb)
	hdlbox.set_border_width(2)  # added 5.6.2003, Erica Andrews
        hdlbox.show()
        return hdlbox

    def icewmcpKeyEdit(self,*args):
	if self.i_am_root==1:  
		kc=self.IceWMCPKeyboard.keypanel(1,getIceWMConfigPath()+'keys')
	else:  
		kc=self.IceWMCPKeyboard.keypanel(1,getIceWMPrivConfigPath()+"keys")	
	kc.createWindow()
	if not self.command.get_text().strip()=='':
		kc.progentry.set_text(self.command.get_text())
	if self.i_am_root==1:
		kc.wallwin.set_title(_("IceWM CP - Key Editor")+"   [ROOT]")

    def icewmcpIcePref2(self,*args):
	#   changed 12.20.2003 - use common Bash shell probing
	#   to fix BUG NUMBER: 1523884
	#   Reported By: david ['-at-'] jetnet.co.uk
	#   Reported At: Fri Oct 31 23:47:12 2003
	fork_process("icepref")
	#  old way: os.popen("icepref &")


    def __initIcePref(self):
        # check if IcePref is there. If not, disable the corresponding menu
        # and toolbar buttons.
        is_there = FALSE
        cmd = string.split(ICEPREF)[0]
        if cmd[0] == os.sep:
            # absolute filename:
            # the file must have read and execute access and be regular:
            if os.access(cmd, R_OK|X_OK) and stat.S_ISREG(os.stat(cmd)[0]):
                is_there = TRUE
        else:
            # search in $PATH:
            for path in PATH:
                fcmd = os.path.join(path, cmd)
                if os.access(fcmd, R_OK|X_OK) and stat.S_ISREG(os.stat(fcmd)[0]):
                    is_there = TRUE
                    break
        self.menu_icepref.set_sensitive(is_there)
        self.tb_icepref.set_sensitive(is_there)

    def appendIconPath(self,ipath): # changed 2.21.2003 - PhrozenSmoke
      try:
        icons=self.icons
	maxsize=18000 # dont append large files, such as wallpapers and logos, will slow down loading
	dir=ipath
        xpmfiles = glob.glob(os.path.join(dir, "*_16x16.xpm"))
        for filename in xpmfiles:
            #name = os.path.basename(filename)[:-10]  # 'short names' removed - 2.23.2003
	    name=filename
            if not icons.has_key(name):
                if os.path.getsize(name)<maxsize: icons[name] = filename
        xpmfiles = glob.glob(os.path.join(dir, "mini", "*.xpm"))
        for filename in xpmfiles:
            #name = os.path.basename(filename)[:-4]  # 'short names' removed - 2.23.2003
	    name=filename
            if not icons.has_key(name):
                if os.path.getsize(name)<maxsize: icons[name] = filename
        xpmfiles = glob.glob(os.path.join(dir, "*.xpm"))
        for filename in xpmfiles:
            #name = os.path.basename(filename)[:-4]  # 'short names' removed - 2.23.2003
	    name=filename
            if not icons.has_key(name):
                if os.path.getsize(name)<maxsize: icons[name] = filename
        xpmfiles = glob.glob(os.path.join(dir, "*.png"))
        for filename in xpmfiles:
            #name = os.path.basename(filename)[:-4]  # 'short names' removed - 2.23.2003
	    name=filename
            if not icons.has_key(name):
                if os.path.getsize(name)<maxsize: icons[name] = filename
      except:
	pass

    def __initIconCache(self, icon_paths):  # changed 2.21.2003 - PhrozenSmoke
      self.icons = {}
      self.IPATHS=[]
      for ii in icon_paths:
	self.IPATHS.append(ii)
      for dir in icon_paths:
	self.appendIconPath(dir)

      # load some default icons:
      # changed here, 5.6.2003 - Erica Andrews, use icewmcp_common's 'getPixDir' instead
      # also, different icons
      self.icons["-"] = \
            getPixDir()+"iceme_empty.png"
      self.icons["$ice_me_clipboard$"] = \
            getPixDir()+"iceme_clipboard.png"
      self.icons["$ice_me_icewm$"] = \
            getPixDir()+"iceme_icewm.png"


    ########################################################################
    # misc. functions
    ########################################################################

    def __updateWidgets(self):
        if not self.do_update_widgets:
            # don't update widgets right now (because a tree drag is running)
            return

        is_menu     = not self.cur_node.is_leaf
        is_sep      = self.tree.isSeparator(self.cur_node)
        is_normal   = not (is_menu or is_sep)
        is_inactive = self.tree.isInactive(self.cur_node)

	# added 12.20.2003, Erica Andrews, disable the 
	# 'start as new wm' and 'add shortcut' buttons if the menu item is 
	# an embedded Gnome-1 or Gnome-2 menu
	is_not_embedded=is_normal
	try:
		tcomm=self.tree.getNodeCommand(self.cur_node)
		if tcomm.startswith("icewm-menu-gnome") and (tcomm.find("--list")>-1):
			# It's an embedded Gnome menu, can't add shortcut keys or mark as new WM
			is_not_embedded=0
	except:
		pass

        # enable/disable items on right frame:
        self.command_label.set_sensitive(is_normal)
        self.command.set_sensitive(is_normal)
	self.command.set_data("sensitive",is_normal)
        self.command_button.set_sensitive(is_normal)
        self.is_restart.set_sensitive(is_not_embedded)  # changed 12.20.2003, Erica Andrews
        self.add_shortcut.set_sensitive(is_not_embedded)  # changed 12.20.2003, Erica Andrews
        self.rightframe.set_sensitive(not (is_inactive or is_sep))
	self.icon_name.set_data("sensitive", not (is_inactive or is_sep))

        # enable/disable toolbar/menu buttons:
        can_up = self.tree.canMoveUp(self.cur_node)
        can_down = self.tree.canMoveDown(self.cur_node)
        self.tb_up.set_sensitive(not is_inactive and can_up)
        self.tb_down.set_sensitive(not is_inactive and can_down)

        # buttons/menus, that delete something:
        can_delete = self.tree.canDelete(self.cur_node)
        self.tb_cut.set_sensitive(can_delete)
        self.menu_cut.set_sensitive(can_delete)
        self.tb_copy.set_sensitive(can_delete)
        self.menu_copy.set_sensitive(can_delete)
        self.tb_delete.set_sensitive(can_delete)
        self.menu_delete.set_sensitive(can_delete)

        # buttons that insert something:
        has_paste = self.tree.hasPasteData()
        can_insert = self.tree.canInsertOn(self.cur_node)
        self.tb_paste.set_sensitive(can_insert and has_paste)
        self.menu_paste.set_sensitive(can_insert and has_paste)
        self.tb_new_entry.set_sensitive(can_insert)
        self.tb_new_sep.set_sensitive(can_insert)
        self.tb_new_menu.set_sensitive(can_insert)
	#     added 12.20.2003, Erica Andrews, support for Gnome1/Gnome2 menus
	#     Fixes Bug Report/Feature Request #488846
	#     Received from: klaumikli ['at'] gmx.de, at: Fri Oct 17 14:46:01 2003
	self.tb_new_gnome1.set_sensitive(can_insert)
	self.tb_new_gnome2.set_sensitive(can_insert)

        # set data items on right frame:
        if not is_sep:
            self.name_entry.set_text(self.tree.getNodeName(self.cur_node))
        else:
            self.name_entry.set_text("")
        self.command.set_text(self.tree.getNodeCommand(self.cur_node))
        self.setIconButton(self.tree.getNodeIconName(self.cur_node))
        is_restart = self.tree.getNodeType(self.cur_node) == MENUTREE_RESTART
        self.is_restart.set_active(is_restart)

    def grabPixbuf(self, filename):
                  img = GDK.pixbuf_new_from_file(filename)
                  pix,mask = img.render_pixmap_and_mask()
		  return (pix,mask,)

    def getCachedIcon(self, shortname):
        "Returns (<filename>, <pixmap>, <mask>)"
	#print "icons:   "+str(self.icons["-"])
        if not shortname:
            shortname = "-"
	val=None
	if shortname.find(os.sep)==-1: 
		# try to give a real path for this icon
		icon_paths=self.IPATHS
		for ii in icon_paths:
			if os.path.exists(os.path.join(ii,shortname)):
				shortname=os.path.join(ii,shortname)
			if os.path.exists(os.path.join(ii,shortname+".xpm")):
				shortname=os.path.join(ii,shortname+".xpm")
			if os.path.exists(os.path.join(ii,shortname+"_16x16.xpm")):
				shortname=os.path.join(ii,shortname+"_16x16.xpm")

	if os.path.exists(shortname): # find paths dynamically from icons used, but not on IconPath, 2/21/2003
		if not self.icons.has_key(shortname):  
			self.icons[shortname]=shortname
		if not (shortname[0:shortname.rfind(os.sep)+1]) in self.IPATHS: 
			self.IPATHS.append(shortname[0:shortname.rfind(os.sep)+1])
			self.appendIconPath(shortname[0:shortname.rfind(os.sep)+1])
		val=shortname

        if not val: val = self.icons.get(shortname)

        if val is None:
            # fall back to empty icon:
             val = self.icons["-"]  # This wasn't good - try to load the icon even if its not on IconPath
        if type(val) == type(""):
            try:
                  newheight,newwidth = 22,22   # 2.21.2003 - added larger icons (PhrozenSmoke [at] yahoo.com)
		  if val.endswith("iceme_icewm.png"): newheight,newwidth = 55,22
                  img = GDK.pixbuf_new_from_file(val)
                  img2 = img.scale_simple(newheight,newwidth,GDK.INTERP_BILINEAR)
                  pix,mask = img2.render_pixmap_and_mask()
            except :
		pix,mask=self.grabPixbuf( self.icons["-"])
	    if mask==None:
		try:
			if (shortname.startswith(os.sep)) and (shortname.endswith(".xpm")):
				pdp,mask=self.grabPixbuf( shortname)
		except:
			pix,mask=self.grabPixbuf( self.icons["-"])
            newval = (val, pix, mask)
            self.icons[shortname] = newval
            return newval
        else:
            return val


    def setIconButton(self, shortname):
        self.icon_name.set_text("")
        self.icon_button.remove(self.icon)
        if shortname is None or shortname == "-":
            shortname = ""
        filename, new_pix, new_mask = self.getCachedIcon(shortname)
	self.icon=Image()
        self.icon.set_from_pixmap(new_pix, new_mask)
        self.icon.show()
        self.icon_button.add(self.icon)
        self.icon_name.set_text(shortname)


    def setStatus(self, text):
        self.statusbar.push(self.statusbar_context, text)
        while events_pending():
            mainiteration()


    def writeTree(self):
        errmsgs = []
	#    changed 12.21.2003, Erica Andrews, allow handling of 
	#    arbitrary menu, programs, and toolbar files
	if self.override_menu_file==None:
        	menu = self.preferences.getIceWMFile("menu", FALSE)
	else: 
		menu=self.override_menu_file
        fd = self.openMenufile("menu", menu, errmsgs)
        if fd: self.writeSubTree(fd, self.tree.getMainMenuNode())
        if fd: fd.close()

	if self.override_programs_file==None:
        	programs = self.preferences.getIceWMFile("programs", FALSE)
	else:
		programs = self.override_programs_file
        fd = self.openMenufile("programs", programs, errmsgs)
        if fd: self.writeSubTree(fd, self.tree.getProgramsNode())
        if fd: fd.close()

	if self.override_toolbar_file==None:
        	toolbar = self.preferences.getIceWMFile("toolbar", FALSE)
	else: 
		toolbar = self.override_toolbar_file
        fd = self.openMenufile("toolbar", toolbar, errmsgs)
        if fd: self.writeSubTree(fd, self.tree.getToolbarNode())
        if fd: fd.close()

        if errmsgs:
            self.setStatus(_("There were some errors."))
            message(_("Error saving menu"), errmsgs, pixmap=ICON_ALERT) 
	    # message() method from IceWMCP_Dialogs via icewmcp_common.py
        else:
            self.setStatus(_("Saved."))


    def openMenufile(self, short_filename, full_filename, errmsgs):
        f = None
        home_filename = os.path.join(HOME_ICEWM, short_filename)
        if self.preferences.getIgnoreHomeOption():
            # try to save to the original location (should only work as root):
            try:
                f = open(full_filename, "w")
            except IOError, msg:
                errmsgs.append(
                    "You specified '--ignore-home', but I couldn't\n"
                    "write the %s file, probably because you\n"
                    "don't have write access. The error message was:\n"
                    "%s\n"
                    "I'll try writing to your $HOME.\n" % (short_filename, msg))
        if not f:
            # try to save in the home directory:
            try:
                if not os.path.isdir(HOME_ICEWM): os.mkdir(HOME_ICEWM)
                f = open(home_filename, "w")
            except IOError, msg:
                errmsgs.append(
                    "I couldn't write the %s file to your home dir.\n"
                    "The error message was:\n"
                    "%s\n"
                    "The file will not be saved!\n" % (short_filename, msg))
                return None
        return f


    def writeSubTree(self, fd, node, level=0):
        for child in node.children:
            if self.tree.isInactive(child): return
            type = self.tree.getNodeType(child)
            indent = "    "*level
            if type == MENUTREE_SEPARATOR:
                fd.write("%sseparator\n" % indent)
            else:
                name = self.tree.getNodeName(child)
                icon = self.tree.getNodeIconName(child)
                if not icon: icon = "-"
                if type == MENUTREE_SUBMENU:
                    fd.write('%smenu "%s" "%s" {\n' % (indent, remove_utf8(name), remove_utf8(icon)))
                    self.writeSubTree(fd, child, level+1)
                    fd.write("%s}\n" % indent)
                else:
                    command = self.tree.getNodeCommand(child)
                    if type == MENUTREE_PROG:
				if command.startswith("icewm-menu-gnome") and (command.find("--list")>-1):
	    				#added 12.20.2003, Erica Andrews, support for 'embedded' menus
	    				#like those from the embedded Gnome1 and Gnome2 menus...to 
	    				#handle menu lines like: 
	    				#menuprog "Gnome Menu" gnome icewm-menu-gnome1 --list /usr/share/gnome/apps
	    				#Fixes Bug Report/Feature Request #488846
	    				#Received from: klaumikli ['at'] gmx.de, at: Fri Oct 17 14:46:01 2003

					fd.write('%smenuprog "%s" "%s" %s\n' % (indent, remove_utf8(name), remove_utf8(icon), remove_utf8(command)))

				else:  	# all others
					fd.write('%sprog "%s" "%s" %s\n'
                                 % (indent, remove_utf8(name), remove_utf8(icon), remove_utf8(command)))
                    elif type == MENUTREE_RESTART:
                        fd.write('%srestart "%s" "%s" %s\n'
                                 % (indent, remove_utf8(name), remove_utf8(icon), remove_utf8(command)))


    ########################################################################
    # callbacks
    ########################################################################

    def on_tree_drag_begin(self, widget, context):
        self.do_update_widgets = 0

    def on_tree_drag_end(self, widget, context):
        self.do_update_widgets = 1
        self.__updateWidgets()

    def on_tree_select_row(self, tree, node, col):
        self.cur_node = node
        self.__updateWidgets()


    def on_command_button_clicked(self, *args): 
		# re-written 4.3.2003, Erica Andrews, eliminated GtkExtra dep.
		# re-written 6.20.2003 Erica Andrews, use common file selector functions (icewmcp_common)
		SELECT_A_FILE(self.file_ok_close,_CP("Select a file..."),"iceme","IceMe")
		self.setStatus(_("Please select an executable."))
		
		
    def file_ok_close(self, *args):  # added 4.3.2003, Erica Andrews
		value=GET_SELECTED_FILE()   # from icewmcp_common
		self.setStatus("")
		if not value==None:
			if not value=='':
				self.last_exec=value
		if not value.endswith(os.sep): self.command.set_text(value)

    def on_icon_button_clicked(self, button):
        global icons
        icons = self.icons
        class MyIconSelectionDialog(IconSelectionDialog.IconSelectionDialog):
            def getPictureList(self):
                keys = icons.keys()
                keys.remove("-")
                keys.remove("$ice_me_clipboard$")
                keys.remove("$ice_me_icewm$")
                keys.sort()
                picturelist = map(lambda x, icons=icons: (x, icons[x]), keys)
                return picturelist

	# bug fix below, if the IconSelectionDialog was killed by 'X'-ing out the window, re-create entire window
	# to avoid 'gtk critical' messages and an empty black box instead of icons, 4/25/2003 - Erica Andrews
        if IconSelectionDialog.KILLED==1:
            self.iconsel_dlg = MyIconSelectionDialog(7)		
        if self.iconsel_dlg is None:
            self.iconsel_dlg = MyIconSelectionDialog(7)
        old_icon_name = self.tree.getNodeIconName(self.cur_node)
        # show icon select dialog and return selected shortname:
        new_icon_name, dummy = self.iconsel_dlg.getSelectedIcon(old_icon_name)
        if new_icon_name is not None:
	  try:  # added 2.24.2003 - catch exceptions, be more graceful
            # a selection was made. update node data:
            self.tree.setNodeIconName(self.cur_node, new_icon_name)
            # update icon button:
            self.setIconButton(new_icon_name)
            # update tree:
            dummy, pix, mask = self.getCachedIcon(new_icon_name)
            self.tree.setNodeIcon(self.cur_node, pix, mask)
	  except:
            pass


    def setDrag(self,*args): # drag-n-drop support, added 2.23.2003
	drago=args
	if len(drago)<7: 
		return
	else: 
		#print str(drago[4].data)  # file:/root/something.txt
		try:
			new_icon_name=""+str(drago[4].data).replace("\r\n","")
			new_icon_name=new_icon_name.replace("\x00","").replace("file:","").strip()
			if (drago[0].get_data("is_command")) :
				# handle the 'command' entry, 12.21.2003, Erica Andrews	
				if not self.command.get_data("sensitive"): return
				self.command.set_text(new_icon_name)
				return

			# handle icon dropping on icon entry or icon button
			if (not new_icon_name.endswith(".xpm")) and (not new_icon_name.endswith(".png")): 
				return
			# Fix, 12.21.2003 - don't allow dropping of icons if the 
			# icon entry or button is insensitive - Erica Andrews 
			if not self.icon_name.get_data("sensitive"): return

			dummy, pix, mask = self.getCachedIcon(new_icon_name)
			self.tree.setNodeIcon(self.cur_node, pix, mask)
			self.tree.setNodeIconName(self.cur_node, new_icon_name)
			self.setIconButton(new_icon_name)
		except: 
			pass


    def on_name_changed(self, namefield):
        if self.tree.isSeparator(self.cur_node):
            return
        new_text = namefield.get_text()
        self.tree.setNodeName(self.cur_node, new_text)

    def on_command_changed(self, commandfield):
        new_text = commandfield.get_text()
        self.tree.setNodeCommand(self.cur_node, new_text)

    def on_icon_name_focus_out_event(self, iconnamefield, event):
        "fired when the user changed the icon text and leaves the field"
        # get new icon name:
        icon_name = iconnamefield.get_text()
        # update node data:
        self.tree.setNodeIconName(self.cur_node, icon_name)
        # update icon button:
        self.setIconButton(icon_name)
        # update tree:
        dummy, pix, mask = self.getCachedIcon(icon_name)
        self.tree.setNodeIcon(self.cur_node, pix, mask)

    def on_is_restart_toggled(self, button):
        # update node data:
        if button.get_active():
            self.tree.setNodeType(self.cur_node, MENUTREE_RESTART)
        else:
            self.tree.setNodeType(self.cur_node, MENUTREE_PROG)

    def on_save(self, x=None, y=None):
        self.setStatus(_("Saving..."))
        self.writeTree()
        if self.key_editor_dlg:
            self.key_editor_dlg.save()

    def on_revert(self, x=None, y=None):
        self.setStatus(_("Reverting..."))
        self.__initTree()
        self.setStatus(_("Reverted to the last saved version."))

    def on_preview(self, x=None, y=None):
        self.setStatus(_("Displaying preview window..."))
        empty_name, empty_pix, empty_mask = self.getCachedIcon("-")
        win = PreviewWindow(self.tree, empty_pix, empty_mask)
        win.show()
        self.setStatus(_("Done."))

    def on_icepref(self, x=None, y=None):
        self.setStatus(_("Running IcePref2")+"...")
        popen2(ICEPREF)
        self.setStatus(_("Done."))

    def on_about(self, x=None, y=None):
        commonAbout("About IceMe",    "IceMe v."+VERSION+"\n\n"+_("A menu editor for IceWM written in Python and GTK.")+"\n\n"+ "Copyright (c) 2000-2002 by Dirk Moebius <dmoebius@gmx.net> and Mike Hostetler  <thehaas@binary.net>.  Copyright (c) 2003-2004  Erica Andrews <phrozensmoke [at] yahoo.com>.\n\n"+"See the file COPYING for license information (GPL).\n"+"Please visit the IceMe homepage at:\n"+"http://iceme.sourceforge.net\n\n"+_("This is a Special Edition of IceMe for IceWM Control Panel,\nwith modifications and optimizations by")+" Erica  Andrews.\n<PhrozenSmoke [at] yahoo.com>\nhttp://icesoundmanager.sourceforge.net",0 )

    def on_new_entry(self, button):
        node = self.tree.insertNode(self.cur_node, MENUTREE_PROG,
                                    _("New entry"), "file", "")
        self.tree.select(node)

    def on_new_submenu(self, button):
        node = self.tree.insertNode(self.cur_node, MENUTREE_SUBMENU,
                                    _("New submenu"), "folder", "")
        self.tree.select(node)

    def on_new_sep(self, button):
        node = self.tree.insertNode(self.cur_node, MENUTREE_SEPARATOR,
                                    None, None, None)
        self.tree.select(node)


    def on_new_gnome1(self, button):
	# added 12.20.2003, Erica Andrews, support for creating embedded Gnome1 menus
	# Fixes Bug Report/Feature Request #488846
	# Received from: klaumikli ['at'] gmx.de, at: Fri Oct 17 14:46:01 2003
        node = self.tree.insertNode(self.cur_node, MENUTREE_PROG,
            "Gnome 1", "gnome", "icewm-menu-gnome1 --list /usr/share/gnome/apps")
        self.tree.select(node)

    def on_new_gnome2(self, button):
	# added 12.20.2003, Erica Andrews, support for creating embedded Gnome2 menus
	# Fixes Bug Report/Feature Request #488846
	# Received from: klaumikli ['at'] gmx.de, at: Fri Oct 17 14:46:01 2003
        node = self.tree.insertNode(self.cur_node, MENUTREE_PROG,
            "Gnome 2", "gnome", "icewm-menu-gnome2 --list /opt/gnome2/share/applications/")
        self.tree.select(node)

    def on_move_up(self, button):
        node = self.cur_node
        self.tree.move(node, node.parent, self.tree.getNodeUpperSibling(node))
        self.__updateWidgets()

    def on_move_down(self, button):
        node = self.cur_node
        self.tree.move(node, node.parent, node.sibling.sibling)
        self.__updateWidgets()

    def on_cut(self, x=None, y=None):
        # remember current node/position:
        if self.cur_node.sibling:
            next_node = self.cur_node.sibling
        else:
            next_node = self.cur_node.parent
        # cut node to clipboard:
        self.tree.cut(self.cur_node)
        # select next node/position:
        self.cur_node = next_node
        self.tree.select(next_node)

    def on_copy(self, x=None, y=None):
        self.tree.copy(self.cur_node)

    def on_paste(self, x=None, y=None):
        self.tree.paste(self.cur_node)

    def on_delete(self, x=None, y=None):
        # remember current node/position:
        if self.cur_node.sibling:
            next_node = self.cur_node.sibling
        else:
            next_node = self.cur_node.parent
        # delete node
        self.tree.delete(self.cur_node)
        # select next node/position:
        self.cur_node = next_node
        self.tree.select(next_node)


############################################################################
# main
############################################################################

def usage():
    print "Usage: %s [OPTION]... [DIR]" % sys.argv[0]
    print """A menueditor for IceWM.
If DIR is specified, the global icewm menu, programs and toolbar files as
well as the icons subdir are taken from this directory. Otherwise the program
will look in some well known standard locations, usually /usr/local/lib/icewm
and /usr/X11R6/lib/X11/icewm. See the FAQ for information about file and icon
lookup.

  -r, --run-as-root    Don't look in $HOME/.icewm for configuration files and
                       icons. Use this, if you are root and want to edit the
                       global menu files in the IceWM system directory.
  -h, --help           Print this help and exit.

Report bugs to <dmoebius@gmx.net>.
"""


def main():
    prefs = Preferences([])

    if prefs.getErrorCode() or prefs.getHelpOption():
        usage()
        sys.exit(prefs.getErrorCode())
    
    i = IceMe(prefs)
    hideSplash()
    mainloop()


if __name__ == "__main__":
    main()
