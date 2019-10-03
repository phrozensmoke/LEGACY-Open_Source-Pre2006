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

import string, stat
from popen2 import popen2
from constants import *

#set translation support
from icewmcp_common import *

def _(somestr):
	return to_utf8(translateME(somestr))

class PicMenuItem(MenuItem):

    def __init__(self, text, pix, mask):
        MenuItem.__init__(self)
        hbox = HBox(FALSE, 5)
        if (pix, mask) != (None, None):
	    icon=Image()
            icon.set_from_pixmap(pix, mask)
            icon.show()
            hbox.pack_start(icon, FALSE, FALSE)
        if text:
            label = Label(text)
            label.set_justify(JUSTIFY_LEFT)
            label.show()
            hbox.pack_start(label, FALSE, FALSE)
        hbox.show()
        self.add(hbox)



class PreviewWindow(Window):

    def __init__(self, tree, empty_pix, empty_mask):
        Window.__init__(self, WINDOW_TOPLEVEL)
	self.set_wmclass("iceme","IceMe")
	self.set_title( "IceMe: "+_("Menu Preview"))
	set_basic_window_icon(self)
        self.set_default_size(400, -1)
        self.connect("delete_event", self.close)
        self.tree = tree
        self.empty_pix = empty_pix
        self.empty_mask = empty_mask
        menu = MenuBar()
        #menu.set_shadow_type(SHADOW_OUT)
        nodes = tree.base_nodes()
        self.createMenu(menu, nodes[0])
        toolbar_menu = nodes[2]
        for n in toolbar_menu.children:
            self.createMenu(menu, n, 1)
        menu.show()

	# added 4/25/2003 - Erica Andrews, let's create a little space around the menu so it doesn't look so 
	# cluttered - some icons were too close to the Preview window's edge
	hb=HBox(0,0)
	hb.set_border_width(2)
	hb.pack_start(menu,1,1,0)
	hb.show_all()

        #self.add(menu) #  disabled, 4/25/2003-  Erica Andrews
	self.add(hb)    #  added, 4/25/2003-  Erica Andrews

    def close(self, x=None, y=None):
        self.hide()
        self.destroy()

    def createMenu(self, menu, node, toolbar=0):
        type = self.tree.getNodeType(node)
        if type == MENUTREE_SEPARATOR:
            mi = MenuItem()
            menu.append(mi)
            mi.show()
        elif type == MENUTREE_SUBMENU:
            name = self.tree.getNodeName(node)
            pix, mask = self.tree.getNodeIcon(node)
            if toolbar:
                if pix == self.empty_pix:
                    pix, mask = None, None
                else:
                    name = None
            mi = PicMenuItem(name, pix, mask)
            submenu = Menu()
            mi.set_submenu(submenu)
            for n in node.children:
                self.createMenu(submenu, n)
            submenu.show()
            menu.append(mi)
            mi.show()
        else:
            command = self.tree.getNodeCommand(node)
            if self.command_ok(command):
                name = self.tree.getNodeName(node)
                pix, mask = self.tree.getNodeIcon(node)
                if toolbar:
                    if pix == self.empty_pix:
                        pix, mask = None, None
                    else:
                        name = None
                mi = PicMenuItem(name, pix, mask)
                mi.set_data("command", command)
                mi.set_data("type", type)
                mi.connect("activate", self.on_menu_activate)
                menu.append(mi)
                mi.show()

    def command_ok(self, command_string):
        "Check if command_string starts with an existing executable."
        if not command_string:
            return 0
        cmd = string.split(command_string, " ")[0]
        # the following code mimics IceWM's executable detection closely:
        if cmd[0] == os.sep:
            # absolute filename:
            # the file must have read and execute access and be regular:
            return os.access(cmd, R_OK|X_OK) and \
                   stat.S_ISREG(os.stat(cmd)[0])
        # search in $PATH:
        for path in PATH:
            fullcmd = os.path.join(path, cmd)
            if os.access(fullcmd, R_OK|X_OK) and \
               stat.S_ISREG(os.stat(fullcmd)[0]):
                return 1
        return 0

    def on_menu_activate(self, menuitem):
        command = menuitem.get_data("command")
        type = menuitem.get_data("type")
        if type == MENUTREE_RESTART:
            message("Info",
                    "This menu entry quits IceWM\n"
                    "and starts %s\n"
                    "as the new window manager.\n"
                    "\n"
                    "IceMe cannot perform this\n"
                    "operation." % command)
        else:
            #print "IceMe: executing", command
            popen2(command)

