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
import pythonol_festival,pythonol_help

color_dict={"blanco":"white","amarillo":"khaki1","anaranjado":"DarkOrange1","azul":"CornflowerBlue","beige":"OldLace","café":"chocolate4","carmesí":"firebrick","colorado":"firebrick3","gris":"SlateGray3","lila":"thistle2","marrón":"DarkMagenta","morado":"DarkOrchid4","naranjo":"orange","negro":"black","pardo":"coral4","prieto":"grey0","purpúreo":"MediumOrchid1","rojo":"red","rosado":"plum1","verde":"green3","azulceleste":"SkyBlue"}

class palettewin :
    def __init__ (self,*args) :
	palettewin = Window (WINDOW_TOPLEVEL)
	palettewin.set_wmclass(WINDOW_WMC,WINDOW_WMN)
	set_window_icon(palettewin,pythonol_icon)
	palettewin.set_title (to_utf8(filter("Pythoñol: Spanish Color Palette")))
	palettewin.set_position (WIN_POS_CENTER)
	palettewin.realize()
	self.palettewin = palettewin
	vbox1 = VBox(0, 3)
	vbox1.set_border_width(5)
	vbox1.set_spacing(7)
	vbox1.pack_start(Label (" Colors in Spanish "), 0, 0, 3)
	scrolledwindow1 = ScrolledWindow ()
	scrolledwindow1.set_border_width (2)
	scrolledwindow1.set_size_request (310, 300)
	viewport1 =Viewport ()
        table1 = Table (3, len(color_dict), 0)
	self.table1 = table1
	table1.set_row_spacings(2)
	table1.set_col_spacings(5)
	scrolledwindow1.add (viewport1)
        viewport1.add(table1)
	vbox1.pack_start(scrolledwindow1, 1, 1, 0)
	closebutt=pythonol_help.getIconButton (palettewin,STOCK_CANCEL," Close",1) 
	TIPS.set_tip(closebutt,to_utf8("Close\nCerrar la ventana"))
	closebutt.connect("clicked",self.hideh)
	vbox1.pack_start(closebutt, 0, 0, 0)	
	palettewin.add(vbox1)
	palettewin.connect("destroy",self.hideh)
	self.showPalette()
	palettewin.show_all()

    def hideh(self,*args):      
	self.palettewin.hide()
	self.palettewin.destroy()
	self.palettewin.unmap()
	return TRUE

    def showPalette(self):
      l=color_dict.keys()
      l.sort()
      tpcounter=0
      for i in l:
        color_button=ColorButton(color_dict[i])
	mbtext=Label() # hidden label for Festival 
	mbtext.set_data("word", filter(i))
	mb=pythonol_festival.FestivalBar(self.palettewin,mbtext,1)
        self.table1.attach(mb, 0, 1, tpcounter, tpcounter+1, (FILL), (0), 1, 1)
        self.table1.attach(Label("  "+to_utf8(filter(i))+"  "), 1, 2, tpcounter, tpcounter+1, (FILL), (0), 1, 1)
        self.table1.attach(color_button.frameit, 2,3, tpcounter, tpcounter+1, (FILL+EXPAND), (1), 1, 1)
	tpcounter=tpcounter+1


class ColorButton:
   def __init__(self,color_name):
     f=Frame()
     f.set_shadow_type(SHADOW_ETCHED_OUT)
     drawing_area = DrawingArea()
     drawing_area.set_size_request(90, 12)
     drawing_area.show() 
     self.drawing_area=drawing_area
     f.add(drawing_area)
     self.frameit=f
     f.show_all()
     self.gc=None
     self.new_color=color_name
     self.win=drawing_area.window
     drawing_area.connect("expose_event", self.expose_event)
     drawing_area.set_events(GDK.EXPOSURE_MASK |
     GDK.LEAVE_NOTIFY_MASK |
     GDK.BUTTON_PRESS_MASK )

   def expose_event(self,*args):
     self.setColor(self.new_color)

   def setColor(self,color_nm):
     if not self.win: self.win=self.drawing_area.window
     if not self.gc: self.gc=self.win.new_gc()
     if not self.win.get_colormap(): return 
     self.gc.foreground=self.win.get_colormap().alloc_color(str(color_nm).strip())
     self.win.draw_rectangle(self.gc,TRUE,0,0,self.win.get_size()[0],self.win.get_size()[1])
     return FALSE 


if __name__== "__main__" :  # for testing
	palettewin()
	threads_init()
	mainloop()
