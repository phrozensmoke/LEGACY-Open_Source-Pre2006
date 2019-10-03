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

from pyfilter import *

def loadThesaurus(file_name): # return a SQLite database connection
   try:
	import pythonol_sqlite
	thesconn=pythonol_sqlite.Connection(db=getDataDir()+str(file_name))
	return thesconn
   except:
	return None

def thesaurusLookup(full_word,cat_cursor):
	try:
		#print "look:  "+unfilter(full_word).lower().replace("'","\"").strip()
		resultslist=[]
		getSQL(cat_cursor,"SELECT searchwords from definitions where searchwords LIKE  '%"+unfilter(full_word.lower()).replace("'","\"").strip()+";%'  OR searchwords LIKE  '%;"+unfilter(full_word).lower().replace("'","\"").strip()+"%'   LIMIT 8")
		resdict={}
		for row in cat_cursor.fetchall():
			ngroup=str(row[0]).split(";")
			for ii in ngroup:
					resdict[filter(ii)]="1"
		resultslist=resdict.keys()
		resultslist.sort()
		# print str(resultslist)
		return resultslist
	except:
		return []

def getClosestMatches(full_word,cat_cursor):
	try:
		#print "close:  "+str(full_word)
		resultslist=[]
		getSQL(cat_cursor,"SELECT searchwords from definitions where searchwords LIKE  '"+unfilter(full_word).lower().replace("'","\"").strip()+"' LIMIT 20")
		resdict={}
		for row in cat_cursor.fetchall():
					resdict[filter(row[0])]="1"
		resultslist=resdict.keys()
		resultslist.sort()
		print str(resultslist)
		return resultslist
	except:
		return []

