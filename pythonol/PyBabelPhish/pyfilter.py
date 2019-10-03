# -*- coding: ISO-8859-1 -*-

##########################
#  Pythoñol
#
#  Copyright (c) 2002-2004
#  Erica Andrews
#  PhrozenSmoke ['at'] yahoo.com
#  http://pythonol.sourceforge.net/
#
#  This software is distributed 
#  free-of-charge and open source 
#  under the terms of the Pythoñol 
#  Free Education Initiative License.
#  
#  You should have received a copy 
#  of this license with your copy of    
#  this software. If you did not, 		
#  you may get a copy of the 	 	 
#  license at:								 
#									   
#  http://pythonol.sourceforge.net   
##########################


# SOME OF THIS CODE IS BORROWED
# FROM ANOTHER PROJECT OF MINES 
# (see additional credits below)

#############################
#  IceWM Control Panel - Common
#  
#  Copyright 2003 by Erica Andrews
#  PhrozenSmoke ['at'] yahoo.com
#  http://icesoundmanager.sourceforge.net
#  
#  This program is distributed free
#  of charge (open source) under the GNU
#  General Public License
#############################

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



# seems to help some Debian systems
try:
	import pygtk
	pygtk.require("2.0")
except:
	pass


import sys

use_filter=0  # needed for windows
wine_friendly=0  # disable some features that make wine crash

# Some PyGtk2 path settings while we upgrade from PyGtk-1 to PyGtk2
try:
        if use_filter==0:
            sys.path=["/usr/lib/python2.2/site-packages/gtk-2.0"]+sys.path
            sys.path=["/usr/lib/python2.3/site-packages/gtk-2.0"]+sys.path
        if use_filter==1:
            sys.path=["C:\\python23\lib\\site-packages\\gtk-2.0"]+sys.path
        # print str(sys.path)
	import gtk
	import gtk.gdk 
	GDK=gtk.gdk
	GTK=gtk
	from gtk import *
except:
	print "Error: You do not have PyGtk-2 properly installed."
	print "This application will not run without it."
	sys.exit(0)
	pass


def to_utf8(somestr):
	unistr = unicode(str(somestr),"iso8859-1")
	utfstr = unistr.encode("utf-8")
	return utfstr


import sys,os,glob,string
from string import join

PYTHONOL_VERSION="2.1"
PYBABELPHISH_VERSION="2.1"

TIPS=gtk.Tooltips() 

WINDOW_WMC="pythonol"  # wm_class for windows
WINDOW_WMN="Pythonol"  # wm_name for windows
HELP_WMC="pythonol-help"  # wm_class for help windows
HELP_WMN="Pythonol-Help"  # wm_name for help windows
DIALOG_WMC="pythonol-dialog"  # wm_class for dialogs
DIALOG_WMN="Pythonol-Dialog"  # wm_name for dialogs


# COLORS for main app
PYTHONOL_COLOR1=GDK.rgb_get_colormap().alloc_color("#3D3FBA")  # blue
PYTHONOL_COLOR2=GDK.rgb_get_colormap().alloc_color("#A02BA0")  # purple
PYTHONOL_COLOR3=GDK.rgb_get_colormap().alloc_color("#000000")  # black
PYTHONOL_COLOR4=GDK.rgb_get_colormap().alloc_color("#8F8CAB") # grey




if sys.platform.lower().find("win")>-1: use_filter=1 # force filtering, disable some features on windows

# Make sure filtering is disabled on Linux/Unix systems
for plat in ["unix","linux","debian","redhat","mandrake","suse","conectiva","slackware","solaris","posix"]:
	if sys.platform.lower().find(plat)>-1: use_filter=0

# When experimenting on Wine...
if wine_friendly==1: use_filter=1

# Don't remove - Pythonol needs these 
filter_dict={"û":"ã»", "á":"Ã¡","ñ":"Ã±","é":"Ã©","ü":"Ã¼","í":"Ã­","ó":"Ã³","ú":"Ãº","¿":"Â¿","É":"Ã©","Ú":"Ãº","Ñ":"Ã±","Ó":"Ã³","Á":"Ã¡","Ü":"Ã¼","Í":"Ã­"}
unfilter_dict={}
for ii in filter_dict.keys():
	if not ii in ["É","Ú","Ñ","Ó","Á","Ü","Í"]:  unfilter_dict[filter_dict[ii]]=ii


global child_friendly
global perm_child_friendly
child_friendly=0
perm_child_friendly=0

# The method below is now deprecated and will be removed in the next release
# right now, it does nothing and is no longer needed

def filter(some_string):
	return some_string

def unfilter(some_string):
	#if use_filter==0: return some_string
	try:
		mychars=some_string.decode("utf-8")
		return mychars.encode("iso8859-1")
	except:  # catch possible unicode error
		mychars2=to_utf8(some_string)
		mychars=mychars2.decode("utf-8")
		return mychars.encode("iso8859-1")


MAIN_PATH=[""+sys.argv[0][0:sys.argv[0].rfind(os.sep)+1]]

def getPixDir() :
	return os.path.abspath(MAIN_PATH[0]+"pixmaps")+os.sep

def getDocDir() :
	return os.path.abspath(MAIN_PATH[0]+"docs")+os.sep

def getUserDocDir() :
	return os.path.abspath(MAIN_PATH[0]+"user-doc")+os.sep

def getDataDir() :
	return os.path.abspath(MAIN_PATH[0]+"datafiles")+os.sep

def getBaseDir() :
	return os.path.abspath(MAIN_PATH[0])+os.sep

def getFestivalDir() :
	return os.path.abspath(MAIN_PATH[0]+"festival")+os.sep   # need relative path for Festival 


def loadPixmap(picon,windowval=None):
    try:
        p=Image()
	if (picon.startswith("gtk-")) :  # stock icon
		# hmmm, no way to really scale stock icons to a special size
		p.set_from_stock(picon,ICON_SIZE_BUTTON)
	else: p.set_from_file(str(picon).strip())
        p.show_all()
        return p
    except:
        return None

# borrowed from my IceWMCP
def getImage(im_file,lab_err="") : # GdkPixbuf Image loading
    try:
        myim= gtk.Image()
	if (im_file.startswith("gtk-")) :  # stock icon
		# hmmm, no way to really scale stock icons to a special size
		myim.set_from_stock(im_file,ICON_SIZE_BUTTON)
        else: myim.set_from_file(str(im_file).strip())
        return myim
    except:
        return gtk.Label(str(lab_err))

# borrowed from my IceWMCP
def loadScaledImage(picon,newheight=40,newwidth=40, is_stock=0):
    try:		
	if is_stock==1:
		tempmen=MenuItem()
		img=tempmen.render_icon(picon,ICON_SIZE_SMALL_TOOLBAR)
		tempmen.destroy()
	else:
        	img = gtk.gdk.pixbuf_new_from_file(picon)
	if newheight>-1:
        	img2 = img.scale_simple(newheight,newwidth,gtk.gdk.INTERP_BILINEAR)
       		pix,mask = img2.render_pixmap_and_mask()
	else: pix,mask = img.render_pixmap_and_mask()
        icon = gtk.Image()
        icon.set_from_pixmap(pix, mask)
        icon.show()
        return icon
    except:
        return None



def getSQL(sql_cursor,query_string):
	try:
		sql_cursor.execute(query_string)	
	except:
		pass


# drag-n-drop support
def addDragSupport(widget,some_func): 
	widget.drag_dest_set(gtk.DEST_DEFAULT_ALL, [('STRING', 0, 0), ('text/plain', 0,1), ('text/uri-list', 0, 2) ], GDK.ACTION_COPY | GDK.ACTION_MOVE)
	#widget.connect("drag_drop",some_func)
	#widget.connect("drag_begin",some_func)
	#widget.connect("drag_end",some_func)
	#widget.connect("drag_data_get",some_func)
	widget.connect("drag_data_received",setDrag)

def setDrag(*args): # drag-n-drop support
	#print str(args)+"  len: "+str(len(args))
	drago=args
	if len(drago)==7: 
		try:
			imfile=""+str(drago[4].data).replace("\r\n","").replace("\x00","").replace("file:","").strip()
                        # bug fix to make drag and drop work on Windows...
                        if use_filter==1:
                            if sys.platform.lower().startswith("win"):
                                imfile=imfile.replace("///","").replace("/",os.sep)
			#print "imfile:  "+imfile
			f=open(imfile,"r",0)
			ff=f.read()
			f.close()			
			text_buffer_set_text(args[0],filter(str(ff)))
		except: 
			pass



#### GTK Constants - eliminate need for py on Unix and GTKconst.py on Windowsc####

#
# Constants for gtk.py
#

# true/false
FALSE = 0
TRUE  = 1


# GtkReliefStyle's
RELIEF_NORMAL = 0
RELIEF_HALF   = 1
RELIEF_NONE   = 2


# GtkShadowType's
SHADOW_NONE       = 0
SHADOW_IN         = 1
SHADOW_OUT        = 2
SHADOW_ETCHED_IN  = 3
SHADOW_ETCHED_OUT = 4


# GtkAnchorType's
ANCHOR_CENTER     = 0
ANCHOR_NORTH      = 1
ANCHOR_NORTH_WEST = 2
ANCHOR_NORTH_EAST = 3
ANCHOR_SOUTH      = 4
ANCHOR_SOUTH_WEST = 5
ANCHOR_SOUTH_EAST = 6
ANCHOR_WEST       = 7
ANCHOR_EAST       = 8
ANCHOR_N          = ANCHOR_NORTH
ANCHOR_NW         = ANCHOR_NORTH_WEST
ANCHOR_NE         = ANCHOR_NORTH_EAST
ANCHOR_S          = ANCHOR_SOUTH
ANCHOR_SW         = ANCHOR_SOUTH_WEST
ANCHOR_SE         = ANCHOR_SOUTH_EAST
ANCHOR_W          = ANCHOR_WEST
ANCHOR_E          = ANCHOR_EAST


def getPixButton (iwin,picon,lab="?",both=0, nobutton=0) :  
        try:
	  if nobutton==0: b=Button()
	  if both==1:
	    h=HBox(0,0)
	    h.set_spacing(4)
	    h.set_border_width(2)
            h.pack_start(getImage(str(picon),to_utf8(lab)),0,0,0)
	    l=Label(str(lab))
	    l.set_alignment(0,0.5)
            h.pack_start(l,1,1,0)
            h.show_all()
	    if nobutton==0: b.add(h)
	    else: b=h
	  else:
	    if nobutton==0: b.add(loadPixmap(str(picon),None))
	    else: b=loadPixmap(str(picon),None)
          b.show_all()        
          return b
        except:
          if nobutton==0: return Button(to_utf8(str(lab)))
	  else: return Label(to_utf8(str(lab)))


#### COMMON DIALOGS BORROWED FROM MY ICEWMCP PROJECT
#### CREDITS BELOW


#######################################
# IceMe - Message Boxes
# =====
#
# Copyright 2000-2002, Dirk Moebius <dmoebius@gmx.net> 
# and Mike Hostetler <thehaas@binary.net>
#
# This software is distributed under the GNU General Public License
#######################################
########
#  With Modifications by Erica Andrews 
#  (PhrozenSmoke ['at'] yahoo.com), April 2003
#  This is a modified version of the 'MessageBox.py" module 
#  from IceMe 1.0.0, optimized for IceWM ControlPanel.
#  Copyright (c) 2003 Erica Andrews
#
#  Changes to the original IceMe message box code are noted 
#  throughout the source as comments.
##################


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


ICON_STOP     = 4 
ICON_ALERT    = 1
ICON_INFO     = 2
ICON_QUESTION = 3
ICON_ERROR    = 4


class _MessageBox(gtk.Dialog):
    def __init__(self, 
                 title="Message", 
                 message="", 
                 buttons=("Ok",),
                 pixmap=ICON_INFO, 
                 modal=gtk.TRUE):
        """
        message: either a string or a list of strings
        pixmap: either a filename string, or one of: ICON_STOP, ICON_ALERT,
                ICON_INFO or ICON_QUESTION, or None

        """
        gtk.Dialog.__init__(self)
	set_window_icon(self,STOCK_DIALOG_QUESTION,1)
	self.set_position(WIN_POS_CENTER)
        self.set_title(title)
        self.set_wmclass(DIALOG_WMC,DIALOG_WMN) # added, 4.2.2003 - Erica Andrews
        self.connect("destroy", self.quit)
        self.connect("delete_event", self.quit)
        self.connect("key-press-event", self.__press)
        self.set_modal(gtk.TRUE)
        hbox = gtk.HBox(spacing=5)
        hbox.set_border_width(10)
        self.vbox.pack_start(hbox)
        hbox.show()
        if pixmap:
            self.realize()
	    try:
            	pic = gtk.Image()
            	if type(icons[pixmap]) == type(""):
                	pic.set_from_stock(icons[pixmap],ICON_SIZE_DIALOG)
            	else:
                	p, m = gtk.gdk.pixmap_create_from_xpm_d(self.window, 
                                                        None, 
                                                        icons[pixmap])
                	pic.set_from_pixmap(p, m)
            	pic.set_alignment(0.5, 0.0)
            	hbox.pack_start(pic, expand=gtk.FALSE)
            	pic.show()
	    except:
		pass
        if type(message) == type(""):
            label = gtk.Label(message)
        else:
            label = gtk.Label(join(message, "\n"))
        label.set_justify(gtk.JUSTIFY_LEFT)
        label.set_line_wrap(1)  # added 4.4.2003 - for better readibility on low-res screens, Erica Andrews
        hbox.pack_start(label)
        label.show()
        if not buttons:
            buttons = ("OK",)
        tips=gtk.Tooltips()
        for text in buttons:
	    if text=="Cancel": b=getPixButton (None,STOCK_CANCEL, "Cancel" ,1)
	    elif text=='Cancel Search': b=getPixButton (None,STOCK_CANCEL, 'Cancel Search' ,1)
	    elif text=="OK": b=getPixButton (None,STOCK_OK, "OK" ,1)
	    elif text=="Yes": b=getPixButton (None,STOCK_YES, "Yes" ,1)
	    elif text=="No": b=getPixButton (None,STOCK_NO, "No" ,1)
            else: b=getPixButton (None,STOCK_APPLY, text.strip() ,1)
            tips.set_tip(b,text)
            if text == buttons[0]:
                # the first button will always be the default button:
                b.set_flags(gtk.CAN_FOCUS|gtk.CAN_DEFAULT|gtk.HAS_FOCUS|gtk.HAS_DEFAULT)
            b.set_flags(gtk.CAN_DEFAULT)
            b.set_data("user_data", text)
            b.connect("clicked", self.__click)
            self.action_area.pack_start(b)
            b.show()
        self.default = buttons[0]
        self.ret = None

    def quit(self, *args):
        self.hide()
        self.destroy()
        gtk.mainquit()

    def __click(self, button):
        self.ret = button.get_data("user_data")
        self.quit()

    def __press(self, widget, event):
        if event.keyval == gtk.keysyms.Return:
            self.ret = self.default
            self.quit()


def dialog_message(title="Message", message="", buttons=("Ok",),
            pixmap=ICON_INFO, modal=gtk.TRUE):
    "create a message box, and return which button was pressed"
    win = _MessageBox(title, message, buttons, pixmap, modal)
    win.show()
    gtk.mainloop()
    return win.ret


################################################################################
# some icons that can be used for the dialog
################################################################################


pybabel_icon = [
"32 32 9 1",
" 	c None",
".	c #000080",
"+	c #FF0000",
"@	c #FFFFFF",
"#	c #5E455A",
"$	c #448AD4",
"%	c #DAB3D5",
"&	c #A752FE",
"*	c #FFFF00",
".........++++++++++++           ",
".@.@.@.@.@@@@@@@@@@@@           ",
"..@.@.@..++++++++++++           ",
".@.@.@.@.@@##@###@@@@           ",
"..@.@.@..+#$$#$$$#+++           ",
".@.@.@.@.#######$$#@@           ",
".........#%%#%%%#$#++           ",
"@@@@@@@@@#%%#%%%#$#@@           ",
"+++++++++#%%#%%%#$#++           ",
"@@@@@@@@@#%%#%%%#$##@           ",
"++++++++####$###$$$####         ",
"@@@@@@##$$$$$$$$$$$#$$$##       ",
"+++++#$$$$$$$$$$$$$&#$$$$#      ",
"@@@@#$$$$$$$$$$$&$&&&#$$$$#     ",
"    #$#$$#$$$$&$&&&&&#$$$$#     ",
"    #$$$$$$$&$&&##&&&#$$$$$#    ",
"    #&&$&$&$&&&##&$&#&$$$$$#    ",
"    #&&&&&&&&###$&&#&&&$$$$#    ",
"     #&$&$&$###&$&##$$&$$$$#    ",
"      ###$$###$&&#+#&&&$$$$#++++",
"      ##$&##$&&&#++#$&$$$$#+++++",
"      #&$&$&&&##++#&&$$$$$#+++++",
"      #&&&&&##**#$$&$$$$$#******",
"       #####***#&&&$$$$$#*******",
"            *#$$&&$$$$$#********",
"           ##&&&$$$$$$#*********",
"          #$$$&$$$$$$##*********",
"            ********************",
"            ********************",
"            ++++++++++++++++++++",
"            ++++++++++++++++++++",
"            ++++++++++++++++++++"
]

pythonol_icon = [
"32 32 9 1",
" 	c None",
".	c #000080",
"+	c #FF0000",
"@	c #FFFFFF",
"#	c #334C49",
"$	c #99BF32",
"%	c #9CC7C1",
"&	c #40FD5B",
"*	c #FFFF00",
".........++++++++++++           ",
".@.@.@.@.@@@@@@@@@@@@           ",
"..@.@.@..++++++++++++           ",
".@.@.@.@.@@##@###@@@@           ",
"..@.@.@..+#$$#$$$#+++           ",
".@.@.@.@.#######$$#@@           ",
".........#%%#%%%#$#++           ",
"@@@@@@@@@#%%#%%%#$#@@           ",
"+++++++++#%%#%%%#$#++           ",
"@@@@@@@@@#%%#%%%#$##@           ",
"++++++++####$###$$$####         ",
"@@@@@@##$$$$$$$$$$$#$$$##       ",
"+++++#$$$$$$$$$$$$$&#$$$$#      ",
"@@@@#$$$$$$$$$$$&$&&&#$$$$#     ",
"    #$#$$#$$$$&$&&&&&#$$$$#     ",
"    #$$$$$$$&$&&##&&&#$$$$$#    ",
"    #&&$&$&$&&&##&$&#&$$$$$#    ",
"    #&&&&&&&&###$&&#&&&$$$$#    ",
"     #&$&$&$###&$&##$$&$$$$#    ",
"      ###$$###$&&#+#&&&$$$$#++++",
"      ##$&##$&&&#++#$&$$$$#+++++",
"      #&$&$&&&##++#&&$$$$$#+++++",
"      #&&&&&##**#$$&$$$$$#******",
"       #####***#&&&$$$$$#*******",
"            *#$$&&$$$$$#********",
"           ##&&&$$$$$$#*********",
"          #$$$&$$$$$$##*********",
"            ********************",
"            ********************",
"            ++++++++++++++++++++",
"            ++++++++++++++++++++",
"            ++++++++++++++++++++"
]


icon_alert = STOCK_DIALOG_WARNING
icon_stop = STOCK_DIALOG_ERROR
icon_info = STOCK_DIALOG_INFO
icon_question = STOCK_DIALOG_QUESTION


# changed, 4.2.2003 - Erica Andrews, added 'icon_stop' as 5th list element
icons = [ icon_stop, icon_alert, icon_info, icon_question, icon_stop ]



#############
# easy dialog callbacks
#############

DIALOG_OK="OK"
DIALOG_CANCEL="Cancel"

def msg_info(wintitle,message):
	if str(message).lower().find("error")>-1:
		msg_err(wintitle,message)
	else:
		dialog_message(wintitle,message.split("\n"),(DIALOG_OK,),2,1)

def msg_warn(wintitle,message):
	dialog_message(wintitle,message.split("\n"),(DIALOG_OK,),1,1)

def msg_err(wintitle,message):
	dialog_message(wintitle,message.split("\n"),(DIALOG_OK,),4,1)

def msg_confirm(wintitle,message,d_ok=DIALOG_OK,d_cancel=DIALOG_CANCEL):
	ret=dialog_message(wintitle,message.split("\n"),(d_ok,d_cancel,),3,1)
	if str(ret)==d_ok: return 1
	else: return 0



### COMMON FILE SELECTIONS, borrowed from my IceWMCP Project

def keyPressClose(widget, event,*args):
    if event.keyval == gtk.keysyms.Escape:
        widget.hide()
        widget.destroy()
        widget.unmap()
    elif event.keyval == gtk.keysyms.Return:
        if widget.get_data("ignore_return")==None: # some windows shouldn't close on 'Return'
            widget.hide()
            widget.destroy()
            widget.unmap()


# added 6.18.2003 - common file selection functionality
PYTHONOL_FILE_WIN=None
PYTHONOL_LAST_FILE=os.getcwd()
FILE_SELECTOR_TITLE="Select a file..."

def CLOSE_FILE_SELECTOR(*args):
	   try:
		PYTHONOL_FILE_WIN.hide()
		PYTHONOL_FILE_WIN.destroy()
		PYTHONOL_FILE_WIN.unmap()
	   except:
		pass

def GET_SELECTED_FILE(*args):
		gfile=PYTHONOL_FILE_WIN.get_filename()
		global PYTHONOL_LAST_FILE
		if not gfile==None: PYTHONOL_LAST_FILE=gfile
		CLOSE_FILE_SELECTOR()
		return gfile

def SET_SELECTED_FILE(file_name):
		global PYTHONOL_LAST_FILE
		PYTHONOL_LAST_FILE=str(file_name)

def SELECT_A_FILE(file_sel_cb,title=FILE_SELECTOR_TITLE,wm_class="pythonol-dialog",wm_name="Pythonol-Dialog",widget=None,ok_button_title=None,cancel_button_title=None,ok_button_icon=STOCK_SAVE):
		global PYTHONOL_FILE_WIN
		PYTHONOL_FILE_WIN = FileSelection(title)
		set_window_icon(PYTHONOL_FILE_WIN, STOCK_SAVE,1)
		PYTHONOL_FILE_WIN.set_wmclass(wm_class,wm_name)
		PYTHONOL_FILE_WIN.ok_button.connect('clicked', file_sel_cb)
		value = PYTHONOL_LAST_FILE
		if not widget==None: 
			PYTHONOL_FILE_WIN.ok_button.set_data("cfg_path",widget.get_data("cfg_path"))
			if value=='': value=widget.get_data("cfg_path").get_text()
		if not ok_button_title==None:
			PYTHONOL_FILE_WIN.ok_button.remove(PYTHONOL_FILE_WIN.ok_button.get_children()[0])
			PYTHONOL_FILE_WIN.ok_button.add( 
				getPixButton (None,ok_button_icon, str(ok_button_title) ,1, 1))			
		if not cancel_button_title==None:
			PYTHONOL_FILE_WIN.cancel_button.remove(PYTHONOL_FILE_WIN.cancel_button.get_children()[0])
			PYTHONOL_FILE_WIN.cancel_button.add( 
				getPixButton (None,STOCK_CANCEL, str(cancel_button_title) ,1, 1))			
		PYTHONOL_FILE_WIN.cancel_button.connect('clicked', CLOSE_FILE_SELECTOR)
		PYTHONOL_FILE_WIN.connect("destroy",CLOSE_FILE_SELECTOR)
		if value != '""':
			PYTHONOL_FILE_WIN.set_filename(value)
		#print "Last File:  "+str(value)
		PYTHONOL_FILE_WIN.set_data("ignore_return",1)
		PYTHONOL_FILE_WIN.connect("key-press-event", keyPressClose)
		PYTHONOL_FILE_WIN.set_modal(1)
                PYTHONOL_FILE_WIN.set_position(WIN_POS_CENTER)
		PYTHONOL_FILE_WIN.show_all()

# end - common file selection functionality


####### Text Rendering Code, borrowed from IceWMCP

LANGCODE="iso8859-1"  # good code for Spanish, English and most Euro languages
global RENDER_TAG
RENDER_TAG=0

def get_renderable_tab(mybuffer, mycolor):
	global RENDER_TAG
	RENDER_TAG=RENDER_TAG+1
	tagname="render-tag"+str(RENDER_TAG)
	texttag=mybuffer.create_tag(tagname)
	if mycolor: texttag.set_property("foreground-gdk",mycolor)
	texttag.set_property("font","Arial 12")
	texttag.set_property("language",LANGCODE)
	return tagname

def text_area_insert(ta, mycolor,mytext):
	textbuf=ta.get_buffer()
	textbuf.freeze_notify()
	tags=[get_renderable_tab(textbuf,mycolor)]
	textbuf.insert_with_tags_by_name(textbuf.get_end_iter(), to_utf8(mytext), *tags)
	textbuf.thaw_notify()

def text_buffer_set_text(sctext,stuff):
	sctext.get_buffer().freeze_notify()
	tags=[get_renderable_tab(sctext.get_buffer(),None)]
        sctext.get_buffer().set_text("")
	sctext.get_buffer().insert_with_tags_by_name(sctext.get_buffer().get_end_iter(), to_utf8(stuff), *tags)
        #sctext.get_buffer().set_text(to_utf8(stuff))
	sctext.get_buffer().thaw_notify()

def text_buffer_insert_simple(sctext,stuff):
	sctext.get_buffer().freeze_notify()
	sctext.get_buffer().insert_at_cursor(to_utf8(str(stuff)))
	sctext.get_buffer().thaw_notify()

def text_buffer_get_text(sctext):
	sctextbuf=sctext.get_buffer()	
	stuffz=sctextbuf.get_text(sctextbuf.get_start_iter(),sctextbuf.get_end_iter())
	return stuffz


def set_window_icon(mywindow,my_xpm, is_stock=0):
	if is_stock==1:
		try:
			tempmen=MenuItem()
			# steal the GTK stock icon for window frame
			mywindow.set_icon(tempmen.render_icon(my_xpm,ICON_SIZE_DIALOG))
			tempmen.destroy()
		except:
			pass
	try:
		mywindow.set_icon(GDK.pixbuf_new_from_xpm_data(my_xpm))
	except:
		pass




