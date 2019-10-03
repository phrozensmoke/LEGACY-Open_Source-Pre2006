#! /usr/bin/env python
##################################################################################
#  gAnim8 and the gAnim8 Conversion Tool are distributed under the W3C License.
#  Copyright 2003 Erica Andrews (PhrozenSmoke@yahoo.com).  
#  All rights reserved.
#  
#  Permission to use, copy, modify, and distribute this software and its documentation, with or 
#  without modification,  for any purpose and WITHOUT FEE OR ROYALTY is hereby granted, 
#  provided these credits are left in-tact and a copy of the 'LICENSE' document accompanies 
#  all copies of this software. COPYRIGHT HOLDERS WILL NOT BE LIABLE FOR ANY DIRECT,
#  INDIRECT, SPECIAL OR CONSEQUENTIAL DAMAGES ARISING OUT OF ANY USE OF 
#  THE SOFTWARE OR DOCUMENTATION.  See the document 'LICENSE' for a complete copy
#  of this software license agreement.
##################################################################################
import os,GDK, GTK,gtk,GtkExtra,GdkImlib,gettext,sys
from gtk import *
#from gnome.ui import *  # Gnome support no longer required

global cversion
cversion="v. 0.2b"

global read_formats
read_formats=["8BIM Photoshop resource format","AVS AVS X image","BMP Microsoft Windows bitmap image","BMP24 Microsoft Windows 24bit bitmap image","DIB Microsoft Windows bitmap image","CACHE Magick Persistent Cache image format","CMYK Raw cyan, magenta, yellow, and black bytes","DCM Digital Imaging and Communications in Medicine image","DPS Display Postscript","FAX Group 3 FAX","G3 Group 3 FAX","FITS Flexible Image Transport System","FPX FlashPix Format","GIF CompuServe graphics interchange format","GIF87 CompuServe graphics interchange format (version 87a)","GRADIENT Gradual passing from one shade to another","GRADATION Gradual passing from one shade to another","GRAY Raw gray bytes","HDF Hierarchical Data Format","ICO Microsoft icon","ICON Microsoft icon","IPTC IPTC Newsphoto","BIE Joint Bilevel Image experts Group interchange format","JBG Joint Bilevel Image experts Group interchange format","JBIG Joint Bilevel Image experts Group interchange format","JPEG24 Joint Photographic Experts Group JFIF format","JPEG Joint Photographic Experts Group JFIF format","JPG Joint Photographic Experts Group JFIF format","LABEL Text image format","GRANITE Granite texture","LOGO ImageMagick Logo","NETSCAPE Netscape 216 color cube","MAP Colormap intensities and indices","MIFF Magick image format","MONO Bilevel bitmap in leastsignificantbyte first order","MTV MTV Raytracing image format","MVG Magick Vector Graphics","NULL Constant image of uniform color","PCD Photo CD","PCDS Photo CD","DCX ZSoft IBM PC multipage Paintbrush","PCX ZSoft IBM PC Paintbrush","PDB Pilot Image Format","EPDF Encapsulated Portable Document Format","PDF Portable Document Format","PCT Apple Macintosh QuickDraw/PICT","PICT Apple Macintosh QuickDraw/PICT","PICT24 24bit Apple Macintosh QuickDraw/PICT","PIX Alias/Wavefront RLE image format","PLASMA Plasma fractal image","MNG Multipleimage Network Graphics","PNG Portable Network Graphics","P7 Xv thumbnail format","PBM Portable bitmap format (black and white)","PGM Portable graymap format (gray scale)","PNM Portable anymap","PPM Portable pixmap format (color)","EPI Adobe Encapsulated PostScript Interchange format","EPS Adobe Encapsulated PostScript","EPSF Adobe Encapsulated PostScript","EPSI Adobe Encapsulated PostScript Interchange format","PS Adobe PostScript","PSD Adobe Photoshop bitmap","PWP Seattle Film Works","RGB Raw red, green, and blue bytes","RGBA Raw red, green, blue, and matte bytes","RLA Alias/Wavefront image","RLE Utah Run length encoded image","SCT Scitex HandShake","SFW Seattle Film Works","SGI Irix RGB image","STEGANO Steganographic image","RAS SUN Rasterfile","SUN SUN Rasterfile","SVG Scalable Vector Gaphics","ICB Truevision Targa image","TGA Truevision Targa image","VDA Truevision Targa image","VST Truevision Targa image","PTIF Pyramid encoded TIFF","TIF Tagged Image File Format","TIFF Tagged Image File Format","TIFF24 24bit Tagged Image File Format","TILE Tile image with a texture","TIM PSX TIM","PAL 16bit/pixel interleaved YUV","UYVY 16bit/pixel interleaved YUV","VICAR VICAR rasterfile format","VID Visual Image Directory","VIFF Khoros Visualization image","XV Khoros Visualization image","WBMP Wireless Bitmap (level 0) image","WPG Word Perfect Graphics","XBM X Windows system bitmap (black and white)","XC Constant image uniform color","PM X Windows system pixmap (color)","XPM X Windows system pixmap (color)","XWD X Windows system window dump (color)","YUV CCIR 601 4:1:1","WMF Windows Metafile"        , "PALM Pixmap Format","PICON Personal Icon", "ROSE 70x64 Truecolor Test Image","CMYKA Raw cyan, magenta, yellow, and matte bytes","APP1 Photoshop resource format","CUT Dr Hallo ", "ART PF1 1st Publisher","H Internal Format","ICC Color Profile","JP2 JPEG-2000 JP2 File Format Syntax","JPC JPEG-2000 JPC Code Stream Syntax","XCF Gimp Image","VID Visual Image Directory","DPX Digital Moving Picture Exchange","ILBM Amiga IFF","RAD Radiance Image File","MNG Multipleimage Network Graphics"]

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
    def __init__ (self,dodestroy=1) :
        write_formats=["8BIM Photoshop resource format","AVS AVS X image","BMP Microsoft Windows bitmap image","BMP24 Microsoft Windows 24bit bitmap image","DIB Microsoft Windows bitmap image","CACHE Magick Persistent Cache image format","CMYK Raw cyan, magenta, yellow, and black bytes","EPT Adobe Encapsulated PostScript with TIFF preview","FAX Group 3 FAX","G3 Group 3 FAX","FITS Flexible Image Transport System","FPX FlashPix Format","GIF CompuServe graphics interchange format","GIF87 CompuServe graphics interchange format (version 87a)","GRAY Raw gray bytes","HDF Hierarchical Data Format","HISTOGRAM Histogram of the image","IPTC IPTC Newsphoto","BIE Joint Bilevel Image experts Group interchange format","JBG Joint Bilevel Image experts Group interchange format","JBIG Joint Bilevel Image experts Group interchange format","JPEG24 Joint Photographic Experts Group JFIF format","JPEG Joint Photographic Experts Group JFIF format","JPG Joint Photographic Experts Group JFIF format","GRANITE Granite texture","LOGO ImageMagick Logo","MAP Colormap intensities and indices","MATTE MATTE format","MIFF Magick image format","MONO Bilevel bitmap in leastsignificantbyte first order","MTV MTV Raytracing image format","PCD Photo CD","PCDS Photo CD","PCL Page Control Language","DCX ZSoft IBM PC multipage Paintbrush","PCX ZSoft IBM PC Paintbrush","EPDF Encapsulated Portable Document Format","PDF Portable Document Format","PCT Apple Macintosh QuickDraw/PICT","PICT Apple Macintosh QuickDraw/PICT","PICT24 24bit Apple Macintosh QuickDraw/PICT","PNG Portable Network Graphics","PBM Portable bitmap format (black and white)","PGM Portable graymap format (gray scale)","PNM Portable anymap","PPM Portable pixmap format (color)","EPI Adobe Encapsulated PostScript Interchange format","EPS Adobe Encapsulated PostScript","EPSF Adobe Encapsulated PostScript","EPSI Adobe Encapsulated PostScript Interchange format","PS Adobe PostScript","EPS2 Adobe Level II Encapsulated PostScript","PS2 Adobe Level II PostScript","PS3 Adobe Level III PostScript","PSD Adobe Photoshop bitmap","RGB Raw red, green, and blue bytes","RGBA Raw red, green, blue, and matte bytes","SGI Irix RGB image","RAS SUN Rasterfile","SUN SUN Rasterfile","ICB Truevision Targa image","TGA Truevision Targa image","VDA Truevision Targa image","VST Truevision Targa image","PTIF Pyramid encoded TIFF","TIF Tagged Image File Format","TIFF Tagged Image File Format","TIFF24 24bit Tagged Image File Format","UIL XMotif UIL table","PAL 16bit/pixel interleaved YUV","UYVY 16bit/pixel interleaved YUV","VICAR VICAR rasterfile format","VIFF Khoros Visualization image","XV Khoros Visualization image","WBMP Wireless Bitmap (level 0) image","XBM X Windows system bitmap (black and white)","PM X Windows system pixmap (color)","XPM X Windows system pixmap (color)","XWD X Windows system window dump (color)","YUV CCIR 601 4:1:1"       , "PALM Pixmap Format","PICON Personal Icon", "ROSE 70x64 Truecolor Test Image","CMYKA Raw cyan, magenta, yellow, and matte bytes","APP1 Photoshop resource format","H Internal Format","ICC Color Profile","JP2 JPEG-2000 JP2 File Format Syntax","JPC JPEG-2000 JPC Code Stream Syntax","VID Visual Image Directory","P7 Xv Thumbnail Format",    "M2V Mpeg 2 Video Stream (Single Frame)","MPEG Mpeg 1 Video Stream (Single Frame)","MPG Mpeg 1 Video Stream (Single Frame)","OTB On-the-air Bitmap","DPX Digital Moving Picture Exchange","ILBM Amiga IFF","RAD Radiance Image File","MNG Multipleimage Network Graphics"]
        self.targets = [('STRING', 0, 0), ('text/plain', 0,1), ('text/uri-list', 0, 2) ]
	# gettext locale support
	self.xtext=gettext.NullTranslations()
	try:
		# open ganim8ctmo in /usr/share/locale
		self.xtext=gettext.Catalog("ganim8ct")
	except:
		self.xtext=gettext.NullTranslations()
	self.tips=GtkTooltips()
	global cversion
        global read_formats
        read_formats.sort()
        write_formats.sort()
	convwindow =GtkWindow (GTK.WINDOW_TOPLEVEL)
        convwindow.set_position (GTK.WIN_POS_CENTER)
        if dodestroy==1: convwindow.connect("destroy",quit)
	if dodestroy==1: convwindow.set_wmclass("ganim8ct","gAnim8CT")
        else: convwindow.connect("destroy",self.hideit)
	self._root = convwindow
	convwindow.realize ()
        self.sel_file=None
        self.sel_file2=None
	convwindow.set_title ("gAnim8 Conversion Tool "+cversion)
	convwindow.set_position (GTK.WIN_POS_NONE)
	self.convwindow = convwindow
	hbox1 = GtkHBox (0, 0)
	self.hbox1 = hbox1
	vbox1 = GtkVBox (0, 0)
	self.vbox1 = vbox1
        try:
          header=self.loadImage(getPixDir()+"ganim8_ct.gif")
        except:
	  header =GtkLabel ("gAnim8 Conversion Tool")
        if not header: header =GtkLabel ("gAnim8 Conversion Tool")
	vbox1.pack_start (header, 1, 1, 5)
	vbox1.pack_start (GtkHSeparator(), 1, 1, 1)

	table1=GtkTable(2,2,0)
	table1.set_border_width(1)

	vbox2 = GtkVBox (0, 0)
	label3 =GtkLabel (self.trans_text("Convert FROM:"))
	label3.set_justify (GTK.JUSTIFY_LEFT)
	label3.set_alignment ( 0, 0.5)
	vbox2.pack_start ( label3, 0, 0, 2)
	label6 =GtkLabel (self.trans_text("Type")+":   ")
	label6.set_alignment ( 0, 0.5)
	table1.attach(label6,0,1,0,1,(GTK.EXPAND+GTK.FILL),(GTK.EXPAND+GTK.FILL),0,0)
	fromtype = GtkCombo ()
        fromtype.set_popdown_strings(read_formats)
	self.fromtype = fromtype
	fromtypeentry = fromtype.entry
        fromtypeentry.set_text("JPEG Joint Photographic Experts Group JFIF format")
        fromtypeentry.set_editable(0)
	self.fromtypeentry = fromtypeentry
	table1.attach(fromtype,1,2,0,1,(GTK.EXPAND+GTK.FILL),(GTK.EXPAND+GTK.FILL),0,0)
	hbox3 = GtkHBox (0, 0)
	hbox3.set_spacing(2)
	label7 =GtkLabel (self.trans_text("File")+": ")
	label7.set_alignment ( 0, 0.5)
	table1.attach(label7,0,1,1,2,(GTK.EXPAND+GTK.FILL),(GTK.EXPAND+GTK.FILL),0,0)
	fromfileentry = GtkEntry ()
	self.fromfileentry = fromfileentry
        fromfileentry.set_usize(210,-2)
        self.fromfileentry.drag_dest_set(gtk.DEST_DEFAULT_ALL, self.targets, gtk.GDK.ACTION_COPY | gtk.GDK.ACTION_MOVE)
	self.fromfileentry.connect("drag_drop",self.setDrag)
	self.fromfileentry.connect("drag_begin",self.setDrag)
	self.fromfileentry.connect("drag_data_received",self.setDrag)
	hbox3.pack_start (fromfileentry, 1, 1, 0)
	frombrowse =self.getPixButton (getPixDir()+"gtksnap_open.xpm",self.trans_text("Browse..."),0)
        frombrowse.connect("clicked",self.getFromFile)
	self.frombrowse = frombrowse
	hbox3.pack_start ( frombrowse, 0, 1, 0)
	table1.attach(hbox3,1,2,1,2,(GTK.EXPAND+GTK.FILL),(GTK.EXPAND+GTK.FILL),0,0)
	vbox2.pack_start (table1, 0, 0, 0)
	vbox2.pack_start (GtkHSeparator(), 0,0, 7)
	vbox1.pack_start ( vbox2, 1, 1, 0)
	vbox3 = GtkVBox (0, 0)

	label5 =GtkLabel (self.trans_text("Convert TO:"))
	label5.set_alignment ( 0, 0.5)
	table2=GtkTable(2,2,0)
	table2.set_border_width(1)
	vbox3.pack_start ( label5, 0, 0, 0)
	label8 =GtkLabel (self.trans_text("Type")+":   ")
	label8.set_alignment (0, 0.5)
	table2.attach(label8,0,1,0,1,(GTK.EXPAND+GTK.FILL),(GTK.EXPAND+GTK.FILL),0,0)
	totype =GtkCombo ()
        totype.set_popdown_strings(write_formats)
	self.totype = totype
	totypeentry = totype.entry
        totypeentry.set_text("GIF CompuServe graphics interchange format")
        totypeentry.set_editable(0)
	self.totypeentry = totypeentry
	table2.attach(totype,1,2,0,1,(GTK.EXPAND+GTK.FILL),(GTK.EXPAND+GTK.FILL),0,0)
	hbox5 = GtkHBox (0, 0)
	hbox5.set_spacing(2)
	label9 =GtkLabel (self.trans_text("File")+": ")
	label9.set_alignment ( 0, 0.5)
	table2.attach(label9,0,1,1,2,(GTK.EXPAND+GTK.FILL),(GTK.EXPAND+GTK.FILL),0,0)
	tofileentry = GtkEntry ()
        tofileentry.set_usize(210,-2)
	self.tofileentry = tofileentry
        self.tofileentry.drag_dest_set(gtk.DEST_DEFAULT_ALL, self.targets, gtk.GDK.ACTION_COPY | gtk.GDK.ACTION_MOVE)
	self.tofileentry.connect("drag_drop",self.setDrag)
	self.tofileentry.connect("drag_begin",self.setDrag)
	self.tofileentry.connect("drag_data_received",self.setDrag)
	hbox5.pack_start (tofileentry, 1, 1, 0)
	tobrowse=self.getPixButton (getPixDir()+"gtksnap_open.xpm",self.trans_text("Browse..."),0)
        tobrowse.connect("clicked",self.getToFile)

	hbox5.pack_start ( tobrowse, 0, 0, 0)
	table2.attach(hbox5,1,2,1,2,(GTK.EXPAND+GTK.FILL),(GTK.EXPAND+GTK.FILL),0,0)
	vbox3.pack_start (table2, 1, 1, 0)
	vbox3.pack_start (GtkHSeparator(), 0,0, 6)
	vbox1.pack_start (vbox3, 0, 0, 0)
	hbox7 = GtkHBox (1, 0)
	hbox7.set_spacing(3)
	compbutt = self.getPixButton (getPixDir()+"gtksnap_compress.xpm","Compress GIF",1)
        compbutt.connect("clicked",self.doCompress)      
	self.compbutt = compbutt
        self.compbutt.set_sensitive(0)
	hbox7.pack_start (compbutt, 1, 1, 3)
	convertbutt =self.getPixButton (getPixDir()+"gtksnap_save.xpm","CONVERT",1)
        convertbutt.connect("clicked",self.startConvert)
	hbox7.pack_start (convertbutt, 1, 1, 3)
	vbox1.pack_start (hbox7, 0, 1, 4)
	hbox6 = GtkHBox (0, 0)
	aboutbutt = GtkButton (self.trans_text("About..."))
        aboutbutt.connect("clicked",self.showAbout)
	hbox6.pack_start ( aboutbutt, 1, 1, 3)
	closebutt = GtkButton (self.trans_text("Exit"))
        if dodestroy==1: closebutt.connect("clicked",quit)
        else: closebutt.connect("clicked",self.hideit)
	hbox6.pack_start (closebutt, 1, 1, 3)
	vbox1.pack_start ( hbox6, 0, 0, 4)
	hbox1.pack_start (vbox1, 0, 0, 3)
	imagewindow =GtkScrolledWindow ()
	self.imagewindow = imagewindow
        toplay=GtkLayout(None,None)
        self.toplay=toplay
        toplay.set_size(200,500)
        toplay.set_hadjustment(imagewindow.get_hadjustment())
	imagewindow.add(toplay)
	hbox1.pack_start (imagewindow, 1, 1, 0)
        vbox20=GtkVBox(0,0)
        vbox20.pack_start(hbox1,1,1,2)
	statusbar = GtkStatusbar ()
	self.statusbar = statusbar
        statid=statusbar.get_context_id("ganim8cv")
        self.statid=statid
        vbox20.pack_start(statusbar,0,0,2)
	convwindow.add (vbox20)
        vbox20.drag_dest_set(gtk.DEST_DEFAULT_ALL, self.targets, gtk.GDK.ACTION_COPY | gtk.GDK.ACTION_MOVE)
	vbox20.connect("drag_drop",self.setWinDrag)
	vbox20.connect("drag_begin",self.setWinDrag)
	vbox20.connect("drag_data_received",self.setWinDrag)
        convwindow.set_usize(585,-2)
        self.showStatus(self.trans_text("Copyright 2003 by Erica Andrews. All rights reserved."))
	self.setTip(fromtypeentry,"Select the image format to convert FROM")
	self.setTip(fromtype,"Select the image format to convert FROM")
	self.setTip(frombrowse,"Select the image you wish to convert")
	self.setTip(fromfileentry,"Select the image you wish to convert")
	self.setTip(aboutbutt,"About")
	self.setTip(closebutt,"Exit")
	self.setTip(compbutt,"Compress the GIF (if applicable)\nRequires the optional Gifsicle program to be on your path.")
	self.setTip(convertbutt,"Convert the image")
	self.setTip(tobrowse,"Select the file to save the new image to")
	self.setTip(tofileentry,"Select the file to save the new image to")
	self.setTip(totype,"Select the image format to convert TO")
	self.setTip(totypeentry,"Select the image format to convert TO")

	convwindow.show_all ()

    def setTip(self,widget,tip_str):
	self.tips.set_tip(widget,self.trans_text(tip_str))

    def setWinDrag(self,*args):
	drago=args
	if len(drago)<7: 
		args[0].drag_highlight()
		#args[0].grab_focus()
	else: 
		try:
			self.fromfileentry.set_text(""+str(drago[4].data).replace("\r\n","").replace("\x00","").replace("file:",""))
		except: 
			pass
		args[0].drag_unhighlight()

    def trans_text(self,some_string):
	return self.xtext.gettext(some_string.replace("_",""))

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

    def showAbout(self,*args):
        global cversion
        ih2=icehelp2()
        ih2.setHelpText("Copyright 2003 Erica Andrews (PhrozenSmoke@yahoo.com). All rights reserved.\n\nhttp://ganim8.sourceforge.net.  The gAnim8 Conversion Tool is part of gAnim8, a suite for building, viewing, and editing animated GIFS and small videos.  The gAnim8 Conversion Tool allows easy conversion from one image format to another via the \"convert\" utility from ImageMagick.  Technically, it\"s just a quick frontend for ImageMagick's 'convert' utility, but may be useful nonetheless.  NOTE:  The gAnim8 Conversion Tool  is still EXPERIMENTAL, but seems to be working rather well.   \n\nSome image formats listed may not be supported on your system and may fail to convert or even crash this program. If you have trouble converting to very obscure formats, please check the ImageMagick documentation for additional information on extra filters and third-party programs which may be required for the format. Also, make sure your have a recent version of ImageMagick.\n\nRunning the program:  You should be able to launch the program by simply executing 'ganim8ct', or by executing 'python /usr/share/gAnim8/gAnim8ConversionTool.py'.\n\n \nThis program requires Python 2.2 or better, Python Numeric, PyGTK 0.6.9 or better,  Gtk+ 1.2.8-4 or better, Imlib 1.9.8.1-6 or better, ImageMagick 5.2.3 or better (5.4.6 or better recommended).  Recommended but not required: gifsicle 1.3.0 or better for GIF compression.\n\n\nGETTING GIFSICLE:\nYou can get the latest Gifsicle source code and binary packages (RPM) from  http://www.lcdf.org/gifsicle/\n\nNOTE: This program is built on top of Gifsicle, so you can do nothing with gAnim8 until you get Gifsicle:\nhttp://www.lcdf.org/gifsicle/\n\nThere are some RPMS for Gifsicle on  speakeasy.rpmfind.net and http://www.lcdf.org/gifsicle/\n\n\nGETTING IMAGEMAGICK:\nThe ImageMagick package can be found on both speakeasy.rpmfind.net and www.imagemagick.org .\n\n\ngAnim8, the gAnim8 Conversion Tool, and GtkSnapshot are distributed under the W3C License. This program and its source may be freely modified and redistributed for non-commercial,  non-profit use only. Permission to use, copy, modify, and distribute this software and its documentation, with or without modification,  for any purpose and WITHOUT FEE OR ROYALTY is hereby granted, provided these credits are left in-tact and a copy of the 'LICENSE' document accompanies all copies of this software. COPYRIGHT HOLDERS WILL NOT BE LIABLE FOR ANY DIRECT,INDIRECT, SPECIAL OR CONSEQUENTIAL DAMAGES ARISING OUT OF ANY USE OF THE SOFTWARE OR DOCUMENTATION.  See the document 'LICENSE' for a complete copyof this software license agreement.")
        ih2.icehelp.show_all()

    def hideit(self,*args):
      os.popen("rm -f core") # Delete any core dumps
      os.popen("rm -f core.*") # Delete any core dumps
      self.convwindow.hide()
      self.convwindow.unmap()

    def showStatus(self,statustext,*args):
      global cversion 
      self.statusbar.push(self.statid," "+self.trans_text(str(statustext)))

    def extractDirectory(self, fdir):
      if not fdir: return "./"
      f=fdir
      if not f.endswith("/"): f=f[0:f.rfind("/")+1].strip()
      if f.find("/")== -1: f="./"
      return f

    def getToFile(self,*args):
      if self.sel_file:
       try:
         os.popen("rm -f core") # Delete any core dumps
         os.popen("rm -f core.*") # Delete any core dumps
         os.chdir(self.extractDirectory(self.sel_file))
       except:
         pass
      f=GtkExtra.file_sel_box(self.trans_text("Select A File..."))
      if not f: return
      self.sel_file=f      
      if not f.endswith("/"): self.tofileentry.set_text(str(f))

    def getFromFile(self,*args):
      if self.sel_file2:
       try:
         os.popen("rm -f core") # Delete any core dumps
         os.popen("rm -f core.*") # Delete any core dumps
         os.chdir(self.extractDirectory(self.sel_file2))
       except:
         pass
      f=GtkExtra.file_sel_box(self.trans_text("Select A File..."))
      if not f: return
      self.sel_file2=f      
      if not f.endswith("/"): self.fromfileentry.set_text(str(f))

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

    def loadImage(self, picon):
       try:
         i=GdkImlib.Image(str(picon).strip().replace(" ","\\ "))
         i.render()
         i.changed_image() # reload from disk, not GdkImlib cache, force GdkImlib to refresh
         p=GdkImlib.GtkPixmap(i.get_pixmap()[0])
         p.show_all()
         return p
       except:
         return None

    def addButton (self,picon) :  
      b=self.getIconButton(picon)
      try:
         self.toplay.remove(self.imagebutt)
         self.imagebutt.destroy()
      except:
         pass
      self.imagebutt=b
      b.show_all()
      self.toplay.put (b, 2, 2)
      self.toplay.set_size(b.size_request()[0]+30,b.size_request()[1]+30)
      self.toplay.set_size(b.size_request()[0]+30,b.size_request()[1]+30)
      self.toplay.drag_source_set(gtk.GDK.BUTTON1_MASK, self.targets, gtk.GDK.ACTION_COPY)
      self.toplay.connect("drag_begin",self.setDragAway)
      self.toplay.connect("drag_data_get",self.setDragAway)
      self.toplay.show_all()

    def setDragAway(self,*args):
	drago=args
	if len(drago)<3: 
		args[0].drag_highlight()
		args[0].grab_focus()
	else: 
		global CURRENT_FILE
		drago[2].set(drago[2].target,8,""+CURRENT_FILE)

    def getIconButton (self,picon) :  
       try:
         b=GtkFrame()
         b.add(self.loadImage(picon))
         b.show_all()
         return b
       except:
         return GtkLabel("  "+self.trans_text("Your image was successfully converted, but is not previewable with GdkImlib.")+"  ("+str(picon).strip()+")")

    def doCompress (self,*args) :  
      global CURRENT_FILE
      global IS_GIF
      if CURRENT_FILE==None: return
      if not IS_GIF==1: return
      if not haveGifsicle():
       self.showMessage("The 'gifsicle' binary from Gifsicle was not found on your PATH.\nThe gAnim8 Conversion Tool requires Gifsicle to compress GIFs.\nIf you do not have Gifsicle, you may still convert images, but cannot compress GIFs.\nGifsicle can be downloaded from: http://www.lcdf.org/gifsicle/ .\nGIF compression is an OPTIONAL feature of the gAnim8 Conversion Tool.")
       return
      f=os.popen("gifsicle "+CURRENT_FILE+" -O1 -o "+CURRENT_FILE)
      while not f.readline().strip()=="":
         pass
      if os.path.exists(CURRENT_FILE): 
         self.addButton(CURRENT_FILE)
         fsize="unknown"
         try:
           fsize=os.path.getsize(CURRENT_FILE)
         except:
           pass
         self.showStatus(self.trans_text("GIF Compressed")+": "+CURRENT_FILE+" : "+str(fsize)+" "+self.trans_text("bytes"))
      else: 
         self.showMessage("An unknown error occurred compressing your GIF file with Gifsicle.\nReverting back to your original, uncompressed GIF file.")
         self.fromfileentry.set_text(CURRENT_FILE)
         self.startConvert()

    def showMessage (self,mess) :  
      GtkExtra.message_box("gAnim8 Conversion Tool",self.trans_text(str(mess)),["Ok"])

    def showMessage2(self,mess) :  
      ic=icehelp2("gAnim8 Conversion Tool")
      ic.setHelpText(mess)
      ic.icehelp.show_all()

    def showConfirm (self,mess) :  
      i=GtkExtra.message_box("GtkSnapshot",str(mess),[self.trans_text("YES"),self.trans_text("NO")])
      if i==self.trans_text("YES"): return 1
      return 0

    def startConvert (self,*args) : 
      self.showStatus("Converting....please wait.")
      if len(self.tofileentry.get_text().strip())>0:
	if os.path.exists(self.tofileentry.get_text().strip()):
		if not self.showConfirm(self.trans_text("The selected file already exists")+":\n"+self.tofileentry.get_text().strip()+"\n"+self.trans_text("Do you wish to overwrite it?"))==1:
			self.showStatus("Canceled.")
			return
      timeout_add(20,self.startConvert1)

    def startConvert1 (self,*args) :  
      global IS_GIF
      stat=doConversion(self.fromfileentry.get_text(),  self.fromtype.entry.get_text() , self.tofileentry.get_text(),  self.totype.entry.get_text())
      if not stat[0]: 
        stat=doConversion(self.fromfileentry.get_text(),  self.fromtype.entry.get_text() , self.tofileentry.get_text(),  self.totype.entry.get_text(),1)
        if not stat[0]:
          self.showMessage2(self.trans_text("Problem Converting")+":\n\n"+self.trans_text(stat[1])+"\n\n"+self.trans_text("Be sure you have selected the correct 'Convert FROM' format.\nBe sure that you have the 'convert' binary from ImageMagick\n(version 5.2.3 or better) on your PATH.")+"\n")
          self.showStatus("Done. Image conversion failed.")
          self.compbutt.set_sensitive(0)
          return
      if stat[0]:
        self.addButton(stat[2])
        self.showStatus(self.trans_text("Image converted")+": "+str(stat[2])+" : "+str(stat[3])+" "+self.trans_text("bytes"))
        if IS_GIF==1: self.compbutt.set_sensitive(1)
        else: self.compbutt.set_sensitive(0)
        if len(stat[1]): self.showMessage2(self.trans_text("Your image was successfully converted with the following warnings/messages")+":\n\n"+stat[1])
      os.popen("rm -f core")
      os.popen("rm -f core.*")

class icehelp2 :
    def __init__ (self,mytitle="About gAnim8 Conversion Tool") :
	icehelp = GtkWindow (GTK.WINDOW_TOPLEVEL)
        icehelp.realize()
	icehelp.set_title (mytitle)
	icehelp.set_policy (1, 1, 0)
	icehelp.set_position (GTK.WIN_POS_CENTER)
        icehelp.set_usize(300,375)
        icehelp.connect("destroy",self.hideh)
	self.icehelp = icehelp
	vbox1 = GtkVBox (0, 0)
	vbox1.set_border_width (11)
	scrolledwindow1 = GtkScrolledWindow ()
	helptext =GtkText ()
	helptext.set_editable (0)
        helptext.set_word_wrap(1)
	self.helptext = helptext
	scrolledwindow1.add (helptext)
	vbox1.pack_start (scrolledwindow1, 1, 1, 0)
	closebutt = GtkButton("Close  (Cerrar)")
        closebutt.connect("clicked",self.hideh)
	vbox1.pack_start (closebutt, 0, 0, 13)
	icehelp.add (vbox1)

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
  return IMAGE_MAGICK

def changeType(TOTYPE):
  global im_synonyms
  if im_synonyms.has_key(TOTYPE): return im_synonyms[TOTYPE]
  return TOTYPE

def doConversion(from_file,from_type,to_file,to_type,special=0):
  if not haveImageMagick(): return [0,"The 'convert' binary from the ImageMagick program is NOT on your PATH\nImageMagick's 'convert' utility is required for all image conversions and must be on your PATH.\nImageMagick 5.2.3 or better is required. Version 5.4.6.1 or better is recommended.\nYou can find versions of ImageMagick on speakeasy.rpmfind.net or www.imagemagick.org ."]
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
       if not os.path.exists(tofile): tofile=tofile+".0" # This happens when we convert from a multi-framed image (animated gif, etc)
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


def run (sfile="") :
    cw=convwindow ()
    cw.fromfileentry.set_text(sfile)
    gtk.mainloop()

def quit(*args):
  os.popen("rm -f core") # Delete any core dumps
  os.popen("rm -f core.*") # Delete any core dumps
  gtk.mainquit()

def getPixDir() :
	return sys.argv[0][0:sys.argv[0].rfind(os.sep)+1]+"pixmaps"+os.sep	

if __name__ == "__main__" :
    start_file=""
    if len(sys.argv) > 1:
      argy=sys.argv[1].strip()
      if (argy.lower()=="help") or (argy.lower()=="-help") or (argy.lower()=="--help"):  
        print "\n\nUsage:  ganim8ct  [file_name]         where [file_name] is the optional file to open immediately upon starting.\n"
        sys.exit(0)
      start_file=argy
    run (start_file)