#! /usr/bin/env python
# -*- coding: ISO-8859-1 -*-

##########################
#  Pythoñol
#
#  Copyright (c) 2003-2004
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

import os,sys

global INSTALL_LABEL
INSTALL_LABEL=None

def updateInstallLabel(some_text):
	try:
		if INSTALL_LABEL==None: return
		import gtk
		INSTALL_LABEL.set_text(str(some_text))
		while gtk.events_pending():
			gtk.maininteration()
	except:
		pass

try:
	mypath=os.path.abspath(sys.argv[0])[0:os.path.abspath(sys.argv[0]).rfind(os.sep)]
	if mypath.endswith("dictionary"):
		sys.path.append(mypath[0:mypath.rfind("dictionary")-1])
	import sqlite

except:
	print "Sorry, you do not have the PySqlite/Sqlite extension properly installed.\nDatabases cannot be built yet."
	sys.exit(0)

def getDictDir() :
	mod=os.path.abspath(sys.argv[0])
	pmod=mod[0:mod.rfind(os.sep)+1]
	if not pmod.endswith("dictionary"+os.sep): pmod=pmod+"dictionary"+os.sep
	#print pmod
	return pmod

def getCopyDir() :
	mod=os.path.abspath(sys.argv[0])
	pmod=mod[0:mod.rfind(os.sep)]
	if pmod.endswith("dictionary"): pmod=pmod[0:pmod.rfind(os.sep)]
	#print pmod
	pmod=pmod+os.sep+"datafiles"+os.sep
	try:
		os.mkdirs(pmod)
	except:
		pass
	return pmod

BUILD_COUNTER=0

t1_header="CREATE TABLE definitions (spanish varchar(175) NOT NULL, english varchar(175) NOT NULL, PRIMARY KEY(spanish,english));"

t2_header="CREATE TABLE definitions (word_group varchar(15) NOT NULL, searchwords varchar(255) NOT NULL, PRIMARY KEY(word_group));"


friendly=['data_dict5.txt','data_dict1.txt' ,'data_dict3.txt','data_dict4.txt']
mature=['data_mature.txt']
idioms=['data_dict2.txt','data_idioms.txt']
thescat=['data_thes_cat.txt']  # t2


def getInsertStr(spanish,english):
	return "INSERT into definitions values (\""+spanish.replace("\"","'")+"\",\""+english.replace("\"","'")+"\")"

def createSQL(filename,file_list,connection,file_header):
   try:
	d={}
	global BUILD_COUNTER
	"""f=open(filename,"w")
	f.write(file_header+"\n\n")"""
	for ii in file_list:
		ff=open(getDictDir()+ii)
		flist=ff.read().replace("\r","").split("\n")
		ff.close()
		for yy in flist:
			pair = yy.split(":")
			try:
				d[pair[0]]=pair[1]
			except:
				pass
	cursor=connection.cursor()
	cursor.execute(file_header)
	connection.commit()
	for gg in d.keys():
		istring=getInsertStr(gg,d[gg])+";"
		"""f.write(istring+"\n")"""
		try:
			cursor.execute(istring)
		except:
			pass
	global INSTALL_LABEL
	if INSTALL_LABEL==None: print "Wrote:  "+str(len(d))+" definitions."
	BUILD_COUNTER=BUILD_COUNTER+1
	if INSTALL_LABEL==None: print "Built database "+str(BUILD_COUNTER)+" of 4."
	else: updateInstallLabel("Built database "+str(BUILD_COUNTER)+" of 4.")
	connection.commit()
	connection.close()
	"""f.flush()
	f.close()"""

	return 1

   except:
	return 0

def dodelete(filename):
	try:
		os.remove(filename)
	except:
		pass

def cpfile(filename,filename2):
	try:
		f=open(filename,"rb",0)  # big bug fix for Python 2.3 on windows
		ff=f.read()
		f.close()
		f=open(filename2,"wb")  # big bug fix for Python 2.3 on windows
		f.write(ff)
		f.flush()
		f.close()
		return 1
	except:
		return 0


def createDatabases():
   try:
	for ifilez in ["mature.db","all.db", "idioms.db", "cats.db", "words.db"]:
		dodelete(getDictDir()+ifilez)


	connm = sqlite.connect(db=getDictDir()+"cats.db", mode=077,converters={},autocommit=0,encoding="iso8859-1")
	if not createSQL("cats.sql",thescat,connm,t2_header)==1: raise TypeError
	if not cpfile(getDictDir()+"cats.db",getCopyDir()+"thesaurus-cats.db")==1: raise TypeError

	connm = sqlite.connect(db=getDictDir()+"idioms.db", mode=077,converters={},autocommit=0,encoding="iso8859-1")
	if not createSQL("idioms.sql",idioms,connm,t1_header)==1: raise TypeError
	if not cpfile(getDictDir()+"idioms.db",getCopyDir()+"dictionary-idioms.db")==1: raise TypeError

	connm = sqlite.connect(db=getDictDir()+"mature.db", mode=077,converters={},autocommit=0,encoding="iso8859-1") # dont throw exception here, child-friendly
	createSQL("mature.sql",mature,connm,t1_header)
	cpfile(getDictDir()+"mature.db",getCopyDir()+"dictionary-mature.db")

	connm = sqlite.connect(db=getDictDir()+"all.db", mode=077,converters={},autocommit=0,encoding="iso8859-1")
	if not createSQL("all.sql",friendly,connm,t1_header)==1: raise TypeError
	if not cpfile(getDictDir()+"all.db",getCopyDir()+"dictionary.db")==1: raise TypeError

	"""f=os.popen("tar -cIf sql-scripts.tar.bz2 all.sql mature.sql idioms.sql words.sql cats.sql")
	f.readlines()
	os.popen("rm -f mature.sql")
	os.popen("rm -f all.sql")
	os.popen("rm -f words.sql")
	os.popen("rm -f idioms.sql")
	os.popen("rm -f cats.sql")"""

	for ifilez in ["mature.db","all.db", "idioms.db", "cats.db", "words.db"]:
		dodelete(getDictDir()+ifilez)

	return 1
   except:
	return 0


if __name__=="__main__":
	retval=createDatabases()
	if retval==1: print "Databases for Pythonol successfully built."
	else: print "ERROR while building Pythonol databases."

