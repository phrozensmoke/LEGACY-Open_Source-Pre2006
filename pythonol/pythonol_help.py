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
import pyprint
import pythonol_festival,pythonol_help_anim


help_dict={
21:["General Help",1,1],
22:["Making Pythonol Child-Friendly",1,1],
23:["Conjugate Help",0,1,['conj1.png','conj2.png','conj3.png']],
24:["Dictionary Help",0,1, ['dict1.png','dict2.png','dict3.png','dict4.png']],
25:["Idiom Help",0,1,['id1.png','id2.png','id3.png','segway.png','id4.png','id5.png','id6.png']],
26:["Thesaurus Help",0,1, ['thes1.png','thes2.png','thes3.png','thes4.png']],
27:["Pronounce Help",0,1, ['pron1.png','pron2.png','pron3.png']],
266:["Verb Browser Help",0,1],
267:["Verb Families Help",0,1],
268:["Translate Help",0,1,['bab1.png','bab2.png','bab3.png']],

847:["Festival Extension Readme",1,1],
848:["Faq",1,1],
849:["Acknowledgements",1,1],
851:["Pythonol-LICENSE",1,1],
747:["Vocabulary Quiz",0,1,['vocab1.png','vocab2.png','vocab3.png','vocab4.png']],
757:["Translation Exercise",0,1,['trans1.png','trans2.png','trans3.png','trans4.png']],

32:["Cardinal Numbers",1,0],
33:["Ordinal Numbers",1,0],

35:["Pronouns",1,0],
36:["Subject Pronouns",1,0],
37:["Direct Object Pronouns",1,0],
38:["Indirect Object Pronouns",1,0],
39:["Prepositional Object Pronouns",1,0],
40:["Demonstrative Pronouns",1,0],
41:["Interrogative Pronouns",1,0],
42:["Reciprocal Reflexive Pronouns",1,0],
43:["Miscellaneous Pronouns",1,0],
201:["Pronoun Placement",1,0],

211:["Different Tenses Different Meanings",1,0],
212:["Estar Ser",1,0],
215:["Verbs Like Gustar",1,0],
213:["Saber Conocer",1,0],
214:["Tener",1,0],
270:["Acaber",1,0],
271:["Buscar",1,0],
272:["Caber",1,0],
273:["Dar",1,0],
274:["Gustar",1,0],
275:["Nacer",1,0],
276:["Pensar",1,0],

587:["computer_jargon",1,0],
588:["computer_jargon2",1,0],
589:["computer_jargon3",1,0],

44:["Prepositions",1,0],
45:["Por Para",1,0],

200:["Alphabets",0,0],
202:["Accents",1,0],
203:["Articles",1,0],
204:["Personal A",1,0],
205:["Comparisons",1,0],
206:["Passive Voice",1,0],
207:["Duration Of Time",1,0],

74:["data_comp1",1,0],
75:["data_comp2",1,0],
76:["data_comp3",1,0],
77:["data_comp4",1,0],
78:["data_comp5",1,0],
79:["data_comp6",1,0],
80:["data_comp7",1,0],
81:["data_comp8",1,0],
82:["data_comp9",1,0],
83:["data_comp10",1,0],
84:["data_comp11",1,0],
85:["data_comp12",1,0],
86:["data_comp13",1,0],
87:["data_comp14",1,0],
88:["data_comp15",1,0],
89:["data_comp16",1,0],
90:["data_comp17",1,0],
91:["data_comp18",1,0],
92:["data_comp19",1,0],
93:["data_comp20",1,0],
94:["data_comp21",1,0],
95:["data_comp22",1,0],
96:["data_comp23",1,0],

239:["Subjunctive",1,0],
216:["Present Tense",1,0],
217:["Present Progressive Tense",1,0],
218:["Present Subjunctive Tense",1,0],
219:["Present Perfect Tense",1,0],
220:["Present Perfect Subjunctive Tense",1,0],
221:["Preterit Tense",1,0],
222:["Imperfect Tense",1,0],
223:["Imperfect Progressive Tense",1,0],
224:["Past Subjunctive Tense",1,0],
225:["Past Perfect Tense",1,0],
226:["Past Perfect Subjunctive Tense",1,0],
227:["Preterit Perfect Tense",1,0],
228:["Future Tense",1,0],
229:["Future Tense",1,0],
230:["Future All Tense",1,0],
231:["Future Subjunctive Tense",1,0],
232:["Future All Tense",1,0],
233:["Future All Tense",1,0],
234:["Future Perfect Subjunctive Tense",1,0],
235:["Conditional Tense",1,0],
236:["Conditional Perfect Tense",1,0],
237:["Imperative Tense",1,0],
238:["Negative Imperative Tense",1,0],
277:["Superlatives",1,0],
278:["Spanish Questions",1,0],
279:["Linked Verbs",1,0],
280:["Days Of The Week",1,0],
649:["Useful Verb Constructions",1,0],
517:["Weather",1,0]
}

help_topics={}
for ht in help_dict.keys():
	if not help_dict[ht][0].startswith("data_comp"): help_topics[filter(help_dict[ht][0])]=ht
#########
class pyhelpui :
    def __init__ (self,wide=0,noedit=0,save_button=0,save_method=None,anim=[],short_block=0) :
	icehelp = Window (WINDOW_TOPLEVEL)
	set_window_icon(icehelp,STOCK_DIALOG_INFO,1)
	icehelp.set_wmclass(HELP_WMC,HELP_WMN)
        icehelp.realize()
	icehelp.set_title ("")
	# icehelp.set_policy (1, 1, 0)
	icehelp.set_position (WIN_POS_CENTER)
        icehelp.set_default_size(335,375)
	self.savemeth=save_method
	if wide==1: icehelp.set_default_size(640,375)
        icehelp.connect("destroy",self.hideh)
	self.icehelp = icehelp
	if len(anim)>0: 
		icehelp.set_default_size(495,375)
		animbox=HBox(0,0)
		animbox.set_spacing(0)
		animhelp=pythonol_help_anim.HelpAnimation(anim,2700, getIconButton)
		animhelp.set_border_width(9)
	vbox1 = VBox (0, 0)
	vbox1.set_border_width (9)
	scrolledwindow1 = ScrolledWindow ()
	self.scrolledwindow1 = scrolledwindow1
	helptext =TextView ()
	helptext.set_double_buffered(1)
	self.helptext=helptext
	helptext.set_editable(1)
        helptext.set_wrap_mode(WRAP_WORD)
	scrolledwindow1.add (helptext)	
	if noedit==1: 
		helptext.set_editable(0)
		vbox1.pack_start (scrolledwindow1, 1, 1, 0)
	else:  # add the Festival component on the left side
		scrolledwindowbox=HBox(0,0)
		scrolledwindowbox.pack_start(pythonol_festival.FestivalBar(icehelp,helptext,2,short_block) ,0,0,0)
		scrolledwindowbox.pack_start(scrolledwindow1, 1, 1, 0)
		vbox1.pack_start (scrolledwindowbox, 1, 1, 0)		
	if save_button==1:
		sbutt =getIconButton (icehelp,STOCK_SAVE,"Save To File",1) 
		Tooltips().set_tip(sbutt,"Save to file.\nGuardar.")
        	sbutt.connect("clicked",self.savet)
		vbox1.pack_start (sbutt, 0, 0,3)
	bbox=HBox(0,0)
	bbox.set_spacing(10)
	pbutt =getIconButton (icehelp,STOCK_PRINT," Print...",1) 
	TIPS.set_tip(pbutt,"Print.\nMandarlo a la impresora.")
        pbutt.connect("clicked",self.printTextArea)
	bbox.pack_start (pbutt, 1, 1, 0)
	closebutt =getIconButton (icehelp,STOCK_CANCEL,"Close",1) 
	TIPS.set_tip(closebutt,"Close this window.\nCerrar esta ventana.")
        closebutt.connect("clicked",self.hideh)
	bbox.pack_start (closebutt, 1, 1, 0)
	vbox1.pack_start (bbox, 0, 0, 8)
	if len(anim)>0: 
		animbox.pack_start(vbox1,1,1,0)
		if wine_friendly==0: animbox.pack_start(animhelp,0,0,0)
		animbox.show_all()
		icehelp.add(animbox)		
	else: icehelp.add (vbox1)

    def savet(self,*args):
	if self.savemeth: self.savemeth(self.helptext)

    def setHelpText(self, helpstuff):
	text_buffer_set_text(self.helptext,helpstuff)

    def getText(self,text_widget) :
	return text_buffer_get_text(text_widget)

    def hideh(self,*args):      
	self.icehelp.hide()
	self.icehelp.destroy()
	self.icehelp.unmap()
	return TRUE

    def printTextArea(self,*args) :
	if use_filter==1:  # We are running Windows, no printing avail.
		showHelp("SORRY: Printing is not available in the Windows version of Pythoñol.\nPrinting is only supported in the LINUX / UNIX version of Pythoñol.\nKeep in mind that Pythoñol is a LINUX application ported to Windows.\n\nHowever, you may click 'Save To File' in whatever section you are viewing.\nThen, open the saved file and print from another program, like Notepad.\nThere are no immediate plans to support printing under Windows at this time.")
		return
	ss=self.getText(self.helptext).strip()
	if not len(ss)>0:
		return
	tempfile=pyprint.makeTempFile(ss)
	if not tempfile==None:
		printw=pyprint.pyprintwin()
		printw.setFile(tempfile)
	else:  msg_err("Print Failure", "ERROR: Printing failed.\nCould not create a temporary file in the /tmp/ directory for printing.")

#########
class helpbrowser :
    def __init__ (self) :
	browsewin = Window (WINDOW_TOPLEVEL)
	set_window_icon(browsewin,STOCK_DIALOG_INFO,1)
	browsewin.set_wmclass(HELP_WMC,HELP_WMN)
        browsewin.realize()
	browsewin.set_title (to_utf8(filter("Pythoñol Help Topics")))
	browsewin.set_position (WIN_POS_CENTER)
        #browsewin.set_size_request(300,375)
	self.browsewin=browsewin
	vbox1=VBox(0,0)
	vbox1.set_spacing(10)
	vbox1.pack_start(Label(" Select a Help topic below, then click 'View Topic':  "),0,0,6)
	bcombo=Combo()
	bcombo_entry=bcombo.entry
	self.bcombo_entry=bcombo_entry
	bcombo_entry.set_editable(0)
	hlist=help_topics.keys()
	hlist.sort()
	bcombo.set_popdown_strings(hlist)
	vbox1.pack_start(bcombo,1,1,0)
	hbox=HBox(0,0)
	hbox.set_spacing(10)
	cancelbutt =getIconButton (browsewin,STOCK_CANCEL,"Close",1) 
	TIPS.set_tip(cancelbutt,to_utf8("Close this window.\nCerrar esta ventana."))
	viewbutt=getIconButton (browsewin,STOCK_ZOOM_IN,"View Topic",1)
	TIPS.set_tip(viewbutt,to_utf8("View the selected help topic.\nLeer el asunto seleccionado."))
	cancelbutt.connect("clicked",self.hideh)
	viewbutt.connect("clicked",self.viewTopic)
	hbox.pack_start(viewbutt,1,1,0)
	hbox.set_border_width(5)
	hbox.pack_start(cancelbutt,1,1,0)
	vbox1.pack_start(hbox,1,1,0)
	vbox1.set_border_width(6)
	browsewin.add (vbox1)
	browsewin.connect("destroy",self.hideh)
	browsewin.show_all()

    def viewTopic(self,*args):   
	if len(self.bcombo_entry.get_text())==0: return
	if help_topics.has_key(self.bcombo_entry.get_text()):
		openHelp(help_topics[self.bcombo_entry.get_text()])

    def hideh(self,*args):      
	self.browsewin.hide()
	self.browsewin.destroy()
	self.browsewin.unmap()
	return TRUE
#########
def openHelp(help_num,read_comp=0):
	if help_dict.has_key(help_num):
		filename=help_dict[help_num][0]
		wide=help_dict[help_num][1]
		edit=help_dict[help_num][2]
		anim=[]
		if len(help_dict[help_num])>3: anim=help_dict[help_num][3] #animations available, use them

		if read_comp==1:  showHelpFile(getDataDir()+filename+".txt","Pythoñol: "+filename,wide,edit)
		else: showHelpFile(getDocDir()+filename+".txt","Pythoñol: "+filename,wide,edit,anim)
	else: showHelp("[PHERROR]Error:  Help topic #"+str(help_num)+" not found.")

def showHelpFile(helpfile,wintitle="Pythoñol",wide=0,noedit=0,anim=[]):
	try:
		f=open(str(helpfile))
		helpt=f.read()
		f.close()
	except:
		helpt="[PHERROR]ERROR: Could not load the data or help file named '"+str(helpfile)+"'."
	showHelp(helpt,wintitle,wide,noedit,anim)

myhelp_tags=["[PHERROR]","[PHTITLE]","[PHSECTION]","[PH-HI]"]

def showHelp(helptext,wintitle="Pythoñol",wide=0,noedit=0,anim=[]):
	is_help_text=0
	ich=pyhelpui(wide,noedit,0,None,anim)
	ich.helptext.freeze_notify()
	ich.helptext.get_buffer().freeze_notify()
	helptext=helptext+"\n"
	for zz in myhelp_tags:
		if helptext.find(zz)>-1: 
			is_help_text=1
			break
	if is_help_text==1: 
		ich.setHelpText("")
		renderHelp(ich.helptext,helptext.split("\n"))
	else: 
		ich.setHelpText(helptext)
	ich.helptext.thaw_notify()
	ich.helptext.get_buffer().thaw_notify()
	ich.icehelp.set_title(to_utf8(filter(wintitle)))
	ich.icehelp.show_all()

def getIconButton (iwin,picon,lab="?",both=0) :  
	return getPixButton(iwin,picon,lab,both)


##############################
# This is code for a Help rendering system
# borrowed from my IceWMCP project 
##############################

COL_BLUE=GDK.rgb_get_colormap().alloc_color('SkyBlue3')
COL_PURPLE=GDK.rgb_get_colormap().alloc_color('DarkOrchid4')
if use_filter==1: COL_PURPLE=GDK.rgb_get_colormap().alloc_color('DarkOrchid3')
COL_GRAY=GDK.rgb_get_colormap().alloc_color('RoyalBlue3')
COL_BLACK=GDK.rgb_get_colormap().alloc_color('black')
COL_RED=GDK.rgb_get_colormap().alloc_color('IndianRed3')

HELP_FONT1="Helvetica 14"
if use_filter==1: HELP_FONT1="Arial 14"

HELP_FONT2="Arial 12"

HELP_FONT3="Helvetica Bold 18"
if use_filter==1: HELP_FONT3="Arial Bold 18"

APP_HELP_STRR="PYTHOÑOL"
global HRENDER_TAG
HRENDER_TAG=0

def get_renderable_help_tab(mybuffer, myfont,mycolor,mylangcode):	
	global HRENDER_TAG
	HRENDER_TAG=HRENDER_TAG+1
	tagname="hrender-tag"+str(HRENDER_TAG)
	texttag=mybuffer.create_tag(tagname)
	texttag.set_property("font",myfont)
	texttag.set_property("foreground-gdk",mycolor)
	texttag.set_property("language",mylangcode)
	return tagname

def help_insert(textbuf, mytag,mytext):
	tags=[mytag]
	textbuf.insert_with_tags_by_name(textbuf.get_end_iter(), to_utf8(mytext), *tags)

def renderHelp(texta,mesg):
	textbuf=texta.get_buffer()
	for ii in mesg:
		mline=ii.replace("\r","").replace("\x94","")

		if mline.find("[PHTITLE]")>-1:
			help_insert(textbuf, get_renderable_help_tab(textbuf,HELP_FONT3,COL_BLUE,LANGCODE),APP_HELP_STRR+":  "+mline.replace("[PHTITLE]","").strip()+"\n\n")
			#textbuf.insert(HELP_FONT3,COL_BLUE,APP_HELP_STRR+":  "+mline.replace("[PHTITLE]","").strip()+"\n\n") 
			continue
		elif mline.find("[PHSECTION]")>-1:
			help_insert(textbuf, get_renderable_help_tab(textbuf,HELP_FONT1,COL_PURPLE,LANGCODE),"\n"+mline.replace("[PHSECTION]","").strip()+"\n")
			continue
		elif mline.find("[PHERROR]")>-1:
			help_insert(textbuf, get_renderable_help_tab(textbuf,HELP_FONT1,COL_RED,LANGCODE),mline.replace("[PHERROR]","").strip()+"\n")
			continue
		elif mline.find("[PH-HI]")>-1:
			mlist=mline.split("[PH-HI]")
			for yy in mlist:
				if yy.find("[/PH-HI]")>-1: 
					help_insert(textbuf, get_renderable_help_tab(textbuf,HELP_FONT2,COL_GRAY,LANGCODE),yy[0:yy.find("[/PH-HI]")])
					help_insert(textbuf, get_renderable_help_tab(textbuf,HELP_FONT2,COL_BLACK,LANGCODE),yy[yy.find("[/PH-HI]")+8:len(yy)])		
				else:  
					help_insert(textbuf, get_renderable_help_tab(textbuf,HELP_FONT2,COL_BLACK,LANGCODE),yy)
			help_insert(textbuf, get_renderable_help_tab(textbuf,HELP_FONT2,COL_BLACK,LANGCODE),"\n")
		else:
			help_insert(textbuf, get_renderable_help_tab(textbuf,HELP_FONT2,COL_BLACK,LANGCODE),mline+"\n")

##############################
# End code for a Help rendering system
##############################



if __name__== "__main__" :  # for testing
	openHelp(23,0)
	helpbrowser()
	mainloop()
