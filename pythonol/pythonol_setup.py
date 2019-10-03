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

import pyfilter
from pyfilter import *
import pytheme
import time

def showGtkMessage(mess_str):
    	pytheme.installTheme(None)  # make sure some kind of font is loaded 
	msg_info(to_utf8(filter("Pythoñol")),to_utf8(filter(mess_str)))	

try:
	sys.path.append(getBaseDir()[0:getBaseDir().rfind(os.sep)])
	import sqlite
except:
	showGtkMessage("Sorry, you do not have the PySqlite/Sqlite extension library properly installed. Databases cannot be built yet, and Pythoñol cannot run without it.\n\nPlease re-download  Pythoñol from:\nhttp://pythonol.sourceforge.net\nand re-install the application.")
	sys.exit(0)

db_source_files=["data_dict1.txt","data_dict2.txt","data_dict3.txt","data_dict4.txt","data_dict5.txt","data_idioms.txt","data_thes_cat.txt"]

db_built_files=['dictionary.db','dictionary-idioms.db','thesaurus-cats.db']

def getDictDir():
	return getBaseDir()+"dictionary"+os.sep

def getCopyDir():
	return getBaseDir()+"datafiles"+os.sep

def checkMakeDb(*argies):
	try:
		if not getDictDir()[0:getDictDir().rfind(os.sep)] in sys.path: sys.path.append(getDictDir()[0:getDictDir().rfind(os.sep)])
		global makedb
		import makedb
		makedb.getDictDir=getDictDir
		makedb.getCopyDir=getCopyDir
		return 1
	except:
		return 0
	return 1

def showNoMakeDb(*argies):
	showGtkMessage("Sorry, the 'makedb.py' module could not be imported from "+os.path.abspath(getDictDir())+"\n\nPythoñol cannot create the necessary databases without this file.\n\nPlease re-download Pythoñol from:\nhttp://pythonol.sourceforge.net\nand re-install the application.")

def checkBuiltDb(*argies):
	for ii in db_built_files:
		try:
			if os.path.getsize(getDataDir()+ii)==0: return 0
		except:
			return 0
	return 1

def checkFiles(*argies):
	MISSING=[]
	for ii in db_source_files:
		try:
			if os.path.getsize(getDictDir()+ii)==0: raise TypeError
		except:
			MISSING.append(ii)
	return MISSING

def showMissingFiles(missing_files):
	showGtkMessage("Sorry, Pythoñol can't find the following files:\n\n"+"\n".join(missing_files)+"\n\nThese files are needed to create the Pythoñol language databases. Please re-download the Pythonol data files from:\nhttp://pythonol.sourceforge.net\nand re-install the application.")

def checkMatureFile(*argies):
	try:
		return os.path.getsize(getDictDir()+"data_mature.txt")>0
	except:
		return 0

def showMissingMature(*argies):
	showGtkMessage("WARNING:\nThe data file needed to create the 'mature content' database could not be found. Pythoñol databases were created with mature content DISABLED.\n\nIf you wish to enable 'mature content' please re-download the Pythoñol data files from:\n\nhttp://pythonol.sourceforge.net")	


def savePreferences(*argies):
	global child_friendly
	global perm_child_friendly
	try:
		pref_file=os.environ['HOME']+os.sep+".pythonol_pref"
		if use_filter==1: 
			pref_file=os.getenv("HOME", os.getenv("WINDIR","") )+os.sep+"pythonol_pref"
		f=open(pref_file,"w")
		f.write("child_friendly="+str(int(child_friendly))+"\nperm_child_friendly="+str(int(perm_child_friendly)))
		f.flush()
		f.close()
	except:
		pass

def readPreferences(*argies):
	global child_friendly
	global perm_child_friendly
	pyfilter.child_friendly=child_friendly
	pyfilter.perm_child_friendly=perm_child_friendly
	try:
		ddict={}
		pref_file=os.environ['HOME']+os.sep+".pythonol_pref"
		if use_filter==1: 
			pref_file=os.getenv("HOME", os.getenv("WINDIR","") )+os.sep+"pythonol_pref"
		f=open(pref_file)
		flist=f.read().replace("\r","").split("\n")
		f.close()
		for ii in flist:
			if ii.find("=")==-1: continue
			ddict[ii.split("=")[0].strip()]=ii.split("=")[1].strip()
		try:
			ig=int(ddict["child_friendly"])
			if (ig==0) or (ig==1): 
				child_friendly=ig
				pyfilter.child_friendly=ig
		except:
			child_friendly=0
			pyfilter.child_friendly=0
		try:
			ig=int(ddict["perm_child_friendly"])
			if (ig==0) or (ig==1): 
				perm_child_friendly=ig
				pyfilter.perm_child_friendly=ig
		except:
			perm_child_friendly=0
			pyfilter.perm_child_friendly=0
	except:
		pass

def checkPermFriendly(*argies):
	global child_friendly
	global perm_child_friendly
	if not perm_child_friendly==1: return
	flist=[getDataDir()+"dictionary-mature.db",getDictDir()+"data_mature.txt"]
	for ii in flist:
		try:
			os.remove(ii)
		except:
			pass
	swarn=""
	for ii in flist:
		try:
			if os.path.getsize(ii)>0: swarn=swarn+ii+"\n"
		except:
			pass
	if len(swarn)>0: 
		showGtkMessage("WARNING:\nPythoñol was unable to delete the following files:\n\n"+swarn+"\n\nThis means that Pythoñol is NOT running in PERMENANT child-friendly mode.  You must delete these files yourself, then re-start Pythoñol if you wish to make this application PERMENANTLY child-friendly.")
	child_friendly=1
	perm_child_friendly=1	


global INSTALL_LABEL
INSTALL_LABEL=None
global INSTALL_WINDOW
INSTALL_WINDOW=None


def killInstallWin(*argies):
	try:
		global INSTALL_WINDOW
		INSTALL_WINDOW.hide()
		INSTALL_WINDOW.destroy()
		INSTALL_WINDOW.unmap()
	except:
		pass

def showInstallWindow():
	global INSTALL_WINDOW
	if use_filter==1: 
		INSTALL_WINDOW=Window(WINDOW_POPUP)
		#set_window_icon(INSTALL_WINDOW,pythonol_icon)
	else: 
		INSTALL_WINDOW=Window(WINDOW_POPUP)
	pytheme.installTheme(INSTALL_WINDOW)
	INSTALL_WINDOW.set_wmclass("pythonol-splash","Pythonol-Splash")	
	INSTALL_WINDOW.set_title (to_utf8(filter('Pythoñol: Setup')))
	INSTALL_WINDOW.set_position (WIN_POS_CENTER)
	INSTALL_WINDOW.realize()
	#INSTALL_WINDOW.set_modal(1)
	vbox1 =VBox (0, 2)
	vbox1.set_border_width(5)
	pixlap = getImage(getPixDir()+"title.gif", to_utf8(filter('PYTHOÑOL'))  )
	vbox1.pack_start(pixlap,0,0,2)
	vbox1.pack_start(Label(to_utf8("Copyright (c) 2002-2004 Erica Andrews.\n")),0,0,5)
	vbox1.pack_start(Label(to_utf8(filter("BUILDING PYTHONOL LANGUAGE DATABASES\n\nPythoñol is building the language databases needed\nfor this application. You will only see this window again if\nyour databases are deleted or you re-install the databases.\nThis may take 2-3 minutes: Please be patient.\n"))),0,0,0)
	global INSTALL_LABEL
	INSTALL_LABEL=Label(to_utf8("                 Please wait...                    "))
	vbox1.pack_start(INSTALL_LABEL,1,1,12)
	INSTALL_WINDOW.add(vbox1)
	INSTALL_WINDOW.show_all()		
	return 0

def createDBs(*argies):
	time.sleep(2)
	while events_pending():
		mainiteration()
	showInstallWindow()
	global INSTALL_LABEL
	makedb.INSTALL_LABEL=INSTALL_LABEL
	while events_pending():
		mainiteration()
	db_results=makedb.createDatabases()
	killInstallWin()
	if db_results==1: return 1
	else:
		showGtkMessage("ERROR:\nOne or more of the necessary Pythoñol language databases could not be created.  Pythoñol cannot run without all necessary databases.\n\nYou may either re-start Pythoñol and try again, or re-download Pythoñol from: \n\nhttp://pythonol.sourceforge.net\n\nRemember: NO TECHNICAL SUPPORT is provided for the general public. Troubleshoot the problem on your own.")


def checkWantChildFriendly(*argies):
	global child_friendly
	global perm_child_friendly
	if msg_confirm(to_utf8("Child-Friendly Settings"), to_utf8(filter("CHILD-FRIENDLY OPTIONS\n\nYou may choose to make Pythoñol PERMANENTLY Child-friendly. Making Pythoñol PERMANENTLY Child-friendly removes 'mature' content from the Pythoñol language databases, making this application more suitable for children and sensitive users. If you ever wish to re-enable 'mature' content, you will have to re-download the data files for Pythonol from: http://pythonol.sourceforge.net\n\nDo you wish to make Pythoñol  PERMANENTLY  Child-friendly?\n")), "Yes", "No"): 
		perm_child_friendly=1
		checkPermFriendly()
		savePreferences()
		return 1
	else: 
		perm_child_friendly=0
		child_friendly=0
		savePreferences()
	return 0
		


def checkPythonolSetup(force_it=0):  # force_it for re-installing database while Pythonol is already running
	readPreferences()
	if (checkBuiltDb()==0) or (force_it==1): # initial installation, because .db files not there
		if checkMakeDb()==0:
			showNoMakeDb()	
			if force_it==1: return 
			sys.exit(0)
		ckfiles=checkFiles()
		if len(ckfiles)>0:
			showMissingFiles(ckfiles)
			if force_it==1: return 
			sys.exit(0)
		if createDBs()==1:
			showGtkMessage("Your Pythoñol language databases were successfully created.")
		else: 			
			if checkBuiltDb()==0:  sys.exit(0)
		if checkMatureFile()==0: showMissingMature()
		checkWantChildFriendly()
		showGtkMessage("Setup of Pythoñol is now complete.")
	else: checkPermFriendly()
