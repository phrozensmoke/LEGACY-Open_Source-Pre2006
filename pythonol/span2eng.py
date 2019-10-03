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

import pydictionaries
from pyfilter import *

verb_exceptions={}
pydictionaries.loadTextDictionary(verb_exceptions,getDataDir()+"data_eng_verbs.txt")
verbs_loaded=len(verb_exceptions)>0

eng_pronouns={"Yo":"I","Tú":"You","Ella / Él / Usted":"She","Nosotros":"We","Vosotros":"You all","Ellos / Ustedes":"They"}

be_verbs={"Yopres":"am","Túpres":"are","Ella / Él / Ustedpres":"is","Nosotrospres":"are","Vosotrospres":"are","Ellos / Ustedespres":"are",   "Yopast":"was","Túpast":"were","Ella / Él / Ustedpast":"was","Nosotrospast":"were","Vosotrospast":"were","Ellos / Ustedespast":"were"}

go_verbs={"Yofpop":"am going to","Túfpop":"are going to","Ella / Él / Ustedfpop":"is going to","Nosotrosfpop":"are going to","Vosotrosfpop":"are going to","Ellos / Ustedesfpop":"are going to"}

reflex={"me":"myself","te":"yourself","se":"herself","nos":"ourselves","os":"yourselves"} # exception for 'themselves'

haber_dict={"he":"have","has":"have","ha":"has","hemos":"have","habéis":"have","han":"have","había":"had","habías":"had","había":"had","habíamos":"had","habíais":"had","habían":"had","hube":"had","hubiste":"had","hubo":"had","hubimos":"had","hubisteis":"had","hubieron":"had","habré":"will have","habrás":"will have","habrá":"will have","habremos":"will have","habréis":"will have","habrán":"will have","habría":"would have","habrías":"would have","habría":"would have","habríamos":"would have","habríais":"would have","habrían":"would have","hubiere":"will [possibly] have","hubieres":"will [possibly] have","hubiere":"will [possibly] have","hubiéremos":"will [possibly] have","hubiéreis":"will [possibly] have","hubieren":"will [possibly] have","hubiera":"[possibly] had","hubieras": "[possibly] had","hubiera": "[possibly] had","hubiéramos": "[possibly] had", "hubierais":"[possibly] had","hubieran":"[possibly] had","haya2":"[possibly]  have","hayas":"[possibly]  have", "haya":"[possibly]  has","hayamos":"[possibly]  have","hayáis":"[possibly]  have","hayan":"[possibly]  have"}

estar_dict={"estado":"been","estaré":"will be","estarás":"will be","estará":"will be","estaremos":"will be","estaréis":"will be","estarán":"will be"}

vowels=["a","e","i","o","u","y"]

def getVerb(verb_definition):
	vdef=str(verb_definition).lower().strip().replace("(","").replace(")","").replace("[","").replace("]","").replace(",","/").replace(";","/")
	if vdef.find("/")>-1:
		vlist=vdef.split("/")
		for ii in vlist: 
			if ii.strip(): 
				vdef=ii.strip()
				break
	if vdef.startswith("to "): vdef=vdef[2:len(vdef)].strip()
	vextra=""
	if vdef.find(" ")>-1:
		vextra=vdef[vdef.find(" "):len(vdef)].strip()
		vdef=vdef[0:vdef.find(" ")].strip()
	if len(vdef)>0: return [vdef,vextra]
	return []

def getVerbException(verb_word,verb_number):
	if verb_exceptions.has_key(str(verb_word).lower().strip()):
		try:
			return verb_exceptions[str(verb_word).lower().strip()].split(";")[verb_number]
		except:
			pass
	return ""

def getGoVerb(pronoun):
	pnoun=str(pronoun).strip()
	if go_verbs.has_key(pnoun+"fpop"): return go_verbs[pnoun+"fpop"]
	return "are going to"	

def getPresentVerb(verb_word,pronoun):
	pverb=str(verb_word).lower().strip()
	pnoun=str(pronoun).strip()
	if len(pverb)<2: return ""
	if pverb=="be":
		if be_verbs.has_key(pnoun+"pres"): return be_verbs[pnoun+"pres"]	
	if not pnoun=="Ella / Él / Usted": return pverb
	if pverb.endswith("y"): pverb=pverb[0:len(pverb)-1].strip()+"ie"
	if pverb.endswith("s"): return pverb+"es"
	if pverb=="have":
		if pnoun=="Ella / Él / Usted": return "has"
	return pverb+"s"

def getPresentParticiple(verb_word):
	pverb=str(verb_word).lower().strip()
	if len(pverb)<2: return ""
	ve=getVerbException(pverb,2)
	if len(ve)>0: return ve
	if not (pverb[len(pverb)-1] in vowels) and not pverb[len(pverb)-1]=="w":
		if pverb[len(pverb)-2] in ["a","i","o","u"] or pverb.endswith("t"):  
			if len(pverb)>2:
				if not pverb[len(pverb)-3] in vowels:  # dont add extra consonants to verbs like 'sneak','peek','seek','speak'
					pverb=pverb+pverb[len(pverb)-1]  # trip -> tripping
			else: pverb=pverb+pverb[len(pverb)-1]  # trip -> tripping
	if pverb.endswith("e"): 
		if (len(pverb)>2) and not (pverb.endswith("ee")):  pverb=pverb[0:len(pverb)-1].strip()
	return pverb+"ing"

def getPastVerb(verb_word,pronoun,verb_number=1):
	pverb=str(verb_word).lower().strip()
	pnoun=str(pronoun).strip()
	if pverb=="be":
		if be_verbs.has_key(pnoun+"past"): return be_verbs[pnoun+"past"]
	ve=getVerbException(pverb,verb_number)
	if len(ve)>0: return ve
	if len(pverb)<2: return ""
	if not (pverb[len(pverb)-1] in vowels) and not pverb[len(pverb)-1]=="w":
		if pverb[len(pverb)-2] in ["a","i","o","u"]  or pverb.endswith("t"):  
			if len(pverb)>2:
				if not pverb[len(pverb)-3] in vowels:  # dont add extra consonants to verbs like 'sneak','peek','seek','speak'
					pverb=pverb+pverb[len(pverb)-1]  # trip -> tripped
			else: pverb=pverb+pverb[len(pverb)-1]  # trip -> tripped
	if pverb.endswith("y"): pverb=pverb[0:len(pverb)-1].strip()+"ie"
	if not pverb.endswith("e"): return pverb+"ed"
	return pverb+"d"

def getPastParticiple(verb_word):
	return getPastVerb(verb_word,"bgedtde",0)

def translate(pronoun,verb_phrase,verb_definition,verb_scheme,subjunctive=0):	
	if not str(verb_definition).strip(): return ""
	if not str(verb_phrase).strip(): return ""

	# verb scheme:  1=present,2=present progressive,3=past,4=future,5=conditional,6=imperative,7=negative imperative
	#   					8=past participle, 9=present participle,10=past progressive,11=future progressive,12=future popular

	pnoun=str(pronoun).strip()
	myverb=getVerb(verb_definition)
	if not myverb: 
		return ""
	if verb_scheme==9: 
		if len(myverb[1])>0: return str(getPresentParticiple(myverb[0]))+" ("+myverb[1]+")"
		else: return str(getPresentParticiple(myverb[0]))
	reflex_phrase=""	
	verbplist=str(verb_phrase).lower().replace("  "," ").split(" ")
	if len(verbplist)==0: return ""
	trans_phrase=""

	imperative=0
	if verb_scheme==7 or verb_scheme==6: imperative=1
	if (pnoun=="Yo") and (imperative==1): return ""  # imperative for Yo does not exist
	# "Ella / Él / Usted":"She","Nosotros":"We","Vosotros":"You all","Ellos / Ustedes":"They"
	for ii in reflex.keys():
		if ii in verbplist:
			reflex_phrase=" ("+reflex[ii]+") "
			if pnoun=="Ellos / Ustedes": reflex_phrase=" (themselves) "
			del verbplist[verbplist.index(ii)]
	if imperative==1:
		if verb_scheme==7:
			if "no" in verbplist: 
				del verbplist[verbplist.index("no")]
			trans_phrase="don't "
		if not pnoun=="Tú":
			if pnoun=="Ella / Él / Usted": trans_phrase=trans_phrase+"let her "
			if pnoun=="Nosotros": trans_phrase=trans_phrase+"let us "
			if pnoun=="Vosotros": trans_phrase=trans_phrase+"let you all "
			if pnoun=="Ellos / Ustedes": trans_phrase=trans_phrase+"let them "
		else: trans_phrase=trans_phrase+"you "
		return str(trans_phrase+" "+myverb[0]+" "+myverb[1]+" "+reflex_phrase).replace("  "," ").replace("  "," ").strip().capitalize()+"."

	if imperative==0:
		if eng_pronouns.has_key(pnoun): trans_phrase=eng_pronouns[pnoun]+" "+trans_phrase
	if verb_scheme==5: 
		trans_phrase=trans_phrase+" would "+myverb[0] # conditional
		return str(trans_phrase+" "+myverb[1]+" "+reflex_phrase).replace("  "," ").replace("  "," ").strip().capitalize()+"."
	if subjunctive==1:
		if verb_scheme==1: trans_phrase=trans_phrase+" [might] be "
		if verb_scheme==3: trans_phrase=trans_phrase+" [might] have "
		if verb_scheme==4: trans_phrase=trans_phrase+" will [possibly] "
	if verb_scheme==1:
		if not subjunctive==1:
			return str(trans_phrase+" "+getPresentVerb(myverb[0],pnoun)+" "+myverb[1]+" "+reflex_phrase).replace("  "," ").replace("  "," ").strip().capitalize()+"."
		else: 	return str(trans_phrase+" "+getPresentParticiple(myverb[0])+" "+myverb[1]+" "+reflex_phrase+" now ").replace("  "," ").replace("  "," ").strip().capitalize()+"."

	if (verb_scheme==4) and (subjunctive==0): trans_phrase=trans_phrase+" will "
	if verb_scheme==4:
		return str(trans_phrase+" "+myverb[0]+" "+myverb[1]+" "+reflex_phrase).replace("  "," ").replace("  "," ").strip().capitalize()+"."
	if verb_scheme==3:
		if subjunctive==0:
			trans_phrase=trans_phrase+" "+getPastVerb(myverb[0],pnoun)
		else:
			trans_phrase=trans_phrase+" "+getPastParticiple(myverb[0])
		return str(trans_phrase+" "+myverb[1]+" "+reflex_phrase).replace("  "," ").replace("  "," ").strip().capitalize()+"."
	if verb_scheme==2:
		trans_phrase=trans_phrase+" "+getPresentVerb("be",pnoun)+" "+getPresentParticiple(myverb[0])+" "
		return str(trans_phrase+" "+myverb[1]+" "+reflex_phrase).replace("  "," ").replace("  "," ").strip().capitalize()+"."
	if verb_scheme==10:
		trans_phrase=trans_phrase+" "+getPastVerb("be",pnoun)+" "+getPresentParticiple(myverb[0])+" "
		return str(trans_phrase+" "+myverb[1]+" "+reflex_phrase).replace("  "," ").replace("  "," ").strip().capitalize()+"."

	if verb_scheme==12:
		#pp="I"
		#if eng_pronouns.has_key(pnoun): pp=eng_pronouns[pnoun]
		trans_phrase=trans_phrase+" "+getGoVerb(pnoun)+" "+myverb[0]+" "
		return str(trans_phrase+" "+myverb[1]+" "+reflex_phrase).replace("  "," ").replace("  "," ").strip().capitalize()+"."

	# now the only thing that should be left is past-participle,8 and future progressive-11
	if pnoun=="":  # just a bare past participle
		if len(myverb[1])>0: return  str(getPastParticiple(myverb[0]))+" ("+myverb[1]+")"
		else: return  str(getPastParticiple(myverb[0]))
	for hh in haber_dict.keys():
		if hh in verbplist:
			if verb_scheme==8:  
				if (hh=="haya") and (pnoun=="Yo"): trans_phrase=trans_phrase+" "+haber_dict["haya2"]+" "+getPastParticiple(myverb[0])
				else: trans_phrase=trans_phrase+" "+haber_dict[hh]+" "+getPastParticiple(myverb[0])
			else:   trans_phrase=trans_phrase+" "+haber_dict[hh]+" "  # future progressive
	if verb_scheme==11:
		for hh in estar_dict.keys():
			if hh in verbplist: trans_phrase=trans_phrase+" "+estar_dict[hh]+" "+getPresentParticiple(myverb[0])
	return str(trans_phrase+" "+myverb[1]+" "+reflex_phrase).replace("  "," ").replace("  "," ").strip().capitalize()+"."
