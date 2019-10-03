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
#import re,time
#if wine_friendly==0: from threading import Thread
#global threads_report
#threads_report=0

def loadDictionary(file_name):
   try:
	import pythonol_sqlite
	thesconn=pythonol_sqlite.Connection(db=getDataDir()+str(file_name))
	return thesconn
   except:
	return None

def loadTextDictionary(mydict,dict_file):
	try:
		f=open(dict_file)
		ff=filter(f.read())
		f.close()
		flist=ff.split("\n")
		for ii in flist:
			try:
				ipair=ii.strip().split(":")
				iword=ipair[0].strip()
				idef=ipair[1].strip()
				mydict[iword]=idef
			except:
				pass			
	except:
		pass
	return mydict


def getExactDefinition(dict_cursor,full_word,verbose=1,spanish=1,english=1):
	defines=getDefinitions2(dict_cursor,full_word,verbose,spanish,english,1)
	#print str(defines)
	return defines



def getDefinitions2(dict_cursor,full_word,verbose=1,spanish=1,english=1,exact=0): # spanish=span2eng, english=eng2span
	fw=str(unfilter(full_word)).strip().lower().replace("'","\"")
	defins=[]

	if spanish==1:
		query="SELECT english,spanish from definitions where spanish LIKE '%"+fw+"%' LIMIT 1004"
		if exact==1: query="SELECT english,spanish from definitions where spanish='"+fw+"' LIMIT 1"		
		getSQL(dict_cursor,query)
		for row in dict_cursor.fetchmany(1001):
			definition=row[0]
			explan=""
			if verbose==1: explan=row[1]
			defins.append([filter(definition),filter(explan)])
			if exact==1: break
	if (exact==1) and (len(defins)>0): 
		return defins
	if (english==1) and (len(defins)<1000):
		query="SELECT spanish,english from definitions where english LIKE '%"+fw+"%' LIMIT 1004"
		if exact==1: query="SELECT spanish,english from definitions where english='"+fw+"'  LIMIT 1"
		getSQL(dict_cursor,query)
		for row in dict_cursor.fetchmany(1001):
			definition=row[0]
			explan=""
			if verbose==1: explan=row[1]
			defins.append([filter(definition),filter(explan)])
			if exact==1: break

	if len(defins)>1000: 
		defins=defins[0:1000]
		defins.append(["",""])
		defins.append(["****  MAXIMUM NUMBER OF 1000 RESULTS EXCEEDED ****",""])
	return defins

