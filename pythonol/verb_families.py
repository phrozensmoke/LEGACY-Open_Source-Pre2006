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

import copy
from pyfilter import *
verb_families={}

def loadVF():
   try:
	f=open(getDataDir()+"verb_families.txt")
	ff=f.read()
	f.close()
	flist=ff.split("\n")
	for ii in flist:
		if ii.find(":")<0: continue
		cpair=ii.split(":")
		if len(cpair)>1:
			verb_families[cpair[0].strip()]=cpair[1].strip().split("|")
   except:
	pass

loadVF()
def getVerbFamily(full_word):
	fw=unfilter(str(full_word)).strip().lower()
	for ii in verb_families.keys():
		complist=verb_families[ii]
		if fw in complist: return copy.copy(complist)
	return []

if len(verb_families)==0: loadVF()
