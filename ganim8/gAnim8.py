#! /usr/bin/env python
##################################################################################
#  gAnim8 is distributed under the W3C License.
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
import os,sys, string, GDK, GTK, copy,gtk,math,random,GtkExtra,GdkImlib,gAnim8ConversionTool,GtkSnapshot,gettext,pyprint
from gtk import *
from gnome.ui import *
from threading import Thread

# gettext locale support
global myxtext
myxtext=gettext.NullTranslations()
try:
	# open ganim8.mo in /usr/share/locale
	myxtext=gettext.Catalog("ganim8")
except:
	myxtext=gettext.NullTranslations()

def translate(some_string):
	global myxtext
	return myxtext.gettext(some_string)


global version
version="v. 0.3b-1"
global editor_cmd
editor_cmd="gimp-remote %f"  # the '%f' is required as a variable for the name of the file
global mpeg_cmd
global avi_cmd
global swf_cmd
global rm_cmd
global asf_cmd
global wmv_cmd
global m4v_cmd
mpeg_cmd="mplayer -vo sdl %f -loop 30"
avi_cmd="mplayer -vo sdl %f -loop 30"
asf_cmd="mplayer -vo sdl %f -loop 30"
wmv_cmd="mplayer -vo sdl %f -loop 30"
m4v_cmd="mplayer -vo sdl %f -loop 30"
swf_cmd="gflashplayer %f"
rm_cmd="realplay %f"
global first_run
first_run=0
global options_file
options_file=".ganim8_options"
global fs
global sel_file
global temp_dir
global load_file
load_file=None
global save_file
save_file=None
global extract_file
extract_file=None
sel_file=None
fs=None
temp_dir="/tmp/"
global statusbar
global statid
global gifsicle_installed
global gifview_installed
gifsicle_installed=0
gifview_installed=0
global iminfo
iminfo=None
global active_frame
active_frame=None
global image_info
image_info={}
global col_palette
col_palette={}
global frame_to_activate
frame_to_activate=0
global alpha_num_map
alpha_num_map={"0":0,"1":1,"2":2,"3":3,"4":4,"5":5,"6":6,"7":7,"8":8,"9":9,"a":10,"b":11,"c":12,"d":13,"e":14,"f":15}
global extract_no
extract_no=0
global e_file
e_file=None
global silent_cp
silent_cp=None
global color_selection
color_selection=None
global active_color_button
active_color_button=None
global palette_window
palette_window=None
global current_palette_edit
current_palette_edit=None
global trans_sig
trans_sig=None
global real_names
real_names={}
global insert_at
insert_at=0
global rot_window
rot_window=None
global do_save_as
do_save_as=1
global default_comment
default_comment=translate("Created by gAnim8 for Linux.")
global startup_file # a file issued on the command line '%f'
startup_file=None
global editor_win
editor_win=None
global dim_window
dim_window=None
global dim_scalex
global dim_scaley
dim_scalex=0
dim_scaley=0
global LOCKER
LOCKER=0
global CHANGED
CHANGED=0
global NEW_PENDING
global QUIT_PENDING # used for preventing hanging and minimizing Gtk "critical" messages on shutdown
QUIT_PENDING=0
NEW_PENDING=0
global GANIM
GANIM=None
global TIPS
TIPS=None
global LOAD_PENDING
LOAD_PENDING=0
global mpegdec
mpegdec=None
global frameview
frameview=None
global HAVE_MPEG
HAVE_MPEG=0
global HAVE_MPEGE
HAVE_MPEGE=0
global MPEG_OPEN_WARN
MPEG_OPEN_WARN=0
global MPEG_SAVE_WARN
MPEG_SAVE_WARN=0
global GIF_WARN
GIF_WARN=0
global movie1_file
movie1_file=None
global IS_DETACHED
IS_DETACHED=0
global UNSUPPORTED
UNSUPPORTED=['.wma','.asx','.py','.pyc','.pyo','.ttf','.doc','.text','.txt','.tex','.mp3','.mp4','.mpeg3','.m3v','.m4v','.mpeg4','.htm','.html']

class ganim :
    def __init__ (self) :
        global version
        global GANIM
        GANIM=self
        global mpegdec
	self.im_targets = [('STRING', 0, 0), ('text/plain', 0,1), ('text/uri-list', 0, 2) ]
        mpegdec=MovieDecoder()
	ganim = GtkWindow(GTK.WINDOW_TOPLEVEL)
	self._root = ganim
	ganim.realize()
	ganim.set_title ("gAnim8 "+version)
	ganim.set_position (GTK.WIN_POS_CENTER)
        global TIPS
        TIPS=GtkTooltips()
        images={}
        load_errors=[]
        self.load_errors=load_errors
        self.images=images
	self.session_created={}
	self.ganim = ganim
	vbox1 = GtkVBox (0, 0)
	vbox1.set_border_width (4)
	self.vbox1 = vbox1
	menubar = GtkMenuBar ()
	self.menubar = menubar

        # MENUS
        ag = GtkAccelGroup()
        itemf = GtkItemFactory(GtkMenuBar, "<main>", ag)
        itemf.create_items([
            # path              key           callback    action#  type
  (translate("/_File"),  "<alt>F",  None,  0, "<Branch>"),
  (translate("/_File/_New..."), "<control>N", self.resetDefaults, 1, ""),
  (translate("/_File/_Open..."), "<control>O", self.askGifOpen, 2, ""),
  (translate("/_File/_Extract All Frames To Directory..."), "<control>E", self.extractToDir, 9, ""),
  (translate("/_File/sep5"), None,  None,  44, "<Separator>"),
  (translate("/_File/_Save Animated GIF"), "<control>S", self.startSave, 10, ""),
  (translate("/_File/Save _As Animated GIF..."), None, self.doSaveAs, 11, ""),
  (translate("/_File/sep1"), None,  None,  3, "<Separator>"),
  (translate("/_File/Save As VIDEO"), None, None, 699, "<Branch>"),
  (translate("/_File/Save As VIDEO/Save As MPEG-1..."), None, self.askSaveMpeg, 700, ""),
  (translate("/_File/Save As VIDEO/Save As AVI..."), None, self.askSaveAvi, 701, ""),
  (translate("/_File/Save As VIDEO/Save As SWF (Shockwave Flash)..."), None, self.askSaveSwf, 702, ""),
  (translate("/_File/Save As VIDEO/Save As RM (RealPlayer Video)..."), None, self.askSaveRm, 703, ""),
  (translate("/_File/Save As VIDEO/Save As ASF..."), None, self.askSaveAsf, 704, ""),
  (translate("/_File/Save As VIDEO/Save As WMV (Windows Media Video)..."), None, self.askSaveWmv, 705, ""),
  (translate("/_File/Save As VIDEO/Save As M4V..."), None, self.askSaveM4v, 706, ""),
  (translate("/_File/sep9"), None,  None,  95, "<Separator>"),
  (translate("/_File/Preview/Preview As Animated _GIF"), "<control>P", self.doGifPreview, 800, ""),
  (translate("/_File/Preview/Preview As _MPEG-1 Video"), "<control>M", self.initMpegPreview, 801, ""),
  (translate("/_File/Preview/Preview As AVI Video"),None, self.initAviPreview, 802, ""),
  (translate("/_File/Preview/Preview As SWF (Shockwave Flash) Video"),None, self.initSwfPreview, 803, ""),
  (translate("/_File/Preview/Preview As RM (RealPlayer) Video"),None, self.initRmPreview, 804, ""),
  (translate("/_File/Preview/Preview As ASF Video"),None, self.initAsfPreview, 805, ""),
  (translate("/_File/Preview/Preview As WMV (Windows Media) Video"),None, self.initWmvPreview, 806, ""),
  (translate("/_File/Preview/Preview As M4V Video"),None, self.initM4VPreview, 807, ""),

  (translate("/_File/sep2"), None,  None,  5, "<Separator>"),
  (translate("/_File/_Print this image frame..."), "<control>P", self.doPrintIm,  479, ""),

  (translate("/_File/sep27"), None,  None,  5, "<Separator>"),
  (translate("/_File/_Quit"), "<control>Q", quit,  4, ""),

  (translate("/_Options"),  "<alt>O",  None,  20, "<Branch>"), 
  (translate("/_Options/_Default Image Editor..."), None, showEditorWin, 21, ""),
  (translate("/_Options/_MPEG-1 Video Previewer..."), None, showCmdLineOther, 2003, ""),
  (translate("/_Options/_AVI Video Previewer..."), None, showCmdLineOther, 2003, ""),
  (translate("/_Options/_SWF (Shockwave Flash) Video Previewer..."), None, showCmdLineOther, 2004, ""),
  (translate("/_Options/_RM (RealPlayer) Video Previewer..."), None, showCmdLineOther, 2005, ""),
  (translate("/_Options/AS_F Video Previewer..."), None, showCmdLineOther, 2006, ""),
  (translate("/_Options/_WMV (Windows Media) Video Previewer..."), None, showCmdLineOther, 2007, ""),
  (translate("/_Options/M_4V Video Previewer..."), None, showCmdLineOther, 2008, ""),
  (translate("/_Tools"),  "<alt>T",  None,  32, "<Branch>"), 
  (translate("/_Tools/_View Supported File Types"), "<shift>F", self.showSupported, 35, ""),
  (translate("/_Tools/My _Briefcase"), "<alt>B", self.showBriefcase, 439, ""),
  (translate("/_Tools/gAnim8 Image _Conversion Tool"), "<shift>C", self.doConversionTool, 33, ""),
  (translate("/_Tools/_Screenshot..."), "<alt>S", self.doSnapshot, 330, ""),
  (translate("/_Tools/_Record Video..."), "<shift>R", self.openRecorder, 339, ""),
  (translate("/This _Animation Sequence"),  "<alt>A",  None,  25, "<Branch>"), 
  (translate("/This _Animation Sequence/Show _Information"), "<control>I", self.showAnimationInfo, 26, ""),
  (translate("/This _Animation Sequence/Set _Dimensions..."), "<alt>D", self.showDimensions, 27, ""),
  (translate("/_Help"),  "<alt>H",  None, 16, "<LastBranch>"), 
  (translate("/_Help/_License Agreement"), "F1", self.showLicense, 30, ""),
  (translate("/_Help/_About gAnim8..."), "F2", self.showAbout, 17, ""),
  (translate("/_Help/About _Gifsicle..."), "F3", self.showAboutGifsicle, 18, ""), 
  (translate("/_Help/sep45"), None,  None,  49, "<Separator>"),
  (translate("/_Help/gAnim8 Help"), "F4", self.showHelp, 19, ""),
  (translate("/_Help/What's New?"), "F9", self.showWhatsNew, 989, ""),
  (translate("/_Help/First-time Users"), "F8", self.showFirstRunHelp, 919, ""),
  (translate("/_Help/System Requirements"), "F5", self.showSysReq, 48, ""),
  (translate("/_Help/Video Support"), "F6", self.showMpegHelp, 42, ""),
  (translate("/_Help/sep46"), None,  None,  51, "<Separator>"),
  (translate("/_Help/Bugs and Comments"), "F7", self.showBugHelp, 52, "")

        ])
        ganim.add_accel_group(ag)
        menubar = itemf.get_widget("<main>")
        menubar.show()
       
        try:
          rc_parse(getBaseDir()+"framerc")
        except:
          pass
        global color_selection
        color_selection=GtkColorSelectionDialog()
        color_selection.ok_button.connect("clicked",self.acceptPaletteEdit)
        color_selection.cancel_button.connect("clicked",self.hidePaletteEdit)
	vbox1.pack_start(menubar, 0, 0, 0)
        vbox1.set_spacing(3)
        opts_box=GtkHBox(0,0)
        opts_box.set_spacing(17)
        self.opts_box=opts_box
        left_box=GtkVBox(1,0)
        right_box=GtkVBox(1,0)
        self.right_box=right_box

        try:
           header=loadImage("./pixmaps/ganim8_logo.xpm")
        except: 
           header=GtkLabel("gAnim8")

        self.trans_sig=None
        self.bg_sig=None

	hbox1 = GtkHBox (0, 0) # left side
	self.hbox1 = hbox1
	loopcheck = GtkCheckButton(translate("Loop"))
        setTip(loopcheck,"Turn looping on/off.  ON to make the animation play over and over. OFF to make the animation play once only.")
	self.loopcheck = loopcheck
	hbox1.pack_start(loopcheck, 0, 0, 0)
	hbox1.pack_start(GtkLabel("   "), 1, 1, 0)
	hbox1.pack_start(GtkLabel(translate("Count")+": "), 0, 0, 0)
	loopc_entry = GtkEntry()
	loopc_entry.set_usize (50, -2)
	self.loopc_entry = loopc_entry
        setTip(loopc_entry,"Enter how many times you want the animation to loop. Enter 0 to make the animation loop forever.")
	hbox1.pack_start(loopc_entry, 0, 1, 0)
	hbox1.pack_start(GtkLabel("      "), 1, 1, 0)
	hbox1.pack_start(GtkLabel(translate("Delay")+": "), 0, 0, 0)
	loopd_entry = GtkEntry()
        setTip(loopd_entry,"Enter the length of time, in hundredths of a second between frame changes in the Animated GIF. WARNING: Setting this value to over 50 may make your animation appear to not be moving at all. This setting has NO effect on videos created by gAnim8")
	loopd_entry.set_usize (35, -2)
	self.loopd_entry = loopd_entry
	hbox1.pack_start(loopd_entry, 0, 0, 0)
        hbox1.pack_start(GtkLabel(" "),1,1,0)
	left_box.pack_start(hbox1, 0, 0, 2)

        hbox1a = GtkHBox (0, 0) # right side
	hbox1a.pack_start(GtkLabel("        "), 1, 1, 0)
	previewbutt = getPixmapButton("./pixmaps/ganim8_preview.xpm","Preview")
        previewbutt.connect("clicked",self.doGifPreview)
        setTip(previewbutt,"Build this Animated GIF and preview it with 'gifview'")
	self.previewbutt = previewbutt
	hbox1a.pack_start(previewbutt, 0, 0, 2)  ####
	spreviewbutt = getIconButton("./pixmaps/ganim8_stop.xpm","Stop")
        setTip(spreviewbutt,"Stop the preview")
        spreviewbutt.connect("clicked",self.stopGifPreview)
	self.spreviewbutt = spreviewbutt
	hbox1a.pack_start(spreviewbutt, 0, 0, 2) 
	right_box.pack_start(hbox1a, 0, 0, 2) 

	hbox2a = GtkHBox (0, 0) # left side
        hbox2a.pack_start(GtkLabel(translate("Optimization")+": "),0,0,1)
	noradio =GtkRadioButton (None, translate("None"))
        setTip(noradio,"No optimization: Your GIF file may be larger in size, but should be readable by almost any web browser.")
	noradio.set_border_width (5)
	self.noradio = noradio
	hbox2a.pack_start(noradio, 0, 0, 0)
	lightradio = GtkRadioButton(self.noradio, translate("Light"))
        setTip(lightradio,"Light optimization: Usually the best choice. Generally decreases the file size of your GIF, but leaves it readable for most web browsers.")
	lightradio.set_border_width (5)
	self.lightradio = lightradio
	hbox2a.pack_start(lightradio, 0, 0, 0)
	heavyradio = GtkRadioButton(self.noradio, translate("Heavy"))
        setTip(heavyradio, "Heavy optimization: Generally only applies to animations with transparency. Shrinks the GIF size as much as possible, but some non-standard web browsers and image viewers may have trouble displaying your image correctly.")
	heavyradio.set_border_width (5)
	self.heavyradio = heavyradio
	hbox2a.pack_start (heavyradio, 0, 0, 0)
	left_box.pack_start(hbox2a, 1, 1, 0)

	hbox3a = GtkHBox (0, 0) # left side
	hbox3a.pack_start(GtkLabel(translate("Background Color")+": "), 0, 0, 2)
	bgpicker =GnomeColorPicker ()
	self.bgpicker = bgpicker
	hbox3a.pack_start(bgpicker, 0, 0, 3)
        setTip(bgpicker,"Set the background for this animation. Does not apply to all animations, particularly not to those with transparency in the first frame. This feature applies to the WHOLE animation sequence.")
	hbox3a.pack_start(GtkLabel("  "), 1, 1, 0)
	intercheck = GtkCheckButton(translate("Interlaced"))
	self.intercheck = intercheck
	hbox3a.pack_start(intercheck, 0, 0, 6)
	left_box.pack_start(hbox3a, 0, 0, 0)

	hbox3 = GtkHBox (0, 0) # right side
        self.hbox3=hbox3
	hbox3.set_border_width (3)
	hbox3.pack_start(GtkLabel(" "), 1, 1, 0)
	hbox3.pack_start(GtkLabel(translate("Transparent Color")+": "), 0, 0, 2)
        setTip(intercheck,"Turn interlacing on/off.  This may affect the file size of your GIF by a few bytes or so. This feature applies to the WHOLE animation sequence.")
	transpicker =ColorButton((65500,65500,65500),1)
        transpicker.button.connect("clicked",self.showTransEditor)
        setTip(transpicker.button,"Choose the transparent color for the selected frame ONLY.  You must set/unset transparency for each frame in this animation sequence.")
	self.transpicker = transpicker
	hbox3.pack_start(transpicker.button, 0, 0, 0)
	right_box.pack_start(hbox3, 0, 0, 0)

	hbox33 = GtkHBox (0, 0) # right side
	hbox33.pack_start(GtkLabel(" "), 1, 1, 0)
        self.hbox33=hbox33
	transcheck = GtkCheckButton(" "+translate("Transparent"))
        setTip(transcheck,"Enable/disable transparency for the selected frame ONLY.  You must set/unset transparency for each frame in this animation sequence.")
	self.transcheck = transcheck
        global trans_sig
        trans_sig=transcheck.connect("toggled",self.toggleTransparency)
	hbox33.pack_start(transcheck, 0, 0, 0)
	right_box.pack_start(hbox33, 0, 0, 0)

	opts_box.pack_start(left_box, 0,0, 3) 
	opts_box.pack_start(header, 1,1, 3) 
	opts_box.pack_start(right_box, 0,0, 3) 

	hbox2 = GtkHBox (0, 0) # next row under header
	hbox2.set_border_width (4)
	hbox2.pack_start(GtkLabel(translate("Comment")+":    "), 0, 0, 0)
	comment_entry = GtkEntry()
        setTip(comment_entry,"Set the comment to be embedded into this animation. WARNING: Large comments can greatly increase the file size of your animation. Set this comment to nothing if you are trying to shrink your file size as much as possible. Otherwise, short comments are suggested.")
	comment_entry.set_usize (175, -2)
	self.comment_entry = comment_entry
	hbox2.pack_start(comment_entry, 1, 1, 0)
        
        imbox=GtkHBox(0,0)
        self.imbox=imbox
        global iminfo
        iminfo=GtkLabel(" ")
        self.iminfo=iminfo
        imbox.pack_start(iminfo,1,1,2)
        detbutton=GtkButton(translate("Detach"))
        self.detbutton=detbutton
        imbox.pack_start(detbutton,0,0,2)
        detbutton.connect("clicked",self.checkDetach)
        setTip(detbutton,"Detach/Re-attach the frame window")

	framesscrollwin =GtkScrolledWindow ()
	framesscrollwin.set_border_width (3)
	framesscrollwin.set_usize (-2, 250)
	self.framesscrollwin = framesscrollwin
        toplay=GtkLayout(None,None)
        self.toplay=toplay
        toplay.set_size(700,700)
        toplay.set_hadjustment(framesscrollwin.get_hadjustment())
	framesscrollwin.add(toplay)

        global statusbar
	statusbar = GtkStatusbar ()
	self.statusbar = statusbar
        global statid
        statid=statusbar.get_context_id("ganim8t")

	vbox1.pack_start(GtkHSeparator (), 0, 0, 1)
	vbox1.pack_start(opts_box, 0,0, 9) 
	vbox1.pack_start(GtkHSeparator (), 0, 0, 1)
	vbox1.pack_start(hbox2, 0, 0, 0)
	vbox1.pack_start(GtkHSeparator (), 0, 0, 0)
        vbox1.pack_start(imbox,0,0,4)
	vbox1.pack_start(framesscrollwin, 1, 1, 0)
	vbox1.pack_start(statusbar, 0, 0, 0)
        framesscrollwin.drag_dest_set(gtk.DEST_DEFAULT_ALL, self.im_targets, gtk.GDK.ACTION_COPY | gtk.GDK.ACTION_MOVE)
        framesscrollwin.connect("drag_drop",self.setIMDrag)
        framesscrollwin.connect("drag_begin",self.setIMDrag)
        framesscrollwin.connect("drag_data_received",self.setIMDrag)

	ganim.add (vbox1)
	ganim.show_all()
        t=Thread(None,loadOptions)
        t.start()
        setTempDir()
        checkGifsicle()
        global startup_file
        if not startup_file: self.resetDefaults()
        else: 
          cleanupFiles()
          self.unpackIt(startup_file)
        global first_run
        if first_run: self.showFirstRunHelp()
	gtk.timeout_add(5,showStatus1)

    def doPrintIm(self,*args) :
	if active_frame:
		pyprint.printFile(active_frame.image)
	else:
		showStatus(translate("No images are selected. Nothing to print."))

    def loadTextHelp(self,file_name) :
	try:
		f=open(getBaseDir()+str(file_name))
		ff=f.read()
		f.close()
		return ff
	except:
		return translate("ERROR: Could not load help document")+":\n\n"+getBaseDir()+str(file_name)


    def showFirstRunHelp(self,*args) :
	showModalHelp(translate("VERY IMPORTANT INFORMATION FOR NEW USERS. Please note that gAnim8 works with TEMPORARY copies of all files ONLY.  This means you should have no fear of playing around with your GIFs and animations, while not having to worry about destroying the original copies.  This ALSO means that these temporary copies of your GIFs are automatically deleted from your system by gAnim8 when they are no longer needed. So, if you edit your GIFs (particularly the frames in your animations) be sure to click the 'Save...' button next to the frame and save it as a permanent file if you wish to keep those changes, as they will be automatically deleted by gAnim8 when you exit this program or start a new animation.  TO REPEAT: If you open an animation, you will be working with a COPY of that animation, not the original animated Gif.  If you add a GIF frame to an animation, you will be working with a COPY of your GIF, not the original.  By the way, temporary files are stored in either your home directory under ~/.ganim8/tmp/ or under /tmp/ (for those of you who care). You will not see this message again unless you do a fresh installation under your username, or your Options file becomes disappears or becomes damaged  or unreadable."))

    def showBugHelp(self,*args) :
      showModalHelp(translate("Send Bugs and Comments To PhrozenSmoke@yahoo.com. When reporting bugs, please tell me the versions of the following packages installed on your computer: gAnim8, python, pygtk, pygnome, gifsicle, ImageMagick, ffmpeg, gtk, Imlib, and Gnome/Gnome-libs.  Include a DETAILED description of the problem you experienced."))

    def showSysReq(self,*args) :
      showModalHelp(self.loadTextHelp("SYSTEM_REQUIREMENTS.txt"))

    def showMpegHelp(self,*args) :
      showModalHelp(self.loadTextHelp("VIDEO_SUPPORT.txt"))

    def showHelp(self,*args) :
      showModalHelp(self.loadTextHelp("GAnim8_Help.txt"))

    def showWhatsNew(self,*args) :
      showModalHelp(self.loadTextHelp("GAnim8_Whats_New.txt"))

    def showSupported(self,*args) :
      global gifsicle_installed
      sup_formats=translate("SUPPORTED SAVE FORMATS:\n")
      if gifsicle_installed: sup_formats=sup_formats+"  GIF\n"
      else: sup_formats=sup_formats+translate("  (GIF not available: 'gifsicle' not found)\n")
      if haveMpegE() and gAnim8ConversionTool.haveImageMagick(): sup_formats=sup_formats+translate("  MPEG Video\n  AVI Video\n  ASF Video\n  WMV Video\n  SWF Video\n  RealPlayer Video\n  M4V Video\n\n-------------\nSUPPORTED INPUT FORMATS:\n")
      else: sup_formats=sup_formats+translate("  (VIDEO support not available: missing ImageMagick or 'ffmpeg')\n\n-------------\nSUPPORTED INPUT FORMATS:\n")
      if gifsicle_installed: sup_formats=sup_formats+"  GIF\n"
      else: sup_formats=sup_formats+translate("  (GIF not available: 'gifsicle' not found)\n")
      if gAnim8ConversionTool.haveImageMagick():
        if haveMpeg(): sup_formats=sup_formats+translate("  MPEG Video \n  AVI Video \n  ASF Video \n  WMV Video \n  SWF Video \n  RealPlayer Video \n  M4V Video \n  MOV Video\n")
        else: sup_formats=sup_formats+"\n"+translate("  (VIDEO support not available: missing 'ffmpeg')\n\n")
        ll=copy.copy(gAnim8ConversionTool.read_formats)
        ll.sort()
        for gg in ll: 
          sup_formats=sup_formats+"  "+gg+"\n"
        ll=None
      else:  sup_formats=sup_formats+translate("  (Support for non-GIFs not available: 'convert' from ImageMagick not found)\n  (Video support not available: missing ImageMagick)" )    
      showModalHelp(sup_formats)

    def showDimensionsHelp(self,*args) :
      showModalHelp(translate("This is simple WIDTH x HEIGHT dimensions setting (in pixels). Here you can set the dimensions of the GIF animation or single-framed GIF that Gifsicle will produce (300 x 150, 20 x 10, etc.).  You can easily SCALE the size of your image (and preserve the aspect ratio) by setting only the width and leaving the height set to '0', or by setting the height and leaving the width set to '0'.  In this case, Gifsicle will automatically scale the image for you, keeping the aspect ratio intact.  For example, if you want to scale the image to a width of 300 pixels. Simply type '300' for width and leave the 'height' set to '0'.  Gifsicle will pick the best height for a width of 300.  It's that easy.  8-)"))

    def showAbout(self,*args):
        global version
        about = GnomeAbout('gAnim8',version, 'Copyright 2003', ['Erica Andrews (PhrozenSmoke@yahoo.com)'], translate('http://ganim8.sourceforge.net.\n  \ngAnim8 is a program for creating animated GIFs and small videos.   gAnim8 is  built on top of the wonderful Gifsicle program.  gAnim8 is a frontend for Gifsicle and more.  However, gAnim8 and its author are in NO way affiliated with or endorsed by Gifsicle or the authors of Gifsicle.  gAnim8 is not an  "official" extension to Gifsicle and is NOT a modification of the Gifsicle code in anyway.  gAnim8 relies on the excellent functionality of Gifsicle, but does not incorporate any of its code.  gAnim8 is not intended to be the next Gimp. It is intended to be a simple, effective GUI for creating and retouching animated GIFs and small videos. \n \nWritten with Python/PyGTK/PyGnome.     \n \nLicense: W3C (open source). View  "License" on the Help menu for more details.  This program and its source may be freely modified and redistributed for non-commercial,  non-profit use only.'))
        about.connect('destroy', about.unmap)
        about.show()

    def showAboutGifsicle(self,*args):
        global version
        about = GnomeAbout('Gifsicle','', 'Copyright by Eddie Kohler', ['Eddie Kohler <eddietwo@lcs.mit.edu>','Anne Dudfield <anne@lvld.hp.com>','David Hedbor <david@hedbor.org>','Emil Mikulic <darkmoon@connexus.net.au>','Hans Dinsen-Hansen <dino@danbbs.dk>'], 'Gifsicle and Ungifsicle are wonderful GIF manipulation programs written and copyright by Eddie Kohler <eddietwo@lcs.mit.edu>.   Neither gAnim8 or its author are in anyway affliated with or endorsed by Gifsicle/Ungifsicle or the authors of Gifsicle/Ungifsicle.  Please direct your comments and suggestions about Gifsicle to one of the authors listed above.  Gifsicle/Ungifsicle is distributed under a GPL-like license, completely separate from the W3C license under which gAnim8 is distributed.   For the Gifsicle/Ungifsicle license agreement view the \'README\' file under you gifsicle/ungifsicle directory.  You can also find the license agreement at the official Gifsicle web site: http://www.lcdf.org/gifsicle/ .  The lastest source code and pre-built binaries can also be found at that web site.  If you have downloaded the version of gAnim8 which includes a bundled binary version of Gifsicle, know that your bundled version of Gifsicle came from that site and is Gifsicle (not Ungifsicle).\n \n ')
        about.connect('destroy', about.unmap)
        about.show()

    def showLicense(self,*args) :
      showModalHelp(translate("gAnim8 and the gAnim8 Conversion Tool are distributed under the W3C License.\n\n\nW3C SOFTWARE NOTICE AND LICENSE\n\nCopyright 2003 Erica Andrews (PhrozenSmoke@yahoo.com).  All rights reserved.  http://ganim8.sourceforge.net.\n\nThis work (including software, documents, or other related items) is being provided by the copyright holders under the following license. By obtaining, using and/or copying this work, you (the licensee) agree that you have read, understood, and will comply with the following terms and conditions:\n\nPermission to use, copy, modify, and distribute this software and its documentation, with or without modification,  for any purpose and without fee or royalty is hereby granted, provided that you include the following on ALL copies of the software and documentation or portions thereof, including modifications, that you make:\n\n1. The full text of this NOTICE in a location viewable to users of the redistributed or derivative work.\n\n2. Any pre-existing intellectual property disclaimers, notices, or terms and conditions. If none exist, a short notice of the following form (hypertext is preferred, text is permitted) should be used within the body of any redistributed or derivative code: \"Copyright 2003 Erica Andrews (PhrozenSmoke@yahoo.com).  All rights reserved.\"\n\n3. Notice of any changes or modifications to the files within the gAnim8 distribution, including the date changes were made. (I recommend you provide URIs to the location from which the code is derived.)\n\nTHIS SOFTWARE AND DOCUMENTATION IS PROVIDED \"AS IS,\" AND COPYRIGHT HOLDERS MAKE NO REPRESENTATIONS OR WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO, WARRANTIES OF MERCHANTABILITY OR FITNESS FOR ANY PARTICULAR PURPOSE OR THAT THE USE OF THE SOFTWARE OR DOCUMENTATION WILL NOT INFRINGE ANY THIRD PARTY PATENTS, COPYRIGHTS, TRADEMARKS OR OTHER RIGHTS.\n\nCOPYRIGHT HOLDERS WILL NOT BE LIABLE FOR ANY DIRECT, INDIRECT, SPECIAL OR CONSEQUENTIAL DAMAGES ARISING OUT OF ANY USE OF THE SOFTWARE OR DOCUMENTATION.\n\nThe name and trademarks of copyright holders may NOT be used in advertising or publicity pertaining to the software without specific, written prior permission. Title to copyright in this software and any associated documentation will at all times remain with copyright holders.")+"\n")

    def showBriefcase(self,*args):
	showBriefcase(self)

    def openRecorder(self,*args):
	openRecorder(self)

    def doSnapshot(self,*args):
	global temp_dir
	GtkSnapshot.convwindow(0,temp_dir)

    def doConversionTool(self,*args):
      global load_file
      gg=gAnim8ConversionTool.convwindow(0)
      gg.sel_file=load_file
      gg.sel_file2=load_file

    def checkDetach(self,*args) :
     global IS_DETACHED
     global detach_win
     if IS_DETACHED: detach_win.destroy()
     else: self.detachFrame()

    def detachFrame(self,*args) :
     global IS_DETACHED
     if IS_DETACHED: return
     global detach_win
     global detach_box
     detach_win=GtkWindow(GTK.WINDOW_TOPLEVEL)
     detach_box=GtkVBox(0,0)
     self.vbox1.remove(self.imbox)
     self.vbox1.remove(self.framesscrollwin)
     self.vbox1.remove(self.statusbar)
     detach_box.pack_start(self.imbox,0,1,3)
     detach_box.pack_start(self.framesscrollwin,1,1,2)
     detach_box.pack_start(self.statusbar,0,0,0)
     self.detbutton.children()[0].set_text(translate("Re-attach"))
     detach_win.connect("destroy",self.reattachFrame)
     IS_DETACHED=1
     detach_win.add(detach_box)
     detach_win.show_all()     

    def reattachFrame(self,*args) :
     global IS_DETACHED
     if not IS_DETACHED: return
     global detach_win
     global detach_box
     detach_box.remove(self.imbox)
     detach_box.remove(self.framesscrollwin)
     detach_box.remove(self.statusbar)
     self.vbox1.pack_start(self.imbox,0,0,3)
     self.vbox1.pack_start(self.framesscrollwin,1,1,2)
     self.vbox1.pack_start(self.statusbar,0,0,0)
     self.vbox1.show_all()
     self.right_box.show_all()
     detach_win.hide()
     detach_win.destroy()
     detach_win.unmap()
     self.ganim.show_all()
     self.detbutton.children()[0].set_text(translate("Detach"))
     IS_DETACHED=0

    def isSupported(self,file_ext) :
     global UNSUPPORTED
     for gg in UNSUPPORTED:
      if file_ext.lower().strip().endswith(gg): 
        showMessage(translate("Sorry. File extension '")+gg+translate("' is not supported by gAnim8.\nClick 'Tools' then 'View Supported File Types' on the menu for a list of supported formats."))
        return 0
     return 1

    def moveFrame(self,iframe,i_pos):
      inum=iframe.image_no
      moveto=inum+i_pos
      if self.images.has_key(moveto) and self.images.has_key(inum):
       movetoimg=self.images[moveto]
       self.images[inum]=movetoimg
       self.images[moveto]=iframe.image
       global frame_to_activate
       frame_to_activate=moveto
       self.changed()
       self.updateLayout()

    def updateLayout(self,*args) :
     global frame_to_activate
     global real_names
     global image_info
     w=2
     h=2
     try:
      c=self.toplay.children()
      for i in c:
         self.toplay.remove(i)
         i.unmap()
      inum=0
      self.load_errors=[]
      imhold={}
      spacer=7
      vb=GtkVBox(0,0)
      insert_butt=getInsertButton()
      insert_butt.connect("clicked",self.doInsert,copy.copy(inum))
      insert_butt.set_data("my_num",copy.copy(inum))
      insert_butt.drag_dest_set(gtk.DEST_DEFAULT_ALL, self.im_targets, gtk.GDK.ACTION_COPY | gtk.GDK.ACTION_MOVE)
      insert_butt.connect("drag_drop",self.setIMDrag)
      insert_butt.connect("drag_begin",self.setIMDrag)
      insert_butt.connect("drag_data_received",self.setIMDrag)
      vb.pack_start(insert_butt,0,0,0)
      vb.pack_start(GtkLabel("  "),1,1,0)
      self.toplay.put(vb,w,2)
      w=w+vb.size_request()[0]+spacer
      image_list=copy.copy(self.images.keys())
      image_list.sort()
      brother=None
      for i in image_list:
         pmap=self.images[i]
         ncolors="Unknown"
         if image_info.has_key(pmap):
           if image_info[pmap].has_key("color_palette"):  ncolors=str(len(image_info[pmap]["color_palette"]))
         img=ImgFrame(brother)  #brother added
         rname=pmap
         if real_names.has_key(pmap): rname=real_names[pmap]
         b=img.setImage(pmap,rname,ncolors)
         if not b: 
           self.load_errors.append(pmap)
           img.hide()
           img.destroy()
           img.unmap()
           img=None
           continue
         if not brother: brother=img.imgframe
         img.setGanim(self)
         imhold[inum]=pmap
         img.setImageNo(inum)
         if frame_to_activate==inum: 
           self.setActiveFrame(img)
         self.toplay.put(img.imgframe,w,2)
         w=w+img.imgframe.size_request()[0]+spacer
         if h < img.imgframe.size_request()[1]: h= img.imgframe.size_request()[1]+20
         inum=inum+1
         vb=GtkVBox(0,0)
         insert_butt=getInsertButton()
         insert_butt.connect("clicked",self.doInsert,copy.copy(inum))
	 insert_butt.set_data("my_num",copy.copy(inum))
         insert_butt.drag_dest_set(gtk.DEST_DEFAULT_ALL, self.im_targets, gtk.GDK.ACTION_COPY | gtk.GDK.ACTION_MOVE)
	 insert_butt.connect("drag_drop",self.setIMDrag)
	 insert_butt.connect("drag_begin",self.setIMDrag)
	 insert_butt.connect("drag_data_received",self.setIMDrag)
         vb.pack_start(insert_butt,0,0,0)
         vb.pack_start(GtkLabel("  "),1,1,0)
         self.toplay.put(vb,w,2)
         w=w+vb.size_request()[0]+spacer
      self.images=imhold
      self.toplay.show_all()
      self.framesscrollwin.show_all()
      self.toplay.set_size(w+20,h+20)
      if len(self.load_errors):  
         h=translate("An ERROR occurred loading the following files:\n\n")
         for le in self.load_errors:
           h=h+"    "+str(le)+"\n"
         showModalHelp(h+"\n"+translate("Please make sure that the file(s) exists and that it is supported by the Imlib image loading library."))
     except:
      pass     
     self.noradio.restore_default_style()
     self.heavyradio.restore_default_style()
     self.lightradio.restore_default_style()

    def changed(self):
       global CHANGED
       CHANGED=1

    def showAnimationInfo(self,*args):
      global temp_dir
      if not len(self.images):
        showStatus("No GIFS in this sequence. No information available.")
        showModalHelp("No GIFS in this sequence. No information available.")
        return
      g=self.generatePreview()
      if not g: return
      if not g[0]:
        showStatus("ERROR: Your GIF could not be created. Save FAILED!")
        showModalHelp("Your GIF could not be created.\nHere is the beginning of the feedback sent by Gifsicle. It may help you troubleshoot the problem:\n\n"+str(g[1]))
        return
      f=os.popen("gifsicle -I --color-info "+temp_dir+"ganim_preview.gif")
      info_str=translate("INFORMATION ABOUT THIS ANIMATION SEQUENCE:\n\n")
      flines=f.readlines()
      for fline in flines: 
         info_str=info_str+"\n"+fline
      f.close()
      showModalHelp(info_str)

    def showDimensions(self,*args):
      global dim_window
      dim_window=gdimension()
      dim_window.ok_button.connect("clicked",self.setImageScale)
      dim_window.cancel_button.connect("clicked",self.hideDimensions)
      dim_window.help_button.connect("clicked",self.showDimensionsHelp)
      dim_window.gdimension.show_all()

    def setImageScale(self,*args):
      global dim_window
      dims=dim_window.getDimensions()
      try:
        xx=string.atoi(dims[0])
        yy=string.atoi(dims[1])
        if (yy < 0) or (xx < 0): 
          showStatus("ERROR: The dimensions must be POSITIVE numbers.")
          return
        global dim_scalex
        global dim_scaley
        dim_scalex=xx
        dim_scaley=yy
        self.hideDimensions()
        self.changed()
      except:
        showStatus(" ERROR: The dimensions must be INTEGERS.")
        return

    def hideDimensions(self,*args):
      global dim_window
      dim_window.gdimension.hide()
      dim_window.gdimension.destroy()
      dim_window.gdimension.unmap()
      return TRUE

    def saveDecide(self):
      global save_file
      global CHANGED
      if not save_file==None: 
        if not save_file.endswith("/"): self.saveAnimation(save_file)
        else: self.doSaveAs()
      else: self.doSaveAs()
      CHANGED=0

    def doSaveAs(self,*args):
      global do_save_as
      do_save_as=1
      self.startSave()      
      
    def askSaveAs(self,*args):
       if not len(self.images):
         showStatus(" No GIFs loaded.  Nothing to save yet.")
         return
       global save_file
       global sel_file
       if save_file: setCurrentFile(save_file)
       else:
          if sel_file: setCurrentFile(extractDirectory(sel_file))
       showFileSelection(translate("Save Animation to file..."), self.recordSaveAs,doFsHide)

    def recordSaveAs(self,*args):
       global save_file
       global do_save_as
       save_file=grabFile()
       try: 
         if os.path.exists(save_file.replace("\\","")):
           if not showConfirmMessage(translate("File ")+save_file+translate(" already exists.  Overwrite?")):
             do_save_as=1
             self.askSaveAs()
             return
       except: 
         pass
       do_save_as=0
       self.startSave()

    def startSave(self,*args):
      global QUIT_PENDING
      global do_save_as
      if do_save_as==1: 
        self.askSaveAs()
        return
      global save_file
      if not save_file:
        if not QUIT_PENDING: showStatus(" No file selected for saving!")
        do_save_as=1
        return
      try:
        g=self.generatePreview()
        if not g: return
        if not g[0]:
          if not QUIT_PENDING: showStatus("ERROR: Your GIF could not be created. Save FAILED!")
          if not QUIT_PENDING: showModalHelp(translate("Your GIF could not be created.\nHere is the beginning of the feedback sent by Gifsicle. It may help you troubleshoot the problem:\n\n")+str(g[1]))
        else: self.saveAnimation(save_file)
      except:
        if not QUIT_PENDING: showStatus("ERROR: Your GIF animation could not be created. No file saved.")
      if QUIT_PENDING: 
         global CHANGED
         CHANGED=0
         QUIT_PENDING=0         
         quit()

    def saveAnimation(self, fname):
      global temp_dir
      global do_save_as
      if not fname: 
         showStatus(" No file selected for saving!")
         do_save_as=1
         return
      if  fname.endswith("/"): 
         showStatus(" No file selected for saving!")
         do_save_as=1
         return
      if not fname.lower().endswith(".gif"): fname=fname+".gif"
      f=os.popen("cp -f "+temp_dir+"ganim_preview.gif "+fname)
      f.readlines()
      f.close()
      try:
        fsize=os.path.getsize(fname.replace("\\",""))
	self.session_created[fname]="1"
        global QUIT_PENDING
        if not QUIT_PENDING: showStatus(translate(" File saved to  ")+fname+"  ("+str(fsize)+translate(" bytes)"))
        global CHANGED
        CHANGED=0
        global NEW_PENDING
        if NEW_PENDING: self.resetDefaults()
        global LOAD_PENDING
        if LOAD_PENDING: self.askGifOpen()
      except:
        showStatus(translate(" ERROR:  Save to file ")+fname+translate(" failed!"))

    def resetDefaults(self,doClean=1,*args):
       global CHANGED
       global NEW_PENDING
       if CHANGED:
         if showConfirmMessage("This image or animation sequence has changed.  Save it?"):
           NEW_PENDING=1
           self.saveDecide()
           return 
       NEW_PENDING=0
       self.images.clear()
       global image_info
       global real_names
       image_info.clear()
       real_names.clear()
       global trans_sig
       self.transcheck.signal_handler_block(trans_sig)
       self.transcheck.set_active(1)
       self.transcheck.signal_handler_unblock(trans_sig)
       self.loopd_entry.set_text("20")
       self.loopc_entry.set_text("1000")
       self.loopcheck.set_active(1)
       self.transpicker.set_i8(0,0,0,255)
       self.bgpicker.set_i8(0,0,0,255)
       self.iminfo.set_text("")
       self.lightradio.set_active(1)
       global dim_scalex
       global dim_scaley
       dim_scaley=0
       dim_scalex=0
       global do_save_as
       do_save_as=1
       global col_palette
       col_palette.clear()
       self.intercheck.set_active(0)
       global default_comment
       self.comment_entry.set_text(default_comment)
       global frame_to_activate
       frame_to_activate=0
       global active_frame
       active_frame=None
       global save_file
       if save_file: save_file=extractDirectory(save_file)
       showStatus(" "+translate("Copyright 2003 by Erica Andrews. All rights reserved."))
       CHANGED=0
       if doClean==1: cleanupFiles()
       self.updateLayout()

    def askRotation(self,*args):
       global rot_window
       rot_window=rotator()
       rot_window.ok_button.connect("clicked",self.startRotation)
       rot_window.rotwin.show_all()

    def startRotation(self,*args):
       global rot_window
       if not rot_window: return
       rot_string=""
       if rot_window.rothor.get_active(): rot_string="--flip-horizontal"
       if rot_window.rotvert.get_active(): rot_string="--flip-vertical"
       if rot_window.rot90.get_active(): rot_string="--rotate-90"
       if rot_window.rot180.get_active(): rot_string="--rotate-180"
       if rot_window.rot270.get_active(): rot_string="--rotate-270"
       if rot_string: self.doImageRotation(rot_string)

    def doImageRotation(self,rot_string):
      global active_frame
      if active_frame:        
        f=os.popen("gifsicle "+rot_string+" "+active_frame.image+" -o "+active_frame.image)
        self.changed()
        f.readlines()
        f.close()       
        self.setFrameAttributes(active_frame.image)  
        self.cleanupPalette()        
        self.updateLayout()        

    def doDelete(self,img_no,*args):
      global image_info
      global real_names
      if self.images.has_key(img_no):        
        im_file=self.images[img_no]
        if image_info.has_key(im_file): del image_info[im_file]
        if real_names.has_key(im_file): del real_names[im_file]
        if self.images.has_key(img_no): del self.images[img_no]
        os.popen("rm -f "+im_file)
        self.changed()
        self.cleanupPalette()
        if not len(self.images): self.iminfo.set_text("")
        self.updateLayout()
        showStatus("  "+translate("File removed from sequence. Editing")+" "+str(len(self.images))+" GIFs.")

    def cleanupPalette(self):
      global col_palette
      global image_info
      used_colors=[]
      for i in image_info.keys():
        if image_info[i].has_key("color_palette"): used_colors=used_colors+image_info[i]["color_palette"].keys()
      for i in col_palette.keys():
        if not col_palette[i] in used_colors: del col_palette[i]

    def writeBWPalette(self):
      global temp_dir
      try:
        f=open(temp_dir+"ganim_palettebw","w")
        f.write("0 0 0\n255 255 255")
        f.flush()
        f.close()
        return 1
      except:
        return 0

    def doGreyScale(self,*args):
     global active_frame
     if not active_frame: return
     if self.images.has_key(active_frame.image_no):
       nfile=self.images[active_frame.image_no]
       if not showConfirmMessage("Are you sure you want to convert this frame to Grey Scale?\nThis feature is EXPERIMENTAL and cannot be undone.\nIf you don't like the Grey Scale image, you will have reopen this frame's image.\nContinue?"): return
       if self.writeBWPalette()==0:
         showMessage("Error: could not write the necessary palette to disk for GreyScale conversion.")
         return
       global temp_dir
       f=os.popen("gifsicle "+nfile+" --use-colormap "+temp_dir+"ganim_palettebw"+" -o "+nfile)
       f.readlines()
       f.close()
       self.setFrameAttributes(nfile)
       self.cleanupPalette()
       self.changed() 
       self.setActiveFrame(active_frame)         
       self.updateLayout()

    def askGifChange(self,*args):
       global active_frame
       if not active_frame: return
       global load_file
       if load_file: setCurrentFile(load_file)
       showFileSelection("Open a file...", self.swapGif,doFsHide)

    def swapGif(self,*args):
      global active_frame
      global image_info
      global real_names
      g=grabFile()
      if not haveGifsicle(): return
      if not g: return     
      global load_file
      load_file=g
      if not self.isSupported(g): return
      n=self.convertToGif(g,1)
      if not n: return 
      if not active_frame: return
      if not g.endswith("/"):
        inum=active_frame.image_no
        if self.images.has_key(inum):            
          currentf=self.images[inum]
          if real_names.has_key(currentf): del real_names[currentf]
          if image_info.has_key(currentf): del image_info[currentf]
          del self.images[inum]
          os.popen("rm -f "+currentf)
          nfile=self.copyGifToTemp(n)
          self.images[inum]=nfile
          real_names[nfile]=g
          self.setFrameAttributes(nfile)
          self.cleanupPalette()
          self.changed()          
          self.updateLayout()
          showStatus("  "+translate("File")+" "+str(g)[str(g).rfind(os.sep)+1:len(str(g))]+" "+translate("added to sequence. Editing")+" "+str(len(self.images))+" GIFs.")

    def doInsert(self,component,inum,*args) :
      if not haveGifsicle(): return
      global insert_at
      insert_at=inum
      global load_file
      if load_file: setCurrentFile(load_file)
      showFileSelection("Select a file...", self.insertGif,doFsHide)

    def setIMDrag(self,*args):
	drago=args
	if len(drago)<7: 
		args[0].drag_highlight()
		args[0].grab_focus()
	else: 
		try:
			global insert_at
			insert_at=args[0].get_data("my_num")
			if insert_at==None:
				insert_at=len(self.images)
			if insert_at<0: insert_at=0
			drag_file=None
			if  drag_file==None:
				drag_file="PHROZEN:"+str(drago[4].data).replace("\r\n","").replace("\x00","").replace("file:","")
			self.insertGif(drag_file)
		except: 
			pass

    def insertGif(self,with_request_box=0,*args):
     global image_info
     global insert_at
     global real_names
     n=None
     if str(with_request_box).startswith("PHROZEN:"): g=str(with_request_box).replace("PHROZEN:","")
     else:
	g=grabFile()
     if not g: return
     if not self.isSupported(g): return
     if self.isMpeg(g):
        if not haveMpegE():
        	showMessage("You do not have the 'ffmpeg' binary from the 'ffmpeg' package on your path.\nThis binary is required by gAnim8 to create animated GIFS from videos.\nYou will have to put the 'ffmpeg' binary (version 0.4.6 or better)\non your path before this function will be enabled in gAnim8.\nYou can find versions of 'ffmpeg' on speakeasy.rpmfind.net, ganim8.sourceforge.net,\nand ffmpeg.sourceforge.net.")
       		return
        if not showConfirmMessage("You are about to insert an entire video file directly into the animation sequence.\nIf you choose to continue, the video will be converted to a LOW-QUALITY\nanimated GIF, represented by ONE frame in the animation window.\nCreating this animated GIF frame may take some time.\nDo you wish to continue?")==1: 
		return
        global temp_dir
	nfile=temp_dir+"ganim8_conversion"+str(random.randrange(999))+str(random.randrange(999))+".gif"
	f=os.popen("ffmpeg -i "+g.replace(" ","\\ ")+" "+nfile)
	f.readlines()
	if not os.path.exists(nfile):
		showMessage(translate("ERROR: Video conversion to animated GIF failed for")+": \n"+g)
		return
	if not os.path.getsize(nfile)>0:
		showMessage(translate("ERROR: Video conversion to animated GIF failed for")+": \n"+g)
		return
	g=nfile
     n=self.convertToGif(g,1)
     if not n: return
     global load_file
     load_file=g
     try:
      if not g.endswith("/"):
       nfile=self.copyGifToTemp(n)
       new_images={}
       ncounter=0
       thelist=self.images.keys()
       thelist.sort()
       for i in thelist:
         if ncounter==insert_at:
           new_images[ncounter]=nfile
           real_names[nfile]=g
           ncounter=ncounter+1
         new_images[ncounter]=self.images[i]
         ncounter=ncounter+1
         if ncounter==insert_at:
           new_images[ncounter]=nfile
           real_names[nfile]=g
           ncounter=ncounter+1
       if not real_names.has_key(nfile):  
         if not len(self.images):   # this is a first frame
           new_images[0]=nfile
           real_names[nfile]=g
           self.images=new_images
           self.setFrameAttributes(nfile)
           self.changed()
           self.cleanupPalette()           
           self.updateLayout()
           showStatus("  "+translate("File")+" "+str(g)[str(g).rfind(os.sep)+1:len(str(g))]+" "+translate("added to sequence. Editing")+" "+str(len(self.images))+" GIFs.")
         else:
           os.popen("rm -f "+nfile)
       else:
         self.images=new_images
         self.changed()
         self.setFrameAttributes(nfile)
         self.cleanupPalette()         
         self.updateLayout()
         showStatus("  "+translate("File")+" "+str(g)[str(g).rfind(os.sep)+1:len(str(g))]+" "+translate("added to sequence. Editing")+" "+str(len(self.images))+" GIFs.")
     except:
      pass 

    def getRandomMovieFileName(self,*args):
      return "ganim_movpreview"+str(random.randrange(9999))+str(random.randrange(999))

    def getRandomFileName(self,*args):
      return "ganim_copy"+str(random.randrange(9999))+str(random.randrange(999))

    def copyGifToTemp(self, org_file):
      if not org_file: return None
      if org_file.endswith("/"): return None
      global temp_dir
      ofile=str(org_file)
      if ofile.rfind("/") > -1: ofile=ofile[ofile.rfind("/")+1:len(ofile)].strip().replace(" ","_")
      nfile=temp_dir+self.getRandomFileName()+"-"+ofile
      f=os.popen("cp -f "+org_file.replace(" ","\\ ")+" "+nfile)
      f.readlines()
      f.close()
      return nfile

    def hidePaletteEdit(self,*args):
      global color_selection
      color_selection.hide()
      global active_color_button
      active_color_button=None
      return TRUE

    def acceptPaletteEdit(self,*args):
      global color_selection
      if color_selection and active_color_button:
         ncolor=color_selection.colorsel.get_color()
         nred=int((ncolor[0]*65535))
         nblue= int((ncolor[1]*65535))
         ngreen=int((ncolor[2]*65535)) 
         active_color_button.setColor((nred,nblue,ngreen))
      self.hidePaletteEdit()

    def doEdit(self,ifile,*args) :
      global editor_cmd  
      if not ifile: return    
      os.popen(str(editor_cmd).replace("%f",ifile)+" &")

    def correctExtension(self,file_name):
	global movie_type
	if movie_type=="mpeg":
		if not file_name.endswith(".mpeg"): return file_name+".mpeg"
	if movie_type=="avi":
		if not file_name.endswith(".avi"): return file_name+".avi"
	if movie_type=="swf":
		if not file_name.endswith(".swf"): return file_name+".swf"
	if movie_type=="rm":
		if not file_name.endswith(".rm"): return file_name+".rm"
	if movie_type=="asf":
		if not file_name.endswith(".asf"): return file_name+".asf"
	if movie_type=="wmv":
		if not file_name.endswith(".wmv"): return file_name+".wmv"
	if movie_type=="m4v":
		if not file_name.endswith(".m4v"): return file_name+".m4v"
	return file_name

    def initMpegPreview(self,*args):
     self.askSaveMpeg(1)

    def askSaveMpeg(self,do_preview=0,*args):
	self.askSaveMovie("mpeg",do_preview)

    def initAviPreview(self,*args):
     self.askSaveAvi(1)

    def askSaveAvi(self,do_preview=0,*args):
	self.askSaveMovie("avi",do_preview)

    def initSwfPreview(self,*args):
     self.askSaveSwf(1)

    def askSaveSwf(self,do_preview=0,*args):
	self.askSaveMovie("swf",do_preview)

    def initRmPreview(self,*args):
     self.askSaveRm(1)

    def askSaveRm(self,do_preview=0,*args):
	self.realmedia_size="176x144"
	rmsize=GtkExtra.message_box(translate("RealPlayer Dimensions"),translate("What size would you like the RealPlayer movie to be?"),[translate("Small - 176x144"),translate("Large - 352x288")])
	if rmsize==translate("Large - 352x288"): self.realmedia_size="352x288"
	self.askSaveMovie("rm",do_preview)

    def initAsfPreview(self,*args):
     self.askSaveAsf(1)

    def askSaveAsf(self,do_preview=0,*args):
	self.askSaveMovie("asf",do_preview)

    def initWmvPreview(self,*args):
     self.askSaveWmv(1)

    def askSaveWmv(self,do_preview=0,*args):
	self.askSaveMovie("wmv",do_preview)

    def initM4VPreview(self,*args):
     self.askSaveM4v(1)

    def askSaveM4v(self,do_preview=0,*args):
	self.askSaveMovie("m4v",do_preview)

    def launchMoviePreview(self):
      global movie1_file
      global movie_type
      global mpeg_cmd
      global avi_cmd
      global swf_cmd
      global rm_cmd
      global asf_cmd
      global wmv_cmd
      global m4v_cmd
      prev_cmd=mpeg_cmd
      if movie_type=="avi": prev_cmd=avi_cmd
      if movie_type=="swf": prev_cmd=swf_cmd
      if movie_type=="rm": prev_cmd=rm_cmd
      if movie_type=="asf": prev_cmd=asf_cmd
      if movie_type=="wmv": prev_cmd=wmv_cmd
      if movie_type=="m4v": prev_cmd=m4v_cmd
      if movie1_file and prev_cmd: os.popen(str(prev_cmd).replace("%f",str(movie1_file))+" &> /dev/null &")

    def askSaveMovie(self,movtype="mpeg",do_preview=0):
       global movie_type
       movie_type=movtype
       if not len(self.images): 
        showStatus(translate(" No images loaded to create ")+str(movie_type).upper()+translate(" from."))
        return
       if not haveMpegE():
        showMessage("You do not have the 'ffmpeg' binary from the 'ffmpeg' package on your path.\nThis binary is required by gAnim8 to create animated GIFS from videos.\nYou will have to put the 'ffmpeg' binary (version 0.4.6 or better)\non your path before this function will be enabled in gAnim8.\nYou can find versions of 'ffmpeg' on speakeasy.rpmfind.net, ganim8.sourceforge.net,\nand ffmpeg.sourceforge.net.")
        return
       if not gAnim8ConversionTool.haveImageMagick():
        showMessage("The 'convert' binary from the ImageMagick program is NOT on your PATH\nImageMagick's 'convert' utility is required for all image conversions and must be on your PATH.\nImageMagick 5.2.3 or better is required. Version 5.4.6.1 or better is recommended.\nYou can find versions of ImageMagick on speakeasy.rpmfind.net.\nUntil you have ImageMagick installed, you can only open GIF files with gAnim8.")
        return
       global movie1_file
       if movie1_file: setCurrentFile(movie1_file)
       global mpeg_preview
       mpeg_preview=do_preview
       global temp_dir
       if mpeg_preview==1: self.saveMovie("PHROZEN:"+temp_dir+self.getRandomMovieFileName())
       else: showFileSelection(translate("Save As ")+str(movie_type).upper()+translate(" Video..."), self.saveMovie,doFsHide)


    def saveMovie(self,supplied_file="",*args):
      global movie1_file
      global movie_type
      global mpeg_preview
      if str(supplied_file).startswith("PHROZEN:"): g=str(supplied_file).replace("PHROZEN:","").strip()
      else: g=grabFile()
      if g:
       if not g.endswith("/"):
        g=self.correctExtension(g)
        if (os.path.exists(g)) and (mpeg_preview==0):
          if not showConfirmMessage(translate("File '")+g+translate("' already exists. Overwrite?")): return
        movie1_file=g
        global MPEG_SAVE_WARN
        if not MPEG_SAVE_WARN:
         if not showConfirmMessage(translate("A BASIC ")+str(movie_type).upper()+translate(" Video of this animation will be created.\nThe created video will be about 2 FPS with no special effects.\nThis may take some time - so be patient.\nIf you need to change the frame rate or tweak this video,\nyou will need a separate program, such as ffmpeg's command-line utility.\ngAnim8 is NOT a 'movie studio'!\nContinue?")): return
        MPEG_SAVE_WARN=1
	ic=self.generatePreview()
	if not ic:
		showMessage("ERROR: No animation could be created from this sequence of images.")
		return
	global temp_dir
	if not os.path.getsize(temp_dir+"ganim_preview.gif") >0:
		showMessage("ERROR: No animation could be created from this sequence of images.")
		return		
	extract_line="convert "+temp_dir+"ganim_preview.gif"+" "+temp_dir+"ganim_movie_frame%d.jpeg"
	f=os.popen("rm -f "+temp_dir+"ganim_movie_frame*.jpeg")
	f.readlines()
	addon=""
	if movie_type=="rm": addon=" -s "+self.realmedia_size+" -g 1 "
	assemble_line="ffmpeg -r 2 -i "+temp_dir+"ganim_movie_frame%d.jpeg -y -r 2 "+addon+g
        f=os.popen(extract_line)
	f.readlines()
        f=os.popen("rm -f "+g) # delete the old mpeg if it exists
        f.readlines()
        f=os.popen(assemble_line)
        f.readlines()
	f=os.popen("rm -f "+temp_dir+"ganim_movie_frame*.jpeg")
	f.readlines()
        if os.path.exists(g):
         if not mpeg_preview==1: showMessage(translate("Successfully created ")+str(movie_type).upper()+translate(" video:\n\n")+g)
	 self.session_created[g]="1"
         try:
           showStatus(str(movie_type).upper()+translate(" video created: ")+g+" / "+str(os.path.getsize(g))+translate(" bytes"))
         except:
           pass

         if mpeg_preview==1: self.launchMoviePreview()
        else:
         showMessage(translate("Creation of ")+str(movie_type).upper()+translate(" video FAILED:\n\n")+g+translate("\n\nFile not created."))

    def isGif(self,im_file):
       if str(im_file).lower().strip().endswith(".gif"): return 1
       return 0
  
    def convertToGif(self,im_file,retry=0):
       if self.isGif(im_file): return im_file
       global temp_dir
       nfile=temp_dir+"ganim8_conversion"+str(random.randrange(999))+str(random.randrange(999))+".gif"
       stat=gAnim8ConversionTool.doConversion(im_file,"jpg",nfile,"gif")#try auto-conversion first
       if stat[0]==0:
        if not gAnim8ConversionTool.haveImageMagick(): 
          showMessage(translate("Attempt to import Non-GIF image FAILED:\n")+translate(str(stat[1])))
          return
        if retry:
          iformat=askFormat()
          if iformat: 
            stat=gAnim8ConversionTool.doConversion(im_file,iformat,nfile,"gif",1)
       if stat[0]==0: 
         showMessage(translate("Attempt to import Non-GIF image FAILED:\n")+translate(str(stat[1])))
         return None
       else:
         nfile=stat[2]
         if os.path.exists(nfile): return nfile
         else: 
             showMessage("Unknown error processing converted image with Gifsicle. Conversion failed.")
             return None

    def askGifOpen(self,*args): 
       global CHANGED      
       if CHANGED:
         if showConfirmMessage("This image or animation sequence has changed.  Save it?"):
           global LOAD_PENDING
           LOAD_PENDING=1
           self.saveDecide()
           return
       CHANGED=0
       LOAD_PENDING=0
       global load_file
       if load_file: setCurrentFile(load_file)
       showFileSelection("Open an Image or Video file...", self.unpackFile,doFsHide)

    def isMpeg(self,g):
	mpeglist=['.mpeg','.mpe','.mpg','.m1v','.m2v','.mpeg2','.mpeg1','.mp1','.mp2','.avi','.wmv','.wmv1','.wmv2','.mov','.qt','.asf','.rm','.rv','.ram']
        for i in mpeglist:
           if g.lower().endswith(i):
             return 1
        return 0

    def unpackFile(self,*args):
      self.unpackIt(grabFile())

    def unpackIt(self,g):
      if not g: return
      global load_file
      if not haveGifsicle():
        return
      load_file=g
      if not g.endswith("/"): 
         if not os.path.exists(g): 
           showStatus(translate("File not found: ")+str(g))
           return
         if not self.isSupported(g): return
         self.resetDefaults()
         if self.isMpeg(g):
           if not haveMpeg():
              showMessage("You do not have the 'ffmpeg' binary from the 'ffmpeg' package on your path.\nThis binary is required by gAnim8 to create animated GIFS from videos.\nYou will have to put the 'ffmpeg' binary (version 0.4.6 or better)\non your path before this function will be enabled in gAnim8.\nYou can find versions of 'ffmpeg' on speakeasy.rpmfind.net, ganim8.sourceforge.net,\nand ffmpeg.sourceforge.net.")
              return
           if not gAnim8ConversionTool.haveImageMagick():
              showMessage("The 'convert' binary from the ImageMagick program is NOT on your PATH\nImageMagick's 'convert' utility is required for all image conversions and must be on your PATH.\nImageMagick 5.2.3 or better is required. Version 5.4.6.1 or better is recommended.\nYou can find versions of ImageMagick on speakeasy.rpmfind.net and www.imagemagick.org .\nUntil you have ImageMagick installed, you can only open GIF files with gAnim8.")
              return
           global MPEG_OPEN_WARN
           if not MPEG_OPEN_WARN:
            if not showConfirmMessage("NOTE: Video support is EXPERIMENTAL!\nLots of temporary disk space may need to be used.\nThe 'preview' window may take a while to display.\nAll video decoding is done with 'ffmpeg',\nthe speed of which gAnim8 cannot control.\ngAnim8 is NOT intended for opening large videos or to be used as a full 'movie studio'.\nSMALL videos (under 1MB/under 1200 frames) should open fine.\nUse at your own risk and be patient. Continue?"): return
           MPEG_OPEN_WARN=1
           if os.path.getsize(g) > 1000000:
             if not showConfirmMessage("WARNING: The Mpeg file you have selected is over 1 MB in size\n.(Probably over 1200 frames.)\ngAnim8 was NOT created to handle Mpegs this large.\ngAnim8 is NOT a full 'movie studio'. It's a simple animation suite.\nIf you continue, LOTS of temporary disk space will need to be used.\n(possibly tens or hundreds of MB, maybe more)\nThe preview window may take a LONG time to open (several minutes? hours?)\nDo NOT attempt to open Mpegs over 15MB with gAnim8.\nYou have been warned! Continue?"): return
           self.loadMpeg(g)
         else: # load as regular image
           n=self.convertToGif(g,1)
           if not n==None: 
            self.loadGif(n)

    def displayAllFrames(self):
         counter=0
         for i in self.images.keys():
           self.setFrameAttributes(self.images[i])
           counter=counter+1
         global active_frame
         active_frame=None
         if  len(self.images):  self.updateLayout()         
         showStatus(translate(" Ready. Extracted ")+str(counter)+translate(" frames."))
         self.framesscrollwin.get_hadjustment().set_value(1)
         self.framesscrollwin.get_vadjustment().set_value(1)

    def showMpegPreview(self, flist):
        global frameview
        frameview=framesel(self)
        frameview.addFrames(flist)

    def loadMpeg(self, mpeg_file):
      global temp_dir
      global mpegdec
      mpegdec.cleanup()
      c=mpegdec.decode(str(mpeg_file),temp_dir)
      flist=mpegdec.getFrameList()
      self.showMpegPreview(flist)
      return

    def loadMpegFrames(self, frames_list):
      global temp_dir
      global frameview
      imhold={}
      for counter in frames_list:
        f=mpegdec.getFrame(counter)
        nfile=self.convertToGif(f)
        if not nfile==None: imhold[counter]=nfile
      self.images=imhold
      frameview.closeit()
      self.displayAllFrames()
      if len(self.images): self.changed()

    def loadGif(self, gif_file,*args):
       if not haveGifsicle(): return
       global temp_dir
       global image_info
       global col_palette
       global real_names
       global frame_to_activate
       if not gif_file:
         showStatus("No Gif file specified. Nothing to open.")
         return
       imhold={}
       showStatus(" Extracting .gif frames....")
       counter=0
       try:
         f=os.popen("gifsicle --explode -U  --output "+str(temp_dir)+"ganim_extract --no-extensions "+gif_file.replace(" ","\\ "))
         f.readlines()
         l=os.listdir(temp_dir)
         lstfile=None
         for i in l:
           if i.startswith("ganim_extract"): 
             lstfile=temp_dir+i
             imhold[counter]=lstfile
             real_names[lstfile]=lstfile
             counter=counter+1
         if not len(imhold):  
           showStatus("No frames extracted from "+gif_file)
         if len(imhold)==1:
           if lstfile: real_names[lstfile]=gif_file
         self.images=imhold
         self.setMainGifAttributes(gif_file)
         self.displayAllFrames()
         if not len(self.images):
            showModalHelp("No frames were extracted from the file you selected. Perhaps, the file you selected was not a legitimate GIF file or is in a format that Gifsicle is unable to open.  You may not have read/write access to this file or the temporary directory. Please ensure that your temporary directory exists at either /tmp/ or ~/.ganim8/tmp/ and that you have read/write access to that directory. Click 'Tools' then 'View Support File Types' on the menu up top to see a list of image and video formats supported by gAnim8.")
            self.resetDefaults()
       except:
          showModalHelp("An error occurred opening the selected Gif file: "+gif_file+"\n\nThis may not be a legitimate gif file, or you may not have read/write access to this file or the temporary directory. Please ensure that your temporary directory exists at either /tmp/ or ~/.ganim8/tmp/ and that you have read/write access to that directory.  Click 'Tools' then 'View Support File Types' on the menu up top to see a list of image and video formats supported by gAnim8.")

    def showTransEditor(self,*args):
      global active_frame
      if not active_frame: 
        showStatus("Load a GIF and selected a frame first.")
        return
      global col_palette
      global palette_window
      bcol=None
      global image_info
      pal=col_palette
      if image_info.has_key(active_frame.image):
        iinfo=image_info[active_frame.image]
        if iinfo.has_key("transparent_color"):
          bcol=iinfo["transparent_color"]
        if iinfo.has_key("color_palette"):
          pal=iinfo["color_palette"]
      p=palettewin(2,pal,self.acceptTransEditor,self.hideColorEditor,bcol)
      p.palettewin.set_modal(1)
      palette_window=p
      p.palettewin.show_all()

    def acceptTransEditor(self,*args):
      global palette_window
      global active_frame
      global image_info
      sel_color=palette_window.getSelectedColor()
      if sel_color and active_frame:
        col8=get_Color8(sel_color)
        if image_info.has_key(active_frame.image): image_info[active_frame.image]["transparent_color"]=col8
        self.showTransColor(col8)
        if  self.transcheck.get_active(): 
          gif_file=active_frame.image
          change_string="gifsicle --transparent "+str(col8[0])+"/"+str(col8[1])+"/"+str(col8[2])+" "+gif_file+" -o "+gif_file
          f=os.popen(change_string)
          f.readline()
          f.close()
          self.changed()
          self.setFrameAttributes(gif_file)
          self.updateLayout()
      self.hideColorEditor()

    def toggleTransparency(self,*args):
      global active_frame
      global image_info
      if  active_frame:
        col8=self.transpicker.get_i8()
        if image_info.has_key(active_frame.image):
          change_string=""
          image_info[active_frame.image]["transparent_color"]=col8
          if self.transcheck.get_active(): 
            image_info[active_frame.image]["transparent"]=1
            change_string="gifsicle --transparent "+str(col8[0])+"/"+str(col8[1])+"/"+str(col8[2])+" "+active_frame.image+" -o "+active_frame.image
          else: 
            image_info[active_frame.image]["transparent"]=0
            change_string="gifsicle --no-transparent "+active_frame.image+" -o "+active_frame.image
          f=os.popen(change_string)
          f.readline()
          f.close()
          self.changed()
          self.setFrameAttributes(active_frame.image)
          self.updateLayout()

    def showColorEditor(self,component=None,gif_file=None,*args):
      global image_info
      if not image_info.has_key(gif_file):
        showStatus("Color palette unavailable for "+str(gif_file))
        return
      if not image_info[gif_file].has_key("color_palette"): 
        showStatus(translate("Color palette unavailable for ")+str(gif_file))
        return
      global palette_window
      global current_palette_edit
      current_palette_edit=gif_file
      p=palettewin(1,image_info[gif_file]["color_palette"],self.acceptColorEditor,self.hideColorEditor)
      p.palettewin.set_modal(1)
      palette_window=p
      p.palettewin.show_all()

    def hideColorEditor(self,*args):
      global palette_window
      palette_window.palettewin.hide()
      palette_window.palettewin.destroy()
      palette_window.palettewin.unmap()
      palette_window=None
      return TRUE

    def acceptColorEditor(self,*args):
      global palette_window
      global current_palette_edit
      global image_info
      global active_frame
      changes=palette_window.getChangedColors()
      self.hideColorEditor()
      if current_palette_edit:
        if len(changes):
          change_string="gifsicle "
          for c in changes.keys():
            oldc=get_Color8(c)
            newc=get_Color8(changes[c])
            change_string=change_string+" --change-color "+str(oldc[0])+"/"+str(oldc[1])+"/"+str(oldc[2])+" "+str(newc[0])+"/"+str(newc[1])+"/"+str(newc[2])
          change_string=change_string+" "+current_palette_edit+" -o "+current_palette_edit
          f=os.popen(change_string)
          f.readline()
          f.close()    
          self.setFrameAttributes(current_palette_edit)
          self.setActiveFrame(active_frame)       
          self.updateLayout()   
      current_palette_edit=None

    def showBGColor(self,rgb_tuple,*args):
      self.bgpicker.set_i8(rgb_tuple[0],rgb_tuple[1],rgb_tuple[2]  ,255)

    def showTransColor(self,rgb_tuple,*args):
      self.transpicker.set_i8(rgb_tuple[0],rgb_tuple[1],rgb_tuple[2]  ,255)

    def extractToDir(self,*args):
       if not len(self.images):
         showStatus("No frames to extract")
         return
       global extract_file
       if extract_file: setCurrentFile(extractDirectory(extract_file))
       else:
         global sel_file
         if sel_file: setCurrentFile(extractDirectory(sel_file))
       showFileSelection("Extract All Frames To....", self.doExtract,doFsHide)

    def doExtract(self,*args):
       f=grabFile()
       if f:
         f=extractDirectory(f)
         global extract_file
         global extract_no
         extract_no=extract_no+1
         extract_file=f
         countit=1
         for i in self.images.values():
           iname=copy.copy(i)
           iname="ganim8extract"+str(extract_no)+"-frame"+str(countit)+".gif"
           os.popen("cp -f "+i+" "+str(f+iname).replace(" ","\\ "))
           countit=countit+1
         showStatus(translate("Frames extracted to directory ")+extract_file)

    def extractToFile(self,fname,*args):
       global extract_file
       global e_file
       e_file=fname
       if extract_file: setCurrentFile(extractDirectory(extract_file)+".gif")
       else:
         global sel_file
         if sel_file: setCurrentFile(extractDirectory(sel_file)+".gif")
       showFileSelection("Extract Frame To....", self.doExtractFile,doFsHide)

    def doExtractFile(self,*args):
       f=grabFile()
       global e_file       
       if f and e_file:
         global extract_file
         extract_file=f
         try:
           if os.path.exists(f):
             if not showConfirmMessage(translate("File ")+f+translate(" already exists.  Overwrite?")):
               self.extractToFile(e_file)
               return
         except:
           pass
         os.popen("cp -f "+e_file.replace(" ","\\ ")+" "+f.replace(" ","\\ "))
         e_file=None
         showStatus(translate("Frame extracted to  ")+f)
         if not f.lower().strip().endswith(".gif"):
           global GIF_WARN
           if not GIF_WARN: 
             showMessage(translate("I noticed that you saved this frame to a file that did not have the '.gif' extension.\nPlease note that this file was saved in GIF format.\nAt this time, individuals frames can only be saved to GIF format\n(No matter what file extension you use.)\n\nIf you need to save the extract frame in some other image format,\nclick 'Tools' then 'gAnim8 Image Conversion Tool' on the menu.\nThen, convert '")+f+translate("' to whatever format you want."))
             GIF_WARN=1


    def setActiveFrame(self,iframe):
      global active_frame
      global frame_to_activate
      global image_info
      global col_palette
      if iframe:
        if active_frame:
          try:
            active_frame.imgframe.set_active(0)
            fdict=image_info[self.images[active_frame.image_no]]
            fdict["transparent"]=self.transcheck.get_active()
          except:
            pass
      iframe.imgframe.signal_handler_block(iframe.tog_sig)
      try:             
             if not  iframe.imgframe.get_active(): 
               iframe.imgframe.set_active(1)
             global iminfo
             global real_names
             im_name=str(iframe.image)
             if real_names.has_key(im_name): im_name=real_names[im_name]
             im_name_short=str(im_name)
             if len(im_name_short) > 30: im_name_short="..."+im_name_short[len(im_name_short)-30:len(im_name_short)]
             iminfo.set_text(translate("Frame ")+str(iframe.image_no+1)+":   "+im_name_short+translate("     Dimensions: ")+str(iframe.imdim[0])+"x"+str(iframe.imdim[1]) +translate("       Size:  ")+str(iframe.imsize)+translate(" bytes"))
             iminfo.show_all()  
             frame_to_activate=iframe.image_no
             active_frame=iframe
             if self.images.has_key(iframe.image_no):
              if image_info.has_key(self.images[iframe.image_no]):
               fdict=image_info[self.images[iframe.image_no]]
               global trans_sig
               self.transcheck.signal_handler_block(trans_sig)
               if fdict.has_key("transparent"):
                 if fdict["transparent"]==1: 
                   self.transcheck.set_active(1)
                 else: 
                   self.transcheck.set_active(0)
               else:
                 self.transcheck.set_active(0)
               if fdict.has_key("transparent_color"):  self.showTransColor(fdict["transparent_color"])
               else: 
                 self.showTransColor((0,0,0))
                 self.transcheck.set_active(0)
               self.transcheck.signal_handler_unblock(trans_sig)
      except:
           self.transcheck.signal_handler_unblock(trans_sig)
      iframe.imgframe.signal_handler_unblock(iframe.tog_sig)

    def setFrameAttributes(self,gif_file):
      global image_info
      this_info={}
      transparent=0
      try:
        f=os.popen("gifsicle -I --color-info "+gif_file)
        flist=f.readlines()
        u=self.updateColorPalette(gif_file,{},flist)
        global default_comment
        need_comm=self.comment_entry.get_text()==default_comment
        for ff in flist:
          if len(this_info) ==3:  break
          fline=ff.replace("\n","")
          try: 
            if fline.strip().startswith("comment"): 
               this_info["comment"]=fline.replace("comment","").strip()
               if need_comm==1: 
                 self.comment_entry.set_text(this_info["comment"])
                 need_comm=0
            if fline.rfind("transparent") > -1:  
               this_info["transparent"]=1
               transparent_index=fline[fline.rfind("transparent")+12:len(fline)].strip()  
               if u.has_key(transparent_index): this_info["transparent_color"]=u[transparent_index]
          except: 
            pass
        if u: this_info["color_palette"]=u
        image_info[gif_file]=this_info
        f.close()
      except:
        pass

    def updateColorPalette(self,gif_file,other_palette=None,preread_lines=None):
      try:
        global col_palette
        if preread_lines==None: f=os.popen("gifsicle -I --color-info "+gif_file.replace(" ","\\ "))
        found_cols=0
        if preread_lines==None: flist=f.readlines()
        else: flist=preread_lines
        for ff in flist:
          fline=ff.replace("\n","")
          try: 
            if fline.strip().startswith("|"): 
              found_cols=1
              col_line=fline.replace("|","").replace("  "," ").replace(": #",":#")
              col_pairs=col_line.split(" ")
              for c in col_pairs:
                 cline=c.strip()
                 if cline:
                   cpair=cline.split(":")
                   if len(cpair)==2:  
                     rgb_val=getRGBForHex(cpair[1].strip())
                     if not rgb_val in col_palette.values():  col_palette[len(col_palette)]=rgb_val  # add to main palette
                     if not other_palette==None:  # add to individual palette
                       if not rgb_val in other_palette.values(): other_palette[cpair[0].strip()]=rgb_val
                       #other_palette[rgb_val]=rgb_val
              if not preread_lines==None: preread_lines.remove(ff)
            else:
               if found_cols==1: break
          except: 
            pass
        if preread_lines==None: f.close()
        return other_palette
      except:
        pass

    def setMainGifAttributes(self,gif_file):
      try:
        f=os.popen("gifsicle -I "+gif_file.replace(" ","\\ "))
        flist=f.readlines()
        for ff in flist:
          fline=ff.replace("\n","")
          try: 
            if fline.strip().startswith("end comment"): self.comment_entry.set_text(fline.replace("end comment","").strip())
            if fline.rfind("delay") > -1:  
              self.loopd_entry.set_text(str( int(string.atof(fline[fline.rfind("delay")+6:len(fline)].replace("s","").strip() )*100) )  )
            if fline.strip().startswith("loop count"): 
              loopcount=fline.replace("loop count","").strip()
              if loopcount.lower()=="forever":  loopcount=0
              loopcount=string.atoi(loopcount)            
              self.loopc_entry.set_text(str(loopcount))
              if loopcount==0: loopcount==1
              if loopcount ==1: 
                self.loopcheck.set_active(0)
              else: 
                self.loopcheck.set_active(1)
          except: 
            pass
        f.close()
        self.updateColorPalette(gif_file)
      except:
        pass

    def writePalette(self,*args):
      global col_palette
      global temp_dir
      if len(col_palette.keys()):
       rgb_tup=(self.bgpicker.get_i8()[0],self.bgpicker.get_i8()[1],self.bgpicker.get_i8()[2])
       if not rgb_tup in col_palette.values(): 
         cindex=len(col_palette)+1
         col_palette[str(cindex)]=rgb_tup
       try:
         f=open(temp_dir+"ganim_palette","w")
         for rgb_tup in col_palette.values():
           f.write(str(rgb_tup[0])+" "+str(rgb_tup[1])+" "+str(rgb_tup[2])+"\n")
           f.flush()
         f.close()
         return 1
       except:
        showStatus(" WARNING: Could not write specialized color palette to disk. Using 256 colors instead.")
        return 0
      return 0

    def doGifPreview(self,*args):
      if not haveGifview(): return
      try:
        g=self.generatePreview()
        if not g: return
        if not g[0]:
          showStatus("ERROR: Preview Gif animation could not be created.")
          showModalHelp("Your preview animation could not be created.\nHere is the beginning of the feedback sent by Gifsicle:\n\n"+str(g[1]))
        else: os.popen("gifview -a "+temp_dir+"ganim_preview.gif &")
      except:
        showStatus("ERROR: Preview Gif animation could not be created.")      

    def stopGifPreview(self,*args):
       os.popen("killall -q -9 gifview")

    def generatePreview(self,*args):
       global QUIT_PENDING
       if not len(self.images):
         showStatus(" No GIFs currently loaded. Cannot create an empty GIF.")
         return  
       try:
         if not string.atoi(self.loopd_entry.get_text().strip()) > -1:
           if not QUIT_PENDING: showStatus(" ERROR:  Loop Delay must be a POSITIVE number. ")
           if not QUIT_PENDING: showModalHelp(" ERROR:  Loop Delay must be a POSITIVE number. ")
           return
         if not string.atoi(self.loopc_entry.get_text().strip()) > -1:
           if not QUIT_PENDING: showStatus(" ERROR:  Loop Count must be a POSITIVE number. ")
           if not QUIT_PENDING: showModalHelp(" ERROR:  Loop Count must be a POSITIVE number. ")
           return
       except:
         if not QUIT_PENDING: showStatus(" ERROR: The Loop Delay and Loop Count must be INTEGERS")
         if not QUIT_PENDING: showModalHelp(" ERROR: The Loop Delay and Loop Count must be INTEGERS")
         return
       global col_palette
       global image_info
       global temp_dir
       os.popen("rm -f "+temp_dir+"ganim_preview.gif")
       if not QUIT_PENDING: showStatus(" Generating preview...")
       global dim_scalex
       global dim_scaley
       scale_it=""
       if dim_scalex or dim_scaley:
         if dim_scalex: scale_it=" --resize "+str(dim_scalex)+"x"
         else:  scale_it=" --resize _x"
         if dim_scaley: scale_it=scale_it+str(dim_scaley)+" "
         else:  scale_it=scale_it+"_ "
       colmap=" --colors 256  "
       if len(col_palette) <  256:  # use a smaller palette
           self.cleanupPalette()
           p=self.writePalette()
           if not p: colmap=" --colors 256  "
           else: 
             colmap=" --use-colormap "+temp_dir+"ganim_palette  "
       if self.comment_entry.get_text().strip(): 
           gcmd="gifsicle "+colmap+scale_it+" --comment \""+self.comment_entry.get_text().strip().replace("\"","'").replace("\n","  ").replace("\t","  ")+"\" "
           for i in self.images.values():
             f=os.popen("gifsicle --no-comments "+i+" -o "+i)
             f.readline()
             f.close()
             f=os.popen("gifsicle --comment \""+self.comment_entry.get_text().strip().replace("\"","'").replace("\n","  ").replace("\t","  ")+"\" "+i+" -o "+i)
             f.readline()
             f.close()
       else: 
           gcmd="gifsicle "+colmap+scale_it
           for i in self.images.values():
             f=os.popen("gifsicle --no-comments "+i+" -o "+i)
             f.readline()
             f.close()
       if self.noradio.get_active(): gcmd=gcmd+" -U --careful"
       if self.intercheck.get_active(): gcmd=gcmd+" -i "
       anim=1
       if len(self.images)==1: anim=0
       if anim:
         gcmd=gcmd+" --delay "+self.loopd_entry.get_text().strip()
         if not self.loopcheck.get_active():
           gcmd=gcmd+" --no-loopcount"
         else:
           if self.loopc_entry.get_text().strip()=="0": gcmd+" --loopcount=forever"
           else: gcmd=gcmd+" --loopcount="+self.loopc_entry.get_text().strip()
       rgb=self.bgpicker.get_i8()
       gcmd=gcmd+str(" --background "+str(rgb[0])+"/"+str(rgb[1])+"/"+str(rgb[2]))
       il=self.images.keys()
       il.sort()
       for i in il:
         img=self.images[i]
         if image_info.has_key(img):
           d=image_info[img]
           try:
             if d["transparent"]==1:
               rgb=col_palette[str(d["transparent_index"])]
               gcmd=gcmd+str(" --transparent "+str(rgb[0])+"/"+str(rgb[1])+"/"+str(rgb[2]))
             else: gcmd=gcmd+" --no-transparent"
           except:
             pass
         gcmd=gcmd+" "+img
       if self.lightradio.get_active(): gcmd=gcmd+" -O1"
       if self.heavyradio.get_active(): gcmd=gcmd+" -O2"
       gcmd=gcmd+" -o "+temp_dir+"ganim_preview.gif"
       f=os.popen(gcmd)
       feedback=f.readline()
       feedback=feedback+f.readline()
       feedback=feedback+f.readline()
       feedback=feedback+f.readline()
       f.close()
       created=0
       try:
         gfilesize=os.path.getsize(temp_dir+"ganim_preview.gif")
	 self.session_created[temp_dir+"ganim_preview.gif"]="1"
         if gfilesize > 0: 
           created=1
           if not QUIT_PENDING: showStatus(translate(" Gif created. Animation size is ")+str(gfilesize)+translate(" bytes."))
       except:
         created=0
       return [created,feedback]

## Image Frame class

class ImgFrame(GtkRadioButton) :
    def __init__ (self,brother=None) :
	GtkRadioButton.__init__(self,brother,"")
	imgframe=self
        imgframe.set_name("ImgFrame")
        try: # apply the RC style from 'framerc' file
          imgframe.set_rc_style()
          imgframe.ensure_style()
        except:
          pass
        imgframe.remove(imgframe.children()[0])
        imgframe.set_mode(0)
        tog_sig=imgframe.connect("toggled",self.doHighlight)
        self.tog_sig=tog_sig
        global TIPS
        self.tips=TIPS   
        image_no=-1
        self.image_no=-1
	self.imgframe = imgframe
        imdim=(0,0)
        imsize=0
        self.imsize=imsize
        self.imdim=imdim
        ganim=None
        self.ganim=ganim
        image=None
        self.image=image
	vbox1 = GtkVBox (0, 0)
	vbox1.set_border_width (2)
	self.vbox1 = vbox1
        hh=GtkHBox(0,0)
        bl=getIconButton("./pixmaps/ganim8_left.xpm","<-")
        setTip(bl,"Move Left")
        br=getIconButton("./pixmaps/ganim8_right.xpm","->")
        setTip(br,"Move Right")
        framelab=GtkLabel("")
        bl.connect("clicked",self.moveLeft)
        br.connect("clicked",self.moveRight)
        hh.pack_start(bl,0,0,2)
        hh.pack_start(framelab,1,1,0)
        hh.pack_start(br,0,0,2)
        self.framelab=framelab
        vbox1.pack_start(hh,1,1,0)
        vbox1.pack_start(GtkHSeparator(),0,0,0)
	hbox1 = GtkHBox (0, 0)
	self.hbox1 = hbox1
        fbox=GtkHBox(0,0)
        self.fbox=fbox
	imgplacehold = GtkLabel (translate(" ERROR: Image not found or could not be loaded via Imlib"))  # replace with pixmap
	self.imgplacehold = imgplacehold
	fbox.pack_start (imgplacehold, 1, 1, 0)
	hbox1.pack_start (fbox, 1, 1, 0)
	hbox1.pack_start (GtkLabel("      "), 1, 1, 0)
	vbox2 = GtkVBox (0, 0)
	vbox2.set_border_width (2)
	self.vbox2 = vbox2
	selbutt = getIconButton("./pixmaps/ganim8_open.xpm","Change")
        setTip(selbutt,"Replace this image with another image")
        selbutt.connect("clicked",self.doChange)
	self.selbutt = selbutt
	vbox2.pack_start(selbutt,0,0,0)
	savebutt = getIconButton("./pixmaps/ganim8_save.xpm","Save...")
	self.savebutt = savebutt
        savebutt.connect("clicked",self.extractToFile)
        setTip(savebutt,"Save this frame to a permanent file")
	vbox2.pack_start(savebutt,0,0,0)
	delbutt = getIconButton("./pixmaps/ganim8_delete.xpm","Delete")
        setTip(delbutt,"Delete this image from the animation sequence")
	self.delbutt = delbutt
	vbox2.pack_start(delbutt,0,0,0)
        delbutt.connect("clicked",self.doDelete)
	editbutt =getIconButton("./pixmaps/ganim8_edit.xpm","Edit")
        setTip(editbutt,"Edit this image with your default image editor")
	self.editbutt = editbutt
        self.editbutt.connect("clicked",self.doEdit)
	vbox2.pack_start(editbutt, 0,0,0)
	refbutt = getIconButton("./pixmaps/ganim8_reload.xpm","Refresh")
        setTip(refbutt,"Reload this image if it has been edited or modified.")
	self.refbutt = refbutt
        refbutt.connect("clicked",self.refresh,self)
	vbox2.pack_start(refbutt, 0,0,0)
	palbutt = getIconButton("./pixmaps/ganim8_palette.xpm","Colors")
	self.palbutt = palbutt  
        palbutt.connect("clicked",self.doPaletteEdit)
        setTip(palbutt,"Show the color palette and edit colors for this image")
	vbox2.pack_start(palbutt,0,0,0)
	greybutt = getIconButton("./pixmaps/ganim8_grey.xpm","GreyScale")
        setTip(greybutt,"Convert to GreyScale (black and white)")
	self.greybutt = greybutt  
        greybutt.connect("clicked",self.doGrey)
	vbox2.pack_start(greybutt,0,0,0)
	rotbutt = getIconButton("./pixmaps/ganim8_rotate.xpm","Flip/Rotate")
        setTip(rotbutt,"Flip or Rotate this image.")
	self.rotbutt = rotbutt  
        rotbutt.connect("clicked",self.doRotate)
	vbox2.pack_start(rotbutt,0,0,0)
	vbox2.pack_start(GtkLabel (" "), 1,1, 0)
	hbox1.pack_start (vbox2, 0, 0, 0)
	vbox1.pack_start (hbox1, 1, 1, 1)
        vbox1.set_spacing(3)


    def setImage(self,pmap,real_name=None,ncolors="Unknown",*args):
       if not pmap: return 0
       try:
         i=GdkImlib.Image(pmap)
         i.render()
         i.changed_image() # reload from disk, not GdkImlib cache, force GdkImlib to refresh
         p=GdkImlib.GtkPixmap(i.get_pixmap()[0])
         if not len(self.imgframe.children())==0: self.imgframe.remove(self.imgframe.children()[0])
         self.fbox.remove (self.imgplacehold)
         self.imgplacehold.destroy()
         self.imgplacehold.unmap()
         self.fbox.pack_start(p,1,1,0)
         p.show_all()
         self.imdim=p.size_request()
         try:
           self.imsize=os.path.getsize(pmap)
         except:
           self.imsize=0
         self.imgplacehold=p
         self.image=pmap
         iname=pmap
         if real_name: iname=real_name
         if iname.rfind("/") > 0:  iname=iname[iname.rfind("/")+1:len(iname)].replace("/","").strip()         
         if len(iname) > 20: self.framelab.set_text(iname[0:19]+"...")
         else: self.framelab.set_text(iname)
         self.framelab.show_all()
         if real_name:
           setTip(self.imgplacehold,real_name+translate("  \nNumber of Colors: ")+str(ncolors)+translate("\n Temporary File: ")+pmap)
           setTip(self.imgframe,real_name+translate("  \nNumber of Colors: ")+str(ncolors)+translate("\n Temporary File: ")+pmap)
         else:
           setTip(self.imgplacehold,pmap)
           setTip(self.imgframe,pmap)
         self.fbox.show_all()
         self.vbox1.resize_children()
         self.vbox1.show_all()
         self.imgframe.add(self.vbox1)         
         self.imgframe.show_all()
         return 1
       except:
         return 0    

    def refresh(self,component,gif_file,*args):
      self.forceHighlight()      
      if self.ganim: 
       try:
        self.ganim.updateLayout()        
        self.ganim.setFrameAttributes(gif_file.image)
        self.ganim.setActiveFrame(self)    
       except:
        pass

    def doChange(self,*args):
       self.forceHighlight()
       self.tips.disable()
       if self.ganim: self.ganim.askGifChange()
       self.tips.enable()

    def moveLeft(self,*args):
      self.forceHighlight()
      self.tips.disable()
      if self.ganim: self.ganim.moveFrame(self,-1)
      self.tips.enable()

    def moveRight(self,*args):
      self.forceHighlight()
      self.tips.disable()
      if self.ganim: self.ganim.moveFrame(self,1)
      self.tips.enable()

    def doRotate(self,*args):
      self.forceHighlight()
      self.tips.disable()
      if self.ganim: self.ganim.askRotation()
      self.tips.enable()

    def doGrey(self,*args):
      self.forceHighlight()
      self.tips.disable()
      if self.ganim: self.ganim.doGreyScale()
      self.tips.enable()

    def doDelete(self,*args):
      #self.forceHighlight() # do not uncomment-creates a bug
      if self.ganim: self.ganim.doDelete(self.image_no)

    def doPaletteEdit(self,*args):
      self.forceHighlight()
      self.tips.disable()
      if self.ganim: self.ganim.showColorEditor(self.imgframe,self.image)
      self.tips.enable()

    def doEdit(self,*args):
      self.forceHighlight()
      if self.ganim: self.ganim.doEdit(self.image)

    def extractToFile(self,*args):
      self.forceHighlight()
      self.tips.disable()
      if self.ganim: self.ganim.extractToFile(self.image)
      self.tips.enable()

    def setImageNo(self,inum):
      self.image_no=inum

    def setGanim(self,g):
      self.ganim=g

    def forceHighlight(self,*args):
      self.tips.enable()
      self.imgframe.set_active(1)

    def doHighlight(self,*args):
      self.tips.enable()
      if self.imgframe.get_active():
        if self.ganim: self.ganim.setActiveFrame(self)


## HELP SECTION

class icehelp :
    def __init__ (self) :
	icehelp = GtkWindow (GTK.WINDOW_TOPLEVEL)
	self._root = icehelp
        icehelp.realize()
	icehelp.set_title (translate("gAnim8 Help"))
	icehelp.set_policy (1, 1, 0)
	icehelp.set_position (GTK.WIN_POS_CENTER)
        icehelp.set_usize(300,375)
        icehelp.connect("destroy",self.hideh)
	setTip(icehelp,"gAnim8 Help")
	self.icehelp = icehelp
	vbox1 = GtkVBox (0, 0)
	vbox1.set_border_width (11)
	self.vbox1 = vbox1
	scrolledwindow1 = GtkScrolledWindow ()
	self.scrolledwindow1 = scrolledwindow1
	helptext =GtkText ()
	helptext.set_editable (0)
        helptext.set_word_wrap(1)
	self.helptext = helptext
	scrolledwindow1.add (helptext)
	vbox1.pack_start (scrolledwindow1, 1, 1, 0)
	closebutt = getPixmapButton("./pixmaps/ganim8_cancel.xpm","Close")
        closebutt.connect("clicked",self.hideh)
	setTip(closebutt, "Close this help window")
	self.closebutt = closebutt
	vbox1.pack_start (closebutt, 0, 0, 13)
	icehelp.add (vbox1)
        self.setHelpText("Generic Help File")

    def setHelpText(self, helpstuff):
      try:
       self.helptext.delete_text(0,self.helptext.get_length())
       self.helptext.delete_text(0,self.helptext.get_length()+1)
      except:
       pass
      self.helptext.insert_defaults("\n"+str(helpstuff))

    def appendHelpText(self, helpstuff):
       self.helptext.insert_defaults("\n"+str(helpstuff))

    def hideh(self,*args):      
      self.icehelp.hide()
      self.icehelp.destroy()
      self.icehelp.unmap()
      return TRUE

# Color Palette

def get_Color8(color_i16):
   global silent_cp
   if not silent_cp: silent_cp=GnomeColorPicker()
   silent_cp.set_i16(color_i16[0], color_i16[1],color_i16[2] ,255)
   return (silent_cp.get_i8()[0],silent_cp.get_i8()[1] ,silent_cp.get_i8()[2])

def get_Color16(color_i8):
   global silent_cp
   if not silent_cp: silent_cp=GnomeColorPicker()
   silent_cp.set_i8(color_i8[0], color_i8[1],color_i8[2] ,255)
   return (silent_cp.get_i16()[0],silent_cp.get_i16()[1] ,silent_cp.get_i16()[2])

class palettewin :
    def __init__ (self,paletteType=1,colorPalette={},okCmd=None,cancelCmd=None,selected_color=None) :
	palettewin = GtkWindow (GTK.WINDOW_TOPLEVEL)
	self._root = palettewin
	palettewin.realize()
	palettewin.set_title (translate("Color Palette"))
	palettewin.set_position (GTK.WIN_POS_CENTER)
	self.palettewin = palettewin
	vbox1 = GtkVBox(0, 3)
	self.vbox1 = vbox1
        self.paletteType=paletteType
        self.selected_color=selected_color
	vbox1.pack_start(GtkLabel (translate("Color Palette")), 0, 0, 3)
	scrolledwindow1 = GtkScrolledWindow ()
	scrolledwindow1.set_border_width (2)
	scrolledwindow1.set_usize (165, 198)
	self.scrolledwindow1 = scrolledwindow1
	viewport1 =GtkViewport ()
	self.viewport1 = viewport1
        snapper=4
        if paletteType==2: snapper=3
        self.snapper=snapper
        if colorPalette:  table1 = GtkTable (snapper, math.ceil(len(colorPalette)/snapper), 1)
	else: table1 = GtkTable (3, 2, 1)
	self.table1 = table1
	scrolledwindow1.add (viewport1)
        viewport1.add(table1)
	vbox1.pack_start(scrolledwindow1, 0, 0, 0)
	hbox1 = GtkHBox (1, 0)
	self.hbox1 = hbox1
	ok_button = GtkButton(translate("OK"))
	self.ok_button = ok_button
	hbox1.pack_start (ok_button, 1, 1, 0)
        setTip(ok_button,"Apply the color changes to this image")
        if okCmd: ok_button.connect("clicked",okCmd)
	hbox1.pack_start (GtkLabel ("              "), 1, 1, 0)
	cancel_button = GtkButton(translate("Cancel"))
	self.cancel_button = cancel_button
        if cancelCmd: cancel_button.connect("clicked",cancelCmd)
	hbox1.pack_start (cancel_button, 1, 1, 0)
        setTip(cancel_button,"Close this window without applying the color changes.")
	vbox1.pack_start(hbox1,  0, 0, 0)
	vbox1.set_border_width(3)
	palettewin.add (vbox1)
        color_buttons=[]
        self.color_buttons=color_buttons
        self.showPalette(copy.copy(colorPalette))
        self.table1.show_all()

    def getSelectedColor(self,*args):
      if self.paletteType==2: 
        for c in self.color_buttons:
          if c.isSelected(): return c.original_color

    def getChangedColors(self,*args):
      if self.paletteType==1: 
        changed_col={}
        for c in self.color_buttons:
          if c.isChanged(): changed_col[c.original_color]=c.new_color
        return changed_col

    def showPalette(self,cPalette):
      if not cPalette: return
      l=cPalette.values()
      l.sort()
      widgetcounter=0
      lrcounter=0
      tpcounter=0
      rdc=None
      showStatus("Loading palette...")
      for i in l:
        ccolor=get_Color16(i)
        color_button=ColorButton(ccolor,self.paletteType,rdc)
	color_button.b.set_rc_style()
	color_button.b.ensure_style
        if not rdc: rdc=color_button.button
        if self.selected_color==i: 
          if self.paletteType==2:
            color_button.button.set_active(1)
        self.table1.attach(color_button.button, lrcounter, lrcounter+1, tpcounter, tpcounter+1, (GTK.FILL), (0), 2, 2)
        self.color_buttons.append(color_button)
        setTip(color_button.button,str(i))
        if self.paletteType==1: color_button.button.connect("clicked",showPaletteEdit,color_button)
        lrcounter=lrcounter+1
        widgetcounter=widgetcounter+1
        if widgetcounter ==  self.snapper:  
          tpcounter=tpcounter+1
          lrcounter=0
          widgetcounter=0
      showStatus(translate(" Palette loaded.  Palette has ")+str(len(cPalette))+translate(" colors."))

## Color Button Widget

def showPaletteEdit(self,component,*args):
      global color_selection
      global active_color_button
      active_color_button=component
      colR=active_color_button.get_i8()
      colR= ( colR[0]/255.0,  colR[1]/255.0 , colR[2]/255.0 )
      color_selection.set_title(translate("Select A New Color..."))
      color_selection.colorsel.set_color(colR)
      color_selection.set_modal(1)
      color_selection.show_all()

class ColorButton:
   def __init__(self,initial_color_i16,btype=1,radio_connector=None):
     if btype==2: 
	b=GtkRadioButton(radio_connector,"")
	b.set_mode(0)
     else: b=GtkButton("")
     self.b=b
     b.remove(b.children()[0]) # looks dumb, but is necessary    
     self.btype=btype
     v=GtkVBox(0,0)
     f=GtkFrame()
     f.set_shadow_type(GTK.SHADOW_ETCHED_OUT)
     global drawing_area
     drawing_area = GtkDrawingArea()
     drawing_area.size(20, 12)
     drawing_area.show() 
     self.drawing_area=drawing_area
     f.add(drawing_area)
     v.pack_start(f,0,0,3)
     b.add(v)
     b.show_all()
     self.button=b
     self.gc=None
     self.original_color=initial_color_i16
     self.new_color=initial_color_i16
     self.win=drawing_area.get_window()
     drawing_area.connect("expose_event", self.expose_event)
     drawing_area.set_events(GDK.EXPOSURE_MASK |
     GDK.LEAVE_NOTIFY_MASK |
     GDK.BUTTON_PRESS_MASK )

   def expose_event(self,*args):
     self.setColor(self.new_color)

   def isChanged(self,*args):
     return (self.new_color==self.original_color)==0

   def isSelected(self,*args):
      if not self.btype==2: return 0
      return self.button.get_active()

   def get_i8(self):
     return get_Color8(self.new_color)

   def set_i8(self,redt,greent,bluet,alphat=255):
     self.setColor(get_Color16((redt,greent,bluet)))

   def setColor(self,color_i16):
     if not self.win: self.win=self.drawing_area.get_window()
     if not self.gc: self.gc=self.win.new_gc()
     self.gc.foreground=self.win.colormap.alloc(color_i16[0],color_i16[1],color_i16[2])
     self.new_color=color_i16
     draw_rectangle(self.win,self.gc,TRUE,0,0,self.win.width,self.win.height)
     return FALSE 

## Image Rotation Widget

class rotator:
    def __init__(self):
	rotwin = GtkWindow (GTK.WINDOW_TOPLEVEL)
	self._root = rotwin
        rotwin.realize()
	rotwin.set_title (translate("Flip/Rotate Image"))
	rotwin.set_position (GTK.WIN_POS_CENTER)
	rotwin.set_modal (1)
	self.rotwin = rotwin
	vbox1 = GtkVBox (0, 0)
	vbox1.set_border_width (9)
	self.vbox1 = vbox1
        rothor=GtkRadioButton(None,translate("Flip Horizontal"))
        self.rothor=rothor
        vbox1.pack_start(rothor,1,1,2)
        rotvert=GtkRadioButton(rothor,translate("Flip Vertical"))
        self.rotvert=rotvert
        vbox1.pack_start(rotvert,1,1,2)
        rot90=GtkRadioButton(rothor,translate("Rotate 90 Degrees"))
        self.rot90=rot90
        vbox1.pack_start(rot90,1,1,2)
        rot180=GtkRadioButton(rothor,translate("Rotate 180 Degrees"))
        self.rot180=rot180
        vbox1.pack_start(rot180,1,1,2)
        rot270=GtkRadioButton(rothor,translate("Rotate 270 Degrees"))
        self.rot270=rot270
        vbox1.pack_start(rot270,1,1,2)
        hbox1=GtkHBox(1,0)
        ok_button=getPixmapButton("./pixmaps/ganim8_ok.xpm","Apply Change")
        self.ok_button=ok_button
        setTip(ok_button,"Apply the selected change")
        hbox1.pack_start(ok_button,1,1,0)
        hbox1.pack_start(GtkLabel(" "),1,1,0)
        cancel_button=getPixmapButton("./pixmaps/ganim8_cancel.xpm","Close")
        setTip(cancel_button,"Close this window")
        self.cancel_button=cancel_button
        hbox1.pack_start(cancel_button,1,1,0)
        cancel_button.connect("clicked",self.doClose)
        vbox1.pack_start(hbox1,1,1,4)
	rotwin.add (vbox1)

    def doClose(self,*args):
      self.rotwin.hide()
      self.rotwin.destroy()
      self.rotwin.unmap()
      return TRUE

## Editor Dialog Box
def showCmdLineOther(*args):
  showEditorWin(args[0]-2000)

def showEditorWin(dia_type=1,*args):
  global editor_win
  global editor_cmd
  global mpeg_cmd
  global avi_cmd
  global swf_cmd
  global rm_cmd
  global asf_cmd
  global wmv_cmd
  global m4v_cmd
  global dialog_type_ed
  dialog_type_ed=dia_type
  editor_win=editorwin(dia_type)
  editor_win.editor_entry.set_text(str(editor_cmd).strip())
  if dia_type==2: 
   editor_win.editor_entry.set_text(str(mpeg_cmd).strip())
  if dia_type==3: 
   editor_win.editor_entry.set_text(str(avi_cmd).strip())
  if dia_type==4: 
   editor_win.editor_entry.set_text(str(swf_cmd).strip())
  if dia_type==5: 
   editor_win.editor_entry.set_text(str(rm_cmd).strip())
  if dia_type==6: 
   editor_win.editor_entry.set_text(str(asf_cmd).strip())
  if dia_type==7: 
   editor_win.editor_entry.set_text(str(wmv_cmd).strip())
  if dia_type==8: 
   editor_win.editor_entry.set_text(str(m4v_cmd).strip())
  editor_win.ok_button.connect("clicked",setEditor)
  editor_win.cancel_button.connect("clicked",hideEditorWin)
  editor_win.editorwin.show_all()

def hideEditorWin(*args):
  global editor_win
  editor_win.editorwin.hide()
  editor_win.editorwin.destroy()
  editor_win.editorwin.unmap()
  editor_win=None

def setEditor(dia_type=1,*args):
  global editor_win
  global editor_cmd
  global mpeg_cmd
  global avi_cmd
  global swf_cmd
  global rm_cmd
  global asf_cmd
  global wmv_cmd
  global m4v_cmd
  global dialog_type_ed
  if not editor_win.getEditor(): 
    showStatus(" ERROR:  You did not fill in the field properly. This field cannot be blank.")
    return
  e=editor_win.getEditor()
  if e.find("%f")== -1: e=e+" %f"
  if  dialog_type_ed==1: editor_cmd=e
  if  dialog_type_ed==2: mpeg_cmd=e
  if dialog_type_ed==3: avi_cmd=e
  if  dialog_type_ed==4: swf_cmd=e
  if  dialog_type_ed==5: rm_cmd=e
  if  dialog_type_ed==6: asf_cmd=e
  if  dialog_type_ed==7: wmv_cmd=e
  if  dialog_type_ed==8: m4v_cmd=e
  hideEditorWin()
  saveOptions()

class editorwin :
    def __init__ (self,dia_type=1) :        
	editorwin = GtkWindow (GTK.WINDOW_TOPLEVEL)
        editorwin.realize()
 	editorwin.set_title (translate("Image Editor Command"))
        if dia_type==2: editorwin.set_title (translate("MPEG Video Previewer"))
        if dia_type==3: editorwin.set_title (translate("AVI Video Previewer"))
        if dia_type==4: editorwin.set_title (translate("SWF Video Previewer"))
        if dia_type==5: editorwin.set_title (translate("RealPlayer Video Previewer"))
        if dia_type==6: editorwin.set_title (translate("ASF Video Previewer"))
        if dia_type==7: editorwin.set_title (translate("WMV Video Previewer"))
        if dia_type==8: editorwin.set_title (translate("M4V Video Previewer"))
	editorwin.set_position (GTK.WIN_POS_CENTER)
	editorwin.set_modal (1)
	self.editorwin = editorwin
	vbox1 = GtkVBox (0, 0)
	vbox1.set_border_width (9)
        if dia_type==1: vbox1.pack_start( GtkLabel (translate("Enter the command to launch your image editor")), 0, 0, 0)
        if dia_type==2: vbox1.pack_start( GtkLabel (translate("Enter the command to launch your MPEG-1 video player.")), 0, 0, 0)
        if dia_type==3: vbox1.pack_start( GtkLabel (translate("Enter the command to launch your AVI player.")), 0, 0, 0)
        if dia_type==4: vbox1.pack_start( GtkLabel (translate("Enter the command to launch your SWF (Shockwave Flash) video player.")), 0, 0, 0)
        if dia_type==5: vbox1.pack_start( GtkLabel (translate("Enter the command to launch your RM (RealPlayer) video player.")), 0, 0, 0)
        if dia_type==6: vbox1.pack_start( GtkLabel (translate("Enter the command to launch your ASF video player.")), 0, 0, 0)
        if dia_type==7: vbox1.pack_start( GtkLabel (translate("Enter the command to launch your WMV (Windows Media Video) player.")), 0, 0, 0)
        if dia_type==8: vbox1.pack_start( GtkLabel (translate("Enter the command to launch your M4V video player.")), 0, 0, 0)
	vbox1.pack_start(GtkLabel (translate("Include a %f where the filename should go.")), 0, 0, 0)
	editor_entry = GtkEntry ()
	self.editor_entry = editor_entry
	vbox1.pack_start(editor_entry, 0, 0, 11)
	hbox1 = GtkHBox (1, 0)
	self.hbox1 = hbox1
	ok_button = getPixmapButton("./pixmaps/ganim8_ok.xpm"," OK")
        setTip(ok_button,"Save changes and close")
	self.ok_button = ok_button
	hbox1.pack_start (ok_button, 1, 1, 0)
	hbox1.pack_start (GtkLabel ("             "), 1, 1, 0)
	cancel_button = getPixmapButton("./pixmaps/ganim8_cancel.xpm","Cancel")
        setTip(cancel_button,"Close this window without saving changes")
	self.cancel_button = cancel_button
	hbox1.pack_start (cancel_button, 1, 1, 0)
	vbox1.pack_start(hbox1, 1, 1, 0)
	editorwin.add (vbox1)

    def getEditor(self,*args):
      return self.editor_entry.get_text().strip()

# Dimension Widget

class gdimension :
    def __init__ (self) :
	gdimension =GtkWindow (GTK.WINDOW_TOPLEVEL)
	self._root = gdimension
	gdimension.realize()
	gdimension.set_title (translate("Animation Dimensions"))
	gdimension.set_position (GTK.WIN_POS_CENTER)
	gdimension.set_modal (1)
	self.gdimension = gdimension
	vbox1 = GtkVBox (0, 0)
        vbox1.set_border_width(5)
	self.vbox1 = vbox1
	vbox1.pack_start(GtkLabel (translate("Set the width and height for this animation")), 0, 0, 0)
	vbox1.pack_start(GtkLabel (translate("or set to 0x0 to accept the natural dimensions")), 0, 0, 0)
	hbox1 = GtkHBox (1, 0)
        hbox1.set_border_width (18)
	self.hbox1 = hbox1
	width_entry = GtkEntry ()
        setTip(width_entry,translate("Enter the desired WIDTH"))
	width_entry.set_usize (42, -2)
	self.width_entry = width_entry
	hbox1.pack_start(width_entry, 0, 0, 0)
	hbox1.pack_start(GtkLabel ("X"), 1, 1, 0)
	height_entry = GtkEntry ()
	height_entry.set_usize (42, -2)
        setTip(height_entry,translate("Enter the desired HEIGHT"))
	self.height_entry = height_entry
	hbox1.pack_start(height_entry, 0, 0, 0)
	vbox1.pack_start(hbox1, 0, 0, 7)
	hbox2 = GtkHBox (1, 0)
	hbox2.set_border_width (8)
	self.hbox2 = hbox2
	ok_button = getPixmapButton("./pixmaps/ganim8_ok.xpm"," OK")
        setTip(ok_button,"Accept changes and close this window")
	self.ok_button = ok_button
	hbox2.pack_start(ok_button, 1, 1, 5)
	cancel_button =getPixmapButton("./pixmaps/ganim8_cancel.xpm","Cancel")
        setTip(cancel_button,"Close this window without saving changes.")
	self.cancel_button = cancel_button
	hbox2.pack_start(cancel_button, 1, 1, 5)
	hbox2.pack_start(GtkLabel ("        "), 1, 1, 0)
	help_button = getPixmapButton("./pixmaps/ganim8_help.xpm","Help Me!")
        setTip(help_button,"Get help on setting dimensions and scaling images")
	self.help_button = help_button
	hbox2.pack_start(help_button, 1, 1, 3)
	vbox1.pack_start(hbox2, 1, 1, 0)
        global dim_scalex
        global dim_scaley
        width_entry.set_text(str(dim_scalex))
        height_entry.set_text(str(dim_scaley))
	gdimension.add (vbox1)

    def getDimensions(self) :
      return (self.width_entry.get_text().strip(),self.height_entry.get_text().strip())


## Class uses ffmpeg to decode  Mpeg/Avi/RealMedia/Flash/Asf/WMV/MOV Video files ##
class MovieDecoder:
  def __init__(self):
     self.my_path=""
     self.images={}
     self.frames_list=[]

  def decode(self,mpeg_file,mpath=""):
    self.cleanup()
    self.my_path=mpath
    if self.my_path==None: self.my_path=""
    if not self.my_path.endswith("/"): self.my_path=self.my_path+"/"
    self.cleanup()
    self.my_path=mpath
    if self.my_path==None: self.my_path=""
    if not self.my_path.endswith("/"): self.my_path=self.my_path+"/"
    if os.path.exists(mpeg_file):
      #f=os.popen("mpeg2decode -q -b "+mpeg_file.replace(" ","\\ ")+" -f -o3 "+self.my_path+"ganim_mpeg%d") # will extract all frames to PPM (.ppm) files
      f=os.popen("ffmpeg -i "+mpeg_file.replace(" ","\\ ")+" "+self.my_path+"ganim_mpeg%d.jpeg")
      f.readlines()
      f.close()
      l=os.listdir(self.my_path)
      counter=1
      for i in l:
        if i.startswith("ganim_mpeg"): 
          self.images[counter]=self.my_path+i
          self.frames_list.append(self.my_path+i)
          counter=counter+1
	  if counter>=201: break # number of frames we can safely display on X, setting higher causes X crash
      return self.getFrameCount()

  def getFrame(self,frame_num):
    if self.images.has_key(frame_num): return str(self.images[frame_num])
    return None

  def getFrameCount(self):
    return len(self.images)

  def getFrameList(self):
     return copy.copy( self.frames_list)

  def cleanup(self):
    os.popen("rm -f "+self.my_path+"ganim_mpeg*")
    self.images.clear()
    self.frames_list=[]
    self.mpeg=None
    self.my_path=""

## Mpeg/Avi/RealMedia/Flash/Asf/MOV/WMV frame previewer

class framesel :
    def __init__ (self,ganim_instance) :
	framesel = GtkWindow(GTK.WINDOW_TOPLEVEL)
        self.targets = [('STRING', 0, 0), ('text/plain', 0,1), ('text/uri-list', 0, 2) ]
	self._root = framesel
        self.GANIM=ganim_instance
        self.num_images=0
	framesel.realize()
        framesel.connect("destroy",self.closeit)
        framesel.connect("delete_event",self.closeit)
	framesel.set_title (translate("Select Frames For Extraction..."))
	framesel.set_position (GTK.WIN_POS_CENTER)
	framesel.set_modal (1)
	self.framesel = framesel
	vbox1 = GtkVBox (0, 0)
	vbox1.set_border_width (3)
	self.vbox1 = vbox1
	scroller = GtkScrolledWindow ()
        scroller.set_usize(350,200)
	self.scroller = scroller

	frameshbox = GtkHBox (0, 0)
        frameshbox.set_spacing(5)        
	self.frameshbox = frameshbox
        toplay=GtkLayout(None,None)
        self.toplay=toplay
        toplay.set_size(100,100)
        toplay.set_hadjustment(scroller.get_hadjustment())
	scroller.add(toplay)

	vbox1.pack_start (scroller, 1, 1, 0)
	vbox2 = GtkVBox (0, 0)
	vbox2.set_border_width (3)
	hbox3 = GtkHBox (0, 0)
	vbox2.pack_start (GtkLabel(translate("Drag and drop frames onto the GAnim8 Window")), 0, 0, 0)
	vbox2.pack_start (GtkLabel(translate("or select a range of frames below.")), 0, 0, 0)
	vbox2.pack_start (GtkHSeparator(), 0, 0, 4)
	label3 = GtkLabel (translate("Frames to extract:  "))
	hbox3.pack_start ( label3, 0, 0, 0)
	frameentry = GtkEntry ()
	self.frameentry = frameentry
	hbox3.pack_start ( frameentry, 1, 1, 0)
	vbox2.pack_start ( hbox3, 1, 0, 0)
	label2 = GtkLabel (translate("Examples:   1-8   or   2,4,6,8    or   1-8,12,34,56"))
	vbox2.pack_start (label2, 0, 0, 0)
	vbox1.pack_start ( vbox2, 0, 0, 4)
	hbox1 = GtkHBox (1, 0)
	self.hbox1 = hbox1
	okbutt =GtkButton(translate("OK"))
	self.okbutt = okbutt
	hbox1.pack_start (okbutt, 1, 1, 0)
        okbutt.connect("clicked",self.extractFrames)
	hbox1.pack_start (GtkLabel(" "), 1, 0, 0)
	cancelbutt =GtkButton(translate("CANCEL"))
        cancelbutt.connect("clicked",self.closeit)
	self.cancelbutt = cancelbutt
	hbox1.pack_start (cancelbutt, 1, 1, 0)
	vbox1.pack_start (hbox1, 0, 0, 10)
	framesel.add ( vbox1)
	framesel.show_all ()

    def setDragAway(self,*args):
	#print str(args)
	drago=args
	if len(drago)<4: 
		args[0].drag_highlight()
		args[0].grab_focus()
	else: 
		drago[2].set(drago[2].target,8,drago[5][0])

    def addFrames(self,frames_list):
      h=2
      w=2
      for pmap in frames_list:
         try:
           try:
             i=GdkImlib.Image(pmap)
             i.render()
             i.changed_image() # reload from disk, not GdkImlib cache, force GdkImlib to refresh
             p=GdkImlib.GtkPixmap(i.get_pixmap()[0])
           except:
             p=GtkLabel(translate("Frame could not be loaded"))
           f=GtkFrame()
           f.set_label(translate("Frame ")+str(self.num_images+1))
	   addon=f.size_request()[1]+2
	   p.show_all()
	   f.add(p)
	   f.show_all()
           hh=GtkLayout(None,None)
	   neededy=p.size_request()[1]+addon	
	   neededx=f.size_request()[0]+3
	   hh.set_size(neededx,neededy)
	   f.remove(p)
	   hh.put(p,0,0)
	   hh.show_all()
           f.add(hh)
	   f.set_usize(neededx,neededy)
           f.show_all()
           hh.drag_source_set(gtk.GDK.BUTTON1_MASK, self.targets, gtk.GDK.ACTION_COPY)
           hh.connect("drag_begin",self.setDragAway,(pmap,))
           hh.connect("drag_data_get",self.setDragAway,(pmap,))
           self.toplay.put (f, w, 2)
           if p.size_request()[1] > h: h=p.size_request()[1]+addon
           w=w+f.size_request()[0]+9
           self.num_images=self.num_images+1
         except: 
           pass
      self.toplay.set_size(w+20,h+20)
      #self.toplay.show_all()
      self.frameshbox.show_all()

    def closeit(self,*args):
      global mpegdec
      self.framesel.destroy()
      self.framesel.unmap()
      mpegdec.cleanup()

    def extractFrames(self,*args):
      l=self.getFrames()
      if not l==None: 
       if len(l) > 25:
         if showConfirmMessage(translate("You have selected more than 25 frames from this Video.\nIt may take a LONG time to process all of these frames into GIFs.\nSo, you will have to be very patient.\ngAnim8 is not a 'movie studio'.\nIt is a suite for SMALL animations.\nSo, this might work, it might not.\n You have been warned!\nContinue anyway?")): 
           self.framesel.hide()
           self.GANIM.loadMpegFrames(l)
       else:  
           self.framesel.hide()
           self.GANIM.loadMpegFrames(l)

    def getFrames(self,*args):
      frames={}
      l=self.frameentry.get_text()
      isBad=0
      if l=="": 
        isBad=1
      flist=l.split(",")
      for i in flist:
       fitem=i.strip()
       if fitem:
         if fitem.find("-")>-1:
           fitems=fitem.split("-")
           if not len(fitems)==2:
              isBad=1
              break
           try:
             start=int(fitems[0].strip())
             end=int(fitems[1].strip())
             if not start > 0:
               isBad=1
               break
             if (end < start) or (end > self.num_images):
               isBad=1
               break
             while start<end+1:
                frames[start]=start
                start=start+1
           except:
             isBad=1
             break
         else:
            try:
               frames[int(fitem.strip())]=int(fitem.strip())
            except:
              isBad=1
              break
      if (len(frames)==0) or isBad:
         showMessage("Invalid Frame Selection! Please re-check your input.")
         return None
      return frames.keys()

###   gAnim8 Briefcase Window - added in version 0.3  ####
def showBriefcase(ganim8_inst):
	briefcase(ganim8_inst)

class briefcase:
    def __init__ (self,ganim8_inst) :
	briefwin = GtkWindow(GTK.WINDOW_TOPLEVEL)
	briefwin.realize()
	briefwin.set_title (translate('gAnim8 Briefcase'))
	briefwin.set_position (GTK.WIN_POS_CENTER)
	self.briefwin = briefwin
	vbox1 = GtkVBox (0, 2)
	vbox1.set_border_width (4)
	vbox1.set_spacing(3)
	vbox1.pack_start (GtkLabel (translate('gAnim8 Briefcase\n\nSession files: These are temporary files which \nwill automatically be deleted when gAnim8 quits.')), 0, 0, 0)
	hbox1 = GtkHBox (0, 4)
	scrollwin = GtkScrolledWindow ()
	scrollwin.set_usize (350, 150)
	self.scrollwin = scrollwin
	myclist = GtkCList (1)
	myclist.column_titles_hide ()
	myclist.set_column_width ( 0, 80)
	myclist.set_shadow_type (GTK.SHADOW_OUT)
	self.myclist = myclist
	myclist.set_column_widget (0, GtkLabel(translate('Files')))
	self.ganim8_inst=ganim8_inst
	scrollwin.add (myclist)
	hbox1.pack_start (scrollwin, 1, 1, 0)
	vbox2 = GtkVBox (0, 0)
	refbutt = GtkButton (translate('Refresh'))
	refbutt.connect("clicked",self.updateRows)
	vbox2.pack_start(refbutt, 1, 0, 0)
	openbutt = GtkButton (translate('Re-Open'))
	openbutt.connect("clicked",self.openFile)
	vbox2.pack_start( openbutt, 1, 0, 0)
	prevbutt = GtkButton (translate('Preview'))
	prevbutt.connect("clicked",self.prevFile)
	vbox2.pack_start( prevbutt, 1, 0, 0)
	savebutt = GtkButton (translate('Save'))
	savebutt.connect("clicked",self.saveFile)
	vbox2.pack_start( savebutt, 1, 0, 0)
	delbutt = GtkButton (translate('Delete'))
	delbutt.connect("clicked",self.delFile)
	vbox2.pack_start( delbutt, 1, 0, 0)
	vbox2.pack_start( GtkHSeparator(), 0, 0, 2)
	cbutt = GtkButton (translate('Close'))
	cbutt.connect("clicked",self.doExit)
	vbox2.pack_start( cbutt, 1, 0, 0)
	hbox1.pack_start (vbox2, 0, 1, 0)
	vbox1.pack_start (hbox1, 1, 1, 0)
	filesize = GtkLabel (' ')
	myclist.set_selection_mode(SELECTION_SINGLE)
	myclist.set_column_width(0,600)
	myclist.connect("select_row",self.select_row_callback,"")
	myclist.connect("unselect_row",self.unselect_callback,"")
	self.filesize = filesize
	vbox1.pack_start (filesize, 0, 0, 0)
	self.updateRows()
	briefwin.add (vbox1)
	self.sel_file=""
	briefwin.connect("destroy",self.doExit)
	briefwin.show_all()

    def select_row_callback(self,widget,row,column,*args):
	self.sel_file=str(self.myclist.get_text(row,0))
	try:
		self.filesize.set_text(str(os.path.getsize(self.sel_file))+translate(" bytes"))
	except:
		self.filesize.set_text(translate("FILE NOT FOUND!"))

    def unselect_callback(self,*args):
	self.sel_file=""
	self.filesize.set_text(" ")

    def doExit(self,*args):
	self.briefwin.destroy()
	self.briefwin.unmap()

    def delFile(self,*args):
	if not len(self.sel_file)>0: return
	if not showConfirmMessage(translate("Are you sure you want to delete this file?\n")+self.sel_file)==1: return
	f=os.popen("rm -f "+self.sel_file)
	f.readlines()
	if not os.path.exists(self.sel_file):
		try:
			del self.ganim8_inst.session_created[sel_file]
		except:
			pass
	self.updateRows()

    def openFile(self,*args):
	if not len(self.sel_file)>0: return
	if self.sel_file.endswith("ganim_preview.gif"): 
		showMessage("Sorry, this file cannot be opened for editing because\nit is an important gAnim8 system file.\n\nIf you really need to edit it,\nSAVE it to another file and re-open it from the new file.")
		return
	global load_file
	load_file=self.sel_file
	self.ganim8_inst.askGifOpen()

    def prevFile(self,*args):
	if not len(self.sel_file)>0: return
	global mpeg_cmd
	global avi_cmd
	global swf_cmd
	global rm_cmd
	global asf_cmd
	global wmv_cmd
	global m4v_cmd
	prev_cmd="gifview -a %f"
	if self.sel_file.endswith(".mpeg"): prev_cmd=mpeg_cmd
	if self.sel_file.endswith(".avi"): prev_cmd=avi_cmd
	if self.sel_file.endswith(".swf"): prev_cmd=swf_cmd
	if self.sel_file.endswith(".rm"): prev_cmd=rm_cmd
	if self.sel_file.endswith(".asf"): prev_cmd=asf_cmd
	if self.sel_file.endswith(".wmv"): prev_cmd=wmv_cmd
	if self.sel_file.endswith(".m4v"): prev_cmd=m4v_cmd
	if len(prev_cmd.strip()) > 0: 
		os.popen( prev_cmd.replace("%f",self.sel_file)+ " &> /dev/null &")

    def saveFile(self,*args):
	if not len(self.sel_file)>0: return
	if not os.path.exists(self.sel_file): return
	myfile=GtkExtra.file_save_box()
	if not myfile: return
	if not len(myfile)>0: return
	if myfile.endswith(os.sep): return
	if os.path.exists(myfile):
		if not showConfirmMessage(translate("The file '")+myfile+translate("' already exists.\nDo you wish to overwrite it?"))==1: return
	f=os.popen("cp -f "+self.sel_file+" "+myfile.replace(" ","\\ "))
	f.readlines()
	if os.path.exists(myfile): showMessage(translate("File successfully saved to:\n")+myfile)
	else: showMessage(translate("ERROR saving file to:\n")+myfile)

    def updateRows(self,*args):
	self.myclist.freeze()
	self.myclist.clear()
	mykeys=self.ganim8_inst.session_created.keys()
	mykeys.sort()
	for ii in mykeys:
		if os.path.exists(str(ii)): 	self.myclist.append([str(ii)])
	self.myclist.thaw()
	self.sel_file=""
	self.filesize.set_text("  ")

## gAnim8 Recorder for V4L Devices - added in version 0.3  ##
def openRecorder(ganim8_inst):
	recordwin(ganim8_inst)

class recordwin:
    def __init__(self,ganim8i) :
	recordwin=GtkWindow(GTK.WINDOW_TOPLEVEL)
	self.ganim8_inst=ganim8i
	self.ffmpeg=None
	self.last_file=""
	self.last_type="mpeg"
	recordwin.realize()
	recordwin.set_title(translate('GAnim8 Recorder'))
	recordwin.set_position(GTK.WIN_POS_CENTER)
	self.recordwin=recordwin
	vbox1=GtkVBox(0,3)
	vbox1.set_border_width(6)
	vbox1.pack_start( GtkLabel(translate('GAnim8 Recorder\n\nRecord from a Video4Linux Device')),0,0,2)
	hbox4=GtkHBox(1,0)
	hbox4.pack_start( GtkLabel(translate('Device:')),0,0,0)
	deventry=GtkEntry()
	self.deventry=deventry
	deventry.set_text("/dev/video0")
	hbox4.pack_start( deventry,1,1,0)
	vbox1.pack_start( hbox4,1,1,0)
	hbox1=GtkHBox(1,9)
	hbox1.pack_start(GtkLabel(translate('Format:')),0,0,0)
	format_combo=GtkCombo()
	format_combo.set_popdown_strings(['MPEG','AVI','SWF (Shockwave Flash)','RM (RealPlayer)','WMV (Windows Media)','ASF','M4V'])
	self.format_combo=format_combo
	format_entry=format_combo.entry
	format_entry.set_text("MPEG")
	format_entry.set_editable(0)
	self.format_entry=format_entry
	hbox1.pack_start(format_combo,1,1,0)
	vbox1.pack_start( hbox1,1,1,0)
	hbox2=GtkHBox(1,7)
	hbox2.pack_start(GtkLabel(translate('Video Size:')),0,0,0)
	size_combo=GtkCombo()
	self.size_combo=size_combo
	size_combo.set_popdown_strings(['176x144','352x288'])
	size_entry=size_combo.entry
	size_entry.set_editable(1)
	size_entry.set_text("352x288")
	self.size_entry=size_entry
	hbox2.pack_start(size_combo,1,1,0)
	vbox1.pack_start( hbox2,0,0,0)
	hbox6=GtkHBox(1,7)
	hbox6.pack_start(GtkLabel(translate('Frame Rate (FPS):')),0,0,0)
	rate_entry=GtkEntry()
	rate_entry.set_editable(1)
	rate_entry.set_text("6")
	self.rate_entry=rate_entry
	hbox6.pack_start(rate_entry,1,1,0)
	vbox1.pack_start( hbox6,0,0,0)
	recbutt=GtkButton(translate('Start Recording'))
	recbutt.connect("clicked",self.startRec)
	recbutt.set_border_width(3)
	vbox1.pack_start( recbutt,0,0,0)
	stopbutt=GtkButton(translate('Stop Recording'))
	stopbutt.set_border_width(3)
	self.stopbutt=stopbutt
	#stopbutt.set_sensitive(0)
	stopbutt.connect("clicked",self.stopRec)
	self.recbutt=recbutt
	vbox1.pack_start( stopbutt,0,0,0)
	hbox3=GtkHBox(1,7)
	prevbutt=GtkButton(translate('Preview Last Video'))
	prevbutt.connect("clicked",self.prevFile)
	self.prevbutt=prevbutt
	hbox3.pack_start( prevbutt,1,1,0)
	savebutt=GtkButton(translate('Save Last Video'))
	savebutt.connect("clicked",self.saveFile)
	self.savebutt=savebutt
	hbox3.pack_start(savebutt,1,1,0)
	vbox1.pack_start( hbox3,0,0,0)
	myclock=GtkClock(0)
	self.myclock=myclock
	#myclock.stop()
	myclock.set_text("00:00:00")
	myclock.set_seconds(0)
	vbox1.pack_start( myclock,0,0,0)
	myclock.set_update_interval(1)
	myclock.set_format("%H:%M:%S")
	filesize=GtkLabel('  ')
	filesize.set_alignment(0,0.5)
	self.filesize=filesize
	vbox1.pack_start( filesize,0,0,0)
	recordwin.add(vbox1)
	recordwin.show_all()
	recordwin.connect("destroy",self.doExit)

    def startRec(self,*args) :
	self.recbutt.set_sensitive(0)
	self.filesize.set_text(translate("Recording..."))
	gtk.timeout_add(90,self.startRec1)

    def startRec1(self,*args) :
	format=self.format_entry.get_text().lower()
	if format.find(" ")>-1: format=format[0:format.find(" ")].strip()
	self.last_type=format
	self.myclock.set_seconds(0)
	self.myclock.set_text("00:00:00")
	self.myclock.start()
	global temp_dir
	self.last_file=temp_dir+"ganim_record."+format
	self.ffmpeg=os.popen2("ffmpeg -vd "+self.deventry.get_text().strip()+" -s "+self.size_entry.get_text().strip()+" -y -r "+self.rate_entry.get_text().strip()+" -g 2 -hq "+self.last_file)

    def doExit(self,*args) :
	self.stopRec()
	self.recordwin.destroy()
	self.recordwin.unmap()

    def saveFile(self,*args):
	self.sel_file=self.last_file
	if not len(self.sel_file)>0: return
	if not os.path.exists(self.sel_file): return
	myfile=GtkExtra.file_save_box()
	if not myfile: return
	if not len(myfile)>0: return
	if myfile.endswith(os.sep): return
	if os.path.exists(myfile):
		if not showConfirmMessage(translate("The file '")+myfile+translate("' already exists.\nDo you wish to overwrite it?"))==1: return
	f=os.popen("cp -f "+self.sel_file+" "+myfile.replace(" ","\\ "))
	f.readlines()
	if os.path.exists(myfile): showMessage(translate("File successfully saved to:\n")+myfile)
	else: showMessage("ERROR saving file to:\n"+myfile)

    def stopRec(self,*args) :
	if self.ffmpeg:
		try:
			self.ffmpeg[0].write("q")
			self.ffmpeg[0].flush()
		except:
			pass
		try:
			self.ffmpeg[0].close()
		except:
			pass
		try:
			self.ffmpeg[1].close()
		except:
			pass
	self.myclock.stop()
	self.recbutt.set_sensitive(1)
	if os.path.exists(self.last_file):
		self.filesize.set_text(self.last_type.upper()+translate(" video is ")+str(os.path.getsize(self.last_file))+translate(" bytes"))
		self.ganim8_inst.session_created[self.last_file]="1"
	else: self.filesize.set_text(translate("ERROR recording video!"))

    def prevFile(self,*args):
	if not len(self.last_file)>0: return
	global mpeg_cmd
	global avi_cmd
	global swf_cmd
	global rm_cmd
	global asf_cmd
	global wmv_cmd
	global m4v_cmd
	prev_cmd="gifview -a %f"
	if self.last_file.endswith(".mpeg"): prev_cmd=mpeg_cmd
	if self.last_file.endswith(".avi"): prev_cmd=avi_cmd
	if self.last_file.endswith(".swf"): prev_cmd=swf_cmd
	if self.last_file.endswith(".rm"): prev_cmd=rm_cmd
	if self.last_file.endswith(".asf"): prev_cmd=asf_cmd
	if self.last_file.endswith(".wmv"): prev_cmd=wmv_cmd
	if self.last_file.endswith(".m4v"): prev_cmd=m4v_cmd
	if len(prev_cmd.strip()) > 0: 
		os.popen( prev_cmd.replace("%f",self.last_file)+ " &> /dev/null &")

## Image Formats Dialog
class _FormatsDialog(GtkDialog):
	def __init__(self, formats=[]):
		GtkDialog.__init__(self)
		self.connect("destroy", self.quit)
		#self.connect("delete_event", self.quit)
		grab_add(self)
                self.set_position (GTK.WIN_POS_CENTER)
                self.set_title(translate("gAnim8 Image Format..."))
		hbox = GtkVBox(spacing=3)
		hbox.set_border_width(5)
		hbox.pack_start(GtkLabel(translate("WHOA! I had a little trouble opening this image.")))
		hbox.pack_start(GtkLabel(translate("Tell me what kind of image this is, and I'll try again:")))
		lcombo=GtkCombo()
                self.lcombo=lcombo
                lcombo.entry.set_editable(0)
                ll=copy.copy(formats)
                ll.sort()
                lcombo.set_popdown_strings(ll)
                lcombo.entry.set_text("")
                lcombo.set_usize(330,-2)
                hbox.pack_start(lcombo,1,1,2)
		self.vbox.pack_start(hbox)
		hbox.show_all()

		b = GtkButton(translate("OK"))
		b.set_flags(CAN_DEFAULT)
		b.connect("clicked", self.selectFormat)
		self.action_area.pack_start(b)
		bb = GtkButton(translate("Cancel"))
		bb.set_flags(CAN_DEFAULT)
		bb.connect("clicked", self.quit)
		self.action_area.pack_start(bb)
		bb.show()
		self.ret = None
	def quit(self, *args):
		self.ret=None                
		self.hide()
		self.destroy()
		self.unmap()
                ll=None
		mainquit()
	def selectFormat(self,*args):
		rr =self.lcombo.entry.get_text()
		self.quit()
                self.ret=rr
                if rr=="": self.ret=None

# ask Format
def askFormat():
	win = _FormatsDialog(gAnim8ConversionTool.read_formats)
	win.show_all()
	mainloop()
	return win.ret

## Hexidecimal Conversions

def getRGBForHex(hexstr):
  if not hexstr: return (0,0,0)
  h=hexstr.replace("#","").strip().lower()
  if not len(h)==6: return None #invalid hex string send
  r_seg=h[0:2]
  g_seg=h[2:4]
  b_seg=h[4:6]
  R=calculateSegment(r_seg)
  G=calculateSegment(g_seg)
  B=calculateSegment(b_seg)
  return (R,G,B)

def calculateSegment(rgb_seg):
   if not rgb_seg: return 0
   if not len(rgb_seg)==2: return 0
   global  alpha_num_map
   try:
     return (alpha_num_map[str(rgb_seg[0:1])]*16)+alpha_num_map[str(rgb_seg[1:2])]
   except:
     return 0

## Shared methods

def showModalHelp(helpText="Help text here.",*args):
   ih=icehelp()
   ih.icehelp.set_modal(1)
   ih.setHelpText(translate(str(helpText)))
   ih.icehelp.show_all()

def showMessage(msg_text):
   global version
   GtkExtra.message_box("gAnim8 "+version,"  "+translate(str(msg_text))+"  ", [translate("Ok")])

def showConfirmMessage(msg_text):
   global version
   answer=GtkExtra.message_box("gAnim8 "+version,"  "+translate(str(msg_text))+"  ", [translate("YES"),translate("NO")])
   if answer==translate("YES"): return 1
   return 0

def getPixmapButton (picon,btext) :  
  b=GtkButton()
  b.add(getPixmapVBox(picon,btext))
  b.show_all()        
  return b

def loadImage(picon):
  try:
    global GANIM
    p=GtkPixmap(GANIM.ganim,str(picon).replace("./pixmaps/",getPixDir()),None)
    p.show_all()
    return p
  except:
    return None

def getIconButton (picon,lab="?") :  
  try:
     b=GtkButton()
     b.add(loadImage(picon))
     b.show_all()
     return b
  except:
     return GtkButton(translate(str(lab)))

def getInsertButton():
   try:
     p=loadImage("./pixmaps/ganim8_insert.xpm")
     if not p: raise TypeError
     v=GtkVBox(0,0)
     v.pack_start(GtkLabel(translate("Insert")),0,0,2)
     v.pack_start(p,0,0,0)
     b=GtkButton()
     b.add(v)
     setTip(b,"Insert a new frame here")
     b.show_all()
     return b
   except:
     b=GtkButton(translate("Insert"))
     setTip(b,"Insert a new frame here")
     return b

def getPixmapVBox(picon,btext):
        try:
	  p=loadImage(picon)
          if not p: raise TypeError
	  v=GtkHBox(1,2)
          v.set_homogeneous(0)
          v.pack_start(p,0,0,1)
          v.pack_start(GtkLabel(translate(str(btext))),0,0,1)
          v.show_all()
          return v
        except:
          v=GtkHBox(1,1)
          v.pack_start(GtkLabel(translate(str(btext))),0,0,1)
          v.show_all()
          return v

def createFileSel(winTitle,ok_method,cancel_method,*args):
        global fs
        global sel_file
        fs = GtkFileSelection()
        fs.set_title(translate(str(winTitle)))
        fs.set_position (GTK.WIN_POS_CENTER)
        fs.connect('destroy', cancel_method)
        if sel_file:
          fs.set_filename(sel_file)
        fs.ok_button.connect('clicked', ok_method)
        fs.set_modal(1)
        fs.cancel_button.connect('clicked', cancel_method)
        return fs

def grabFile(*args):   
        global fs
        global sel_file
        if not fs:
          return ""
 	sel_file=str(fs.get_filename()).strip()
        fs.hide()
        global QUIT_PENDING
        if not QUIT_PENDING: # this will call doFsHide and make the app hang on shutting down
          fs.destroy()
          fs.unmap()
        return sel_file

def doFsHide(*args):
     global fs
     fs.hide()
     fs.destroy()
     fs.unmap()
     global QUIT_PENDING
     if QUIT_PENDING: 
       global CHANGED
       CHANGED=0
       quit()


def showFileSelection(winTitle,ok_method,cancel_method,*args):
    f=createFileSel(translate(winTitle),ok_method,cancel_method)
    f.show_all()
    return f

def setTip(widget,tip_str):
  global TIPS
  TIPS.set_tip(widget,translate(str(tip_str)))

def setCurrentFile(fname):
    global sel_file
    if fname: sel_file=fname

def extractDirectory(fdir):
   if not fdir: return "./"
   f=fdir
   if not f.endswith("/"): f=f[0:f.rfind("/")+1].strip()
   if f.find("/")== -1: f="./"
   return f

def setTempDir(*args):
   global temp_dir
   if os.environ.has_key("HOME"):
     h=os.environ["HOME"]
     if not h.endswith("/"): h=h+"/"
     try:
       os.mkdir(h)
     except:
       pass
     try:
       os.mkdir(h+".ganim8/")
     except:
       pass
     try:
       os.mkdir(h+".ganim8/tmp/")
       temp_dir=h+".ganim8/tmp/"       
     except:
       try:
         os.mkdir("/tmp/")
       except:
         pass
     if os.path.exists(h+".ganim8/tmp/"):
       temp_dir=h+".ganim8/tmp/"
       return
     if os.path.exists("/tmp/"):
       temp_dir="/tmp/"
       showStatus("Could not create directory "+h+".ganim8/tmp/"+" - using /tmp/")
       return
   else:
     try:
       os.mkdir("/tmp/")
     except:
       pass
   if os.path.exists("/tmp/"):
     temp_dir="/tmp/"
     return
   else:
     temp_dir="/tmp/"
     showStatus("ERROR: Could not create/find directory /tmp/ - cannot save/open gif files")
     showModalHelp("ERROR: gAnim8 could not create or find a temporary directory in  /tmp/  or  ~/.ganim8/tmp/ .  You will not be able to open gifs or preview/save gifs until one of these directories is created.  You will have to create one of these directories yourself and restart gAnim8")

def findOptions(*args):
    global options_file
    if os.environ.has_key("HOME"):
      myhome=os.environ["HOME"]
      if not myhome.endswith("/"): myhome=myhome+"/"
      myhome=myhome+".ganim8/"    
      try:
        os.mkdir(myhome)
      except:
        pass
      options_file=myhome+".ganim8_options"

def loadOptions(*args):
    findOptions()
    global options_file
    global first_run
    global editor_cmd
    global mpeg_cmd
    global avi_cmd
    global swf_cmd
    global rm_cmd
    global asf_cmd
    global wmv_cmd
    global m4v_cmd
    global sel_file
    global movie1_file
    try:
      f=open(options_file)
      flines=f.read()
      f.close()
      flist=flines.split("\n")
      for i in flist:
        if not i.find("#") > -1:
          cpairs=i.split("=")
          if len(cpairs)==2:
            if cpairs[0].strip()=="editor_cmd": editor_cmd=cpairs[1].strip()
            if cpairs[0].strip()=="mpeg_cmd": mpeg_cmd=cpairs[1].strip()
            if cpairs[0].strip()=="avi_cmd": avi_cmd=cpairs[1].strip()
            if cpairs[0].strip()=="swf_cmd": swf_cmd=cpairs[1].strip()
            if cpairs[0].strip()=="rm_cmd": rm_cmd=cpairs[1].strip()
            if cpairs[0].strip()=="asf_cmd": asf_cmd=cpairs[1].strip()
            if cpairs[0].strip()=="wmv_cmd": wmv_cmd=cpairs[1].strip()
            if cpairs[0].strip()=="m4v_cmd": m4v_cmd=cpairs[1].strip()
            if cpairs[0].strip()=="last_file": sel_file=extractDirectory(cpairs[1].strip())
            if cpairs[0].strip()=="movie1_file": movie1_file=extractDirectory(cpairs[1].strip())
    except:
      first_run=1

def saveOptions(*args):
    global options_file
    global editor_cmd
    global sel_file
    global load_file
    global save_file
    global mpeg_cmd
    global avi_cmd
    global swf_cmd
    global rm_cmd
    global asf_cmd
    global wmv_cmd
    global m4v_cmd
    last_file=load_file
    global movie1_file
    global version
    if not last_file: last_file=save_file
    if not last_file: last_file=sel_file
    try:
      f=open(options_file,"w")
      f.write("## GAnim8 "+version+" options file. DO NOT EDIT. NOTHING TO SEE HERE.\n")
      f.write("editor_cmd="+str(editor_cmd)+"\n")
      f.write("mpeg_cmd="+str(mpeg_cmd)+"\n")
      f.write("avi_cmd="+str(avi_cmd)+"\n")
      f.write("swf_cmd="+str(swf_cmd)+"\n")
      f.write("rm_cmd="+str(rm_cmd)+"\n")
      f.write("asf_cmd="+str(asf_cmd)+"\n")
      f.write("wmv_cmd="+str(wmv_cmd)+"\n")
      f.write("m4v_cmd="+str(m4v_cmd)+"\n")
      if last_file: f.write("last_file="+str(last_file)+"\n")
      if movie1_file: f.write("movie1_file="+str(movie1_file)+"\n")
      f.flush()
      f.close()
    except:
      global QUIT_PENDING
      if not QUIT_PENDING: showStatus(" ERROR: Could not save your personalized options under "+str(options_file))

def cleanupFiles(*args):
   global temp_dir
   global mpegdec
   mpegdec.cleanup()
   l=["ganim_extract","ganim_palette","ganim_copy","ganim8_conversion","ganim8_movie","ganim_movie"]
   for i in l:
     os.popen("rm -f "+temp_dir+i+"*")

def showStatus(statustext,*args):
    global my_last_stat
    my_last_stat=str(statustext)

def showStatus1(*args):
    global statid
    global statusbar
    global version
    global my_last_stat
    statusbar.push(statid," gAnim8 "+version+": "+translate(str(my_last_stat)))
    #statusbar.show_all()
    return TRUE

def checkGifsicle(*args):
  global gifsicle_installed
  global gifview_installed
  gifsicle_installed=isOnPath("gifsicle")
  gifview_installed=isOnPath("gifview")
  if not gifview_installed:
    showModalHelp("CRITICAL ERROR: Gifview from the Gifsicle package is required by gAnim8 for previewing animated gifs.  However, the 'gifview' executable was not found on your PATH.  You must install Gifsicle with the gifview program and put it on your PATH before you can preview animated gifs in gAnim8.")
  if not gifsicle_installed:
    showModalHelp("CRITICAL ERROR: Gifsicle is required by gAnim8.  However, the 'gifsicle' executable was not found on your PATH.  You must install Gifsicle and put it on your PATH before you can use this program successfully.")


def haveGifsicle():
  global gifsicle_installed
  if not gifsicle_installed:  showModalHelp("CRITICAL ERROR: Gifsicle is required by gAnim8.  However, the 'gifsicle' executable was not found on your PATH.  You must install Gifsicle and put it on your PATH before you can use this program.  You can download the source code for Gifsicle at   http://www.lcdf.org/gifsicle/   and can probaby find RPMs for it on speakeasy.rpmfind.net . Trust me, it's WELL worth the download.")
  return gifsicle_installed

def haveGifview():
  global gifview_installed
  if not gifsicle_installed:  showModalHelp("CRITICAL ERROR: Gifview from the Gifsicle package is required by gAnim8 for previewing animated gifs.  However, the 'gifview' executable was not found on your PATH.  You must install Gifsicle with the gifview program and put it on your PATH before you can preview animated gifs in gAnim8.  You can download the source code for Gifsicle at   http://www.lcdf.org/gifsicle/   and can probaby find RPMs for it on speakeasy.rpmfind.net . Trust me, it's WELL worth the download.")
  return gifview_installed

def haveMpeg():
  global HAVE_MPEG
  if  HAVE_MPEG==1: return 1
  HAVE_MPEG=isOnPath("ffmpeg")
  return HAVE_MPEG

def haveMpegE():
  global HAVE_MPEGE
  if  HAVE_MPEGE==1: return 1
  HAVE_MPEGE=isOnPath("ffmpeg")
  return HAVE_MPEGE

def isOnPath(binary):
  if os.environ.has_key("PATH"):
    paths=os.environ["PATH"].split(":")
    for i in paths:
      p=i
      if not p.endswith("/"): p=p+"/"
      if os.path.exists(p+str(binary)):
        return 1
  return 0

## Main methods

def quit(*args):
  global GANIM
  global CHANGED
  global QUIT_PENDING
  global temp_dir
  if CHANGED:
    if showConfirmMessage("This image or animation sequence has changed.  Save it?"):      
      QUIT_PENDING=1
      GANIM.saveDecide()
    else: 
      cleanupFiles()
      os.popen("rm -f "+temp_dir+"ganim_preview"+"*")
      os.popen("rm -f "+temp_dir+"ganim_movpreview"+"*")
      os.popen("rm -f "+temp_dir+"ganim_record"+"*")
      os.popen("rm -f "+temp_dir+"gtksnapshot"+"*")
      saveOptions()  
      gtk.mainquit()
  else: 
    cleanupFiles()
    os.popen("rm -f "+temp_dir+"ganim_preview"+"*")
    os.popen("rm -f "+temp_dir+"ganim_movpreview"+"*")
    os.popen("rm -f "+temp_dir+"ganim_record"+"*")
    os.popen("rm -f "+temp_dir+"gtksnapshot"+"*")
    saveOptions()  
    gtk.mainquit()

def run (start_file=None) :
    global startup_file
    if start_file: startup_file=start_file
    g=ganim () 
    g.ganim.connect("delete_event",quit)
    gtk.mainloop()

def getPixDir() :
	return sys.argv[0][0:sys.argv[0].rfind(os.sep)+1]+"pixmaps"+os.sep

def getBaseDir() :
	return sys.argv[0][0:sys.argv[0].rfind(os.sep)+1]

if __name__ == "__main__" :
    start_file=None
    global my_last_stat
    my_last_stat=""
    if len(sys.argv) > 1:
      argy=sys.argv[1].strip()
      if (argy.lower()=="help") or (argy.lower()=="-help") or (argy.lower()=="--help"):  
        print translate("\n\nUsage:  ganim8  [file_name]         where [file_name] is the optional file to open immediately upon starting.\n")
        sys.exit(0)
      start_file=argy
    run (start_file)	
    #t=Thread(None,run,None,(start_file,))
    #t.start()