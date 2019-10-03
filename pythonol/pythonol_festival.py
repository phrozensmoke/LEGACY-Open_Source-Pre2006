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
from threading import Thread
import time
SUPPORT_CHECKED=0
IS_SUPPORTED=0
MISSING_FILES=[]
global stop_reading
stop_reading=0

def checkSupport():
	file_list=["festival","tts.spanish"]
	global MISSING_FILES
	for ii in file_list:
		try:
			if not os.path.exists(getFestivalDir()+ii): MISSING_FILES.append(os.path.abspath(getFestivalDir()+ii))
		except:
			return 0
		try:
			if not os.path.getsize(getFestivalDir()+ii) > 0 : return 0 
		except:
			return 0
	return 1

def isSupported():
	if use_filter==1: return 0  # No Festival support on Windows or wine
	global SUPPORT_CHECKED
	global IS_SUPPORTED
	if SUPPORT_CHECKED==1:
		return IS_SUPPORTED
	IS_SUPPORTED=checkSupport()
	SUPPORT_CHECKED=1
	return IS_SUPPORTED

def showNotSupported():
	miss_str=""
	global MISSING_FILES
	if len(MISSING_FILES)>0: 
		miss_str="Your Pythoñol installation is missing the following files needed for text-to-speech:\n"+"\n".join(MISSING_FILES)
	msg_err(to_utf8(filter("Pythoñol: Festival")),to_utf8(filter("Festival text-to-speech audio support is NOT ENABLED.\n\nWINDOWS USERS: Festival text-to-speech support is NOT SUPPORTED \non Windows systems at all! Sorry.\n\nLINUX / UNIX USERS: See http://pythonol.sourceforge.net for information\non getting and setting up Festival text-to-speech audio support \nfor your copy of Pythoñol.\nAlso, see 'Help'->'Festival Text-to-Speech Help...' \non the Pythoñol menu.\n\n")+miss_str))

def getTTSPid(exe_name="festival"):  # find the process id for TTS to control playback
	try:
		f=os.popen("pidof "+exe_name)
		ff=f.readlines()
		f.close()
		return ff[0].replace("\n","").replace("\r","").strip()
	except:
		return ""

def stopPlayback(*args):
	if not isSupported(): 
		return
	global stop_reading
	stop_reading=1
	tts_pid=getTTSPid()
	if not len(tts_pid)>0: tts_pid=getTTSPid("./festival")
	if len(tts_pid)>0:
		try:
			os.popen("kill -9 "+tts_pid+" &> /dev/null &")
			os.popen("kill -9 "+tts_pid+" &> /dev/null &")
			os.popen("kill -9 "+tts_pid+" &> /dev/null &")
			os.popen("kill -9 "+tts_pid+" &> /dev/null &")
			time.sleep(0.5) # take a short half second rest so we dont create sound card problems
		except:
			pass




def doLangFilter(some_str): # filters to make the string 'echo'-acceptable and sound more human-like
	some_str=some_str.replace("\"","").replace("'","").replace("\t"," . ").replace("\r",". ").replace("\n",".").replace("\\"," . ").replace("EXACT MATCHES:","").replace("INFINITIVE:","").replace(":",", ").replace(";",". ").replace("("," . ").replace("["," . ").replace(")"," . ").replace("]"," . ").replace("/"," . ").strip()

	return some_str


	
def playTextThread(some_str):
	if len(str(some_str))==0: return
	stopPlayback()
	global stop_reading
	stop_reading=0
	some_str=doLangFilter(some_str)
	lines_list=some_str.split(".") # split by sentences so we dont slow Festival down with big texts
	try:
		os.chdir(getFestivalDir())
	except:
		pass
	try:
		f1=os.popen2("./tts.spanish ")[0]  # write to the festival terminal
	except:
		pass
	for ii in lines_list:	 
	  try:		
		if len(ii.replace("."," ").replace("."," ").replace("  "," ").replace("  "," ").replace("  "," ").replace("  "," ").replace("  "," ").strip())==0: continue # dont say blank lines
		f1.write("(SayText \""+ii.replace("."," ").replace("."," ").replace("  "," ").replace("  "," ").replace("  "," ").replace("  "," ").replace("  "," ")+"\")"+"\n")
		f1.flush()
		#os.popen("./tts.spanish "+doLangFilter(ii)+" &> /dev/null &")
		#time.sleep(0.1) # to enable sound card breaks, and eliminate "sound card busy" messages		
		if stop_reading==1: break
	  except:
		pass
          if stop_reading==1: break  # user wants Festival to shut-up and be quiet
	try:
		f1[0].close()
	except:
		pass
	  
	try:
		# go back to regular directory
		new_dir=getFestivalDir()[0:getFestivalDir().rfind(os.sep)]
		if new_dir.find("festival")>0: new_dir=new_dir[0:new_dir.rfind(os.sep)]
		if new_dir.find("festival")>0: new_dir=new_dir[0:new_dir.rfind(os.sep)]
		if new_dir.find("festival")>0: new_dir=new_dir[0:new_dir.rfind(os.sep)]
		os.chdir(new_dir)
	except:
		pass
	  
def playText(some_str):
	if not isSupported(): 
		showNotSupported()
		return
	t2=Thread(None,playTextThread,None,(some_str,))
	t2.start()

class FestivalBar(VBox):
	def __init__(self,some_window,text_widget,button_count=2,shortblock=0):
		VBox.__init__(self)
		tips=TIPS
		self.set_border_width(4)
		self.SHORT_BLOCK=shortblock
		self.set_spacing(5)
		self.TA=text_widget
		self.playbutt=self.getIconButton(some_window,getPixDir()+"play.gif","Play")
		tips.set_tip(self.playbutt,"Read the words aloud in Spanish from the current cursor position.")
		self.pack_start(self.playbutt,0,0,0)
		self.playbutt.connect("clicked",self.startPlayback)
		if button_count==2:
			self.stopbutt=self.getIconButton(some_window,getPixDir()+"stop.gif","Stop")
			tips.set_tip(self.stopbutt,"Stop the audio reading")
			self.pack_start(self.stopbutt,0,0,0)
			self.stopbutt.connect("clicked",self.stopPlayback)	
		if button_count==2: self.pack_start(Label(" "),1,1,0)
		self.show_all()

	def getIconButton (self,iwin,picon,lab="?") :  
        	try:
	 		 b=RadioButton(None)
	 		 b.add( getImage (picon,lab) )
			 b.set_mode(0)
			 b.set_relief(2) # flat button
         		 b.show_all()  
          		 return b
        	except:
          		return Button(filter(lab))

    	def getText(self) :
		try:
			return self.TA.get_data("word").strip()
		except:
			try:
				endit=-1
				if len(text_buffer_get_text(self.TA).strip())==0: raise TypeError
				tbuf=self.TA.get_buffer()
				try:
					starti,endi=tbuf.get_selection_bounds()
					endit=endi.get_offset()
				except:
					starti=tbuf.get_iter_at_mark(tbuf.get_insert())
				if endit>0: org_line=tbuf.get_text(starti,tbuf.get_iter_at_offset(endit))
				else: org_line=tbuf.get_text(starti,tbuf.get_end_iter())
				if len(org_line.strip())==0: 
					starti=tbuf.get_start_iter()
					org_line=tbuf.get_text(starti,tbuf.get_end_iter())
				if len(org_line.strip())==0: return ""
				org_line=org_line[0:30000]
				mytext= org_line.strip()
				if self.SHORT_BLOCK==1:
					if mytext.startswith("[") or mytext.startswith("("): mytext=mytext[1:len(mytext)]
					if mytext.find("[")>-1:  # for 'conjugate' and 'idoms' dont read stuff after the [ - which is english
						mytext=mytext[0:mytext.find("[")].strip()
					if mytext.find("]")>-1: 
						mytext=mytext[0:mytext.find("]")].strip()
					if mytext.find("(")>-1: 
						mytext=mytext[0:mytext.find("(")].strip()
					if mytext.find(")")>-1: 
						mytext=mytext[0:mytext.find(")")].strip()

				new_index=starti.get_offset()+org_line.find(mytext)  # highlight the area being read
				if endit>0: end_index=endit
				else: end_index=new_index+len(mytext) 
				if end_index>len(text_buffer_get_text(self.TA))+1: end_index=len(text_buffer_get_text(self.TA))+1
				tbuf.place_cursor(tbuf.get_iter_at_offset(new_index))
				tbuf.move_mark(tbuf.get_selection_bound(), tbuf.get_iter_at_offset(end_index) )
				self.TA.grab_focus()
				return unfilter(mytext.strip())
			except:
				try:
					org_line=text_buffer_get_text(self.TA)
					org_line=org_line[0:30000]
					mytext= org_line.strip()
					if self.SHORT_BLOCK==1:
						if mytext.startswith("[") or mytext.startswith("("): mytext=mytext[1:len(mytext)]
						if mytext.find("[")>-1:  # for 'conjugate' dont read stuff after the [ - which is english
							mytext=mytext[0:mytext.find("[")].strip()
						if mytext.find("]")>-1: 
							mytext=mytext[0:mytext.find("]")].strip()
						if mytext.find("(")>-1: 
							mytext=mytext[0:mytext.find("(")].strip()
						if mytext.find(")")>-1: 
							mytext=mytext[0:mytext.find(")")].strip()
					new_index=0+org_line.find(mytext)  # highlight the area being read
					end_index=new_index+len(mytext) 
					if end_index>len(text_buffer_get_text(self.TA))+1: end_index=len(text_buffer_get_text(self.TA))+1
					tbuf.place_cursor(tbuf.get_iter_at_offset(new_index))
					tbuf.move_mark(tbuf.get_selection_bound(), tbuf.get_iter_at_offset(end_index) )
					self.TA.grab_focus()
					return unfilter(mytext)
				except:				
					return ""

	def stopPlayback(self,*args):
		stopPlayback()
		stopPlayback()
	def startPlayback(self,*args):
		ssd=self.getText()
		if len(ssd.strip())==0: return
		playText(ssd)
	def setTextWidget(self,some_widget):
		self.TA=some_widget
