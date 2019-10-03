######################################
# IceWM Control Panel Wallpaper Settings
# 
# Copyright 2003 by Erica Andrews 
# (PhrozenSmoke@yahoo.com)
# http://icesoundmanager.sourceforge.net
# 
# Centralized drag-n-drop support
# 
# This program is distributed under the GNU General
# Public License (open source).
#######################################

import gtk,GTK,GDK
from gtk import *
from GTK import *
from GDK import *

# drag-n-drop support, added 2.23.2003
# for basic 'string'-based drag-n-drop

def addDragSupport(widget, drag_callback): 
	widget.drag_dest_set(gtk.DEST_DEFAULT_ALL, [('STRING', 0, 0), ('text/plain', 0,1), ('text/uri-list', 0, 2) ], gtk.GDK.ACTION_COPY | gtk.GDK.ACTION_MOVE)
	widget.connect("drag_drop",drag_callback)
	widget.connect("drag_begin",drag_callback)
	widget.connect("drag_data_received",drag_callback)


def initColorDrag():   # mainly for IcePref2, IcePref-TD, and IceWMCPWallpaper - 'color buttons'
	# added 5.16.2003, a hidden color dialog for handling drag-n-drop color support on 'color buttons'
	global COLOR_HOLDER

	# using a 'popup' (frameless) window type, so it wont map to the taskbar and make the screen 'flicker'
	COLOR_HOLDER=GtkWindow(GTK.WINDOW_POPUP) 
	COLOR_HOLDER.colorsel=GtkColorSelection()
	COLOR_HOLDER.add(COLOR_HOLDER.colorsel)
	COLOR_HOLDER.set_position(GTK.WIN_POS_CENTER)
	COLOR_HOLDER.set_usize(1,1)
	COLOR_HOLDER.show_all() # must be called now, before use, to avoid 'GtkCritical' whining on stdout
	COLOR_HOLDER.hide()
	COLOR_HOLDER.colorsel.set_update_policy(UPDATE_CONTINUOUS)

	global setDrag2
	def setDrag2(*args): # drag-n-drop callback, added 5.16.2003, drag-n-drop support for 'color buttons'
		drago=args
		if len(drago)<7: 
			args[0].drag_highlight()
			args[0].grab_focus()
		else: 
			try:			
				global COLOR_HOLDER
				COLOR_HOLDER.show()
				COLOR_HOLDER.set_position(GTK.WIN_POS_MOUSE)
				COLOR_HOLDER.hide()

				""" 5.16.2003 - Erica Andrews
			 	send the same drag-n-drop signal we just received to the hidden COLOR_HOLDER's  
				 'preview' (GtkPreview) box, to set the color - I don't know a work-around
				 for this yet, since the 'application/x-color' returned by the drag-n-drop
			 	GtkSelectionData.data object is some type of unparseable 'native' unicode 
			 	junk that Python doesn't appear to be able to turn into a string, int or 
			 	anything useful - so, we will let the built-in drag-n-drop of the 
			 	GtkColorSelectionDialog handle it the way it knows how since it has direct 
			 	access to the 'secret' C code underneath PyGtk  """

				wprev=COLOR_HOLDER.colorsel.children()[0].children()[0].children()[1].children()[0]	
				wprev.emit("drag_data_received",drago[1],drago[2],drago[3],drago[4],drago[5],drago[6])
				col=COLOR_HOLDER.colorsel.get_color()

				# update the color on the 'color button' can called this drag-n-drop
				drago[0].get_data("colorbutton").updateColorProperties(col[0],col[1],col[2])
			except:
				pass
			args[0].drag_unhighlight()


	global addDragSupportColor
	def addDragSupportColor(widget): # drag-n-drop support, added 5.15.2003 , for color buttons
		widget.drag_dest_set(gtk.DEST_DEFAULT_ALL, [('application/x-icewmcp-color', 0, 0), ("application/x-color", 0, 0)], gtk.GDK.ACTION_COPY | gtk.GDK.ACTION_MOVE)
		widget.connect("drag_drop",setDrag2)
		widget.connect("drag_begin",setDrag2)
		widget.connect("drag_data_received",setDrag2)

