#! /usr/bin/env python
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

class HelpAnimation(VBox):
	def __init__(self,animation_list,duration=1500, getIconButton=None):
		VBox.__init__(self)		
		self.DURATION=duration
		self.set_border_width(4)
		self.set_spacing(5)
		self.CURRENT_PIC=None
		self.run_anim=0
		self.anim_names=animation_list
		try:
			if not self.loadImages(animation_list)==1: raise TypeError
			if len(self.MY_PICS)==0: raise TypeError
			#header=self.loadImage(getDocDir()+"images"+os.sep+"help_penguin.png")
			header=Image()
			header.set_from_stock(STOCK_DIALOG_QUESTION,ICON_SIZE_DIALOG)
			if not header==None: 	
				self.pack_start(header,0,0,1)
				self.pack_start(Label("Help"),0,0,1)

			hb=Frame()
			hb.set_size_request(151,151)
			hb.add(self.MY_PICS[0])
			self.hb=hb
			self.CURRENT_PIC=self.MY_PICS[0]
			self.pack_start(self.hb,0,0,8)
			self.step_label=Label("Step 1")
			self.pack_start(self.step_label,0,0,1)			
			self.STOP=getIconButton (self,STOCK_NO," Stop (Pause)",1) 
			TIPS.set_tip(self.STOP,to_utf8("Stop (Pause).\nDetener."))
			self.STOP.connect("clicked",self.togAnim)
			self.pack_start(self.STOP,0,0,4)
			self.connect("destroy",self.stopAnim)
			self.pack_start(Label(""),1,1,0)
			self.show_all()
			self.run_anim=1
			self.anim_index=0
			timeout_add(self.DURATION,self.runAnimation)
		except:
			pass

	def togAnim(self,*args):
		if self.run_anim==0:
			self.run_anim=1
			timeout_add(self.DURATION,self.runAnimation)
			self.STOP.get_children()[0].get_children()[0].set_from_stock(STOCK_NO,ICON_SIZE_BUTTON)
			self.STOP.get_children()[0].get_children()[1].set_text(" Stop (Pause)")
			TIPS.set_tip(self.STOP,to_utf8("Stop (Pause).\nDetener."))
		else:
			self.run_anim=0
			self.STOP.get_children()[0].get_children()[0].set_from_stock(STOCK_YES,ICON_SIZE_BUTTON)
			self.STOP.get_children()[0].get_children()[1].set_text(" Play ")
			TIPS.set_tip(self.STOP,to_utf8("Start.\nIniciar."))

	def stopAnim(self,*args):
		self.run_anim=0

	def runAnimation(self,*args):
		if self.run_anim==0: return 0
		if len(self.MY_PICS)==0: return 0
		if self.anim_index>=len(self.MY_PICS)-1: self.anim_index=-1
		if self.CURRENT_PIC==None: NEW_PIC=self.MY_PICS[0]
		else:
			try: 
				self.anim_index=self.anim_index+1				
				NEW_PIC=self.MY_PICS[self.anim_index]
			except:				
				NEW_PIC=self.MY_PICS[0]
				self.anim_index=0
		try:
			self.hb.remove(self.CURRENT_PIC)
		except:
			pass
		try:
			self.hb.add(NEW_PIC)
			self.CURRENT_PIC=NEW_PIC
			self.step_label.set_text(" Step "+str(self.anim_index+1)+" ")
		except:
			pass
		#print str(self.anim_index)
		return 1

	def loadImage(self,filename):
		return loadScaledImage(filename,150,150)

	def loadImages(self,anim_list):
		self.MY_PICS=[]
		for ii in anim_list:
			try:
				p=self.loadImage(getDocDir()+"images"+os.sep+ii)
				if not p==None: self.MY_PICS.append(p)
			except:
				return 0
		return 1

# For testing below

def quitAnim(*args):
	args[0].run_anim=0
	mainquit()

if __name__== "__main__" :  # for testing
	wig=HelpAnimation(['bab1.jpg', 'bab2.jpg','bab3.jpg'],2000)
	w=Window(WINDOW_TOPLEVEL)
	w.connect("destroy",quitAnim)
	w.add(wig)
	w.show_all()
	mainloop()
    
