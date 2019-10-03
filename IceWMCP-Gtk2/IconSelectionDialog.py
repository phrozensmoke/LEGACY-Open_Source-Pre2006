#!/usr/bin/env python
#############################################################################
# IceMe
# =====
#
# Copyright 2000-2002, Dirk Moebius <dmoebius@gmx.net> 
# and Mike Hostetler <thehaas@binary.net
#
# This work comes with no warranty regarding its suitability for any purpose.
# The author is not responsible for any damages caused by the use of this
# program.
#
#############################################################################
########
# With Modifications by Erica Andrews (PhrozenSmoke@yahoo.com), February-April
# 2003 This is a modified version of IceMe 1.0.0 ("IceWMCP Edition"), optimized
# for IceWM ControlPanel.
#
# Ported to PyGTK-2 by: David Moore (djm6202@yahoo.co.nz) - June 2003
#
##################

import sys
import glob
import os.path
import string
import gtk

#set translation support
from icewmcp_common import *
_=translateME   # from icewmcp_common.py


# BUG FIX:
# added 4/25/2003, Erica Andrews....to monitor whether the Icon dialog was
# closed with the 'Close' button or was 'X'-ed out of and completely destroyed.
# If KILLED=1 (user 'X'-ed out), then IceMe will completely re-create  the
# destroyed window and all child widgets. If we don't re-create we will get
# 'gtk. Critical' messages and an empty, black useless window instead of the
# icon window.

global KILLED
KILLED=0


class IconSelectionDialog(gtk.Window):

    def __init__(self, num_columns=10):
        gtk.Window.__init__(self, gtk.WINDOW_TOPLEVEL)
        self.num_columns = num_columns
        self.icons_loaded = 0
        self.buttons = []
        self.selected = (None, None)
        self.initGUI()

        # the window has been created, set 'KILLED' to 'false', 4/25/2003 -
        # Erica Andrews
        global KILLED
        KILLED=0

        # signal to monitor whether user 'X'-ed out of the window, 4/25/2003 -
        # Erica Andrews
        self.connect("delete_event",self.destroy) 


    def destroy(self,*args):
        self.do_close()
        return gtk.TRUE
        #DJM Just return TRUE????

        global KILLED
        KILLED=1

        # The window has been completely destroyed, set 'KILLED' to 'true',
        # 4/25/2003 - Erica Andrews The above line sets 'KILLED' to 'true', to
        # force the reloading of the entire Icon window.  Right now, this needs
        # to be done ONLY if the user clicks the window's 'X' button, instead
        # of using The 'Cancel' button that's inside the widget window.  There
        # appears to be no way to stop a call to 'destroy' on the window when
        # the user clicks the 'X' button, so all child widgets, including
        # Images, get destroy If we don't reload the whole window, the next
        # time the user opens the Icon selection dialog, they will see 'gtk.
        # Critical' messages on the console and an empty black window pop up.
        # Any suggestions for how to STOP users from benig able to 'X' out of
        # the Icon selection window? I know it can be done in C, but I have yet
        # to see it done in Python.

        msg_warn("IceMe", _("In the future, you should close the icon window with the 'Cancel' button to avoid having to reload the icons."))
        # show the user a warning, advising them that it is better and faster
        # to use the 'Cancel' button instead 4/25/2003 - Erica Andrews
        self.do_close()


    def initGUI(self):
        # the widget tree:
        #
        # Window self
        #   VBox vbox
        #     ScrolledWindow scr
        #       Table table
        #     HBox hbox
        #       Label label_columns
        #       SpinButton columns
        #       Button ok
        #       Button cancel
        #       Button reload
        #     Progressbar progressbar
        #
        self.set_title(_("Please select an icon."))
        self.set_modal(gtk.TRUE)
        self.set_size_request(400, 390)
        #self.connect("delete_event", self.on_delete)  # disabled, 4/25/2003 - Erica Andrews
        self.connect("key-press-event", self.on_key)

        self.table=gtk.Table(1,1, gtk.FALSE)
        self.table.set_row_spacings(5)
        self.table.set_col_spacings(5)
        self.table.show()

        self.scr = gtk.ScrolledWindow()
        self.scr.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_AUTOMATIC)
        self.scr.set_border_width(10)
        self.scr.add_with_viewport(self.table)
        self.scr.show()

        self.label_columns = gtk.Label(_("Columns:"))
        self.label_columns.show()

        adj = gtk.Adjustment(self.num_columns, 1, 30, 1, 10, 10)
        self.columns = gtk.SpinButton(adj, 1, 0)
        self.columns.set_numeric(gtk.TRUE)
        self.columns.set_editable(gtk.FALSE)
        self.columns.set_update_policy(gtk.UPDATE_IF_VALID)
        adj.connect("value_changed", self.on_columns_changed)
        self.columns.show()

        self.ok = gtk.Button(_("Ok"))
        self.ok.set_flags(gtk.CAN_DEFAULT | gtk.CAN_FOCUS | 
                          gtk.HAS_FOCUS | gtk.HAS_DEFAULT)
        self.ok.connect("clicked", self.do_ok)
        self.ok.show()

        self.cancel = gtk.Button(_("Cancel"))
        self.cancel.set_flags(gtk.CAN_DEFAULT|gtk.CAN_FOCUS)
        self.cancel.connect("clicked", self.do_close)
        self.cancel.show()

        self.reload = gtk.Button(_("Reload"))
        self.reload.set_flags(gtk.CAN_DEFAULT|gtk.CAN_FOCUS)
        self.reload.connect("clicked", self.do_reload)
        self.reload.show()

        self.hbox = gtk.HBox(gtk.FALSE, 10)
        self.hbox.set_border_width(10)
        self.hbox.pack_start(self.label_columns, gtk.FALSE, gtk.FALSE, 0)
        self.hbox.pack_start(self.columns,       gtk.FALSE, gtk.FALSE, 0)
        self.hbox.pack_start(self.ok,            gtk.TRUE,  gtk.TRUE,  0)
        self.hbox.pack_start(self.cancel,        gtk.TRUE,  gtk.TRUE,  0)
        self.hbox.pack_start(self.reload,        gtk.TRUE,  gtk.TRUE,  0)
        self.hbox.show()

        self.progressbar = gtk.ProgressBar()
        self.progressbar.set_text("")
#DJM        self.progressbar.set_show_text(gtk.TRUE)
        self.progressbar.show()

        self.vbox = gtk.VBox(gtk.FALSE, 0)
        self.vbox.pack_start(self.scr,         gtk.TRUE,  gtk.TRUE,  0)
        self.vbox.pack_start(self.hbox,        gtk.FALSE, gtk.FALSE, 0)
        self.vbox.pack_start(self.progressbar, gtk.FALSE, gtk.FALSE, 0)
        self.vbox.show()

        self.add(self.vbox)


    def __initIcons(self):
        if self.icons_loaded:
            return
        i = 0
        self.buttons = []
        self.err_msg=[] # added 4/25/2003, be quiet about image problems, but report errors later, Erica Andrews
        first_butt = None
        picturelist = self.getPictureList()
        size = len(picturelist)
        self.progressbar.set_fraction(0.0)
        self.progressbar.set_text(_("Loading")+" "+str(size)+" "+_("images...please wait.") )
        buttongroup = self.__addButton(None, "-", "-", None, None)
        if picturelist:
            for name, val in picturelist:
                i = i + 1
                self.progressbar.set_fraction(float(i)/size)
                while gtk.events_pending():
                    gtk.mainiteration()
                if len(val) == 3:
                    filename, pix, mask = val
                elif type(val) == type(""):
                    filename, pix, mask = val, None, None
                else:
                    filename, pix, mask = val[0], None, None
#DJM                try:
                self.__addButton(buttongroup, name, filename, pix, mask)
#DJM                except:
#DJM                    pass
        self.icons_loaded = 1
        self.__fillTable()
        self.__setSelectedIcon(None)
        self.progressbar.set_fraction(0.0)
        del picturelist
        if len(self.err_msg)>0:
            msg_warn("IceMe", _("There were some errors.")+"\n"+"\n".join(self.err_msg))
            # added 4/25/2003, be quiet about image loading problems, but
            # report errors later, Erica Andrews let's report errors loading
            # certain icons in a dialog, not on the console like the old IceMe

    def __addButton(self, group, name, filename, pix, mask):
        butt = None
        tips=gtk.Tooltips()
#DJM        try:
        if 1:
            if filename == "-":
                butt = gtk.RadioButton(group, "None")
            else:
                if pix is None or mask is None:
                    newheight,newwidth = 28,28   # 2.21.2003 - added larger icons (PhrozenSmoke@yahoo.com)
                    img = gtk.gdk.pixbuf_new_from_file(filename)
                    img2 = img.scale_simple(newheight,
                                            newwidth,
                                            gtk.gdk.INTERP_BILINEAR)
                    pix,mask = img2.render_pixmap_and_mask()

                    icon = gtk.Image()
                    icon.set_from_pixmap(pix, mask)
                    icon.show()
                    butt = gtk.RadioButton(group)
                    butt.add(icon)
                    tips.set_tip(butt,filename)
                    butt.set_relief(gtk.RELIEF_NONE) # new relief - flat added 2.21.2003
                    butt.set_mode(gtk.FALSE)
                    butt.set_data("filename", filename)
                    butt.set_data("name", name)
                    butt.connect("clicked", self.on_iconbutton_clicked)
                    butt.connect("button-press-event", self.on_iconbutton_pressed)
                    butt.show()
                    ##  if len(self.err_msg)<2: raise TypeError  # this was for testing only, 4/25/2003, Erica Andrews
                    self.buttons.append(butt)
#DJM        except:   # added 4/25/2003, be quiet about image loading problems, but report errors later, Erica Andrews
#DJM            print "error"
#DJM            self.err_msg.append(str(filename))
        return butt

    def __fillTable(self):
        rows = (len(self.buttons) / self.num_columns) + 1
        for button in self.table.get_children():
            self.table.remove(button)
        self.table.resize(rows, self.num_columns)
        self.table.set_row_spacings(5)
        self.table.set_col_spacings(5)
        i = r = c = 0
        for button in self.buttons:
            self.table.attach(button, c, c+1, r, r+1, yoptions=0)
            i = i + 1
            c = c + 1
            if c == self.num_columns:
                c = 0
                r = r + 1

    def __setSelectedIcon(self, name):
        if name is None:
            self.buttons[0].set_active(gtk.TRUE)
            #self.buttons[0].set_flags(gtk.HAS_FOCUS)
            self.buttons[0].grab_focus()
            self.on_iconbutton_clicked(self.buttons[0])
        else:
            for b in self.buttons:
                if b.get_data("name") == name:
                    b.set_active(gtk.TRUE)
                    #b.set_flags(gtk.HAS_FOCUS)
                    b.grab_focus()
                    self.on_iconbutton_clicked(b)
                    return
            for b in self.buttons:
                if b.get_data("filename") == name:
                    b.set_active(gtk.TRUE)
                    #b.set_flags(gtk.HAS_FOCUS)
                    b.grab_focus()
                    self.on_iconbutton_clicked(b)
                    return
            self.buttons[0].set_active(gtk.TRUE)
            #self.buttons[0].set_flags(gtk.HAS_FOCUS)
            self.buttons[0].grab_focus()
            self.on_iconbutton_clicked(self.buttons[0])

    def getSelectedIcon(self, default_selected_icon_name = None):
        self.grab_add()
        self.show()
        self.__initIcons()
        self.__setSelectedIcon(default_selected_icon_name)
        self.progressbar.set_fraction(0.0)
        self.progressbar.set_text(_("Please select an icon."))
        gtk.mainloop()
        self.grab_remove()
        self.hide()
        return self.selected

    def getPictureList(self):
        """Subclasses must override this to return a list of image
        descriptions in the form (shortname, filename) or
        (shortname, filename, pixmap, pixmap_mask).
        """
        pass

    def do_close(self, button = None):
        self.selected = (None, None)
        gtk.mainquit()

    def do_ok(self, button = None):
        gtk.mainquit()

    def do_reload(self, button):
        self.icons_loaded = 0
        self.__initIcons()

    def on_columns_changed(self, x):
        self.num_columns = self.columns.get_value_as_int()
        self.__fillTable()

    def on_iconbutton_clicked(self, button):
        if button.get_active():
            filename = button.get_data("filename")
#            shortname = button.get_data("name")
            shortname = filename
            self.selected = (shortname, filename)
            self.progressbar.set_text(os.path.basename(filename))

    def on_iconbutton_pressed(self, button, event):
        if event.type == gtk.gdk._2BUTTON_PRESS:
            self.do_ok(button)

    def on_delete(self, win, event):
        self.selected = (None, None)
        self.hide()
        gtk.mainquit()

    def on_key(self, widget, keyevent):
        if keyevent.keyval == gtk.keysyms.Escape:
            self.do_close()
        elif keyevent.keyval == gtk.keysyms.Return:
            self.do_ok()


def create_dlg():
    global ic
    global picturelist
    # create picturelist:
    paths = (getIceWMConfigPath()+"icons",
             "/usr/share/icons",
             "/usr/share/pixmaps",
             "/opt/kde3/share/icons") 
    icons = {}
    for dir in paths:
        xpmfiles = glob.glob(os.path.join(dir, "*_16x16.xpm"))
        for filename in xpmfiles:
            #name = os.path.basename(filename)[:-10]
            name=filename
            if not icons.has_key(name):
                icons[name] = filename
        xpmfiles = glob.glob(os.path.join(dir, "mini", "*.xpm"))
        for filename in xpmfiles:
            #name = os.path.basename(filename)[:-4]
            name=filename
            if not icons.has_key(name):
                icons[name] = filename
        xpmfiles = glob.glob(os.path.join(dir, "*.xpm"))
        for filename in xpmfiles:
            #name = os.path.basename(filename)[:-4]
            name=filename
            if not icons.has_key(name):
                icons[name] = filename
        xpmfiles = glob.glob(os.path.join(dir, "*.png"))
        for filename in xpmfiles:
            #name = os.path.basename(filename)[:-4]
            name=filename
            if not icons.has_key(name):
                icons[name] = filename
    picturelist = icons.items()
    picturelist.sort(lambda a,b: cmp(a[0],b[0]))
    # create IconSelectionDialog:
    class MyIconSelectionDialog(IconSelectionDialog):
        def getPictureList(self):
            return picturelist
    ic = MyIconSelectionDialog()

def show_dlg(x):
    (shortname, filename) = ic.getSelectedIcon("folder")
    #print "choose: %s (%s)" % (filename, shortname)

def test():
    create_dlg()
    win = gtk.Window()
    win.set_default_size(80,20)
    win.connect("delete_event", gtk.mainquit)
    win.show()
    b = gtk.Button("Test")
    b.connect("clicked", show_dlg)
    b.show()
    win.add(b)
    gtk.mainloop()

if __name__ == "__main__":
    test()





