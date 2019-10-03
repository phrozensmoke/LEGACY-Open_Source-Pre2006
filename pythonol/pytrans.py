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
import symbolbar,pybabelfish,pythonol_help,pythonol_festival
if wine_friendly==0: import random

class transwin:
    def __init__(self,getIconButton=None) :
	self.pversion=PYTHONOL_VERSION
	self.beg_list=[]
	self.mid_list=[]
	self.adv_list=[]
	transwin=Window(WINDOW_TOPLEVEL)
	transwin.set_title(to_utf8(filter('Pythoñol: Translation Exercise')))
	transwin.set_position(WIN_POS_CENTER)
	transwin.set_wmclass(WINDOW_WMC,WINDOW_WMN)
	set_window_icon(transwin,pythonol_icon)
	transwin.realize()
	self.transwin=transwin
	vbox1=VBox(0,3)
	vbox1.set_border_width(5)
	titlelab=getImage(getPixDir()+"title.gif",to_utf8(filter('Pythoñol ')+self.pversion))
	vbox1.pack_start(titlelab,0,0,0)
	label2=Label('Translation Exercise')
	vbox1.pack_start(label2,0,0,0)
	frame1=Frame(None)
	vbox2=VBox(0,0)
	vbox2.set_border_width(4)
	label7=Label('Sentence / Phrase To Translate:')
	label7.set_alignment(0,0.5)
	vbox2.pack_start(label7,0,0,0)
	questiontext=Entry()
	questiontext.set_editable(0)
	questiontext.set_size_request(350,-1)
	self.questiontext=questiontext
	vbox2.pack_start(questiontext,0,0,0)
	label8=Label('  ')
	vbox2.pack_start(label8,0,0,0)
	label9=Label('Type the best translation you can (in Spanish):')
	label9.set_alignment(0,0.5)
	vbox2.pack_start(label9,0,0,0)
	entry4=Entry()
	self.entry4=entry4
	vbox2.pack_start(entry4,0,0,0)
	symbarr=symbolbar.symbolbar(entry4,transwin)
	vbox2.pack_start(symbarr,0,0,0)
	hseparator2=HSeparator()
	vbox2.pack_start(hseparator2,0,0,0)
	label11=Label('  ')
	label11.set_size_request(-1,15)
	vbox2.pack_start(label11,0,0,0)
	label12=Label("The Answer  (from "+pybabelfish.get_service_name(pybabelfish.search_engine)+"):")
	label12.set_alignment(0,0.5)
	self.label12=label12
	vbox2.pack_start(label12,0,0,0)
	scrolledwindow1=ScrolledWindow()
	scrolledwindow1.set_size_request(-1,80)
	answertext=TextView()
	answertext.set_editable(1)
	self.answertext=answertext
	scrolledwindow1.add(answertext)
	self.answertext.set_wrap_mode(WRAP_WORD)
	scrolledwindow1box=HBox(0,0)
	scrolledwindow1box.pack_start(pythonol_festival.FestivalBar(transwin,answertext,2,0) ,0,0,1)
	scrolledwindow1box.pack_start(scrolledwindow1,1,1,0)
	vbox2.pack_start(scrolledwindow1box,1,1,0)

	label13=Label('Warning: Not all translations from the web are 100% accurate.')
	vbox2.pack_start(label13,0,0,3)
	frame1.add(vbox2)
	vbox1.pack_start(frame1,1,1,9)
	hbox1=HBox(0,6)
	hbox1.set_border_width(2)
	label3=Label('Select Skill Level:')
	hbox1.pack_start(label3,0,0,0)
	skillcombo=Combo()
	skillcombo.set_popdown_strings(['Short sentences/phrases','Medium sentences/phrases','Long sentences/phrases'])
	self.skillcombo=skillcombo
	skillentry=skillcombo.entry
	skillentry.set_text('Short sentences/phrases')
	skillentry.set_editable(0)
	self.skillentry=skillentry
	hbox1.pack_start(skillcombo,1,1,0)
	vbox1.pack_start(hbox1,0,0,0)
	hbox2=HBox(0,5)
	hbox2.set_border_width(4)
	closebutt=getIconButton (transwin,STOCK_CANCEL,'Close',1)
	closebutt.connect("clicked",self.doClose)
	hbox2.pack_start(closebutt,0,0,0)
	hbox2.pack_start(Label('  '),1,1,0)
	hbutt=getIconButton (transwin,STOCK_DIALOG_QUESTION,'Help',1)
	hbutt.connect("clicked",self.openHelp)
	hbox2.pack_start(hbutt,0,0,0)
	hbox2.pack_start(Label('            '),1,1,0)
	nextbutt=getIconButton (transwin,STOCK_GO_FORWARD,'Next Sentence',1)
	nextbutt.connect("clicked",self.makeQuiz)
	hbox2.pack_start(nextbutt,0,0,0)
	answerbutt=getIconButton (transwin,STOCK_CONVERT,'Show The Answer!',1)
	answerbutt.connect("clicked",self.showAnswer)
	hbox2.pack_start(answerbutt,0,0,0)
	vbox1.pack_start(hbox2,0,0,0)
	label14=Label('IMPORTANT: This exercise requires a working internet connection.')
	vbox1.pack_start(label14,0,0,0)
	transwin.add(vbox1)
	transwin.connect("destroy",self.doClose)
	transwin.show_all()
	self.loadData()
	self.makeQuiz()

    def openHelp(self,*args):
	pythonol_help.openHelp(757)

    def showAnswer(self,*args):
	if len(self.questiontext.get_text().strip())==0: return
	self.setText(self.answertext,filter(pybabelfish.doTranslate(self.questiontext.get_text().strip())))
	self.label12.set_text("The Answer  (from "+pybabelfish.get_service_name(pybabelfish.search_engine)+"):")

    def clearQuiz(self):
	self.questiontext.set_text("")
	self.entry4.set_text("")
	self.setText(self.answertext,"")

    def makeQuiz(self,*args):
	self.clearQuiz()
	l=self.adv_list
	if self.skillentry.get_text().lower().startswith("short"): l=self.beg_list
	if self.skillentry.get_text().lower().startswith("medium"): l=self.mid_list
	if len(l)==0: 
		self.questiontext.set_text(to_utf8("Sorry! There is no quiz data available for the selected skill level."))
		return
	self.questiontext.set_text(to_utf8(l[random.randrange(len(l)-1)]))

    def loadData(self):
	warns=[]
	j=self.loadFile(getDataDir()+"trans_beg.txt",self.beg_list)
	if not j==1: warns.append(getDataDir()+"datafiles/trans_beg.txt")
	j=self.loadFile(getDataDir()+"trans_mid.txt",self.mid_list)
	if not j==1: warns.append(getDataDir()+"trans_mid.txt")
	j=self.loadFile(getDataDir()+"trans_adv.txt",self.adv_list)
	if not j==1: warns.append(getDataDir()+"trans_adv.txt")
	if len(warns)>0: self.showMessage("There was an error loading the following datafile(s):\n\n"+str(warns)+"\n\nWithout all of the necessary files,\nthe translation exercise may not function properly!\nPlease restore the missing file or files,then re-run\nthis translation exercise.")

    def loadFile(self,myfile,mylist):
	try:
		f=open(myfile)
		ff=filter(f.read())
		f.close()
		flist=ff.split("\n")
		for ii in flist:
			fword=ii.replace("\n"," ").strip()
			if len(fword)>5:
				mylist.append(fword)
		return 1
	except:
		return 0

    def setText(self,temptext,textdata):
	text_buffer_set_text(temptext, textdata)

    def showMessage(self,message_text):
	return msg_warn(to_utf8(filter("Pythoñol: Translations ")+self.pversion), to_utf8(str(message_text)))	


    def doClose(self,*args):
	self.transwin.hide()
	self.transwin.destroy()
	self.transwin.unmap()
