#! /usr/bin/env python

######################################
# ColorButton
# 
# Copyright 2003 by Erica Andrews 
# (PhrozenSmoke@yahoo.com)
# http://icesoundmanager.sourceforge.net
# 
# A reusable widget for selecting a color
# 
# This program is distributed under the GNU General
# Public License (open source).
# 
# Ported to PyGTK-2 by: David Moore (djm6202@yahoo.co.nz) - June 2003
#
#######################################

import os
import sys
import math
import gtk
from string import atoi

#set translation support
from icewmcp_common import *
_=translateCP   # from icewmcp_common.py


# import drag-n-drop support, 5.16.2003
#import icewmcp_dnd
#from icewmcp_dnd import *
#initColorDrag()      # enable dnd support for 'color buttons'
#addDragSupportColor=icewmcp_dnd.addDragSupportColor

# Clicking on the color button will bring up a color selection
# dialog.  The color selected there will be mirrored on the
# button.

class ColorButton(gtk.Button):
    def __init__(self, rgb = (0x0, 0x33, 0xff)):
        gtk.Button.__init__(self)
        self.set_relief(gtk.RELIEF_NONE)

        # attributes
        self.colorSelectionDlg = None

        self.drawingArea = gtk.DrawingArea()
        self.drawingArea.set_size_request(22, 18)

        f = gtk.Frame()
        f.set_shadow_type(gtk.SHADOW_ETCHED_OUT)
        f.add(self.drawingArea)
        self.add(f)

        self.setRgb(rgb)

        # events
        self.connect("expose_event", self.draw)
        self.connect("clicked", self.selectColor)

    def getRgb(self):
        return ( self.color.red   >> 8,
                 self.color.green >> 8,
                 self.color.blue  >> 8 )

    def setValue(self, value):
        # make allowances for HTML-style color specs
        value=value.lower().replace("\"","").   \
                            replace("'","").    \
                            replace("/","").    \
                            replace("rgb:",""). \
                            replace(" ","").    \
                            replace("#","").strip()
        if not len(value)==6: 
            return
        hvalue = ""+value
        value  = "rgb:"+ hvalue[0:2]+"/"+hvalue[2:4]+ "/"  +hvalue[4:6]
        rgb = getRGBForHex(hvalue)              
        print rgb
        self.setRgb(rgb)

    def setRgb(self, rgb):
        print rgb
        self.color = self.drawingArea.get_colormap().alloc_color(rgb[0] << 8, 
                                                                 rgb[1] << 8, 
                                                                 rgb[2] << 8)

    def draw(self, *args):
        self.drawingArea.modify_bg(gtk.STATE_NORMAL, self.color)
        self.drawingArea.modify_bg(gtk.STATE_PRELIGHT, self.color)
        self.drawingArea.modify_bg(gtk.STATE_ACTIVE, self.color)

    def selectColor(self, widget):
        # Create color selection dialog
        if self.colorSelectionDlg == None:
            self.colorSelectionDlg = gtk.ColorSelectionDialog(
                              "Select background color")
        colorSelection = self.colorSelectionDlg.colorsel
        colorSelection.set_previous_color(self.color)
        colorSelection.set_current_color(self.color)
        colorSelection.set_has_palette(gtk.TRUE)

        response = self.colorSelectionDlg.run()
        if response == gtk.RESPONSE_OK:
            self.color = colorSelection.get_current_color()
            self.draw()
#            self.emit("color_changed", self)
        self.colorSelectionDlg.hide()

def changed(x):
    print x.getRgb()

def test() :
    window = gtk.Window(gtk.WINDOW_TOPLEVEL)
    window.set_title("Color selection test")
    window.set_resizable(gtk.TRUE)
    window.set_border_width(20)
    window.connect("delete_event", gtk.mainquit)

    button = ColorButton()
    button.connect("clicked", changed)
    window.add(button)
    window.show_all()

    gtk.mainloop()

if __name__== "__main__" :
    test()





