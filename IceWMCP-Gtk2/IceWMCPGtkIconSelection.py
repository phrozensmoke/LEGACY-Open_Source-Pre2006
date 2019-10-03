#! /usr/bin/env python

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
# With Modifications by Erica Andrews (PhrozenSmoke@yahoo.com), March 14, 2003 - version 2.0
# This is a modified version of IceMe 1.0.0 ("IceWMCP Edition"), optimized for IceControlPanel.
# This is a modified version of the Icon Selection dialog from IceMe, all changes have 'comments' next to them in the code
#
# Ported to PyGTK-2 by: David Moore (djm6202@yahoo.co.nz) - June 2003
#
##################

import sys
import glob
import os
import os.path
import string
import gtk
from gtk.keysyms import Escape, Return
from icewmcp_common import *

_=translateCP  # set locale support from icewmcp_common

global CACHED # added new image cache, Erica Andrews
CACHED={}

global findIcons

def findIcons(paths):    # added as separate method by Erica Andrews
    icons={}
    for dir in paths:
        xpmfiles = glob.glob(os.path.join(dir, "*_16x16.xpm"))
        for filename in xpmfiles:
            filename=os.path.realpath(filename)
            name=filename
            if not icons.has_key(name):
             try:
                if os.path.getsize(filename)< 12000: icons[name] = filename  # ignore large files that arent likely to be icons
             except:
                pass
        xpmfiles = glob.glob(os.path.join(dir, "mini", "*.xpm"))
        for filename in xpmfiles:
            filename=os.path.realpath(filename)
            name=filename
            if not icons.has_key(name):
             try:
                if os.path.getsize(filename)< 12000: icons[name] = filename
             except:
                pass
        xpmfiles = glob.glob(os.path.join(dir, "*.xpm"))
        for filename in xpmfiles:
            filename=os.path.realpath(filename)
            name=filename
            if not icons.has_key(name):
             try:
                if os.path.getsize(filename)< 12000: icons[name] = filename
             except:
                pass
        xpmfiles = glob.glob(os.path.join(dir, "*.png"))
        for filename in xpmfiles:
            filename=os.path.realpath(filename)
            name=filename
            if not icons.has_key(name):
             try:
                if os.path.getsize(filename)< 11000: icons[name] = filename
             except:
                pass
    return icons

class IconSelectionDialog(gtk.Window):

    def __init__(self, num_columns=8,my_paths=[],update_meth=None):
        gtk.Window.__init__(self, gtk.WINDOW_TOPLEVEL)
        self.MYPATHS=my_paths
        self.UPDATER=update_meth
        self.set_wmclass("icon-selector","Icon-Selector")
        self.set_position(gtk.WIN_POS_CENTER)
        self.num_columns = num_columns
        self.icons_loaded = 0
        self.buttons = []
        self.selected = (None, None)
        self.initGUI()

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
        self.set_title(_("IceWMCP Icon Browser"))
        #self.set_modal(gtk.TRUE)  # commented out by Erica Andrrews
        self.set_size_request(400, 390)
        self.connect("delete_event", self.on_delete)
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

        self.file_entry = gtk.Entry()
        self.file_entry.show()

        self.label_columns = gtk.Label(_("Columns:"))
        self.label_columns.show()

        adj = gtk.Adjustment(self.num_columns, 1, 30, 1, 10, 10)
        self.columns = gtk.SpinButton(adj, 1, 0)
        self.columns.set_numeric(gtk.TRUE)
        self.columns.set_editable(gtk.FALSE)
        self.columns.set_update_policy(gtk.UPDATE_IF_VALID)
        adj.connect("value_changed", self.on_columns_changed)
        self.columns.show()

        self.ok = gtk.Button(_("SELECT"))
        self.ok.set_flags(gtk.CAN_DEFAULT|gtk.CAN_FOCUS|gtk.HAS_FOCUS|gtk.HAS_DEFAULT)
        self.ok.connect("clicked", self.do_ok)
        self.ok.show()

        self.cancel = gtk.Button(_("Close"))
        self.cancel.set_flags(gtk.CAN_DEFAULT|gtk.CAN_FOCUS)
        self.cancel.connect("clicked", self.do_close)
        self.cancel.show()

        self.reload = gtk.Button(_("Reload"))
        self.reload.set_flags(gtk.CAN_DEFAULT|gtk.CAN_FOCUS)
        self.reload.connect("clicked", self.do_reload)
        self.reload.show() 

        self.ipath = gtk.Button(_("Icon Paths..."))
        self.ipath.set_flags(gtk.CAN_DEFAULT|gtk.CAN_FOCUS)
        self.ipath.connect("clicked", self.showIconsPath)
        self.ipath.show()  

        self.hbox = gtk.HBox(gtk.FALSE, 10)
        self.hbox.set_border_width(10)
        self.hbox.pack_start(self.label_columns, gtk.FALSE, gtk.FALSE, 0)
        self.hbox.pack_start(self.columns,       gtk.FALSE, gtk.FALSE, 0)
        if not self.UPDATER==None: self.hbox.pack_start(self.ok,            gtk.TRUE,  gtk.TRUE,  0)
        self.hbox.pack_start(self.cancel,        gtk.TRUE,  gtk.TRUE,  0)
        self.hbox.pack_start(self.reload,        gtk.TRUE,  gtk.TRUE,  0)
        self.hbox.pack_start(self.ipath,        gtk.TRUE,  gtk.TRUE,  0)
        self.hbox.show()

        self.progressbar = gtk.ProgressBar()
        self.progressbar.set_text("")
#DJM        self.progressbar.set_show_text(gtk.TRUE)
        self.progressbar.show()

        self.vbox = gtk.VBox(gtk.FALSE, 0)
        self.vbox.pack_start(self.scr,         gtk.TRUE,  gtk.TRUE,  0)
        self.vbox.pack_start(self.file_entry,        gtk.FALSE, gtk.FALSE, 3)
        self.vbox.pack_start(self.hbox,        gtk.FALSE, gtk.FALSE, 0)
        self.vbox.pack_start(self.progressbar, gtk.FALSE, gtk.FALSE, 0)
        self.vbox.pack_start(gtk.Label(_("You can drag-and-drop icons where needed.")), gtk.FALSE, gtk.FALSE, 4)
        
        self.vbox.show_all()

        self.add(self.vbox)

    # ADDITIONS by Erica Andrews
    def saveMyIcons(self,some_line):
        try:
                some_line=some_line.replace(";",":").replace("\n","").replace("\r","").replace("\t","").replace(" :",":").replace(": ",":").strip()
                f=open(os.environ['HOME']+os.sep+".icewmcp_icons","w")
                f.write(some_line)
                f.close()
        except:
                pass

    def closeIconSetWin(self,*args):
        try:
                if len(args[0].get_data("entry").get_text().strip())>0:
                        self.saveMyIcons(args[0].get_data("entry").get_text().strip())
                        self.MYPATHS=args[0].get_data("entry").get_text().replace(";",":").replace("\n","").replace("\r","").replace("\t","").replace(" :",":").replace(": ",":").strip().split(":")
        except:
                pass
        try:
                args[0].get_data("window").hide()
                args[0].get_data("window").destroy()
                args[0].get_data("window").unmap()
        except:
                pass
        try:
                args[0].get_data("reload")(args[0])  # call reload method
        except:
                pass

    def showIconsPath2(self,some_str,reload_meth):
                w=gtk.Window(gtk.WINDOW_TOPLEVEL)
                w.set_wmclass("icon-selector","Icon-Selector")
                w.realize()
                w.set_title(_("IceWMCP Icon Browser"))
                w.set_position(gtk.WIN_POS_CENTER)
                w.set_default_size(430,200)
                v=gtk.VBox(0,0)
                v.set_spacing(4)
                v.set_border_width(7)
                v.pack_start(gtk.Label(_("Enter a list of icon paths, separated by colons")+":"),1,1,0)
                e=gtk.Entry()
                e.set_text(some_str)
                v.pack_start(e,1,1,2)
                b=gtk.Button(DIALOG_OK)
                b.set_data("entry",e)
                b.set_data("reload",reload_meth)
                b.set_data("window",w)
                w.set_data("window",w)
                w.connect("destroy",self.closeIconSetWin)
                b.connect("clicked",self.closeIconSetWin)
                v.pack_start(b,0,0,4)
                w.add(v)
                w.show_all()


    # END ADDITIONS BY Erica Andrews


    def __initIcons(self):
        if self.icons_loaded:
            return
        i = 0
        self.buttons = []
        first_butt = None
        picturelist = self.getPictureList()
        size = len(picturelist)
#DJM        self.progressbar.configure(0, 0, size - 1)
        self.progressbar.set_fraction(0.0)
        self.progressbar.set_text(_("Loading")+" "+str(size)+" "+_("images...please wait.") )
        buttongroup = self.__addButton(None, "-", "-", None, None)
        if picturelist:
            for name, val in picturelist:
                i = i + 1
                self.progressbar.set_fraction(float(i)/size)
#DJMself.progressbar.set_value(i)
                while gtk.events_pending():
                    gtk.mainiteration()
                if len(val) == 3:
                    filename, pix, mask = val
                elif type(val) == type(""):
                    filename, pix, mask = val, None, None
                else:
                    filename, pix, mask = val[0], None, None
                try:
                        self.__addButton(buttongroup, name, filename, pix, mask)
                except:
                        pass
        self.icons_loaded = 1
        self.__fillTable()
        #self.__setSelectedIcon(None) # commented out by Erica Andrews
#DJMself.progressbar.set_value(0)
        self.progressbar.set_fraction(0.0)
        del picturelist

    def setDragAway(self,*args):
        drago=args
        if len(drago)<3: 
                args[0].drag_highlight()
                args[0].grab_focus()
        else: 
                if str(drago[2].target)=='text/uri-list': drago[2].set(drago[2].target,8,str(args[0].get_data("filename")+"\r\n"))
                else: drago[2].set(drago[2].target,8,str(args[0].get_data("filename")))


    def addDragSupport(self,swidget):
      swidget.drag_source_set(gtk.gdk.BUTTON1_MASK, [('STRING', 0, 0), ('text/plain', 0,1), ('text/uri-list', 0, 2) ], gtk.gdk.ACTION_COPY)
      swidget.connect("drag_begin",self.setDragAway)
      swidget.connect("drag_data_get",self.setDragAway)

    def __addButton(self, group, name, filename, pix, mask):
        butt = None
        tips=gtk.Tooltips()
        if 1:
#DJM        try:
            if filename == "-":
                butt = gtk.RadioButton(group, "None")
            else:
                if pix is None or mask is None:

                  newheight,newwidth = 30,30   # 2.21.2003 - added larger icons (PhrozenSmoke@yahoo.com)
                  global CACHED
                  if not CACHED.has_key(filename):
                        img = gtk.gdk.pixbuf_new_from_file(filename)
                        img2 = img.scale_simple(newheight,
                                                newwidth,
                                                gtk.gdk.INTERP_BILINEAR)

                        pix,mask = img2.render_pixmap_and_mask()
                        CACHED[filename]=[pix,mask]
                  else: # grab from cache
                        pix=CACHED[filename][0]
                        mask=CACHED[filename][1]

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
                  self.addDragSupport(butt)
                  butt.connect("clicked", self.on_iconbutton_clicked)
                  butt.connect("button-press-event", self.on_iconbutton_pressed)
                  butt.show()
                  self.buttons.append(butt)
#DJM        except :
#DJM                pass
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
        #grab_add(self)   # commented out by Erica Andrews
        self.show()
        self.__initIcons()
        #self.__setSelectedIcon(default_selected_icon_name)  # commented out by Erica Andrews
        self.progressbar.set_fraction(0.0)
        self.progressbar.set_text(_("Icons loaded.  Ready."))
        gtk.mainloop()
        #grab_remove(self)  # commented out by Erica Andrews  
        self.hide()
        return self.selected

    def getPictureList(self):
        """Subclasses must override this to return a list of image
        descriptions in the form (shortname, filename) or
        (shortname, filename, pixmap, pixmap_mask).
        """
        pass


    def showIconsPath(self,*args):  # added here by Erica Andrews
        s=string.join(self.MYPATHS,":")
        self.showIconsPath2(s,self.do_reload)
                
        
    def do_close(self, button = None):  # modified here by Erica Andrews
        self.selected = (None, None)
        self.destroy()
        self.unmap()
        gtk.mainquit()

    def do_ok(self, button = None):  # modified here by Erica Andrews
        if not self.UPDATER==None:
                if not self.selected[1]==None:
                        self.UPDATER(self.selected[1])
                        self.destroy()
                        self.unmap()
                        gtk.mainquit()
        else: 
                self.destroy()
                self.ummap()
                gtk.mainquit()

    def do_reload(self, button=None):
        self.icons_loaded = 0
        global findIcons
        icons=findIcons(self.MYPATHS)
        picturelist = icons.items()
        picturelist.sort(lambda a,b: cmp(a[0],b[0]))
        self.getPictureList=lambda : picturelist
        self.__initIcons()

    def on_columns_changed(self, x):
        x = self.columns.get_text()
        self.num_columns = string.atoi(self.columns.get_text())
        self.__fillTable()

    def on_iconbutton_clicked(self, button):
        if button.get_active():
            filename = button.get_data("filename")
#            shortname = button.get_data("name")
            shortname = filename
            self.selected = (shortname, filename)
            self.progressbar.set_text(os.path.basename(filename))
            self.file_entry.set_text(str(filename))

    def on_iconbutton_pressed(self, button, event):
        if event.type == gtk.gdk._2BUTTON_PRESS:
            self.do_ok(button)

    def on_delete(self, win, event):
        self.selected = (None, None)
        self.destroy()
        self.unmap()
        gtk.mainquit()

    def on_key(self, widget, keyevent):
        if keyevent.keyval == Escape:
            self.do_close()
        elif keyevent.keyval == Return:
            self.do_ok()




def create_dlg(MY_PATHS,update_meth=None):
    global ic
    global picturelist
    # create picturelist:
    paths =MY_PATHS 
    icons = findIcons(paths)

    picturelist = icons.items()
    picturelist.sort(lambda a,b: cmp(a[0],b[0]))
    # create IconSelectionDialog:
    class MyIconSelectionDialog(IconSelectionDialog):
        def getPictureList(self):
            return picturelist
    ic = MyIconSelectionDialog(9,MY_PATHS,update_meth)
    return ic

def show_dlg(x):
    (shortname, filename) = ic.getSelectedIcon("folder")
    #print "choose: %s (%s)" % (filename, shortname)

## ADDED EVERYTHING BELOW HERE - 3.14.2003, Erica Andrews

default_icon_paths=(getIceWMConfigPath()+"icons","/usr/share/icons","/usr/share/pixmaps","/opt/kde3/share/icons","/usr/share/icons/hicolor/48x48/apps","/usr/share/icons/hicolor/48x48/devices","/usr/share/icons/hicolor/48x48/filetypes","/usr/share/icons/hicolor/48x48/mimetypes","/usr/share/icons/special","/usr/share/icons/special/sky","/usr/share/icons/special/desk","/usr/share/icons/special/mimetypes","/usr/share/icons/faux","/usr/share/pixmaps/mc","/usr/share/IceControlPanel/applet-icons","/usr/share/IceControlPanel/pixmaps","/usr/local/share/icons") 

#default_icon_paths=("/usr/X11R6/lib/X11/icewm/icons","/usr/share/icons/faux") 

def getMyIcons():
        try:
                f=open(os.environ['HOME']+os.sep+".icewmcp_icons")
                flist=f.read().replace("\n","").replace("\r","").split(":")
                f.close()
                return flist
        except:
                return default_icon_paths


def run_icons(updater=None):  # commandline 
    icpaths=getMyIcons()
    ic=create_dlg(icpaths,updater)
    hideSplash()
    ic.getSelectedIcon()

if __name__ == "__main__":
    run_icons()





