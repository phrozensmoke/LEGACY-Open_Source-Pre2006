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
from irregular_verbs import *
import pydictionaries

verb_endings=['ir','ar','er','irse','arse','erse','írse','ír','ér','érse','ár','árse']

ir_endings={"1pre":"o","2pre":"es","3pre":"e","4pre":"imos","5pre":"ís","6pre":"en","1pas":"í","2pas":"iste","3pas":"ió","4pas":"imos","5pas":"isteis","6pas":"ieron","1fut":"iré","2fut":"irás","3fut":"irá","4fut":"iremos","5fut":"iréis","6fut":"irán","1cop":"ía","2cop":"ías","3cop":"ía","4cop":"íamos","5cop":"íais","6cop":"ían","1pos":"iría","2pos":"irías","3pos":"iría","4pos":"iríamos","5pos":"iríais","6pos":"irían","1pres":"a","2pres":"as","3pres":"a","4pres":"amos","5pres":"áis","6pres":"an","1pass":"iera","2pass":"ieras","3pass":"iera","4pass":"iéramos","5pass":"ierais","6pass":"ieran","1passb":"iese","2passb":"ieses","3passb":"iese","4passb":"iésemos","5passb":"ieseis","6passb":"iesen","1futs":"iere","2futs":"ieres","3futs":"iere","4futs":"iéremos","5futs":"iereis","6futs":"ieren","1imp":"","2imp":"e","3imp":"a","4imp":"amos","5imp":"id","6imp":"an","gerundio":"iendo","participio":"ido"}

er_endings={"1pre":"o","2pre":"es","3pre":"e","4pre":"emos","5pre":"éis","6pre":"en","1pas":"í","2pas":"iste","3pas":"ió","4pas":"imos","5pas":"isteis","6pas":"ieron","1fut":"eré","2fut":"erás","3fut":"erá","4fut":"eremos","5fut":"eréis","6fut":"erán","1cop":"ía","2cop":"ías","3cop":"ía","4cop":"íamos","5cop":"íais","6cop":"ían","1pos":"ería","2pos":"erías","3pos":"ería","4pos":"eríamos","5pos":"eríais","6pos":"erían","1pres":"a","2pres":"as","3pres":"a","4pres":"amos","5pres":"áis","6pres":"an","1pass":"iera","2pass":"ieras","3pass":"iera","4pass":"iéramos","5pass":"ierais","6pass":"ieran","1passb":"iese","2passb":"ieses","3passb":"iese","4passb":"iésemos","5passb":"ieseis","6passb":"iesen","1futs":"iere","2futs":"ieres","3futs":"iere","4futs":"iéremos","5futs":"iereis","6futs":"ieren","1imp":"","2imp":"e","3imp":"a","4imp":"amos","5imp":"ed","6imp":"an","gerundio":"iendo","participio":"ido"}


ar_endings={"1pre":"o","2pre":"as","3pre":"a","4pre":"amos","5pre":"áis","6pre":"an","1pas":"é","2pas":"aste","3pas":"ó","4pas":"amos","5pas":"steis","6pas":"aron","1fut":"aré","2fut":"arás","3fut":"ará","4fut":"aremos","5fut":"aréis","6fut":"arán","1cop":"aba","2cop":"abas","3cop":"aba","4cop":"ábamos","5cop":"abais","6cop":"aban","1pos":"aría","2pos":"arías","3pos":"aría","4pos":"aríamos","5pos":"aríais","6pos":"arían","1pres":"e","2pres":"es","3pres":"e","4pres":"emos","5pres":"éis","6pres":"en","1pass":"ara","2pass":"aras","3pass":"ara","4pass":"áramos","5pass":"arais","6pass":"aran","1passb":"ase","2passb":"ases","3passb":"ase","4passb":"ásemos","5passb":"aseis","6passb":"asen","1futs":"are","2futs":"ares","3futs":"are","4futs":"áremos","5futs":"areis","6futs":"aren","1imp":"","2imp":"a","3imp":"e","4imp":"emos","5imp":"ad","6imp":"en","gerundio":"ando","participio":"ado"}


zar_endings={"1pas":"cé","1pres":"ce","2pres":"ces","3pres":"ce","4pres":"cemos","5pres":"céis","6pres":"cen","3imp":"ce","4imp":"cemos","6imp":"cen"}

car_endings={"1pas":"qué","1pres":"que","2pres":"ques","3pres":"que","4pres":"quemos","5pres":"quéis","6pres":"quen","3imp":"que","4imp":"quemos","6imp":"quen"}

gar_endings={"1pas":"ué","1pres":"ue","2pres":"ues","3pres":"ue","4pres":"uemos","5pres":"uéis","6pres":"uen","3imp":"ue","4imp":"uemos","6imp":"uen"}

def isVerb(full_word):
	for ii in verb_endings:
		if str(full_word).lower().strip().endswith(ii): return 1
	return 0

def getWordRoot(full_word):
	vv=full_word
	for ii in verb_endings:
		if vv.endswith(ii):
			vv=vv[0:vv.rfind(ii)].strip()
			return vv
	return vv

def getRawVerb(full_word):
	fw=str(full_word).lower().strip()
	if fw.endswith("se"): 
		fw=fw[0:fw.rfind("se")].strip()
	return fw

def isReflexive(full_word):
	return str(full_word).lower().strip().endswith("se")

def conjugate(full_word):
	if not isVerb(unfilter(full_word))==1: return {}
	fw=getRawVerb(unfilter(full_word))
	reflexive=isReflexive(unfilter(full_word))
	rootw=getWordRoot(fw)
	conj_dict={}
	if fw.endswith("ir") or fw.endswith("ír"):
		for ii in ir_endings.keys():
			conj_dict[ii]=rootw+ir_endings[ii]
	if fw.endswith("er") or fw.endswith("ér"):
		for ii in er_endings.keys():
			conj_dict[ii]=rootw+er_endings[ii]
	if fw.endswith("ar") or fw.endswith("ár"):
		for ii in ar_endings.keys():
			conj_dict[ii]=rootw+ar_endings[ii]
		if fw.endswith("zar"):
			rww=rootw
			if rww.lower().endswith("z"): rww=rww[0:len(rww)-1]
			for ii in zar_endings.keys():
				conj_dict[ii]=rww+zar_endings[ii]
		if fw.endswith("gar"):
			for ii in gar_endings.keys():
				conj_dict[ii]=rootw+gar_endings[ii]
		if fw.endswith("car"):
			rww=rootw
			if rww.lower().endswith("c"): rww=rww[0:len(rww)-1]
			for ii in car_endings.keys():
				conj_dict[ii]=rww+car_endings[ii]
	if irregular_verbs.has_key(fw):
		irregdict=irregular_verbs[fw]
		for ii in irregdict.keys():
			conj_dict[ii]=irregdict[ii]
	if reflexive==1:
		for ii in conj_dict.keys():
			if ii.startswith("1"): conj_dict[ii]="me "+conj_dict[ii]
			if ii.startswith("2"):
				if ii=="2imp": conj_dict[ii]=conj_dict[ii]+"te" #exception for imperative
				else: conj_dict[ii]="te "+conj_dict[ii]
			if ii.startswith("3"):
				if ii=="3imp": conj_dict[ii]=conj_dict[ii]+"se"
				else: conj_dict[ii]="se "+conj_dict[ii]
			if ii.startswith("4"):
				if ii=="4imp": conj_dict[ii]=conj_dict[ii]+"nos"
				else: conj_dict[ii]="nos "+conj_dict[ii]
			if ii.startswith("5"):
				if ii=="5imp": conj_dict[ii]=conj_dict[ii]+"os"
				else: conj_dict[ii]="os "+conj_dict[ii]
			if ii.startswith("6"):
				if ii=="6imp": conj_dict[ii]=conj_dict[ii]+"se"
				else: conj_dict[ii]="se "+conj_dict[ii]
	addSecondaryConjugations(conj_dict,reflexive,fw)
	conj_dict["1imp"]="[does not exist]"
	conj_dict["1impn"]="[does not exist]"
	if use_filter==1:
		f_dict={}
		for ii in conj_dict.keys():
			f_dict[ii]=filter(conj_dict[ii])
		return f_dict
	else: return conj_dict

def addSecondaryConjugations(c_dict,reflex,infinitive_fw):
	rrdict={"1":"","2":"","3":"","4":"","5":"","6":""}
	if reflex==1: rrdict={"1":"me ","2":"te ","3":"se ","4":"nos ","5":"os ","6":"se "}
	c_dict["1fpop"]=rrdict["1"]+"voy a "+infinitive_fw.strip()
	c_dict["2fpop"]=rrdict["2"]+"vas a "+infinitive_fw.strip()
	c_dict["3fpop"]=rrdict["3"]+"va a "+infinitive_fw.strip()
	c_dict["4fpop"]=rrdict["4"]+"vamos a "+infinitive_fw.strip()
	c_dict["5fpop"]=rrdict["5"]+"vais a "+infinitive_fw.strip()
	c_dict["6fpop"]=rrdict["6"]+"van a "+infinitive_fw.strip()
	if c_dict.has_key("gerundio"):
		c_dict["1ppro"]=rrdict["1"]+"estoy "+c_dict["gerundio"] #present progressive
		c_dict["2ppro"]=rrdict["2"]+"estás "+c_dict["gerundio"]
		c_dict["3ppro"]=rrdict["3"]+"está "+c_dict["gerundio"]
		c_dict["4ppro"]=rrdict["4"]+"estamos "+c_dict["gerundio"]
		c_dict["5ppro"]=rrdict["5"]+"estáis "+c_dict["gerundio"]
		c_dict["6ppro"]=rrdict["6"]+"están "+c_dict["gerundio"]
		c_dict["1fpro"]=rrdict["1"]+"estaré "+c_dict["gerundio"]  #Future Progressive
		c_dict["2fpro"]=rrdict["2"]+"estarás "+c_dict["gerundio"]  
		c_dict["3fpro"]=rrdict["3"]+"estará "+c_dict["gerundio"]  
		c_dict["4fpro"]=rrdict["4"]+"estaremos "+c_dict["gerundio"]  
		c_dict["5fpro"]=rrdict["5"]+"estaréis "+c_dict["gerundio"]  
		c_dict["6fpro"]=rrdict["6"]+"estarán "+c_dict["gerundio"]  
		c_dict["1fpp"]=rrdict["1"]+"habré estado "+c_dict["gerundio"]  #Future Perfect Progressive
		c_dict["2fpp"]=rrdict["2"]+"habrás estado "+c_dict["gerundio"]  
		c_dict["3fpp"]=rrdict["3"]+"habrá estado "+c_dict["gerundio"]  
		c_dict["4fpp"]=rrdict["4"]+"habremos estado "+c_dict["gerundio"]  
		c_dict["5fpp"]=rrdict["5"]+"habréis estado "+c_dict["gerundio"]  
		c_dict["6fpp"]=rrdict["6"]+"habrán estado "+c_dict["gerundio"]  
		c_dict["1ppp"]=rrdict["1"]+"he estado "+c_dict["gerundio"]  #Present Perfect Progressive
		c_dict["2ppp"]=rrdict["2"]+"has estado "+c_dict["gerundio"]  
		c_dict["3ppp"]=rrdict["3"]+"ha estado "+c_dict["gerundio"]  
		c_dict["4ppp"]=rrdict["4"]+"hemos estado "+c_dict["gerundio"]  
		c_dict["5ppp"]=rrdict["5"]+"habéis estado "+c_dict["gerundio"]  
		c_dict["6ppp"]=rrdict["6"]+"han estado "+c_dict["gerundio"]  
		c_dict["1cpp"]=rrdict["1"]+"habría estado "+c_dict["gerundio"]  #Conditional Perfect Progressive
		c_dict["2cpp"]=rrdict["2"]+"habrías estado "+c_dict["gerundio"]  
		c_dict["3cpp"]=rrdict["3"]+"habría estado "+c_dict["gerundio"]  
		c_dict["4cpp"]=rrdict["4"]+"habríamos estado "+c_dict["gerundio"]  
		c_dict["5cpp"]=rrdict["5"]+"habríais estado "+c_dict["gerundio"]  
		c_dict["6cpp"]=rrdict["6"]+"habrían estado "+c_dict["gerundio"]  
		c_dict["1ip"]=rrdict["1"]+"estaba "+c_dict["gerundio"]  #Imperfect Progressive
		c_dict["2ip"]=rrdict["2"]+"estabas "+c_dict["gerundio"]  
		c_dict["3ip"]=rrdict["3"]+"estaba "+c_dict["gerundio"]  
		c_dict["4ip"]=rrdict["4"]+"estábamos "+c_dict["gerundio"]  
		c_dict["5ip"]=rrdict["5"]+"estabais "+c_dict["gerundio"]  
		c_dict["6ip"]=rrdict["6"]+"estaban "+c_dict["gerundio"]  


	if c_dict.has_key("participio"):
		c_dict["1pp"]=rrdict["1"]+"he "+c_dict["participio"]  #present perfect
		c_dict["2pp"]=rrdict["2"]+"has "+c_dict["participio"]  
		c_dict["3pp"]=rrdict["3"]+"ha "+c_dict["participio"]  
		c_dict["4pp"]=rrdict["4"]+"hemos "+c_dict["participio"]  
		c_dict["5pp"]=rrdict["5"]+"habéis "+c_dict["participio"]  
		c_dict["6pp"]=rrdict["6"]+"han "+c_dict["participio"]  
		c_dict["1pasp"]=rrdict["1"]+"había "+c_dict["participio"]  #past perfect
		c_dict["2pasp"]=rrdict["2"]+"habías "+c_dict["participio"]  
		c_dict["3pasp"]=rrdict["3"]+"había "+c_dict["participio"]  
		c_dict["4pasp"]=rrdict["4"]+"habíamos "+c_dict["participio"]  
		c_dict["5pasp"]=rrdict["5"]+"habíais "+c_dict["participio"]  
		c_dict["6pasp"]=rrdict["6"]+"habían "+c_dict["participio"]  
		c_dict["1prep"]=rrdict["1"]+"hube "+c_dict["participio"]  #preterit perfect
		c_dict["2prep"]=rrdict["2"]+"hubiste "+c_dict["participio"]  
		c_dict["3prep"]=rrdict["3"]+"hubo "+c_dict["participio"]  
		c_dict["4prep"]=rrdict["4"]+"hubimos "+c_dict["participio"]  
		c_dict["5prep"]=rrdict["5"]+"hubisteis "+c_dict["participio"]  
		c_dict["6prep"]=rrdict["6"]+"hubieron "+c_dict["participio"]  
		c_dict["1futp"]=rrdict["1"]+"habré "+c_dict["participio"]  #future perfect
		c_dict["2futp"]=rrdict["2"]+"habrás "+c_dict["participio"]  
		c_dict["3futp"]=rrdict["3"]+"habrá "+c_dict["participio"]  
		c_dict["4futp"]=rrdict["4"]+"habremos "+c_dict["participio"]  
		c_dict["5futp"]=rrdict["5"]+"habréis "+c_dict["participio"]  
		c_dict["6futp"]=rrdict["6"]+"habrán "+c_dict["participio"]  
		c_dict["1conp"]=rrdict["1"]+"habría "+c_dict["participio"]  #conditional perfect
		c_dict["2conp"]=rrdict["2"]+"habrías "+c_dict["participio"]  
		c_dict["3conp"]=rrdict["3"]+"habría "+c_dict["participio"]  
		c_dict["4conp"]=rrdict["4"]+"habríamos "+c_dict["participio"]  
		c_dict["5conp"]=rrdict["5"]+"habríais "+c_dict["participio"]  
		c_dict["6conp"]=rrdict["6"]+"habrían "+c_dict["participio"]
		c_dict["1fps"]=rrdict["1"]+"hubiere "+c_dict["participio"]  #future perfect subjunctive
		c_dict["2fps"]=rrdict["2"]+"hubieres "+c_dict["participio"]  
		c_dict["3fps"]=rrdict["3"]+"hubiere "+c_dict["participio"]  
		c_dict["4fps"]=rrdict["4"]+"hubiéremos "+c_dict["participio"]  
		c_dict["5fps"]=rrdict["5"]+"hubiéreis "+c_dict["participio"]  
		c_dict["6fps"]=rrdict["6"]+"hubieren "+c_dict["participio"]  
		c_dict["1pastps"]=rrdict["1"]+"hubiera "+c_dict["participio"]  #present perfect subjunctive
		c_dict["2pastps"]=rrdict["2"]+"hubieras "+c_dict["participio"]  
		c_dict["3pastps"]=rrdict["3"]+"hubiera "+c_dict["participio"]  
		c_dict["4pastps"]=rrdict["4"]+"hubiéramos "+c_dict["participio"]  
		c_dict["5pastps"]=rrdict["5"]+"hubierais "+c_dict["participio"]  
		c_dict["6pastps"]=rrdict["6"]+"hubieran "+c_dict["participio"]  
		c_dict["1presps"]=rrdict["1"]+"haya "+c_dict["participio"]  #present perfect subjunctive
		c_dict["2presps"]=rrdict["2"]+"hayas "+c_dict["participio"]  
		c_dict["3presps"]=rrdict["3"]+"haya "+c_dict["participio"]  
		c_dict["4presps"]=rrdict["4"]+"hayamos "+c_dict["participio"]  
		c_dict["5presps"]=rrdict["5"]+"hayáis "+c_dict["participio"]  
		c_dict["6presps"]=rrdict["6"]+"hayan "+c_dict["participio"]  


	if c_dict.has_key("2pres"):	 c_dict["2impn"]="no "+c_dict["2pres"]  # negative imperatives
	if c_dict.has_key("3pres"):	 c_dict["3impn"]="no "+c_dict["3pres"]  
	if c_dict.has_key("4pres"):	 c_dict["4impn"]="no "+c_dict["4pres"]  
	if c_dict.has_key("5pres"):	 c_dict["5impn"]="no "+c_dict["5pres"]  
	if c_dict.has_key("6pres"):	 c_dict["6impn"]="no "+c_dict["6pres"]

def reverseConjugate(verb_tense,dictionaries):
	if isVerb(unfilter(verb_tense)): return [str(verb_tense).strip(),""]
	# first check to see if this is a known irregular verb
	vtense=str(verb_tense).lower().strip()
	for yy in irregular_verbs.keys():
		infinitive=yy	
		irregdict=irregular_verbs[yy]
		if unfilter(vtense) in irregdict.values():
			vdef=""
			for dd in dictionaries:
				deflist=pydictionaries.getExactDefinition(dd,infinitive,0,1,0)
				if len(deflist)==0: deflist=pydictionaries.getDefinitions2(dd,infinitive,0,1,1,0)
				if len(deflist)>0: 
					vdef=deflist[0][0]
					break
			return [infinitive,vdef]
	irv=1
	erv=2
	arv=3
	carv=4
	zarv=5
	garv=6
	verb_matches={}
	all_endings={1:ir_endings,2:er_endings,3:ar_endings,4:car_endings,5:zar_endings,6:gar_endings}
	for yy in all_endings.keys():
		for ii in all_endings[yy].keys():
			if vtense.endswith(all_endings[yy][ii]): 
				if verb_matches.has_key(yy):
					if  len(all_endings[yy][ii]) > len(all_endings[yy][verb_matches[yy]]) :verb_matches[yy]=ii
				else: verb_matches[yy]=ii
	if len(verb_matches)==0: return []
	poss_verbs=[]
	for yy in verb_matches.keys():
		rep_string=all_endings[yy][verb_matches[yy]]
		vending="ir"
		if yy==2: vending="er"
		if yy==3: vending="ar"
		if yy==4: vending="car"
		if yy==5: vending="zar"
		if yy==6: vending="gar"
		poss_verbs.append(vtense[0:vtense.rfind(rep_string)]+vending)
	verb_matches.clear()
	all_endings=[]
	vdef=""
	selected_verb=""
	#print "poss_verbs:   "+str(poss_verbs)
	# let's make sure the 'constructed' infinitive is a known spanish word
	for yy in poss_verbs:
		if not vtense in conjugate(yy).values(): continue
		if (len(vdef)>0) or (len(selected_verb)>0): break
		for dd in dictionaries:
			deflist=pydictionaries.getExactDefinition(dd,yy,0,1,0)
			if len(deflist)==0: deflist=pydictionaries.getDefinitions2(dd,yy,0,1,1,0)
			if len(deflist)>0: 
				vdef=deflist[0][0]
				selected_verb=yy
				break
	if (len(vdef)>0) and (len(selected_verb)>0): return [selected_verb,vdef]
	return []
	
