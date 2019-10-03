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

class symbolbar(HBox):
    def __init__(self,text_entry_val,pythonol_win,TEXT_TYPE=1) :
	self.textval=text_entry_val
	self.text_type=TEXT_TYPE  # 1=Entry, 2=TextView
	HBox.__init__(self,0,0)
	self.set_homogeneous(0)
	self.set_border_width(1)
	self.set_spacing(1)
	self.pack_start(Label("  "),1,1,0)
	mybuttons=[]
	mysymbols=[
                # pixmaps are not used right now, maybe again in the future
		["s1.gif", "á"], 
		["s2.gif", "é"], 
		["s3.gif", "í"], 
		["s4.gif", "ó"], 
		["s5.gif", "ñ"], 
		["s6.gif", "ú"], 
		["s7.gif","ü"], 
		["s10.gif","Á"], 
		["s11.gif","É"], 
		["s12.gif","Í"], 
		["s13.gif","Ó"], 
		["s14.gif","Ñ"], 
		["s15.gif","Ú"], 
		["s16.gif","Ü"], 
		["s8.gif", "¿"], 
		["s9.gif", "¡"], 
			]
	for bb in mysymbols:
		mgif=bb[0]
		mlet=bb[1]
		sbut=self.getIconButton(pythonol_win,getPixDir()+mgif,mlet)
		sbut.connect("clicked",self.addSymbol,mlet)
		mybuttons.append(sbut)
		self.pack_start(sbut,0,0,0)
	clbutton=Button()
	clbutton.connect("clicked",self.clearTextFields)
	TIPS.set_tip(clbutton,"Erase all text from the field above")
	try:
            clbutton.add(loadScaledImage(STOCK_DIALOG_ERROR,18,18,1))
        except:
            clbutton.add(Label("DEL"))
	self.pack_start(Label("   "),0,0,0)
	self.pack_start(clbutton,0,0,0)
	self.pack_start(Label("  "),1,1,0)
	for rrr in mybuttons:
		TIPS.set_tip(rrr, to_utf8( "Insert this letter at the cursor position.\nInsertar esta letra." ))

    def getIconButton (self,iwin,picon,lab="?") :
        return Button( to_utf8(lab) )
        """
        try:
	  b=Button()
          if use_filter==1: b.add(Label ( to_utf8(lab) ) )
	  else: b.add(getImage (picon, to_utf8(lab) ))
          b.show_all()        
          return b
        except:
          return Button( to_utf8(filter(lab)))
        """

    def addSymbol(self,ww=None,mysymbol=None,*args):
	if self.text_type==1:
		ig=self.textval.get_position()
		self.textval.insert_text(to_utf8(str(mysymbol)), ig)
		self.textval.set_position(ig+1)
	else:
		text_buffer_insert_simple(self.textval,mysymbol)
		self.textval.grab_focus()

    
    def clearTextFields(self,*args):
	if self.text_type==1:
		self.textval.set_text("")
	else:
		text_buffer_set_text(self.textval,"")
		self.textval.grab_focus()



