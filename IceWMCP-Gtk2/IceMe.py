#!/usr/bin/env python

##################################################
# IceMe
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
########
# With Modifications by Erica Andrews (PhrozenSmoke@yahoo.com), February-April 2003
# This is a modified version of IceMe 1.0.0 ("IceWMCP Edition"), optimized for IceWM ControlPanel.
# Copyright (c) 2003 Erica Andrews
#
# Ported to PyGTK-2 by: David Moore (djm6202@yahoo.co.nz) - July 2003
##################

import glob,sys
import os
import stat
from popen2 import popen2
import gtk

from constants import *
from Preferences import Preferences
from IceMenuTree import IceMenuTree
from MenuParser import MenuParser
#from IconSelectionDialog import IconSelectionDialog     # disabled, 4/25/2003 - Erica Andrews
import IconSelectionDialog     # import module, instead of class, 4/25/2003 - Erica Andrews
from PreviewWindow import PreviewWindow
from icewmcp_dnd import *  # 5.16.2003 - drag-n-drop support


#set translation support
from icewmcp_common import *
_=translateME   # from icewmcp_common.py


VERSION = this_software_version+"/IceWMCP Edition"


class IceMe(gtk.Window):

    ########################################################################
    # init GUI
    ########################################################################

    def __init__(self, preferences):
        self.last_exec=''   # added 4.3.2003, Erica Andrews, file selection 'memory'

        gtk.Window.__init__(self, gtk.WINDOW_TOPLEVEL)
        self.set_title("IceMe - IceWMCP Edition")
        self.set_wmclass("iceme","IceMe")  # added 4.4.2003, Erica Andrews
        self.set_default_size(gtk.gdk.screen_width()-140, 
                              gtk.gdk.screen_height()-175)
        self.connect("delete_event", gtk.mainquit)

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

        scr = gtk.ScrolledWindow()
        scr.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_AUTOMATIC)
        scr.add(self.tree)
        scr.show()

        name_label = gtk.Label(_("Name:"))
        name_label.set_alignment(1.0, 0.5)
        name_label.show()

        self.name_ = gtk.Entry()
        self.name_.connect("changed", self.on_name_changed)
        self.name_.show()

        self.command_label = gtk.Label(_("Command:"))
        self.command_label.set_alignment(1.0, 0.5)
        self.command_label.show()

        self.command = gtk.Entry()
        self.command.connect("changed", self.on_command_changed)
        self.command.show()

        cmd_icon =getImage(getPixDir()+"iceme_open.png",translateCP("Browse..."))
        cmd_icon.show()
        self.command_button = gtk.Button()
        self.command_button.add(cmd_icon)
        self.command_button.connect("clicked", self.on_command_button_clicked)
        self.command_button.show()

        command_hbox = gtk.HBox(gtk.FALSE, 5)
        command_hbox.pack_start(self.command)
        command_hbox.pack_start(self.command_button, expand=gtk.FALSE, fill=gtk.FALSE)
        command_hbox.show()

        icon_label = gtk.Label(_("Icon:"))
        icon_label.set_alignment(1.0, 0.5)
        icon_label.show()

        self.icon_name = gtk.Entry()
        addDragSupport(self.icon_name,self.setDrag)  # 2.24.2003 - drag-n-drop support, drop icons on text entry
        self.icon_name.connect("focus-out-event", self.on_icon_name_focus_out_event)
        self.icon_name.show()

#DJM        dummy, pix, mask = self.getCachedIcon("-")
#DJM        self.icon = gtk.Pixmap(pix, mask)
        self.icon = gtk.Image()
        self.icon.set_from_file(self.icons["-"])
    
        self.icon.show()

        self.icon_button = gtk.Button(None)
        self.icon_button.add(self.icon)
        addDragSupport(self.icon_button,self.setDrag)   # 2.24.2003 - drag-n-drop support, drop icons on button
        self.icon_button.connect("clicked", self.on_icon_button_clicked)
        self.icon_button.show()

        icon_hbox = gtk.HBox(gtk.FALSE, 5)
        icon_hbox.pack_start(self.icon_name)
        icon_hbox.pack_start(self.icon_button, expand=gtk.FALSE, fill=gtk.FALSE)
        icon_hbox.show()

        sep = gtk.HSeparator()
        sep.show()

        self.is_restart = gtk.CheckButton(_("Start as a new window manager"))
        self.is_restart.connect("toggled", self.on_is_restart_toggled)
        self.is_restart.show()

        self.add_shortcut = gtk.Button(_("Add shortcut..."))
        self.add_shortcut.connect("clicked", self.icewmcpKeyEdit) # changed 2.21.2003 - run IceWMCP-KeyEdit
        self.add_shortcut.show()

        table = gtk.Table(2, 6, gtk.FALSE)
        table.attach(name_label,         0, 1, 0, 1, yoptions=0)
        table.attach(self.name_,          1, 2, 0, 1, yoptions=0)
        table.attach(self.command_label, 0, 1, 1, 2, yoptions=0)
        table.attach(command_hbox,       1, 2, 1, 2, yoptions=0)
        table.attach(icon_label,         0, 1, 2, 3, yoptions=0)
        table.attach(icon_hbox,          1, 2, 2, 3, yoptions=0)
        table.attach(sep,                0, 2, 3, 4, yoptions=0)
        table.attach(self.is_restart,    1, 2, 4, 5, yoptions=0)
        table.attach(self.add_shortcut,  1, 2, 5, 6, yoptions=0)
        table.set_border_width(10)
        table.set_row_spacings(5)
        table.set_col_spacings(5)
        table.show()

        self.rightframe = gtk.Frame(_("Menu entry"))
        self.rightframe.set_border_width(10)
        self.rightframe.add(table)
        self.rightframe.show()

        hpaned = gtk.HPaned()
        hpaned.pack1(scr, gtk.TRUE, gtk.TRUE)
        hpaned.pack2(self.rightframe, gtk.FALSE, gtk.FALSE)
        hpaned.set_position(300)
        hpaned.show()

        self.statusbar = gtk.Statusbar()
        self.statusbar_context = self.statusbar.get_context_id("main")
        self.setStatus("IceMe v" + VERSION)
        self.statusbar.show()

        vbox = gtk.VBox()
        vbox.pack_start(menu, expand=gtk.FALSE)
        vbox.pack_start(tb, expand=gtk.FALSE)
        vbox.pack_start(hpaned)
        vbox.pack_start(self.statusbar, expand=gtk.FALSE)
        vbox.show()

        self.add(vbox)

        # added 4/27/2003 - Erica Andrews, new prettier start-up process, more informative with Splash screen
        # code comes from 'icewmcp_common.py'
        setSplash(self.initAll, "IceMe:  "+_("Loading menus and icons...please wait."))
        showSplash()


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
        self.show()
        import IceWMCPKeyboard
        self.IceWMCPKeyboard=IceWMCPKeyboard
        gtk.timeout_add(15,hideSplash)  # close the Splash window, from icewmcp_common.py
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
        menu_file = self.preferences.getIceWMFile("menu")
        programs_file = self.preferences.getIceWMFile("programs")
        toolbar_file = self.preferences.getIceWMFile("toolbar")
        rootnode = self.tree.init(menu_file, programs_file, toolbar_file)
        self.do_update_widgets = 1
        self.tree.select(rootnode)


    def __initMenu(self):
        ag = gtk.AccelGroup()
        self.itemf = gtk.ItemFactory(gtk.MenuBar, "<main>", ag)
        self.itemf.create_items( (
            # path              key           callback    action#  type
            (_("/_File"),          None,         None,             0, "<Branch>"),
            (_("/_File/_Save"),    "<control>S", self.on_save,     1, ""),
            (_("/_File/_Revert"),  "<control>T", self.on_revert,   2, ""),
            (_("/_File/_Preview"), "<control>P", self.on_preview,  3, ""),
            (_('/_File/sep1'),     None,         None,             2, '<Separator>'),
            (_("/_File")+"/_"+FILE_RUN,"<control>R", rundlg,421,""), # added 4.2.2003
             (_("/_File")+"/_"+_("Check for newer versions of this program..."), "<control>U", checkSoftUpdate,420,""),
            (_("/_File/sep1"),     None,         None,             4, "<Separator>"),
            (_("/_File/_Exit"),    "<control>Q", gtk.mainquit,     5, ""),
            (_("/_Edit"),          None,         None,             6, "<Branch>"),
            (_("/_Edit/_Cut"),     "<control>X", self.on_cut,      7, ""),
            (_("/_Edit/C_opy"),    "<control>C", self.on_copy,     8, ""),
            (_("/_Edit/_Paste"),   "<control>V", self.on_paste,    9, ""),
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
        ) )
        self.add_accel_group(ag)
        menu = self.itemf.get_widget("<main>")
        menu.show()

        # remember some menu entries that may be disabled later:
        self.menu_cut     = self.itemf.get_widget_by_action(7)
        self.menu_copy    = self.itemf.get_widget_by_action(8)
        self.menu_paste   = self.itemf.get_widget_by_action(9)
        self.menu_delete  = self.itemf.get_widget_by_action(11)
        self.menu_icepref = self.itemf.get_widget_by_action(13)

        hdlbox = gtk.HandleBox()
        hdlbox.add(menu)
        hdlbox.show()
        return hdlbox


    def __getTbButton(self, name, pic, callback, tooltip):
        # method changed 5.6.2003, Erica Andrews - if a toolbar image fails to
        # load don't just dump an 'exception' the console and quit.  Instead,
        # return a regular Button - it's not pretty, but the program is still
        # usable. An absent pixmap is not a reason for a program not to start!
        try:
            # changed here - use the 'loadScaledImage' method for loading
            # toolbar images so we can load something besides just .xpm, comes
            # from 'icewmcp_common' - also, with the 'loadScaledImage' we can
            # force all images to the same size, regardless of the the real
            # image size is 5.6.2003 - Erica Andrews

            # below is the old faulty image loading calls from the old IceMe
            """return self.tb.append_item(name, tooltip, None,
            gtk.Pixmap(self, os.path.join(getBaseDir(), "pixmaps", pic)),
            callback)"""

            # 5.6.2003 - Erica Andrews
            return self.tb.append_item(name, tooltip, None,
                                       loadScaledImage(getPixDir()+pic,23,23), 
                                       callback)
        except:
            b=gtk.Button(name)
            b.show_all()
            return self.tb.append_widget(b,tooltip,name)


    def __initToolbar(self):
        self.tb = gtk.Toolbar()
        self.tb.set_orientation(gtk.ORIENTATION_HORIZONTAL)
        self.tb.set_style(gtk.TOOLBAR_ICONS)
        self.tb.set_border_width(13) # changed 5.7.2003, Erica Andrews...more space

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

        self.tb.show()
        hdlbox = gtk.HandleBox()
        hdlbox.add(self.tb)
        hdlbox.set_border_width(2)  # added 5.6.2003, Erica Andrews
        hdlbox.show()
        return hdlbox

    def icewmcpKeyEdit(self,*args):
        if not self.command.get_text().strip()=='':
                kc=self.IceWMCPKeyboard.keypanel(1,self.command.get_text())
                kc.createWindow()
                kc.progentry.set_text(self.command.get_text())
        else:
                self.IceWMCPKeyboard.keypanel(1).createWindow()

    def icewmcpIcePref2(self,*args):
        os.popen("icepref &")


    def __initIcePref(self):
        # check if IcePref is there. If not, disable the corresponding menu
        # and toolbar buttons.
        is_there = gtk.FALSE
        cmd = string.split(ICEPREF)[0]
        if cmd[0] == os.sep:
            # absolute filename:
            # the file must have read and execute access and be regular:
            if os.access(cmd, R_OK|X_OK) and stat.S_ISREG(os.stat(cmd)[0]):
                is_there = gtk.TRUE
        else:
            # search in $PATH:
            for path in PATH:
                fcmd = os.path.join(path, cmd)
                if os.access(fcmd, R_OK|X_OK) and stat.S_ISREG(os.stat(fcmd)[0]):
                    is_there = gtk.TRUE
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

        # enable/disable items on right frame:
        self.command_label.set_sensitive(is_normal)
        self.command.set_sensitive(is_normal)
        self.command_button.set_sensitive(is_normal)
        self.is_restart.set_sensitive(is_normal)
        self.add_shortcut.set_sensitive(is_normal)
        self.rightframe.set_sensitive(not (is_inactive or is_sep))

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

        # set data items on right frame:
        if not is_sep:
            self.name_.set_text(self.tree.getNodeName(self.cur_node))
        else:
            self.name_.set_text("")
        self.command.set_text(self.tree.getNodeCommand(self.cur_node))
        self.setIconButton(self.tree.getNodeIconName(self.cur_node))
        is_restart = self.tree.getNodeType(self.cur_node) == MENUTREE_RESTART
        self.is_restart.set_active(is_restart)


    def getCachedIcon(self, shortname):
        "Returns (<filename>, <pixmap>, <mask>)"
#DJM!!!
#DJM        print "icons:   "+str(self.icons["-"])
        if not shortname:
            shortname = "-"
        val   = None
        mask  = None
        pix   = None
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
            # and add it to the self.icons dict
            #self.icons[shortname]=shortname
            #val=shortname
        if type(val) == type(""):
            try:
                #if val.find("sciences")>0: print val
                #pix, mask = create_pixmap_from_xpm(self, None, val)
                newheight,newwidth = 22,22   # 2.21.2003 - added larger icons (PhrozenSmoke@yahoo.com)
                if val.endswith("iceme_icewm.png"): newheight,newwidth = 55,22
                img = gtk.gdk.pixbuf_new_from_file(val)
                img2 = img.scale_simple(newheight,newwidth,gtk.gdk.INTERP_BILINEAR)
                pix,mask = img2.render_pixmap_and_mask()

            except :
                #sys.stderr.write("*** couldn't open icon: %s" % (val))
                pix, mask = gtk.gdk.pixmap_create_from_xpm(self.window, None, self.icons["-"])
            if mask==None:
                try:
                    if (shortname.startswith(os.sep)) and (shortname.endswith(".xpm")):
                        pdp, mask = gtk.gdk.pixmap_create_from_xpm(self.window, None, shortname)
                except:
                    pix, mask = gtk.gdk.pixmap_create_from_xpm(self.window, None, self.icons["-"])
            newval = (val, pix, mask)
            self.icons[shortname] = newval
            #if shortname.find("realplay")>-1: print "New Val: "+str(newval)
            return newval
        else:
            return val


    def setIconButton(self, shortname):
        self.icon_name.set_text("")
        self.icon_button.remove(self.icon)
        if shortname is None or shortname == "-":
            shortname = ""
        filename, new_pix, new_mask = self.getCachedIcon(shortname)
        self.icon = gtk.Image()
        self.icon.set_from_pixmap(new_pix, new_mask)
        self.icon.show()
        self.icon_button.add(self.icon)
        self.icon_name.set_text(shortname)


    def setStatus(self, text):
        self.statusbar.push(self.statusbar_context, text)
        while gtk.events_pending():
            gtk.mainiteration()


    def writeTree(self):
        errmsgs = []

        menu = self.preferences.getIceWMFile("menu", gtk.FALSE)
        fd = self.openMenufile("menu", menu, errmsgs)
        if fd: self.writeSubTree(fd, self.tree.getMainMenuNode())
        if fd: fd.close()

        programs = self.preferences.getIceWMFile("programs", gtk.FALSE)
        fd = self.openMenufile("programs", programs, errmsgs)
        if fd: self.writeSubTree(fd, self.tree.getProgramsNode())
        if fd: fd.close()

        toolbar = self.preferences.getIceWMFile("toolbar", gtk.FALSE)
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
                    fd.write('%smenu "%s" "%s" {\n' % (indent, name, icon))
                    self.writeSubTree(fd, child, level+1)
                    fd.write("%s}\n" % indent)
                else:
                    command = self.tree.getNodeCommand(child)
                    if type == MENUTREE_PROG:
                        fd.write('%sprog "%s" "%s" %s\n'
                                 % (indent, name, icon, command))
                    elif type == MENUTREE_RESTART:
                        fd.write('%srestart "%s" "%s" %s\n'
                                 % (indent, name, icon, command))


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


    def on_command_button_clicked(self, *args): # re-written 4.3.2003, Erica Andrews, eliminated gtk.Extra dep.
        self.my_file_win = gtk.FileSelection(translateCP("Select a file..."))
        self.my_file_win.set_wmclass("iceme","IceMe")  # added 4.4.2003, Erica Andrews
        self.my_file_win.ok_button.connect('clicked', self.file_ok_close)
        self.my_file_win.cancel_button.connect('clicked', self.file_cancel)
        value = self.last_exec
        if value != '""':
            self.my_file_win.set_filename(value)
        self.my_file_win.set_modal(1)
        self.setStatus(_("Please select an executable."))
        self.my_file_win.show()

    def file_ok_close(self, *args):  # added 4.3.2003, Erica Andrews
        value = self.my_file_win.get_filename()
        if not value==None:
            if not value=='':
                self.last_exec=value
        self.file_cancel()
        if not value.endswith(os.sep): self.command.set_text(value)

    def file_cancel(self, *args):  # added 4.3.2003, Erica Andrews
        self.my_file_win.hide()
        self.my_file_win.destroy()
        self.my_file_win.unmap()
        self.setStatus("")

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
            args[0].drag_highlight()
            args[0].grab_focus()
        else: 
            #print str(drago[4].data)  # file:/root/something.txt
            try:
                new_icon_name=""+str(drago[4].data).replace("\r\n","").replace("\x00","").replace("file:","").strip()
                if (not new_icon_name.endswith(".xpm")) and (not new_icon_name.endswith(".png")): return                
                dummy, pix, mask = self.getCachedIcon(new_icon_name)
                self.tree.setNodeIcon(self.cur_node, pix, mask)
                self.tree.setNodeIconName(self.cur_node, new_icon_name)
                self.setIconButton(new_icon_name)
            except: 
                pass
            args[0].drag_unhighlight()


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
        commonAbout("About IceMe",    "IceMe v."+VERSION+"\n\n"+_("A menu editor for IceWM written in Python and GTK.")+"\n\n"+ "Copyright (c) 2000-2002 by Dirk Moebius <dmoebius@gmx.net> and Mike Hostetler  <thehaas@binary.net>\n\n"+"See the file COPYING for license information (GPL).\n"+"Please visit the IceMe homepage at:\n"+"http://iceme.sourceforge.net\n\n"+_("This is a Special Edition of IceMe for IceWM Control Panel,\nwith modifications and optimizations by")+" Erica  Andrews.\n<PhrozenSmoke@yahoo.com>\nhttp://icesoundmanager.sourceforge.net",0 )

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

  -i, --ignore-home    Don't look in $HOME/.icewm for configuration files and
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
    gtk.mainloop()


if __name__ == "__main__":
    main()




