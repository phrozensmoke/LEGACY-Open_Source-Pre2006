# -*- coding: ISO-8859-1 -*-


######################
# PyPrint 
#
# Copyright (c) 2002-2004 
# Erica Andrews
# PhrozenSmoke ['at'] yahoo.com
# http://pythonol.sourceforge.net/
#
# This software is 
# distributed under the 
# terms of the GNU 
# General Public License.
######################

from pyfilter import *
import pythonol_help


global PYPRINT_VERSION
PYPRINT_VERSION="0.3.1"

def launch(launch_command):
	os.popen(str(launch_command))

def showMessage(message_text,wintitle="PyPrint"):
	global PYPRINT_VERSION
	return msg_warn(str(wintitle)+" "+PYPRINT_VERSION, to_utf8(str(message_text)))

def showConfirm(message_text,wintitle="PyPrint"):
	global PYPRINT_VERSION
	return msg_confirm(str(wintitle)+" "+PYPRINT_VERSION, to_utf8(str(message_text)))

def showAbout(message_text,myappname="PyPrint"):
	showHelp(str(myappname)+" "+PYPRINT_VERSION+"\n\nCopyright (c) 2002-2004 by Erica Andrews\nPhrozenSmoke@yahoo.com\nhttp://pythonol.sourceforge.net/\n\n" +str(message_text) +"\n\n"+str(myappname)+" is open source under the \nGNU General Public License.\nNO technical support will \nbe provided for this application.\nEnjoy!","About "+myappname)

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
	return pythonol_help.getIconButton (windowval,picon,btext,1)

def loadImage(picon,windowval):
	return loadPixmap(str(picon),None)


def showHelp(helptext,wintitle="PyPrint"):
	pythonol_help.showHelp(helptext,wintitle,0,1)
