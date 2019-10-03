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
import pythonol_help,pythonol_festival
if wine_friendly==0: import random


class quizwin :
    def __init__(self,cursor_list,getIconButton=None,getDBCount=None):
	self.pversion=PYTHONOL_VERSION
	self.quiz_data={}
	self.CURSORS=cursor_list
	self.getDatabaseCount=getDBCount
	quizwin=Window(WINDOW_TOPLEVEL)
	quizwin.set_wmclass(WINDOW_WMC,WINDOW_WMN)
	set_window_icon(quizwin,pythonol_icon)
	quizwin.set_title(to_utf8(filter('Pythoñol: Vocabulary Quiz')))
	quizwin.set_position(WIN_POS_CENTER)
	quizwin.realize()
	self.quizwin=quizwin
	vbox1=VBox(0,4)
	vbox1.set_border_width(2)
	titlelab=getImage(getPixDir()+"title.gif",to_utf8(filter('Pythoñol ')+self.pversion))
	vbox1.pack_start(titlelab,0,0,2)
	label2=Label('Vocabulary Quiz')
	vbox1.pack_start(label2,0,0,3)
	frame1=Frame(None)
	frame1.set_border_width(2)
	table1=Table(6,4,0)
	table1.set_row_spacings(2)
	table1.set_col_spacings(4)
	table1.set_border_width(5)
	label3=Label('Quiz Word/Phrase')
	label3.set_padding(0,6)
	self.label3=label3
	table1.attach(label3,0,2,0,1,(FILL),(0),0,0)
	label4=Label('Your Answer')
	table1.attach(label4,2,3,0,1,(FILL),(0),0,0)
	label5=Label(to_utf8(filter("Pythoñol's Answer")))
	table1.attach(label5,3,4,0,1,(FILL),(0),0,0)
	word1=Label('')
	word1.set_alignment(0,0.5)
	word1.set_size_request(200,-1)
	self.word1=word1
	table1.attach(word1,1,2,1,2,(FILL),(0),0,0)
	word2=Label('')
	word2.set_alignment(0,0.5)
	self.word2=word2
	table1.attach(word2,1,2,2,3,(FILL),(0),0,0)
	word3=Label('')
	word3.set_alignment(0,0.5)
	self.word3=word3
	table1.attach(word3,1,2,3,4,(FILL),(0),0,0)
	word4=Label('')
	word4.set_alignment(0,0.5)
	self.word4=word4
	table1.attach(word4,1,2,4,5,(FILL),(0),0,0)
	word5=Label('')
	word5.set_alignment(0,0.5)
	self.word5=word5
	table1.attach(word5,1,2,5,6,(FILL),(0),0,0)

	# Festival TTS support
	self.festival_widgets={}
	self.festival_widgets[self.word1]=pythonol_festival.FestivalBar(quizwin,None,1)
	self.festival_widgets[self.word2]=pythonol_festival.FestivalBar(quizwin,None,1)
	self.festival_widgets[self.word3]=pythonol_festival.FestivalBar(quizwin,None,1)
	self.festival_widgets[self.word4]=pythonol_festival.FestivalBar(quizwin,None,1)
	self.festival_widgets[self.word5]=pythonol_festival.FestivalBar(quizwin,None,1)
	table1.attach(self.festival_widgets[self.word1],0,1,1,2,(FILL),(0),0,0)
	table1.attach(self.festival_widgets[self.word2],0,1,2,3,(FILL),(0),0,0)
	table1.attach(self.festival_widgets[self.word3],0,1,3,4,(FILL),(0),0,0)
	table1.attach(self.festival_widgets[self.word4],0,1,4,5,(FILL),(0),0,0)
	table1.attach(self.festival_widgets[self.word5],0,1,5,6,(FILL),(0),0,0)
	for gg in self.festival_widgets.keys():
		self.festival_widgets[gg].setTextWidget(Entry())  #invisble text widgets for audio reading


	entry1=Entry()
	entry1.set_size_request(158,-1)
	self.entry1=entry1
	table1.attach(entry1,2,3,1,2,(EXPAND+FILL),(0),0,0)
	entry3=Entry()
	self.entry3=entry3
	table1.attach(entry3,2,3,2,3,(EXPAND+FILL),(0),0,0)
	entry4=Entry()
	self.entry4=entry4
	table1.attach(entry4,2,3,3,4,(EXPAND+FILL),(0),0,0)
	entry5=Entry()
	self.entry5=entry5
	table1.attach(entry5,2,3,4,5,(EXPAND+FILL),(0),0,0)
	entry6=Entry()
	self.entry6=entry6
	table1.attach(entry6,2,3,5,6,(EXPAND+FILL),(0),0,0)
	answer1=Entry()
	answer1.set_editable(0)
	answer1.set_size_request(158,-1)
	self.answer1=answer1
	table1.attach(answer1,3,4,1,2,(EXPAND+FILL),(0),0,0)
	answer2=Entry()
	answer2.set_editable(0)
	self.answer2=answer2
	table1.attach(answer2,3,4,2,3,(EXPAND+FILL),(0),0,0)
	answer3=Entry()
	answer3.set_editable(0)
	self.answer3=answer3
	table1.attach(answer3,3,4,3,4,(EXPAND+FILL),(0),0,0)
	answer4=Entry()
	answer4.set_editable(0)
	self.answer4=answer4
	table1.attach(answer4,3,4,4,5,(EXPAND+FILL),(0),0,0)
	answer5=Entry()
	answer5.set_editable(0)
	self.answer5=answer5
	table1.attach(answer5,3,4,5,6,(EXPAND+FILL),(0),0,0)
	frame1.add(table1)
	vbox1.pack_start(frame1,1,1,0)
	hbox1=HBox(1,7)
	hbox1.set_border_width(9)
	quizbutt=getIconButton (quizwin,STOCK_REFRESH,'Start A New Quiz',1)
	TIPS.set_tip(quizbutt, 'Start A New Quiz.\nCrear otra prueba.')
	quizbutt.connect("clicked",self.loadQuiz)
	self.quizbutt=quizbutt
	hbox1.pack_start(quizbutt,1,1,0)
	langcombo=Combo()
	langcombo.set_popdown_strings(['English-Spanish','Spanish-English'])
	self.langcombo=langcombo
	langentry=langcombo.entry
	langentry.set_text('Spanish-English')
	langentry.set_editable(0)
	self.langentry=langentry
	hbox1.pack_start(langcombo,1,1,0)
	answerbutt=getIconButton (quizwin,STOCK_CONVERT,'Show The Answers!',1)
	TIPS.set_tip(answerbutt, "Show the answers.")
	answerbutt.connect("clicked",self.showAnswers)
	self.answerbutt=answerbutt
	hbox1.pack_start(answerbutt,1,1,0)
	vbox1.pack_start(hbox1,0,0,0)
	hbox7=HBox(0,0)
	hbox7.set_spacing(4)
	closebutt=getIconButton (quizwin,STOCK_CANCEL,'Close',1) 
	closebutt.connect("clicked",self.doClose)
	TIPS.set_tip(closebutt, "Close.\nCerrar esta ventana.")
	closebutt.set_border_width(8)
	hbutt=getIconButton (quizwin,STOCK_DIALOG_QUESTION,'Help',1)
	hbutt.connect("clicked",self.openHelp)
	TIPS.set_tip(hbutt, "Help.\nAyuda.")
	hbutt.set_border_width(8)
	hbox7.pack_start(closebutt,1,1,0)
	hbox7.pack_start(hbutt,1,1,0)
	vbox1.pack_start(hbox7,0,0,0)
	quizwin.add(vbox1)
	quizwin.connect("destroy",self.doClose)
	quizwin.show_all()
	self.loadQuiz()

    def openHelp(self,*args):
	pythonol_help.openHelp(747)

    def clearQuiz(self,*args):
	self.entry1.set_text("")
	self.entry3.set_text("")
	self.entry4.set_text("")
	self.entry5.set_text("")
	self.entry6.set_text("")
	self.answer1.set_text("")
	self.answer2.set_text("")
	self.answer3.set_text("")
	self.answer4.set_text("")
	self.answer5.set_text("")
	self.word1.set_text("")
	self.word2.set_text("")
	self.word3.set_text("")
	self.word4.set_text("")
	self.word5.set_text("")

    def showAnswers(self,*args):
	for ii in self.quiz_data.keys():
		if ii=="1": self.answer1.set_text(to_utf8(self.quiz_data[ii][1]))
		if ii=="2": self.answer2.set_text(to_utf8(self.quiz_data[ii][1]))
		if ii=="3": self.answer3.set_text(to_utf8(self.quiz_data[ii][1]))
		if ii=="4": self.answer4.set_text(to_utf8(self.quiz_data[ii][1]))
		if ii=="5": self.answer5.set_text(to_utf8(self.quiz_data[ii][1]))

    def makeQuiz(self):
	qdict={}
	if self.langentry.get_text()=='English-Spanish': 
		query_str="SELECT english,spanish from definitions LIMIT "
	else: 
		query_str="SELECT spanish,english from definitions LIMIT "
	vcount=5
	dbs=self.CURSORS
	while len(qdict)<vcount:
		if len(dbs)==0: break
		choosedb=dbs[random.randrange(len(dbs))-1]
		counter=random.randrange(self.getDatabaseCount(choosedb)-4)
		#print str(counter)
		query=getSQL(choosedb,query_str+str(counter)+","+str(3))
		for dbrow in choosedb.fetchmany(3):
			pww=dbrow[0]
			pwwdef=dbrow[1]
			if pww.find("[")>0: pww=pww[0:pww.find("[")].strip()
			if pww.find("(")>0: pww=pww[0:pww.find("(")].strip()
			if pww.find("/")>0: pww=pww[0:pww.find("/")].strip()
			if (len(pww)>5) and (len(pww)<28):
				qdict[str(len(qdict)+1)]=[pww,filter(pwwdef)]
				if len(qdict)==vcount: break
			if len(qdict)==vcount: break
	return qdict

    def loadQuiz(self,*args):
	self.clearQuiz()
	d=self.makeQuiz()
	self.quiz_data=d
	self.word1.set_text(to_utf8(filter(d["1"][0])))
	self.festival_widgets[self.word1].TA.set_data("word", d["1"][0])
	self.word2.set_text(to_utf8(filter(d["2"][0])))
	self.festival_widgets[self.word2].TA.set_data("word", d["2"][0])
	self.word3.set_text(to_utf8(filter(d["3"][0])))
	self.festival_widgets[self.word3].TA.set_data("word", d["3"][0])
	self.word4.set_text(to_utf8(filter(d["4"][0])))
	self.festival_widgets[self.word4].TA.set_data("word", d["4"][0])
	self.word5.set_text(to_utf8(filter(d["5"][0])))	
	self.festival_widgets[self.word5].TA.set_data("word", d["5"][0])

    def doClose(self,*args):
	self.quizwin.destroy()
	self.quizwin.unmap()


if __name__== "__main__" :  # for testing
	quizwin(None)
	mainloop()

