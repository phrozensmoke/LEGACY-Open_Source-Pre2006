#! /usr/bin/env python
# -*- coding: ISO-8859-1 -*-

#######################################
#  pY! Voice Chat - pyvoiceui.py:
#    Gtk-1 user interface module
#  
#  Copyright (c) 2003-2006 by Erica Andrews
#  PhrozenSmoke [-AT-] yahoo.com
#  http://phpaint.sourceforge.net/pyvoicechat/
#
#  The first Linux client for Yahoo!
#  voice chat.
#
#  Graphical user interface module.
#  A frontend to the pyvoice.py library.
#  User interface: Gtk+ via PyGtk.
#  Neither Gnome nor PyGnome is 
#  required.
#  
#  Neither pY! Voice Chat nor its author
#  are in anyway endorsed by or 
#  affiliated with Yahoo! (tm) or 
#  DSP Group's TrueSpeech (tm).
#  Yahoo! (tm) is a registered 
#  trademark of Yahoo! 
#  Communications.  TrueSpeech (tm)
#  is a registered trademark of 
#  DSP Group. The TrueSpeech (tm)
#  codec and format are patented 
#  by DSP Group.
#  
#  This program is distributed free
#  of charge (open source) under the 
#  terms of the GNU General Public 
#  License
#######################################

import os,GTK,gtk,GDK,sys,gettext,pyvoice,time
from gtk import *
from pyvoice import *

global my_esd_host   # TODO:  maybe make this a configurable property from within GUI
my_esd_host="localhost"     # which host to use for ESound playback (not record)?? usually localhost

def getBaseDir() :
	if sys.argv[0][0:sys.argv[0].rfind(os.sep)+1]=='': return "."+os.sep
	return sys.argv[0][0:sys.argv[0].rfind(os.sep)+1]

def getPixDir() :
	return getBaseDir()+os.sep+"pixmaps"+os.sep

locale_dirs=[getBaseDir()+'/locale','/usr/share/locale','/usr/X11R6/share/locale','/usr/locale','/usr/local/share/locale']

# gettext locale support - pyvoice
pyvoice_xtext=gettext.NullTranslations()

try:
	# open pyvoice.mo in /usr/share/locale or whatever
	f_gettext=0

	for LOCALE_DIR in locale_dirs:  # open the first catalog we find, try ./locale dir first
		try:
			pyvoice_xtext=gettext.translation("pyvoice",LOCALE_DIR)
			f_gettext=1
			break
		except:
			pass
	if not f_gettext==1: raise TypeError   # couldnt find a gettext catalog, raise exception, use empty catalog
except:
	pyvoice_xtext=gettext.NullTranslations()

_=pyvoice_xtext.gettext
pyvoice._=_



class pyvoicegui:
    """
    Main GUI interface class.
    """
    def __init__(self) :
	"""
	Main GUI interface class initiation method.
	"""
	WMCLASS="pyvoicechat"
	WMNAME="PyVoiceChat"
	self.vserv="66.218.70.37"
	self.version="0.3.5"
	self.states={
		0:_("Not Connected"),
		1:_("Connecting..."),
		2:_("Connected"),
				   }
	self.last_state=0
	self.last_talker=""
	self.last_room=""
	self.selected_name=""
	self.selected_row=0
	self.last_error=""
	self.users={}  #username to alias map
	TIPS=GtkTooltips()
	pyvoicewin=GtkWindow(GTK.WINDOW_TOPLEVEL)
	pyvoicewin.set_wmclass(WMCLASS,WMNAME)
	pyvoicewin.realize()
	pyvoicewin.set_title(_("pY! Voice Chat"))
	pyvoicewin.set_position(GTK.WIN_POS_CENTER)
	self.pyvoicewin=pyvoicewin
	mainvbox1=GtkVBox(0,1)
	mainvbox1.set_spacing(1)
	mainvbox=GtkVBox(0,1)
	mainvbox.set_spacing(0)
	mymenubar=GtkMenuBar()
	self.mymenubar=mymenubar

        ag=GtkAccelGroup()
        itemf=GtkItemFactory(GtkMenuBar, "<main>", ag)
        itemf.create_items([
            # path              key           callback    action#  type
  (_("/_File"),  "<alt>F",  None,  0,"<Branch>"),
  (_("/_File/_Connect"), None, self.start_voice_cb,10,""),
  (_("/_File/_Disconnect"), None, self.stop_voice_cb,10,""),
  (_("/_File/sep4"), None,  None,  49, "<Separator>"),
  (_("/_File/_Voice Server")+"...", "<control>H", self.selectServer,10,""),
  (_("/_File/_Session Information")+"...", "<control>J", self.show_stats,10,""),
  (_("/_File/sep4"), None,  None,  49, "<Separator>"),
  (_("/_File/Enable Debugging"), None, self.dbg_on,10,""),
  (_("/_File/Disable Debugging"), None, self.dbg_off,10,""),
  (_("/_File/sep4"), None,  None,  49, "<Separator>"),
  (_("/_File/_Quit"), "<control>Q", self.doQuit,10,""),

  (_("/_Help"),  "<alt>H",  None,16, "<LastBranch>"), 
  (_("/_Help/_About pY! Voice Chat..."), "F2", self.doAbout,17, ""),
  (_("/_Help/_About PyTSP and PyESD..."), "F3", self.show_sound_engine,17, ""),
  (_("/_Help/_Contributors..."), "F5", self.doContrib,17, ""),

        ])

        pyvoicewin.add_accel_group(ag)
        mymenubar=itemf.get_widget("<main>")
        mymenubar.show()
	mainvbox1.pack_start(mymenubar,0,0,0)
	mainvbox1.pack_start(mainvbox,1,1,0)

	mainvbox.set_spacing(2)
	ptitle=loadImage("pyvoice_head.xpm", pyvoicewin)
	if not ptitle==None: mainvbox.pack_start(ptitle,0,0,2)
	else: mainvbox.pack_start(GtkLabel(_("pY! Voice Chat")),0,0,5)


	# on/off button and mute here
	onbox=GtkHBox(0,0)
	onbox.set_spacing(7)
	onbutton=getIconButton2(pyvoicewin, "pyvoice_on.xpm", _("On")+" ")
	TIPS.set_tip(onbutton,_("On"))
	onbutton.connect("clicked",  self.start_voice_cb)
	offbutton=getIconButton2(pyvoicewin, "pyvoice_off.xpm", _("Off")+" ")
	TIPS.set_tip(offbutton,_("Off"))
	offbutton.connect("clicked",  self.stop_voice_cb)
	mutebutton=GtkCheckButton(" "+_("Mute"))
	TIPS.set_tip(mutebutton,_("Mute"))
	self.onbutton=onbutton
	self.offbutton=offbutton
	self.mutebutton=mutebutton
	onbox.pack_start(onbutton,0,0,0)
	onbox.pack_start(offbutton,0,0,0)
	onbox.pack_start(GtkLabel("  "),1,1,0)
	onbox.pack_start(mutebutton,0,0,0)

	mainvbox.pack_start(onbox,0,0,0)
	mainvbox.pack_start(GtkHSeparator(),0,0,2)

	# 'talk' box here
	talkbox=GtkHBox(0,0)
	talkbox.set_spacing(8)
	talkbox.set_border_width(4)
	#myvolbox=SoundBar()   # will hold our volume monitor
	myvolbox=GtkButton(" "+_("Test")+" ")
	myvolbox.connect("clicked",startMicTest)
	self.myvolbox=myvolbox
	talkbox.pack_start(myvolbox,0,0,0)
	talkbox.pack_start(GtkLabel("    "),0,0,0)
	mymonitor=ColorButton()  # will hold our color indicator, GtkDrawingArea?
	self.mymonitor=mymonitor
	talkbox.pack_start(mymonitor,0,0,0)
	talkbutton=getIconButton2(pyvoicewin, "pyvoice_talk.xpm", " "+_("Talk")+"  ", 1)
	TIPS.set_tip(talkbutton,_("Talk"))
	talkbox.pack_start(talkbutton,1,1,0)
	self.talkbutton=talkbutton	
	talkbutton.connect("toggled", self.toggleTalk)

	mainvbox.pack_start(talkbox,0,0,0)



	# scrollwindow of names here
	hscroll=GtkHBox(0,0)
	hscroll.set_spacing(3)
	# ignore button
	iggbox=GtkVBox(0,0)
	iggbox.pack_start(GtkLabel("  "),1,1,0)
	iggbutton=getIconButton(pyvoicewin, "pyvoice_ignore.xpm", " "+_("Ignore")+" ")
	iggbutton.connect("clicked",self.toggle_ignore)
	TIPS.set_tip(iggbutton,_("Ignore"))
	iggbox.pack_start(iggbutton,0,0,0)
	self.iggbutton=iggbutton
	self.rnum=GtkLabel("[0]")
	iggbox.pack_start(self.rnum,1,1,7)
	infobutton=getIconButton(pyvoicewin, "pyvoice_info.xpm", " "+_("Info")+" ")
	TIPS.set_tip(infobutton,_("User Information"))
	iggbox.pack_start(infobutton,0,0,0)
	self.infobutton=infobutton
	self.infobutton.connect("clicked",self.show_user_info)
	iggbox.pack_start(GtkLabel("  "),1,1,0)
	hscroll.pack_start(iggbox,0,0,0)

	sc=GtkScrolledWindow()
	self.nmlist=GtkCList(3,[' I ',' '+_('Name')+' ',' '+_('Alias')+' '])
	self.nmlist.set_column_width(0,16)
	self.nmlist.set_column_width(1,140)
	self.nmlist.set_column_width(2,210)
	sc.add(self.nmlist)
	hscroll.pack_start(sc,1,1,0)
	mainvbox.pack_start(hscroll,1,1,1)



	mainvbox.pack_start(GtkHSeparator(),0,0,1)

	# box for 'who's talking' goes here
	prevtable = GtkTable (2, 3, 0)
	prevtable.set_row_spacings(2)
	prevtable.set_col_spacings(4)
	prevtable.attach( GtkLabel(' '+_("Speaker")+':'), 0, 1, 0, 1, (GTK.EXPAND), (0), 0, 0)
	tvolbox=SoundBar()  # will hold their sound volume
	self.tvolbox=tvolbox
	prevtable.attach( tvolbox, 1, 2, 0, 1, (GTK.EXPAND+GTK.FILL), (0), 0, 0)
	prevtable.attach( GtkLabel('     '+_("Name")+':'), 0, 1, 1, 2, (GTK.EXPAND), (0), 0, 0)
	prevtable.attach( GtkLabel('     '+_("Alias")+':'), 0, 1, 2, 3, (GTK.EXPAND), (0), 0, 0)
	self.nstatusbar = GtkStatusbar ()
	self.nstatusbar.set_usize(170,-2)
        self.nstatid=self.nstatusbar.get_context_id("screenname")
	prevtable.attach( self.nstatusbar, 1, 2, 1, 2, (GTK.EXPAND+GTK.FILL), (0), 0, 0)
	self.astatusbar = GtkStatusbar ()
	self.astatusbar.set_usize(170,-2)
        self.astatid=self.astatusbar.get_context_id("screenalias")
	prevtable.attach( self.astatusbar, 1, 2, 2, 3, (GTK.EXPAND+GTK.FILL), (0), 0, 0)

	mainvbox.pack_start(prevtable,0,0,1)

	# room name entry
	roomentry=GtkEntry()
	roomentry.set_editable(0)
	self.roomentry=roomentry
	mainvbox.pack_start(roomentry,0,0,0)

	# status label
	statlab=GtkLabel(" ")
	mainvbox.pack_start(statlab,0,0,0)
	self.statlab=statlab

	mainvbox.set_border_width(3)
	pyvoicewin.add(mainvbox1)
	pyvoicewin.set_default_size(275,380)
	pyvoicewin.connect("destroy",self.doQuit)
	self.set_status(_("Not Connected"))
	self.update_name("")
	self.update_alias("")

	self.load_server()
	if len(self.vserv)<8: self.vserv="66.218.70.39"
	set_voice_server(self.vserv)
	self.onbutton.set_sensitive(1)
	self.offbutton.set_sensitive(1)
	pyvoicewin.show_all()
	timeout_add(300,self.update_state)
	timeout_add(50,self.update_recording)
	timeout_add(100,self.update_talker)
	timeout_add(400,self.update_error)
	timeout_add(1500,self.update_room_name)
	timeout_add(10000,self.update_room_users)
	self.nmlist.connect("select_row",self.clist_cb)
	timeout_add(1500,self.check_requested_room)

	timeout_add(25,self.update_vol) 

	self.nmlist.connect("unselect_row",self.clist_unselect)
	self.mutebutton.connect("clicked",self.toggle_mute)
	self.tvolbox.set_volume(2)

    def toggleTalk(self,*args):
	"""
	Enables/disables talking and sending out to the voice chat server.
	"""
	if is_mic_test()==1: return 		
	if self.talkbutton.get_active():
		if get_is_recording()==2: return
		else: recording_on()
	else:
		recording_off()

    def update_vol(self,*args) :
	"""
	Called at regular intervals to update the volume display.
	"""
	self.tvolbox.set_volume(get_esd_volume())
	#self.myvolbox.set_volume(int(pyvoice.pytsp.get_esd_volume(0)))
	return 1

    def dbg_on(self,*args) :
	"""
	Turns debugging on.
	"""
	set_debug(1)
    def dbg_off(self,*args) :
	"""
	Turns debuggin off.
	"""
	set_debug(0)

    def show_stats(self,*args) :
	"""
	Shows statistics about the current voice chat session.
	"""
	message(get_pyvoice_stats(),['  '+_("OK")+'  '],0)


    def show_sound_engine(self,*args) :
	"""
	Displays information about PyESD, PyTSP, and the TrueSpeech codec.
	"""
	message(_("pY! Voice Chat uses the following software for playing Yahoo! voice chat sound")+":\n\n"+get_sound_engine_info(),['  '+_("OK")+'  '],0,1)

    def update_error(self,*args) :
	"""
	Displays an existing error messages, which are polled at regular intervals.
	"""
	i=get_pyvoice_error()
	if i==self.last_error: return 1
	if not i=='': message(_("ERROR")+":\n"+i,['  '+_("OK")+'  '],0)
	self.last_error=i
	return 1

    def show_user_info(self,*args) :
	"""
	Displays information and statistics about the selected voice chat user.
	"""
	if self.selected_name=="": return
	s=_("User")+":  "+self.selected_name+"\n"
	if self.users.has_key(self.selected_name): s=s+_("Nickname")+":  "+self.users[self.selected_name]+"\n"
	if user_is_ignored(self.selected_name):
		s=s+_("Ignored")+":  "+_("Yes")+"\n"
	else:
		s=s+_("Ignored")+":  "+_("No")+"\n"
	s=s+_("Voice Packets Sent")+":  "+ get_stats_for_user(self.selected_name)
	message( s, ['  '+_('OK')+'  '])

    def toggle_mute(self,*args) :
	"""
	Turns mute on/off.
	"""
	set_mute(self.mutebutton.get_active())

    def update_room_users(self,*args) :
	"""
	Called at regular intervals to update the list of members in the voice chat room.
	"""
	users=get_users_in_room()
	self.users.clear()
	for ii in users:
		self.users[ii]=ii
	#should load aliases here
	try:
		if os.environ.has_key("USER"):
			f=open("/tmp/pyvoice_aliases_"+os.environ['USER'])
		else:
			f=open("/tmp/pyvoice_aliases_user")
		flist=f.read().split("\n")
		f.close()
		for ii in flist:
			if ii.find("[pY!]")==-1: continue
			pair=ii.split("[pY!]")
			if len(pair)<2: continue
			if self.users.has_key(pair[0].strip()): self.users[pair[0].strip()]=pair[1].strip()
	except:
		pass

	self.nmlist.freeze()
	self.nmlist.clear()
	inum=0
	lus=self.users.keys()
	lus.sort()
	for ii in lus:
		igged=" "
		if user_is_ignored(ii): igged="X"
		k=[igged,ii,self.users[ii]]
		self.nmlist.append(k)
		self.nmlist.set_row_data(inum,ii)
		inum=inum+1
	try:
		self.nmlist.moveto(self.selected_row,0)
		self.nmlist.select_row(self.selected_row,0)
	except:
		pass
	self.nmlist.thaw()
	self.rnum.set_text("["+str(len(self.users))+"]")
	#self.selected_name=""
	return 1

    def clist_unselect(self,*args):
	"""
	Room member list callback on unselect.
	"""
	self.selected_name=""

    def clist_cb(self,widget, row, col, event):
	"""
	Room member list callback on select.
	"""
	rowd = widget.get_row_data(row)
	self.selected_name=rowd
	self.selected_row=row
	
    def toggle_ignore(self,*args) :
	"""
	Turns ignore on/off for the selected voice chat user.
	"""
	if self.selected_name=="": return
	if user_is_ignored(self.selected_name):
		unignore_user(self.selected_name)
	else: 
		ignore_user(self.selected_name)
	self.update_room_users()

    def update_state(self,*args) :
	"""
	Called at regular intervals to update the color 'state' display.
	"""
	i=get_chat_status()
	if i==self.last_state: return 1
	if self.states.has_key(i):  self.set_status(self.states[i])
	if i==0:
		self.onbutton.set_sensitive(1)
		self.offbutton.set_sensitive(0)
	else:
		self.onbutton.set_sensitive(0)
		self.offbutton.set_sensitive(1)
	self.last_state=i
	self.update_room_users()
	return 1

    def update_talker(self,*args) :
	"""
	Called at regular intervals to update the displayed name of who is talking.
	"""
	i=get_who_is_talking()
	if i==self.last_talker: return 1
	if self.users.has_key(i):
		self.update_name(i)
		self.update_alias(self.users[i])
	elif i==_("MIC TEST"):
		self.update_name(i)
		self.update_alias("*"+_("Talk")+ "*")
	elif i==_("MIC PLAY"):
		self.update_name(i)
		self.update_alias("*"+_("Listen")+ "*")
	else:
		self.update_name("")
		self.update_alias("")
	self.last_talker=i
	return 1

    def update_recording(self,*args) :
	"""
	Called at regular intervals to update the recording color status.
	"""
	i=get_is_recording()
	if i==0:
		self.mymonitor.updateColorProperties(0.10,0.10,0.11)
	if i==1:
		self.mymonitor.updateColorProperties(0.89,0.10,0.10)
	if i==2:
		self.mymonitor.updateColorProperties(0.08,0.80,0.08)
	return 1

    def start_voice_cb(self,*args) :
	"""
	Callback to start a Py Voice Chat connection.
	"""
	if get_chat_status()==0:
		set_voice_server(self.vserv)
		#self.onbutton.set_sensitive(0)
		self.offbutton.set_sensitive(1)
		startPyVoice()

    def stop_voice_cb(self,*args) :
	"""
	Callback to stop a Py Voice Chat connection.
	"""
	if get_chat_status()>0:
		self.onbutton.set_sensitive(1)
		self.offbutton.set_sensitive(0)
		stopPyVoice()

    def load_server(self,*args) :
	"""
	Loads the saved voice chat server.
	"""
	try:
		f=open(os.environ['HOME']+os.sep+".pyvoice")
		self.vserv=f.read().replace("\n","").replace("\r","").replace("\t","").strip()
		f.close()
	except:
		pass
    def save_server(self,*args) :
	try:
		f=open(os.environ['HOME']+os.sep+".pyvoice","w")
		f.write(self.vserv)
		f.close()
	except:
		pass

    def update_name(self,nstatus) :
	"""
	Displays the screen name of who is talking in the status bar.
	"""
	self.nstatusbar.push(self.nstatid,' '+str(nstatus))

    def update_alias(self,nstatus) :
	"""
	Displays the nick name (if any) of who is talking in the status bar.
	"""
	self.astatusbar.push(self.astatid,' '+str(nstatus))

    def update_room_name(self,*args) :
	"""
	Called at regular intervals to update the name of the voice chat room we are in.
	"""
	i=get_chat_room_name()
	if i==self.last_room: return 1
	self.roomentry.set_text(i)
	self.last_room=i
	return 1


    def check_requested_room(self,*args) :
	"""
	Called at regular intervals to see if we have switched chat rooms in GYach 
	Enhanced.  If we have, Py Voice Chat is reconnected to the new chat room.
	"""
	if get_chat_status()<2: return 1
	if is_mic_test(): return 1
	r=get_requested_chat_room()
	#print "req:  "+str(r)
	if r=='': return 1
	i=get_chat_room_name()
	if not i==r:
		self.stop_voice_cb()
		time.sleep(1)
		self.start_voice_cb()
	return 1
	
    def set_status(self,mystat) :
	"""
	Displays any generic string 'mystat' in the status bar.
	"""
	self.statlab.set_text(" "+mystat+" ")


    def doQuit(self,*args) :
	"""
	Callback when we are about to exit PyVoiceChat.
	"""
	self.save_server()
	stopPyVoice()
	close_sound()
	do_cleanup()
	gtk.mainquit()


    def get_translator_credits(self,*args) :
	"""
	Gather a string containing the credits for the translator (if any).
	"""
	try:
		xtext_inf=pyvoice_xtext.info()
		return "\n\n"+_("Translation")+":  "+xtext_inf['language-team']+"\n"+_("Translator")+":  "+xtext_inf['last-translator']
	except:
		return ""

    def doAbout(self,*args) :
	"""
	Displays the 'about' box.
	"""
	s=_("pY! Voice Chat")+"\n\n"+_("The FIRST Yahoo! voice chat program for Linux operating systems.")+"\n\n"+_("Author")+":  Erica Andrews\n"+_("Email")+ ":  <PhrozenSmoke ['at'] yahoo.com>\n"
	s=s+_("License")+":  GNU General Public License\n"+_("Version")+":  "+self.version+"\n\n"
	s=s+_("Neither pY! Voice Chat nor its author are in anyway endorsed by or affiliated with Yahoo! (tm).  Yahoo! (tm) is a registered trademark of Yahoo! Communications.")+"\n\n"
	s=s+"Copyright (c) 2003-2004, Erica Andrews.\n\n'Can't hold a good penguin down!'\n http://phpaint.sourceforge.net/pyvoicechat/"
	s=s+self.get_translator_credits()

	message(s,["  "+_("OK")+"  "],1,1)

    def doContrib(self,*args) :
	"""
	Displays the 'contributors' box.
	"""
	s=_("CONTRIBUTORS")
	s=s+"\n\n"+_("This program would not have been possible without the following contributions")+":\n\n\n"
	s=s+"Modified, slimmed-down version of MPlayer's Wine-based audio codec loader (used in PyTSP):\n- Piotr Karwasz  (karwasz ['at'] clipper.ens.fr)\n\nUseful documentation on Yahoo network packets and example code using the audio codec:\n- Robert Brewer (rbrewer ['at'] op.net)\n\nEthereal network captures and other useful information about Yahoo's inner workings:\n- Jan Andersson (janneand ['at'] yahoo.com)\n\n\nThank you!"

	message(s,["  "+_("OK")+"  "])


    def hide_server(self,*args):
	self.vserv=args[0].get_data("entry").get_text().strip()
	args[0].get_data("window").hide()
	args[0].get_data("window").destroy()
	args[0].get_data("window").unmap()
	self.save_server()


    def selectServer(self,*args):
	"""
	Callback for selecting a voice chat server from a list of available servers.
	"""
	WMCLASS="pyvoicechat"
	WMNAME="PyVoiceChat"
	selwin=GtkWindow(GTK.WINDOW_TOPLEVEL)
	selwin.set_wmclass(WMCLASS,WMNAME)
	selwin.realize()
	selwin.set_title(_("pY! Voice Chat"))
	selwin.set_position(GTK.WIN_POS_CENTER)
	mains=GtkVBox(0,0)
	mains.set_border_width(5)
	mains.set_spacing(3)
	mains.pack_start( GtkLabel(_("Select a voice server")+":"), 0,0,2)
	mainc=GtkCombo()
	mainc.set_popdown_strings(get_vchat_server_list())
	selwin.set_data("entry",mainc.entry)
	mainc.entry.set_editable(0)
	mainc.entry.set_text(self.vserv)
	mains.pack_start(mainc, 0,0,2)
	mains.pack_start(GtkHSeparator(), 0,0,2)
	b=GtkButton(" "+_("OK")+" ")
	selwin.set_data("window",selwin)
	b.set_data("window",selwin)
	b.connect("clicked",self.hide_server)
	b.set_data("entry",mainc.entry)
	selwin.connect("destroy",self.hide_server)
	mains.pack_start(b,0,0,4)
	selwin.add(mains)
	selwin.show_all()



class _MessageBox(GtkDialog):	
    """
    A generic message dialog box class.
    """
    def __init__(self, title="Message", message="", buttons=("Ok",), modal=1):
        """
        message: either a string or a list of strings

        """
        GtkDialog.__init__(self)
        self.set_title(title)
	self.modal=modal
	self.set_wmclass("pyvoicechat","PyVoiceChat") 
        self.connect("destroy", self.quit)
        self.connect("delete_event", self.quit)
        self.connect("key-press-event", self.__press)
        self.modal = modal
        if self.modal:
            grab_add(self)
        hbox = GtkHBox(spacing=5)
        hbox.set_border_width(10)
        self.vbox.pack_start(hbox,1,1,0)
        hbox.show()
        if type(message) == type(""):
            label = GtkLabel(message)
        else:
            label = GtkLabel(join(message, "\n"))
        label.set_justify(JUSTIFY_LEFT)
	label.set_line_wrap(1)  
        hbox.pack_start(label,1,1,0)
        label.show()
        if not buttons:
            buttons = ("Ok",)
	tips=GtkTooltips()
        for text in buttons:
            b = GtkButton(' '+text+ ' ')
	    tips.set_tip(b,text)
            if text == buttons[0]:
                # the first button will always be the default button:
                b.set_flags(CAN_FOCUS|CAN_DEFAULT|HAS_FOCUS|HAS_DEFAULT)
            b.set_flags(CAN_DEFAULT)
            b.set_data("user_data", text)
            b.connect("clicked", self.__click)
            self.action_area.pack_start(b)
            b.show()
        self.default = buttons[0]
        self.ret = None

    def quit(self, *args):
        if self.modal:
            grab_remove(self)
        self.hide()
        self.destroy()
        if self.modal: mainquit()

    def __click(self, button):
        self.ret = button.get_data("user_data")
        self.quit()

    def __press(self, widget, event):
        if event.keyval == GDK.Escape:
            self.quit()
        elif event.keyval == GDK.Return:
            self.ret = self.default
            self.quit()


def message(message="", buttons=("Ok",),modal=1, big=0):
    """
    Create a message box, and return which button was pressed.
    """
    win = _MessageBox(_("pY! Voice Chat"), message, buttons,  modal)
    if big==1: win.set_default_size(375,-2)
    win.show_all()
    if modal==1:
    	mainloop()
    	return win.ret



def loadImage(picon,windowval):
	"""
	Internal GUI method.
	"""
	try:
		p=GtkPixmap(windowval,getPixDir()+str(picon),None)
		p.show_all()
		return p
	except:
		return None

def getIconButton(winval,icon,label):
	"""
	Internal GUI method.
	"""
	ic=loadImage(icon,winval)
	if ic==None: return GtkButton(label)
	b=GtkButton()
	b.add(ic)
	b.show_all()
	return b
def getIconButton2(winval,icon,label, toggle=0):
	"""
	Internal GUI method.
	"""
	ic=loadImage(icon,winval)
	if ic==None: 
		if toggle==1: return GtkToggleButton(label,0)
		return GtkButton(label)
	if toggle==1: b=GtkToggleButton(None,0)
	else: b=GtkButton()
	hb=GtkHBox(0,0)
	hb.set_border_width(1)
	hb.set_spacing(3)
	hb.pack_start(ic,0,0,0)
	hb.pack_start(GtkLabel(label),1,1,0)
	b.show_all()
	b.add(hb)
	return b


class ColorButton(GtkHBox):
	"""
	A class for a 'color indicator' button, which changes colors based on the state of our 
	connection.
	"""
	def __init__(self):
		GtkHBox.__init__(self)
		self.gc=None
		self.color_i16=[0.10,0.10,0.11]
		drawing_area = GtkDrawingArea()
		drawing_area.size(20, 10)
		drawing_area.show() 
		self.drawing_area=drawing_area
		self.drawwin=drawing_area.get_window()
		drawing_area.connect("expose_event", self.setColor)
		drawing_area.set_events(GDK.EXPOSURE_MASK |
		GDK.LEAVE_NOTIFY_MASK |
		GDK.BUTTON_PRESS_MASK )
		self.pack_start(drawing_area,0,0,0)
		self.set_border_width(3)
		self.show_all()

	def setColor(self,*args):
		if not self.drawwin: self.drawwin=self.drawing_area.get_window()
		if not self.gc: self.gc=self.drawwin.new_gc()
		self.gc.foreground=self.drawwin.colormap.alloc(self.color_i16[0]*65535,self.color_i16[1]*65535,self.color_i16[2]*65535)
		draw_rectangle(self.drawwin,self.gc,TRUE,0,0,self.drawwin.width,self.drawwin.height)
	

	def updateColorProperties(self, r, g, b): # new method,functionality separation, 5.16.2003
		self.color_i16=[r,g,b]
		self.setColor()



class SoundBar(GtkHBox):
	"""
	A class for a 'sound bar' that lights up as the volume of the sound changes.
	"""
	def __init__(self):
		GtkHBox.__init__(self)
		self.set_spacing(1)
		self.color_i16={}
		self.methods=[self.setColor1,self.setColor2,self.setColor3,self.setColor4,self.setColor5,self.setColor6,self.setColor7,self.setColor8,self.setColor9,self.setColor10]

		self.lights=[
			[0.0,0.0,0.72],
			[0.21,0.04,0.80],
			[0.51,0.43,0.84],
			[0.57,0.047,0.87],
			[0.66,0.047,0.90],
			[0.747,0.05,0.937],
			[0.826,0.066,0.882],
			[0.898,0.05,0.74],
			[0.91,0.05,0.635],
			[0.96,0.03,0.35],
	
		]

		self.gcs={}
		self.drawwins={}
		i=0
		self.drawings={}
		while i<10:
			drawing_area = GtkDrawingArea()
			drawing_area.size(10, 6)
			drawing_area.show() 
			self.drawing_area=drawing_area
			self.color_i16[i]=[0.10,0.10,0.11]
			self.drawwins[i]=drawing_area.get_window()
			self.gcs[i]=None
			drawing_area.connect("expose_event", self.methods[i])
			drawing_area.set_events(GDK.EXPOSURE_MASK |
			GDK.LEAVE_NOTIFY_MASK |
			GDK.BUTTON_PRESS_MASK )
			self.pack_start(drawing_area,0,0,0)
			self.drawings[i]=drawing_area
			i=i+1

		self.set_border_width(7)
		self.show_all()

	def setColor1(self,*args):
		self.updateColor(0)
	def setColor2(self,*args):
		self.updateColor(1)
	def setColor3(self,*args):
		self.updateColor(2)
	def setColor4(self,*args):
		self.updateColor(3)
	def setColor5(self,*args):
		self.updateColor(4)
	def setColor6(self,*args):
		self.updateColor(5)
	def setColor7(self,*args):
		self.updateColor(6)
	def setColor8(self,*args):
		self.updateColor(7)
	def setColor9(self,*args):
		self.updateColor(8)
	def setColor10(self,*args):
		self.updateColor(9)

	def updateColor(self,some_block):
		i=some_block
		if not self.drawwins[i]: self.drawwins[i]=self.drawings[i].get_window()
		if not self.gcs[i]: self.gcs[i]=self.drawwins[i].new_gc()
		self.gcs[i].foreground=self.drawwins[i].colormap.alloc(self.color_i16[i][0]*65535,self.color_i16[i][1]*65535,self.color_i16[i][2]*65535)
		draw_rectangle(self.drawwins[i],self.gcs[i],TRUE,0,0,self.drawwins[i].width,self.drawwins[i].height)
	

	def updateColorProperties(self, which_one, r, g, b): 
		self.color_i16[which_one]=[r,g,b]
		self.methods[which_one]()

	def set_volume(self,vnum):
		i=0
		if vnum>10: vnum=10
		if vnum<0: vnum=0
		while i<vnum:
			light=self.lights[i]
			self.updateColorProperties(i,light[0],light[1],light[2])
			i=i+1
		while i<10:
			light=[0.10,0.10,0.11]
			self.updateColorProperties(i,light[0],light[1],light[2])
			i=i+1


def do_cleanup(*args):
	"""
	Cleans up temporary files in /tmp/ .
	"""
	os.popen("rm -f /tmp/pyvoice_*")
	os.popen("rm -f /tmp/pytsp_*")


def runApp():
	"""
	Main method for launching the application.
	"""
	pyvoicegui()	
	setup_esd_host(my_esd_host)	
	open_sound()
	#startPyVoice()
	mainloop()

if __name__ == '__main__':
	runApp()

