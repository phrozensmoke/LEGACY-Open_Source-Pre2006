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
irregular_verbs={}

def loadIrreg():
   try:
	f=open(getDataDir()+"irregular_verbs.txt")
	ff=f.read()
	f.close()
	flist=ff.split("\n")
	for ii in flist:
		if ii.find(":")<0: continue
		cpair=ii.split(":")
		if len(cpair)>1:
			verb=cpair[0].strip()
			dlist=cpair[1].strip().split(",")
			ddict={}
			for gg in dlist:
				gpair=gg.split("|")
				if len(gpair)==2:
					ddict[gpair[0].strip()]=gpair[1].strip()
			if len(ddict)>0:
				irregular_verbs[verb]=ddict
   except:
	pass

if len(irregular_verbs)==0: loadIrreg()
