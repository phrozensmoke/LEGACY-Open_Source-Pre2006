import os,GTK,sys
from gtk import *
#############################################
#  PySpool 0.2.3
#  Copyright 2003 by Erica Andrews
#  PhrozenSmoke@yahoo.com
#  All rights reserved.
#  
#  PySpool is a simple Gtk-based spool monitor and 
#  manager written in 100% Python. It is intended to 
#  monitor LPRng and LPR-based printer spools. 
#  This is BETA code. PySpool has only been tested 
#  with LPRng 3.8.12 on local printers. PySpool 
#  requires the following executables on your PATH: 
#  lpstat, cancel, checkpc
#  
#  PySpool is open source under the General Public 
#  License (GPL). PySpool is part of the PyPrint
#  utility package (same author).
#  
#  NO technical support will be provided for this 
#  application. 
#  Enjoy!
#############################################

#set translation support
from icewmcp_common import *
_=translateCP   # from icewmcp_common.py

global PYPRINT_VERSION
PYPRINT_VERSION="0.2.3"

def launch(launch_command):
	os.popen(str(launch_command))

def showMessage(message_text,wintitle="PyPrint"):
	global PYPRINT_VERSION
	msg_info(str(wintitle)+" "+PYPRINT_VERSION,str(message_text))

def showConfirm(message_text,wintitle="PyPrint"):
	global PYPRINT_VERSION
	return msg_confirm(str(wintitle)+" "+PYPRINT_VERSION,str(message_text),_('Yes'),_('No'))


def showAbout(message_text,myappname="PyPrint"):
	showHelp(str(myappname)+" "+PYPRINT_VERSION+"\n\nCopyright (c) 2003 by Erica Andrews\nPhrozenSmoke@yahoo.com\n"+_("All rights reserved.")+"\n\n" +str(message_text) +"\n\n"+str(myappname)+_(" is open source under the General Public License (GPL).\nNO technical support will be provided for this application.\nEnjoy!"),_("About ")+myappname)

def readOSLines(os_popen_cmd):
	try:
		return list(os.popen(str(os_popen_cmd)).xreadlines())
	except:
		return []

def getPrinterNames() :
	xlines=readOSLines("lpstat -v")
	printers=[]
	for pp in xlines:
		pline=pp.strip()
		if pline.find("system for") > -1:
			if pline.find(":")> pline.find("system for"):
				printer_name=pline[pline.find("system for")+len("system for"):pline.find(":")].strip()
				if not printer_name=="all": printers.append(printer_name)
	return printers

def isOnPath(binary):
  if os.environ.has_key("PATH"):
    paths=os.environ["PATH"].split(":")
    for i in paths:
      p=i
      if not p.endswith("/"): p=p+"/"
      if os.path.exists(p+str(binary)):
        return 1
  return 0

def getPixmapButton (picon,btext,windowval) :  
	b=GtkButton()
	b.add(getPixmapVBox(picon,btext,windowval))
	b.show_all()        
	return b

def loadImage(picon,windowval):
	try:
		p=GtkPixmap(windowval,getPixDir()+str(picon),None)
		p.show_all()
		return p
	except:
		return None

def getPixmapVBox(picon,btext,windowval):
        try:
	  p=loadImage(picon,windowval)
          if not p: raise TypeError
	  v=GtkHBox(1,2)
          v.set_homogeneous(0)
	  v.set_spacing(3)
          v.pack_start(p,0,1,1)
          v.pack_start(GtkLabel(str(btext)),0,1,1)
          v.show_all()
          return v
        except:
          v=GtkHBox(1,1)
          v.pack_start(GtkLabel(str(btext)),1,1,1)
          v.show_all()
          return v

def showHelp(helptext,wintitle="PyPrint"):
	commonAbout(wintitle,helptext,0)
