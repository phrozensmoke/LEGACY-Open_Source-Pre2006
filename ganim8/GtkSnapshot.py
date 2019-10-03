#! /usr/bin/env python
##############################################################
#  Copyright 2003 Erica Andrews(PhrozenSmoke@yahoo.com). 
#  All rights reserved.
#  http://ganim8.sourceforge.net.  
#  
#  GtkSnapshot  is part of gAnim8, a suite for building, viewing, and editing animated GIFS, but is also 
#  available as a stand-alone version.   GtkSnapshot is a Gtk-based program for taking screenshots of 
#  X desktops, individual windows, or a portion of an X screen. Technically, it"s just a quick frontend for 
#  ImageMagick 'import', 'convert', and 'mogrify' utilities, but may be useful nonetheless.  
#  
#  gAnim8, the gAnim8 Conversion Tool, and GtkSnapshot are distributed under the W3C License.
#  
#  Permission to use, copy, modify, and distribute this software and its documentation, with or 
#  without modification,  for any purpose and WITHOUT FEE OR ROYALTY is hereby granted, 
#  provided these credits are left in-tact and a copy of the 'LICENSE' document accompanies 
#  all copies of this software. COPYRIGHT HOLDERS WILL NOT BE LIABLE FOR ANY DIRECT,
#  INDIRECT, SPECIAL OR CONSEQUENTIAL DAMAGES ARISING OUT OF ANY USE OF 
#  THE SOFTWARE OR DOCUMENTATION.  See the document 'LICENSE' for a complete copy
#  of this software license agreement.
##############################################################
import os,GTK,gtk,GtkExtra,GdkImlib,random,sys,gettext,pyprint,math
from gtk import *

global cversion
cversion="v. 0.1b"
global temp_dir
temp_dir="/tmp/"
global IMAGE_MAGICK
IMAGE_MAGICK=0
global GIF_SICLE
GIF_SICLE=0
global CURRENT_FILE
CURRENT_FILE=None
global IS_GIF
IS_GIF=0
global im_synonyms
im_synonyms={"ras":"sun","ppm":"pnm","jpg":"jpeg","gif87":"gif","bmp24":"bmp","dib":"bmp","jpeg24":"jpeg","pict24":"pict","pct":"pict","tif":"tiff","tiff24":"tiff","ptif":"tiff","icb":"tga","vst":"tga","vda":"tga","bie":"jbig","jbg":"jbig","g3":"fax","dcx":"pcx","pcds":"pcd","pal":"uyvy","xv":"viff","app1":"8bim","cmyka":"cmyk","rgba":"rgb"}

class convwindow :
    def __init__(self,dodestroy=1,tempd="/tmp/") :
	global temp_dir
	temp_dir=tempd
	self.temp_dir=tempd
        write_formats=["8BIM Photoshop resource format","AVS AVS X image","BMP Microsoft Windows bitmap image","BMP24 Microsoft Windows 24bit bitmap image","DIB Microsoft Windows bitmap image","CACHE Magick Persistent Cache image format","CMYK Raw cyan, magenta, yellow, and black bytes","EPT Adobe Encapsulated PostScript with TIFF preview","FAX Group 3 FAX","G3 Group 3 FAX","FITS Flexible Image Transport System","FPX FlashPix Format","GIF CompuServe graphics interchange format","GIF87 CompuServe graphics interchange format(version 87a)","GRAY Raw gray bytes","HDF Hierarchical Data Format","HISTOGRAM Histogram of the image","IPTC IPTC Newsphoto","BIE Joint Bilevel Image experts Group interchange format","JBG Joint Bilevel Image experts Group interchange format","JBIG Joint Bilevel Image experts Group interchange format","JPEG24 Joint Photographic Experts Group JFIF format","JPEG Joint Photographic Experts Group JFIF format","JPG Joint Photographic Experts Group JFIF format","GRANITE Granite texture","LOGO ImageMagick Logo","MAP Colormap intensities and indices","MATTE MATTE format","MIFF Magick image format","MONO Bilevel bitmap in leastsignificantbyte first order","MTV MTV Raytracing image format","PCD Photo CD","PCDS Photo CD","PCL Page Control Language","DCX ZSoft IBM PC multipage Paintbrush","PCX ZSoft IBM PC Paintbrush","EPDF Encapsulated Portable Document Format","PDF Portable Document Format","PCT Apple Macintosh QuickDraw/PICT","PICT Apple Macintosh QuickDraw/PICT","PICT24 24bit Apple Macintosh QuickDraw/PICT","PNG Portable Network Graphics","PBM Portable bitmap format(black and white)","PGM Portable graymap format(gray scale)","PNM Portable anymap","PPM Portable pixmap format(color)","EPI Adobe Encapsulated PostScript Interchange format","EPS Adobe Encapsulated PostScript","EPSF Adobe Encapsulated PostScript","EPSI Adobe Encapsulated PostScript Interchange format","PS Adobe PostScript","EPS2 Adobe Level II Encapsulated PostScript","PS2 Adobe Level II PostScript","PS3 Adobe Level III PostScript","PSD Adobe Photoshop bitmap","RGB Raw red, green, and blue bytes","RGBA Raw red, green, blue, and matte bytes","SGI Irix RGB image","RAS SUN Rasterfile","SUN SUN Rasterfile","ICB Truevision Targa image","TGA Truevision Targa image","VDA Truevision Targa image","VST Truevision Targa image","PTIF Pyramid encoded TIFF","TIF Tagged Image File Format","TIFF Tagged Image File Format","TIFF24 24bit Tagged Image File Format","UIL XMotif UIL table","PAL 16bit/pixel interleaved YUV","UYVY 16bit/pixel interleaved YUV","VICAR VICAR rasterfile format","VIFF Khoros Visualization image","XV Khoros Visualization image","WBMP Wireless Bitmap(level 0) image","XBM X Windows system bitmap(black and white)","PM X Windows system pixmap(color)","XPM X Windows system pixmap(color)","XWD X Windows system window dump(color)","YUV CCIR 601 4:1:1" , "PALM Pixmap Format","PICON Personal Icon", "ROSE 70x64 Truecolor Test Image","CMYKA Raw cyan, magenta, yellow, and matte bytes","APP1 Photoshop resource format","H Internal Format","ICC Color Profile","JP2 JPEG-2000 JP2 File Format Syntax","JPC JPEG-2000 JPC Code Stream Syntax","VID Visual Image Directory","P7 Xv Thumbnail Format",    "M2V Mpeg 2 Video Stream(Single Frame)","MPEG Mpeg 1 Video Stream(Single Frame)","MPG Mpeg 1 Video Stream(Single Frame)","OTB On-the-air Bitmap","DPX Digital Moving Picture Exchange","ILBM Amiga IFF","RAD Radiance Image File","MNG Multipleimage Network Graphics"]

	# gettext locale support
	self.xtext=gettext.NullTranslations()
	try:
		# open gtksnapshot.mo in /usr/share/locale
		self.xtext=gettext.Catalog("gtksnapshot")
	except:
		self.xtext=gettext.NullTranslations()

        write_formats.sort()
	self.CAPTURE=None
	global cversion
	convwindow=GtkWindow(GTK.WINDOW_TOPLEVEL)
	tips=GtkTooltips()
	if dodestroy==1: convwindow.set_wmclass("gtksnapshot","GtkSnapshot")
        convwindow.set_position(GTK.WIN_POS_CENTER)
        if dodestroy==1: convwindow.connect("destroy",quit)
        else: convwindow.connect("destroy",self.hideit)
	convwindow.realize()
        self.sel_file=None
	convwindow.set_title("GtkSnapshot "+cversion)
	convwindow.set_position(GTK.WIN_POS_CENTER)
	self.convwindow=convwindow
	hbox1=GtkHBox(0, 0)
	vbox1=GtkVBox(0, 0)
        try:
          header=GtkPixmap(convwindow,getPixDir()+"gtksnap_logo.xpm",None)
        except:
	  header=GtkLabel("GtkSnapshot "+cversion)
        if not header: header=GtkLabel("GtkSnapshot "+cversion)
	self.header=header
	vbox1.pack_start(header, 0, 0, 2)

	imagewindow=GtkScrolledWindow()
	self.imagewindow=imagewindow
        toplay=GtkLayout(None,None)
        self.toplay=toplay
        toplay.set_size(500,500)
        toplay.set_hadjustment(imagewindow.get_hadjustment())
	imagewindow.add(toplay)
	imagewindow.set_usize(205,155)
	vbox1.pack_start( imagewindow, 1, 1, 0)
	vbox3=GtkVBox(0, 0)
	table1=GtkTable(2,2,0)
	table1.set_border_width(1)
	label8=GtkLabel(self.trans_text("Save As:")+"  ")
	label8.set_alignment( 0, 0.5)
	table1.attach(label8,0,1,0,1,(GTK.EXPAND+GTK.FILL),(GTK.EXPAND+GTK.FILL),0,0)
	totype=GtkCombo()
	tips.set_tip(totype,self.trans_text("Select the image format you want the screenshot saved as"))
        totype.set_popdown_strings(write_formats)
        totype.set_usize(270,-2)
	self.totype=totype
	totypeentry=totype.entry
	tips.set_tip(totypeentry,self.trans_text("Select the image format you want the screenshot saved as"))
        totypeentry.set_text("GIF CompuServe graphics interchange format")
        totypeentry.set_editable(0)
	self.totypeentry=totypeentry
	table1.attach(totype,1,2,0,1,(GTK.EXPAND+GTK.FILL),(GTK.EXPAND+GTK.FILL),0,0)
	hbox5=GtkHBox(0, 0)
	label9=GtkLabel(self.trans_text("File")+":         ")
	label9.set_alignment( 0, 0.5)
	table1.attach(label9,0,1,1,2,(GTK.EXPAND+GTK.FILL),(GTK.EXPAND+GTK.FILL),0,0)
	tofileentry=GtkEntry()
        tofileentry.set_usize(210,-2)
	self.tofileentry=tofileentry
	tips.set_tip(tofileentry,self.trans_text("Enter a file name to save the screenshot to"))
        self.targets = [('STRING', 0, 0), ('text/plain', 0,1), ('text/uri-list', 0, 2) ]
        self.tofileentry.drag_dest_set(gtk.DEST_DEFAULT_ALL, self.targets, gtk.GDK.ACTION_COPY | gtk.GDK.ACTION_MOVE)
	self.tofileentry.connect("drag_drop",self.setDrag)
	self.tofileentry.connect("drag_begin",self.setDrag)
	self.tofileentry.connect("drag_data_received",self.setDrag)
	try:
		tofileentry.set_text(os.environ["HOME"]+"/screenshot.gif")
	except:
		pass
	hbox5.pack_start( tofileentry, 1, 1, 0)
	tobrowse=self.getPixButton (getPixDir()+"gtksnap_open.xpm",self.trans_text("Browse..."),0)
	tips.set_tip(tobrowse,self.trans_text("Select a file to save the screenshot to"))
        tobrowse.connect("clicked",self.getToFile)
	hbox5.pack_start( tobrowse, 0, 0, 1)
	table1.attach(hbox5,1,2,1,2,(GTK.EXPAND+GTK.FILL),(GTK.EXPAND+GTK.FILL),0,0)
	vbox3.pack_start( table1, 1, 1, 0)

	buttbox=GtkHBox(0,0)
	buttbox.set_homogeneous(1)
	buttbox.set_spacing(5)
	compbutt=self.getPixButton (getPixDir()+"gtksnap_compress.xpm","Compress GIF",1)
	tips.set_tip(compbutt,self.trans_text("Compress the GIF (if applicable)\nRequires the optional Gifsicle program to be on your path."))
        compbutt.connect("clicked",self.doCompress)       
	self.compbutt=compbutt
        self.compbutt.set_sensitive(0)
	buttbox.pack_start(compbutt,1,1,0)
	convertbutt=self.getPixButton (getPixDir()+"gtksnap_save.xpm","SAVE",1)
	tips.set_tip(convertbutt,self.trans_text("Save this screenshot to the selected file"))
        convertbutt.connect("clicked",self.startSave)
	buttbox.pack_start( convertbutt, 1, 1, 0)
	vbox3.pack_start( buttbox, 0, 0, 4)

	vbox3.pack_start( GtkHSeparator(), 0,0, 3)
	vbox1.pack_start( vbox3, 0, 0, 0)

	hbox6=GtkVBox(0, 0)
	grabw=self.getPixButton (getPixDir()+"gtksnap_capture.xpm","CAPTURE Window or Screen Area",1)
	tips.set_tip(grabw,self.trans_text("Take a screenshot of a specific window or a selectable area of the screen"))
        grabw.connect("clicked",self.doGrabWindow)
	hbox6.pack_start( grabw, 1, 1, 1)
	self.includeframe=GtkCheckButton(" "+self.trans_text("Include Window Frame"))
	tips.set_tip(self.includeframe,self.trans_text("If checked, the window's frame will be included in the screenshot (if applicable)."))
	hbox6.pack_start( self.includeframe, 1, 1, 1)
	self.includeframe.set_active(1)
	hbox6.pack_start( GtkHSeparator(), 0,0, 3)
	grabd=self.getPixButton (getPixDir()+"gtksnap_capture.xpm","CAPTURE Entire Desktop",1)
	tips.set_tip(grabd,self.trans_text("Take a screenshot of the WHOLE desktop"))
        grabd.connect("clicked",self.doGrabDesktop)
	hbox6.pack_start(grabd, 1, 1, 1)
	vbox1.pack_start( hbox6, 0, 0, 3)
	hbox1.pack_start( vbox1, 1, 1, 6)

        vbox20=GtkVBox(0,0)

	mymeth=quit
        if not dodestroy==1: mymeth=self.hideit

	mymenubar=GtkMenuBar()
	ag=GtkAccelGroup()
        itemf=GtkItemFactory(GtkMenuBar, "<main>", ag)
        itemf.create_items([
        # path              key           callback    action#  type
  	("/ _"+self.trans_text("File"),  "<alt>F",  None,  0,"<Branch>"),	
   	("/ _"+self.trans_text("File")+"/"+self.trans_text("Capture _Window or Screen Area")+"...", "<alt>W", self.doGrabWindow,865,""),
   	("/ _"+self.trans_text("File")+"/"+self.trans_text("Capture Entire _Desktop")+"...", "<alt>D", self.doGrabDesktop,864,""),
   	("/ _"+self.trans_text("File")+"/_"+self.trans_text("Save"), "<control>S", self.startSave,863,""),
   	("/ _"+self.trans_text("File")+"/_"+self.trans_text("Print..."), "<control>P", self.startPrint,877,""),
   	("/ _"+self.trans_text("File")+"/sep1", None, None,869,"<Separator>"),   
   	("/ _"+self.trans_text("File")+"/_"+self.trans_text("Exit"), "<control>Q", mymeth,867,""),   	   		  
  	("/ _"+self.trans_text("Help"),  "<alt>H",  None, 900,"<LastBranch>"),   
  	("/ _"+self.trans_text("Help")+"/_"+self.trans_text("About..."), "F2", self.showAbout,901,""),  	
	])
	convwindow.add_accel_group(ag)
        mymenubar=itemf.get_widget("<main>")
        mymenubar.show()
	vbox20.pack_start(mymenubar,0,0,0)

        vbox20.pack_start(hbox1,1,1,2)
	statusbar=GtkStatusbar()
	self.statusbar=statusbar
        statid=statusbar.get_context_id("ganim8snap")
        self.statid=statid
        vbox20.pack_start(statusbar,0,0,2)
	convwindow.add(vbox20)
        self.showStatus(self.trans_text("Copyright 2003 by Erica Andrews. All rights reserved."))
	os.popen("rm -f "+temp_dir+"gtksnapshot* &") # Delete any temporary files
  	os.popen("rm -f /tmp/gtksnapshot* &") # Delete any temporary files
	convwindow.show_all()

    def setDrag(self,*args):
	drago=args
	if len(drago)<7: 
		args[0].drag_highlight()
		args[0].grab_focus()
	else: 
		#print str(drago[4].data)  # file:/root/something.txt
		try:
			args[0].set_text(""+str(drago[4].data).replace("\r\n","").replace("\x00","").replace("file:",""))
		except: 
			pass
		args[0].drag_unhighlight()

    def setDragAway(self,*args):
	drago=args
	if len(drago)<3: 
		args[0].drag_highlight()
		args[0].grab_focus()
	else: 
		drago[2].set(drago[2].target,8,self.CAPTURE)

    def trans_text(self,some_string):
	return self.xtext.gettext(some_string.replace("_",""))

    def showAbout(self,*args):
        global cversion
        ih2=icehelp2()
        ih2.setHelpText("Copyright 2003 Erica Andrews  (PhrozenSmoke@yahoo.com). \nAll rights reserved.\nhttp://ganim8.sourceforge.net\n\nGtkSnapshot is part of gAnim8, a suite for building, viewing, and editing animated GIFS and small videos, but is also available as a stand-alone version.   GtkSnapshot is a Gtk-based program for taking screenshots of X desktops, individual windows, or a portion of an X screen. Technically, it\"s just a quick frontend for ImageMagick's \"import\", \"convert\", and \"mogrify\" utilities.  NOTE:  GtkSnapshot is still EXPERIMENTAL, but seems to be working rather well.  It is a new addition to the gAnim8 suite.  This program uses ImageMagick's 'import' utility for making screenshots - so, please do not whine to me if GtkSnapshot doesn't take large screenshots as fast as you want: GtkSnapshot is ONLY a frontend for ImageMagick's 'import' utility.  So, all actual creation of screenshots is done by ImageMagick.\n\nSome image formats listed may not be supported on your system and may fail to convert or even crash this program. If you have trouble converting or saving screenshots to very obscure formats, please check the ImageMagick documentation for additional information on extra filters and third-party programs which may be required for the format. Also, make sure your have a recent version of ImageMagick.\n\nRunning the program:  You should be able to launch the program by simply executing 'gtksnapshot', or by executing 'python /usr/share/gAnim8/GtkSnapshot.py'.\n\n\nThis program requires Python 2.2 or better, Python Numeric, PyGTK 0.6.9 or better,  Gtk+ 1.2.8-4 or better, Imlib 1.9.8.1-6 or better, ImageMagick 5.2.3 or better (5.4.6 or better recommended).  Recommended but not required: gifsicle 1.3.0 or better for GIF compression.\n\n\nGETTING GIFSICLE:\nYou can get the latest Gifsicle source code and binary packages (RPM) from  http://www.lcdf.org/gifsicle/\n\nNOTE: This program is built on top of Gifsicle, so you can do nothing with gAnim8 until you get Gifsicle:\nhttp://www.lcdf.org/gifsicle/\n\nThere are some RPMS for Gifsicle on  speakeasy.rpmfind.net and http://www.lcdf.org/gifsicle/\n\n\nGETTING IMAGEMAGICK:\nThe ImageMagick package can be found on both speakeasy.rpmfind.net and www.imagemagick.org .\n\n\ngAnim8, the gAnim8 Conversion Tool, and GtkSnapshot are distributed under the W3C License. This program and its source may be freely modified and redistributed for non-commercial,  non-profit use only. Permission to use, copy, modify, and distribute this software and its documentation, with or without modification,  for any purpose and WITHOUT FEE OR ROYALTY is hereby granted, provided these credits are left in-tact and a copy of the 'LICENSE' document accompanies all copies of this software. COPYRIGHT HOLDERS WILL NOT BE LIABLE FOR ANY DIRECT,INDIRECT, SPECIAL OR CONSEQUENTIAL DAMAGES ARISING OUT OF ANY USE OF THE SOFTWARE OR DOCUMENTATION.  See the document 'LICENSE' for a complete copyof this software license agreement.")
        ih2.icehelp.show_all()

    def hideit(self,*args):
      os.popen("rm -f "+self.temp_dir+"gtksnapshot*") # Delete any temporary files
      os.popen("rm -f /tmp/gtksnapshot*") # Delete any temporary files
      os.popen("rm -f core") # Delete any core dumps
      os.popen("rm -f core.*") # Delete any core dumps
      self.convwindow.hide()
      self.convwindow.unmap()

    def doGrabWindow(self,*args):
	self.showStatus(self.trans_text("Creating screenshot...please wait."))
	self.convwindow.hide()
	self.convwindow.unmap()
	gtk.timeout_add(100,self.doGrabWindow1)

    def doGrabWindow1(self,*args):
	addon=""
	if self.includeframe.get_active(): addon=" -frame "
	self.doGrab("import -quality 100 "+addon+" -comment 'Snapshot created by GtkSnapshot'")

    def doGrabDesktop(self,*args):
	self.showStatus(self.trans_text("Creating screenshot...please wait."))
	self.convwindow.hide()
	self.convwindow.unmap()
	gtk.timeout_add(100,self.doGrabDesktop1)

    def doGrabDesktop1(self,*args):
	addon=""
	if self.includeframe.get_active(): addon=" -frame "
	self.doGrab("import -window root "+addon+" -quality 100 -comment 'Snapshot created by GtkSnapshot'")

    def doGrab(self,cmd_line):
	if not haveImageMagick()==1:
		self.showMessage(self.trans_text("ERROR:\nSorry, screen capture is not possible\nbecause you do not have all the necessary ImageMagick\nbinaries on your path: convert, import, mogrify\n\nYou must install these ImageMagick binaries on your path first."))
		self.convwindow.show_all()
	try:
		os.popen("rm -f /tmp/gtksnapshot*") # Delete any temporary files
      		os.popen("rm -f "+self.temp_dir+"gtksnapshot*") # Delete any temporary files
		tempf=getRandomFileName()
		f=os.popen(cmd_line+" "+tempf)
		f.readline()
		if not os.path.exists(tempf):
			self.showMessage(self.trans_text("Unknown Error:\nScreen capture failed!\nNo screenshot image was produced."))			
		else:
			self.addButton(tempf,1)
			self.CAPTURE=tempf
      			self.toplay.set_data("PHROZEN_FILE",tempf)
			self.showStatus(self.trans_text("Screenshot successfully created."))
	except: 
		self.showMessage(self.trans_text("Unknown Error:\nScreen capture failed!\nNo screenshot image was produced."))
		self.showStatus(self.trans_text("ERROR: Screen capture failed."))
	self.convwindow.show_all()	
	self.convwindow.map()
	if self.includeframe.get_active(): 
		self.convwindow.get_window().lower()
		self.convwindow.get_window()._raise()

    def showStatus(self,statustext,*args):
      global cversion 
      self.statusbar.push(self.statid," "+str(statustext)+" ")

    def extractDirectory(self, fdir):
      if not fdir: return "./"
      f=fdir
      if not f.endswith("/"): f=f[0:f.rfind("/")+1].strip()
      if f.find("/")==-1: f="./"
      return f

    def getToFile(self,*args):
      if self.sel_file:
       try:
         os.popen("rm -f core") # Delete any core dumps
         os.popen("rm -f core.*") # Delete any core dumps
         os.chdir(self.extractDirectory(self.sel_file))
       except:
         pass
      f=GtkExtra.file_sel_box(self.trans_text("Save As..."))
      if not f: return
      self.sel_file=f      
      if not f.endswith("/"): self.tofileentry.set_text(str(f))

    def loadImage(self, picon,scale=0):
       try:
         i=GdkImlib.Image(str(picon).strip().replace(" ","\\ "))
         i.changed_image() # reload from disk, not GdkImlib cache, force GdkImlib to refresh
         i.render()
	 p=GdkImlib.GtkPixmap(i.get_pixmap()[0])
	 p.show_all()
	 d=i
	 if scale==1:
		sug_x=400
		aspect=float(p.size_request()[0])/float(p.size_request()[1])
		if p.size_request()[0]<sug_x: sug_x=p.size_request()[0]
		sug_y=int(math.floor(sug_x/aspect))
		d=i.clone_scaled_image(sug_x,sug_y)
		d.render()
		p=d.make_pixmap()
	 else:
         	p=GdkImlib.GtkPixmap(d.get_pixmap()[0])
         p.show_all()
         return p
       except:
         return None

    def getPixButton (self,picon,lab="?",both=1,vert=0) :  
        try:
	  b=GtkButton()
	  if both==1:
	    if vert==0: 
	    	h=GtkHBox(0,0)
	    	h.set_spacing(4)
	    else: 
	    	h=GtkVBox(0,0)
	    	h.set_spacing(3)	
            h.pack_start(GtkPixmap(self.convwindow,str(picon),None),0,0,0)
	    l=GtkLabel(self.trans_text(str(lab)))
	    l.set_alignment(0,0.5)
	    if vert==1: l.set_alignment(0.5,0.5)
	    h.set_border_width(1)
            h.pack_start(l,1,1,0)
            h.show_all()
	    b.add(h)
	  else:
	    b.add(GtkPixmap(self.convwindow,str(picon),None))
          b.show_all()        
          return b
        except:
          return GtkButton(self.trans_text(str(lab)))	

    def addButton(self,picon,scale=0) :  
      b=self.getIconButton(picon,scale)
      try:
         self.toplay.remove(self.imagebutt)
         self.imagebutt.destroy()
      except:
         pass
      self.imagebutt=b
      b.show_all()
      self.toplay.put(b, 2, 2)
      self.toplay.set_size(b.size_request()[0]+30,b.size_request()[1]+30)
      self.toplay.drag_source_set(gtk.GDK.BUTTON1_MASK, self.targets, gtk.GDK.ACTION_COPY)
      self.toplay.connect("drag_begin",self.setDragAway)
      self.toplay.connect("drag_data_get",self.setDragAway)
      self.toplay.show_all()

    def getIconButton(self,picon,scale=0) :  
       try:
         b=GtkFrame()
         b.add(self.loadImage(picon,scale))
         b.show_all()
         return b
       except:
         return GtkLabel("  "+self.trans_text("Your snapshot was successfully taken,\nbut is not previewable with GdkImlib.")+"\n("+str(picon).strip()+")")

    def doCompress(self,*args) :  
      global CURRENT_FILE
      global IS_GIF
      if CURRENT_FILE==None: return
      if not IS_GIF==1: return
      if not haveGifsicle():
       self.showMessage(self.trans_text("The 'gifsicle' binary from Gifsicle was not found on your PATH.  GtkSnapshot requires Gifsicle to compress GIFs.\nIf you do not have Gifsicle, you may still save your screenshots as UNCOMPRESSED GIFs.  Gifsicle can be downloaded from: http://www.lcdf.org/gifsicle/ .  GIF compression is an OPTIONAL feature of GtkSnapshot."))
       return
      f=os.popen("gifsicle "+CURRENT_FILE+" -O1 -o "+CURRENT_FILE)
      while not f.readline().strip()=="":
         pass
      if os.path.exists(CURRENT_FILE): 
         fsize="unknown"
         try:
           fsize=os.path.getsize(CURRENT_FILE)
         except:
           pass
         self.showStatus(self.trans_text("GIF Compressed")+": "+str(fsize)+" "+self.trans_text("bytes"))
      else: 
         self.showMessage(self.trans_text("An unknown error occurred compressing your GIF file with Gifsicle: Reverting back to your original, uncompressed GIF file."))
         self.fromfileentry.set_text(CURRENT_FILE)
         self.startConvert()

    def showMessage(self,mess) :  
        ih2=icehelp2("GtkSnapshot")
        ih2.setHelpText(self.trans_text(str(mess)))
        ih2.icehelp.show_all()

    def showConfirm(self,mess) :  
      i=GtkExtra.message_box("GtkSnapshot",str(mess),[self.trans_text("YES"),self.trans_text("NO")])
      if i==self.trans_text("YES"): return 1
      return 0

    def startPrint(self,*args):
	if not self.CAPTURE: return
	pyprint.printFile(self.CAPTURE)

    def startSave(self,*args):
	if not self.CAPTURE: return
	if not len(self.tofileentry.get_text().strip()) > 0: return
	if os.path.exists(self.tofileentry.get_text()) or os.path.exists(self.tofileentry.get_text().strip()):
		if not self.showConfirm(self.trans_text("The file already exists.")+"\n"+self.tofileentry.get_text().strip()+"\n"+self.trans_text("Do you wish to overwrite it?"))==1: return
	self.showStatus(self.trans_text("Saving (and converting)....please wait."))
	gtk.timeout_add(5,self.startSave1)

    def startSave1(self,*args) :  
	self.startConvert(self.CAPTURE,"JPEG")

    def startConvert(self,from_file,from_type) :  
      global IS_GIF
      stat=doConversion(from_file,  from_type , self.tofileentry.get_text(),  self.totype.entry.get_text())
      if not stat[0]: 
        stat=doConversion(from_file,  from_type ,self.tofileentry.get_text(),  self.totype.entry.get_text(),1)
        if not stat[0]:
          self.showMessage(self.trans_text("Problem Converting/Saving Screenshot")+":\n\n"+self.trans_text(stat[1])+"\n\n"+self.trans_text("Be sure that you have the 'convert' binary from ImageMagick (version 5.2.3 or better) on your PATH.")+"\n")
          self.showStatus(self.trans_text("Done: Save failed."))
          self.compbutt.set_sensitive(0)
          return
      if stat[0]:
        #self.addButton(stat[2])
        self.showStatus(self.trans_text("File saved")+":  "+str(stat[3])+" "+self.trans_text("bytes"))
        if IS_GIF==1: self.compbutt.set_sensitive(1)
        else: self.compbutt.set_sensitive(0)
        if len(stat[1]): self.showMessage(self.trans_text("Your screenshot was successfully saved with the following warnings/messages")+":\n\n"+stat[1])
      os.popen("rm -f core")
      os.popen("rm -f core.*")


class icehelp2 :
    def __init__(self,mytitle="About GtkSnapshot") :
	icehelp=GtkWindow(GTK.WINDOW_TOPLEVEL)
	icehelp.set_wmclass("gtksnapshot","GtkSnapshot")
	self._root=icehelp
        icehelp.realize()
	icehelp.set_title(mytitle)
	icehelp.set_policy(1, 1, 0)
	icehelp.set_position(GTK.WIN_POS_CENTER)
        icehelp.set_usize(300,375)
        icehelp.connect("destroy",self.hideh)
	self.icehelp=icehelp
	vbox1=GtkVBox(0, 0)
	vbox1.set_border_width(11)
	self.vbox1=vbox1
	scrolledwindow1=GtkScrolledWindow()
	self.scrolledwindow1=scrolledwindow1
	helptext=GtkText()
	helptext.set_editable(0)
        helptext.set_word_wrap(1)
	self.helptext=helptext
	scrolledwindow1.add(helptext)
	vbox1.pack_start(scrolledwindow1, 1, 1, 0)
	closebutt=GtkButton("Close  (Cerrar)")
        closebutt.connect("clicked",self.hideh)
	self.closebutt=closebutt
	vbox1.pack_start(closebutt, 0, 0, 13)
	icehelp.add(vbox1)

    def setHelpText(self, helpstuff):
      try:
       self.helptext.delete_text(0,self.helptext.get_length())
       self.helptext.delete_text(0,self.helptext.get_length()+1)
      except:
       pass
      self.helptext.insert_defaults("\n"+str(helpstuff))

    def hideh(self,*args):      
      self.icehelp.hide()
      self.icehelp.destroy()
      self.icehelp.unmap()
      return TRUE

def isOnPath(binary):
  if os.environ.has_key("PATH"):
    paths=os.environ["PATH"].split(":")
    for i in paths:
      p=i
      if not p.endswith("/"): p=p+"/"
      if os.path.exists(p+str(binary)):
        return 1
  return 0

def haveGifsicle():
  global GIF_SICLE
  if  GIF_SICLE==1: return 1
  GIF_SICLE=isOnPath("gifsicle")
  return GIF_SICLE        

def haveImageMagick():
  global IMAGE_MAGICK
  if  IMAGE_MAGICK==1: return 1
  IMAGE_MAGICK=isOnPath("convert")
  if not IMAGE_MAGICK==1: return 0
  IMAGE_MAGICK=isOnPath("import")
  if not IMAGE_MAGICK==1: return 0
  IMAGE_MAGICK=isOnPath("mogrify")
  return IMAGE_MAGICK

def changeType(TOTYPE):
  global im_synonyms
  if im_synonyms.has_key(TOTYPE): return im_synonyms[TOTYPE]
  return TOTYPE

def doConversion(from_file,from_type,to_file,to_type,special=0):
  if not haveImageMagick(): return [0,"You do not have all the necessary ImageMagick\nbinaries on your path: convert, import, mogrify\n\nYou must install these ImageMagick binaries on your path first. The 'convert' binary from the ImageMagick program is NOT on your PATH ImageMagick's 'convert' utility is required for all image conversions and must be on your PATH. ImageMagick 5.2.3 or better is required. Version 5.4.6.1 or better is recommended.  You can find versions of ImageMagick on speakeasy.rpmfind.net or www.imagemagick.org ."]
  if not str(from_file).strip(): return [0,"No file specified to convert FROM"]
  if not str(to_file).strip(): return [0,"No file specified to convert TO"]
  if not str(from_type).strip(): return [0,"No FORMAT specified to convert FROM"]
  if not str(to_type).strip(): return [0,"No FORMAT specified to convert TO"]
  if str(to_file).strip().endswith(os.sep): return [0,"The file to convert TO is a directory, not a file"]
  if str(from_file).strip().endswith(os.sep): return [0,"The file to convert FROM is a directory, not a file"]
  if not os.path.exists(str(from_file).strip()): return [0,"The file you are trying to convert FROM does not exist"]
  TOTYPE=str(to_type).lower().strip()
  if TOTYPE.find(" ") > -1: TOTYPE=TOTYPE[0:TOTYPE.find(" ")].strip()
  TOTYPE=changeType(TOTYPE)
  FROM_TYPE=str(from_type).lower().strip()  
  if FROM_TYPE.find(" ") > -1: FROM_TYPE=FROM_TYPE[0:FROM_TYPE.find(" ")].strip()
  FROM_TYPE=changeType(FROM_TYPE)
  addon=""
  if special==1: addon=FROM_TYPE+":"
  try: # remove the to_file if it exists
    os.popen("rm -f "+str(to_file).strip())
    os.popen("rm -f core") # Delete any core dumps
    os.popen("rm -f core.*") # Delete any core dumps
  except:
    pass
  try:
    fmessage=""
    f=os.popen("convert "+addon+str(from_file).strip().replace(" ","\\ ")+" "+TOTYPE+":"+str(to_file).strip().replace(" ","\\ ") )
    fmessage=f.readline().strip()
    f.close()
    if fmessage: fmessage="ImageMagick Warnings: "+fmessage
    tofile=str(to_file).strip()
    if TOTYPE=="otb": 
       if not os.path.exists(tofile): tofile=tofile+".0" # This happens when we convert from a multi-framed image(animated gif, etc)
    if os.path.exists(tofile): 
      global IS_GIF
      global CURRENT_FILE
      if TOTYPE=="gif": IS_GIF=1
      else: IS_GIF=0
      CURRENT_FILE=tofile
      return [1,fmessage,tofile,os.path.getsize(tofile)]
    else: return [0,fmessage+"\nConversion failed:\nImageMagick may require a supplemental library for this conversion.\nPlease check the ImageMagick documentation for the requirements for converting this format."]
  except:
    return [0,"An unknown error occurred: Conversion failed."]
  return [0,"An unknown error occurred: Conversion failed."]


def run() :
    cw=convwindow()
    gtk.mainloop()

def quit(*args):
  global temp_dir
  os.popen("rm -f "+temp_dir+"gtksnapshot*") # Delete any temporary files
  os.popen("rm -f /tmp/gtksnapshot*") # Delete any temporary files
  os.popen("rm -f core") # Delete any core dumps
  os.popen("rm -f core.*") # Delete any core dumps
  mainquit()

def getRandomFileName(prefix="gtksnapshot",extension=".jpg"):
	global temp_dir
	tmpf=temp_dir+prefix+str(random.randrange(9999))+str(random.randrange(999))+extension
	return tmpf

def getPixDir() :
	return sys.argv[0][0:sys.argv[0].rfind(os.sep)+1]+"pixmaps"+os.sep	

if __name__=="__main__" :
    run()