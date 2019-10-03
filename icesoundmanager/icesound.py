# ! /usr/bin/python
############################################################################
# ICE SOUND MANAGER version 0.2beta1
# Copyright 2003 by Erica Andrews (PhrozenSmoke@yahoo.com).  All rights reserved.
# 
# Ice Sound Manager is freeware and open source. All legal, non-profit use, modification, 
# and distribution of this software is permitted provided all these credits are left in-tact and 
# unmodified.  This software is distributed as-is, without any warranty or guarantee of any 
# kind. Use at your own risk!
#
# Ice Sound Manager was designed to ease the management 
# of sound events, sound themes, and the sound server in the IceWM environment.  It is also 
# intended to be an improvement upon the noble, but primitive icesndcfg. The project is still 
# in its formative stages, so expect bugs.  (Report bugs to PhrozenSmoke@yahoo.com)
# 
# Ice Sound Manager is in no way 'officially' affiliated with IceWM, IceSound Server, 
# or its creators.  Ice Sound Manager is a FRONTEND to the IceSound Server. 
#
# Visit   http://members.aol.com/ButchWhipAppeal/  for my other software projects.
############################################################################
import sys, os, types, string, signal, copy, time
from _gtk import *
from GTK import *
from gtk import *
import GTK
from threading import RLock, Thread

global ism_version
ism_version="v. 0.2b-1"
global icemain
global statid
global statmessid
global statusbar
global iceo
global fs
global isetup
global exec_run
global audio_run
global wav_run
global pref_run
global server_exec
global sel_file
sel_file=None
global fdir
global themew
global icea
global wav_dir
global wav_cmd
global need_setup
global wav_edit
global use_terminal
use_terminal=0
global copy_wavs
copy_wavs=0
global start_server
start_server=0
global wav_pid
global themes
global audio_file
audio_file=".IceSoundManagerAudioRC"
global pref_file
pref_file=".IceSoundManagerRC"
themes={}
wav_pid=0
wav_edit="sweep %f &"
wav_cmd="wavplay -q %f &"
wav_dir="~/.icewm/sounds/"
fs=None
global disabled_sounds
disabled_sounds={}
global sound_events
global event_desc
global icey
global xterm
xterm="xterm"
global filelock
filelock=RLock()
global filelock2
filelock2=RLock()
global reg_lock
reg_lock=RLock()
need_setup=0
global event_sig
global audio_defaults
global server_version
global help_file
audio_defaults={"oss_dev":"/dev/dsp","yiff_host":"localhost","yiff_port":"9433","yiff_auto":"1","esd_host":"localhost","esd_port":"16001","interface":"esd"}
sound_events={"startup":"[NONE]","shutdown":"","restart":"[NONE]","closeAll":"[NONE]","launchApp":"[NONE]","workspaceChange":"[NONE]","windowOpen":"[NONE]","windowClose":"[NONE]","dialogOpen":"[NONE]","dialogClose":"[NONE]","windowMin":"[NONE]","windowMax":"[NONE]","windowRestore":"[NONE]","windowHide":"[NONE]","windowLower":"[NONE]","windowRollup":"[NONE]","windowSized":"[NONE]","windowMoved":"[NONE]","startMenu":"[NONE]"}
event_desc={"IceWM Startup":"startup","IceWM Shutdown":"shutdown","IceWM Restart":"restart","Close All":"closeAll","Launch Application":"launchApp","Work Space Changed":"workspaceChange","Window Opened":"windowOpen","Window Closed":"windowClose","Dialog Opened":"dialogOpen","Dialog Closed":"dialogClose","Window Minimized":"windowMin","Window Maximized":"windowMax","Window Restored":"windowRestore","Window Hidden":"windowHide","Window Lowered":"windowLower","Window Rolled Up":"windowRollup","Window Sized":"windowSized","Window Moved":"windowMoved","Start Menu Clicked":"startMenu"}
global current_theme_file
global current_theme
current_theme="[NONE]"
current_theme_file="/usr/share/IceSoundManager/sample_theme.stheme"
server_exec="/usr/X11R6/bin/icesound"

help_file="ICE SOUND MANAGER "+ism_version+" HELP FILE\n\nREQUIREMENTS:\nObviously, you need IceWM and the IceSound Server  (IceWM's sound server for sound events).  I suggest the most recent versions of IceWM and IceSound Server (versions 1.2.0 and above are best).  You also need to know WHERE the IceSound Server executable is located on your file system and which directories your IceSound Server is configured to look for .wav files for playing sound events.  Generally, these directories are  ~/.icewm/sounds/  (under your home directory) and  /usr/X11R6/lib/X11/icewm/sounds/.   NOTE: You need to make sure you have the necessary permissions necessary to start and stop the IceSound Server for your username and that you have read/write permissions for the directories where you will be storing your .wav files for use with IceSound Server.  (This is YOUR duty to deal with these issues).  If you cannot start and stop your server, permissions problems may be the reason.  So make sure you have that worked out first.  It is suggested that your IceSound Server executable be somewhere in your $PATH; however, this is should not be mandatory.  You also need to be using a system capable of using OSS, ESD, or YIFF as your sound interface.  If you are using OSS, you MUST know the location of your digital signal processing device, such as /dev/dsp.   If you are using the ESD or YIFF sound servers, you need to know the host and port number of those sound servers.  Lastly, you need some kind of sound card  (this is a no-brainer: no sound card = no sound events.)\n\nIn addition, you need recent versions of Python and PyGTK.  Neither TK/Tkinter nor Gnome are required to run this application.  This application was designed to be low on system resource usage.  So, you should not need a terribly fast computer to get good functionality from Ice Sound Manager.  Here is the environment in which this application has been tested: Mandrake 8.1, SuSe 8.1, Kernel 2.4.8 and 2.4.19, Glibc 2.2.4, XFree86 4.0.1, XFree 4.0.3, XFree 4.2.0, Python 2.2, PyGtk 0.6.9,  Gtk version 1.2.10, IceWM 1.0.9, IceWM 1.2.2, IceWM 1.2.5, IceWM-Gnome 1.0.9, with both 'icesound' and 'icesound-gnome' sound server executables.  The test computers (3) were all Pentium 1, ranging in memory from 64MB to 256MB - nothing spectacular.  \n\n\nWHAT IS ASSUMED:\nThat you have all the necessary software (and sound hardware) installed.  That your system meets all the requirements above. That you can troubleshoot on your own.  That you have common sense.  That you can tell the difference between a problem with your IceSound Server and a 'bug' in Ice Sound Manager.  (A bug/problem with your sound server is NOT a bug in this software).  I also assume that you understand that Ice Sound Manager is in no way 'officially' affiliated with IceWM, IceSound Server, or its creators.  Ice Sound Manager is a FRONTEND to the IceSound Server.  Lastly, I assume that you have run IceSound Server before, that you KNOW what your are doing, and are simply usings this software to make your sound events management job a little easier.  This application is not yet suitable for 'newbies' not ready to troubleshoot.\n\n\nGETTING STARTED:\nThe first thing you need to do is configure Ice Sound Manager to work with your IceSound Server and customize Ice Sound Manager.  You can do this easily by clicking 'Options' then 'Run Setup'.  This will launch the setup up program and get you on your way.  You should see many 'What's This?' buttons throughout Ice Sound Manager.  Click these buttons for help on specific features.  I HIGHLY recommend you use ESD as your audio interface for IceSound Server, because the OSS interface has a tendency to play sounds at the wrong sample rate, and I have not personally tested the YIFF audio interface. \n\n\nSOUND THEMES WITH ICE SOUND MANAGER "+ism_version+":\nOne of Ice Sound Manager's best features is giving the user the ability to manage sound event 'themes', similiar to Windows (gasp!).  To save a theme, click the 'Save' button in the themes section.  You can give your theme a descriptive name and specify a file for saving.   I HIGHLY suggest you save your Ice Sound Manager themes with the .stheme extension.  At a minimum, you should include the word  'theme' somewhere in the file name  (See the section below for an explanation).\n\n\nLOADING SOUND THEMES WITH ICE SOUND MANAGER "+ism_version+":\nWhen Ice Sound Manager is started, it will attempt to reload your last used theme, so that it can quickly reactivate your last-used sound events.  In addition, when Ice Sound Manager is started, it will  search the directory of your last-used sound theme for any other themes you may have created.  However, Ice Sound Manager will only attempt to inspect files that have the word 'theme' somewhere in the file name  (this is for the sake of speed, as directories may hold hundreds or thousands of files).  For this reason, it is suggested that you save your files with the '.stheme' extension, but you may choose another extension like '.theme'.  The important thing is to place the word 'theme' (case INsensitive) somewhere in the file name to make things easier on yourself.\n\nYou may also manually load sound themes, by clicking the 'Load' button in the themes section, which will allow you to browse the file system for your desired sound theme.  NOTE:  If you select a previously loaded theme from the drop-down themes list, you MUST hit the 'Reload' button with the green arrows next to the drop-down list, to reload the selected sound theme.\n\n\nWAV STORAGE DIRECTORY:\nThe Wav Storage Directory is the directory where your IceSound Server will look for the .wav files for your sound events.  You can configure this directory by clicking 'IceSound Server' then 'Wav Storage Directory' on the menu.   IMPORTANT NOTE:  As a general rule, you CANNOT specify random directories as your Wav Storage Directory.  You MUST know the directories your IceSound Server is willing to look for .wav files.  Generally, these directories are  ~/.icewm/sounds/  (under your home directory) and  /usr/X11R6/lib/X11/icewm/sounds/.  However, you should check the documentation for your particular version of the IceSound Server, as these directories may have been changed at compile-time.  It is YOUR job to know where your IceSound Server looks for .wavs.   If you specify your Wav Storage Directory as a random directory, such as  /usr/MySounds/, you may find that your IceSound Server does not play your sound events, because it is not looking in that directory.  KNOW where your IceSound Server is willing to look for .wav files.  Personally, I have had NO luck specifying anything other than ~/.icewm/sounds/  and  /usr/X11R6/lib/X11/icewm/sounds/  as my Wav Storage Directories.   NOTE:  If your Wav Storage Directories are ~/.icewm/sounds/  and  /usr/X11R6/lib/X11/icewm/sounds/,  you may use  /usr/X11R6/lib/X11/icewm/sounds/  to store system-wide sound events for all users, then use ~/.icewm/sounds/  on a per-user basis to specify personalized sound events.  On my systems, user-defined sounds in ~/.icewm/sounds/  override system-wide sounds in  /usr/X11R6/lib/X11/icewm/sounds/.\n\n\nPREFERNCES FILES IN ICE SOUND MANAGER:  \nIce Sound Manager has only 2 preferences files in your home directory:  .IceSoundManagerRC (basic preferences)  and  .IceSoundManagerAudioRC (your audio interface preferences.  Under NO circumstances should you modify these files by hand.  There is no need to.  Should these files disappear from your home directory or become damaged, you will see Ice Sound Manager run the 'Setup' program the next time you run Ice Sound Manager.\n\n\nTROUBLESHOOTING:\nHere are some quick answers, suggestions, and insight on some problems you may experience using Ice Sound Manager and IceSound Server.  These answers are in NO way authorative, but they might be helpful to someone, somewhere in some distant galaxy.\n\n\n\tPROBLEM: I've loaded a new sound theme, or changed around some sound events, and don't hear the changes taking affect in IceWM.  I'm still hearing the old sounds.\n\tSOLUTION:  Click 'Apply Sound Theme'.  This should restart your IceSound Server and make your changes take effect.  If you selected a theme from the drop-down theme list, be sure to click the 'Reload' button next to the drop-down list (the button with the green arrows) to reload your selected theme, then click '(Re)Start IceSound Server'.\n\n\tPROBLEM:  I can't get my IceSound Server started!\n\tSOLUTION:  Ok. This isn't a solution, but a step towards trouble shooting the problem.  Open a terminal window.  Then copy and paste the text from the field 'Suggested Sound Server Command Line' into the terminal window.  Run the command, and see what error messages you get.  Troubleshoot those errors, then try, try again.  Keep in mind that Ice Sound Manager is just a FRONTEND to IceSound Server. I am NOT the designer of IceWM or IceSound Server.\n\n\tPROBLEM:  My IceSound Server starts, but I see the following message at the console: 'Audio File Error: Unix Read Error'\n\tSOLUTION:  I, too, have had this problem occur when using some Windows .wav files compressed with certain proprietary codecs.  You'll have to do a trial-and-error thing where you disable all your .wavs, then re-enable them one-by-one until you determine which .wav is causing IceSound Server to choke.  Then, I would suggest you either take the offending .wav out of your pool of sound events or convert the .wav to a standard format your system can read.  NOTE: The 'Audio File Error' may be caused by other problems I am unaware of (I am NOT an expert on the 'icesound' server.)  However, since I experienced this problem, I thought I would share my insight.  The IceSound server seems to choke on some Windows-created .wavs and not choke on others.\n\n\tPROBLEM:  My sound events are not playing!\n\tSOLUTION:  Make sure your IceSound Server is running, by trying 'pidof icesound' and 'pidof icesound-gnome'.  If it's not running, get it running!  No sound server, no sound events will be played.   If the server IS running, make sure that your volume is not turned down or on mute.  If you are using OSS as your audio interface, make sure you have specified the RIGHT digital signal processing device  (everybody's is NOT /dev/dsp ...your job to find that out).  If you are using ESD or YIFF, make sure there is a ESD or YIFF sound server running on the host and port you have specified.  You can edit these properties by clicking 'IceSound Server' then 'Audio Interface' on the menu.  Third, and this is VERY important, scroll back up and read the section called 'Wav Storage Directory' in this help file.  If you have specified a  Wav Storage directory your IceSound Server does not accept, you are storing your .wavs in the WRONG directory and your sound events will not play.  Fourth, you should MANUALLY check your chosen Wav Storage Directory to make sure that your .wav files ARE present in that directory, and that there are no dead links (if you use the linking method).  Fifth, make sure all the files you have specified for your sound events are valid .wav files.  The IceSound Server may have trouble playing certain types of .wav, especially some times of Windows-created .wav files with special proprietary compression.  Sixth, make sure you have all your desired sound events ENABLED.  There is a checkmark in the sound events section of Ice Sound Manager for enabling/disabling the sound event.  If that checkmark is NOT checked, you have it disabled.  Finally, click '(Re)Start IceSound Server' to restart your IceSound Server.  This will make any changes in your sound events take affect provided you have no trouble running your IceSound Server.\n\n\tPROBLEM:  My sound events were playing earlier, but now I don't hear them. \n\tSOLUTION:  Make sure that your IceSound Server is running by clicking '(Re)Start IceSound Server'.  Your IceSound Server may need to be started EVERY time IceWM is started or restarted.  (It does on mine.  IceSound Server shuts down when I shutdown IceWM.)  Double check to make sure IceSound Server is running by typing 'pidof icesound' or 'pidof icesound-gnome', or run a utility like KPM to see your currently running processes.  If you are using the 'linking' method of setting your .wav files, make sure you haven't moved the original files.  You may have dead links in your 'Wav Storage Directory'.  Manually check your 'Wav Storage Directory' to make sure all your .wav files are there.  Check the sound volume on your computer!  Make sure your sound is not muted and that you have your sound set at an audible volume.  Lastly, make sure that you didn't accidently disable your sound events by unchecking the 'Enabled' button in the sound events section.\n\n\tPROBLEM: My sound events play, but they are scratchy or distorted.\n\tSOLUTION:  You probably need to readjust your 'Audio Interface' under 'IceSound Server' on the menu.  You may be using the wrong audio interface, or may be using the wrong DSP device for OSS.  It is YOUR job to find out what audio interface your system uses\n\n\tPROBLEM:  Some of my sound events play and some don't.\n\tSOLUTION: Make sure that you didn't accidently disable your sound events by unchecking the 'Enabled' button in the sound events section.  This button must be CHECKED for that sound event to be enabled.  Also, click '(Re)Start IceSound Server' to make sure any changes you made to your sound scheme take effect.  Lastly, understand that on some systems, certain sound events just don't seem to play.  On my system, I can't get IceSound Server to play the 'IceWM Startup' or 'IceWM Shutdown' events at all.  I don't know what causes this, as I am not the designer of the IceSound Server for IceWM.  I just build the frontends  8-).  \n\n\tPROBLEM: Some of my sound events play for the WRONG events.  Example:  'Window Hidden' plays when I 'minimize' and 'Window Minimized' plays when I actually 'restored' a window.\n\tSOLUTION:  This is not a 'bug' in Ice Sound Manager.  I have a similar issue with on my system.  I have checked this problem extensively and verified that Ice Sound Manager does set the 'Window Minimized' sound event to the windowMin.wav file in the Wav Storage Directory, and that Ice Sound Manager sets the 'Window Restored' sound event to the windowRestore.wav file in the Wav Storage Directory.  For some reason, my IceSound Server seems to want to play windowMin.wav when I restore a window, and wants to play windowHide.wav when I minimize a window.  Perhaps this is a 'bug' in the IceSound Server, or simply IceWM having a different interpretation of window 'events'.\n\n\tPROBLEM:  How do I make my sound events play every time I start IceWM?\n\tSOLUTION:  You have to set up IceWM to start IceSound Server everytime you start IceWM.  Click 'Help' then 'Sample Server Script' for a quick script you can use to do just that.   There is also an explanation of how to use the 'Sample Server Script' in that section.  After this, you are on your own to decide HOW you want to go about making IceWM start your IceSound server at startup.\n\n\tPROBLEM:  My sound events are not playing at the right speed, or sound distorted or 'lag' as they play.\n\tSOLUTION:  This is a common problem if you use OSS as your audio interface.  I HIGHLY recommend you use the ESD audio interface offered by IceSound Server.  It appears to be more stable, and I have had few problems with it."

def getServerExec(*args):
   global server_exec
   return str(server_exec).strip()

def getServerExecShort(*args):
   return getServerExec()[getServerExec().rfind("/")+1:len(getServerExec())].strip()

def getSuggestedCommandLine(*args):
   global server_exec
   global wav_dir
   global audio_defaults
   wavd=str(wav_dir)
   if wavd.endswith("/"):  wavd=wavd[0:len(wavd)-1]
   def_se=getServerExec()+" -i ESD -S localhost:16001 -v -s "+wavd+" &"
   if str(wav_dir).endswith("/"):
     def_se=getServerExec()+" -i ESD -S localhost:16001 -v -s "+wavd+" &"
   try:
     se=getServerExec()+" "
     if audio_defaults.has_key("interface"):
       iface=str(audio_defaults["interface"]).lower()
       se=se+"-i "+iface.upper()
       if iface=="oss":
         if audio_defaults.has_key("oss_dev"): se=se+" -D "+str(audio_defaults["oss_dev"])
       if iface=="esd":
         if audio_defaults.has_key("esd_host"): se=se+" -S "+str(audio_defaults["esd_host"])+":"
         if audio_defaults.has_key("esd_port"): 
           se=se+str(audio_defaults["esd_port"])
         else:
           se=se+"16001"
       if iface=="yiff":
         if audio_defaults.has_key("yiff_host"): se=se+" -S "+str(audio_defaults["yiff_host"])+":"
         if audio_defaults.has_key("yiff_port"): 
           se=se+str(audio_defaults["yiff_port"])
         else:
           se=se+"9433"
         if audio_defaults.has_key("yiff_auto"): 
           if str(audio_defaults["yiff_auto"])=="1":
             se=se+" --audio-mode-auto"
       return se+" -v -s "+wavd+" &"
     else:
       return def_se
   except:
     return def_se

class icewindow :
    def __init__ (self) :
	icewindow = GtkWindow (GTK.WINDOW_TOPLEVEL)
        global icey
        icey=self
        tips = GtkTooltips()
        icewindow.realize()
	icewindow.set_title ("Ice Sound Manager v. 0.2b-1")
	icewindow.set_policy (1, 1, 0)
	icewindow.set_position (GTK.WIN_POS_CENTER)
	tips.set_tip (icewindow, "Ice Sound Manager")
        self.tips=tips
	self.icewindow = icewindow
	mainvbox = GtkVBox (0, 0)
	mainvbox.set_border_width (2)
	self.mainvbox = mainvbox

        # MENUS
        ag = GtkAccelGroup()
        itemf = GtkItemFactory(GtkMenuBar, "<main>", ag)
        itemf.create_items([
            # path              key           callback    action#  type
            ("/_File",          "<alt>F",         None,             0, "<Branch>"),
            ("/_File/_Load Sound Theme...",    "<control>L", showThemeLoad,     1, ""),
            ("/_File/_Save Sound Theme...",  "<control>S", showThemeWindow,   2, ""),
            ("/_File/sep1",     None,         None,             3, "<Separator>"),
            ("/_File/_Quit",    "<control>Q", quit,         4, ""),
            ("/_IceSound Server",           "<alt>I",         None,             5, "<Branch>"),
            ("/_IceSound Server/_Wav Storage Directory",     "<control>W", askWavDir,      6, ""),
            ("/_IceSound Server/_Audio Interface",    "<control>A", showAudio,     7, ""),
            ("/_IceSound Server/IceSound _Executable",   "<control>E", askExec,    9, ""),
            ("/_IceSound Server/sep2",     None,         None,            10, "<Separator>"),
            ("/_IceSound Server/_Stop Sound Server",  None,         stopServer,  11, ""),
            ("/_IceSound Server/sep3",     None,         None,            12, "<Separator>"),
            ("/_IceSound Server/(_Re)Start Sound Server", None, startServer, 13, ""),
            ("/_Options", "<alt>O",       None, 14, "<Branch>"),
            ("/_Options/_Preferences...",  "<control>P",         showOptions,  15, ""),
            ("/_Options/Run _Setup",  None, runSetup,  21, ""),
            ("/_Help",          "<alt>H",         None,            16, "<LastBranch>"), 
            ("/_Help/_About...", "F1",        showAbout,   17, ""),
            ("/_Help/Ice Sound _Manager Help", "F4",        showGenericHelp,   18, ""), 
            ("/_Help/IceSound _Server Help", "F2",        showServerHelp,   19, ""),
            ("/_Help/_Sample Server Script", "F3",        showScriptHelp,   20, ""),
            ("/_Help/_Bug Reports and Comments", "F5",        showBugHelp,   22, "")
        ])
        icewindow.add_accel_group(ag)
        menubar1 = itemf.get_widget("<main>")
        menubar1.show()

	mainvbox.pack_start( menubar1, 0, 0, 0)
        headerpixmap=GtkLabel("Ice Sound Manager")
        try:
	  headerpixmap = GtkPixmap(icewindow,"./pixmaps/ism_header.xpm",None)
        except:
          headerpixmap=GtkLabel("Ice Sound Manager")
	self.headerpixmap = headerpixmap
	mainvbox.pack_start( headerpixmap, 1, 1, 4)
	themehbox = GtkHBox (0, 0)
	self.themehbox = themehbox
	themelab = GtkLabel ("Sound Theme")
	self.themelab = themelab
	themehbox.pack_start( themelab, 0, 0, 10)
        themelist=GtkCombo()
        self.themelist=themelist
	themetext = themelist.entry
	themetext.set_text ( '[NONE]')
	themetext.set_editable ( 1)
	tips.set_tip (themetext, "The current sound theme, if any")
	self.themetext = themetext
	themehbox.pack_start( themelist, 1, 1, 0)
        themeselbutton=getIconButton(icewindow,"./pixmaps/ism_reload.xpm","Reload")
        themeselbutton.connect("clicked",selectTheme)
        themehbox.pack_start(themeselbutton,0,0,2)
        self.themeselbutton=themeselbutton
        spacer=GtkLabel("    ")
        themehbox.pack_start(spacer,0,0,6)
        tips.set_tip(themeselbutton,"Switch to the selected theme")
        savebutt=getPixmapButton(icewindow,"./pixmaps/ism_save.xpm","Save...")
        tips.set_tip(savebutt,"Click to SAVE this theme.")
        savebutt.connect("clicked",showThemeWindow)
	self.savebutt = savebutt
	themehbox.pack_start( savebutt, 0, 0, 4)
        loadbutt=getPixmapButton(icewindow,"./pixmaps/ism_load.xpm","Load...")
        tips.set_tip(loadbutt,"Click to LOAD a new theme.")
        loadbutt.connect("clicked",showThemeLoad)
	self.loadbutt = loadbutt
	themehbox.pack_start( loadbutt, 0, 0, 0)
	frame1 = GtkLabel(" ")
	self.frame1 = frame1
	themehbox.pack_start( frame1, 1, 1, 10)
	mainvbox.pack_start( themehbox, 1, 1, 3)
	eventvbox = GtkVBox (0, 0)
	self.eventvbox = eventvbox
	eventtable = GtkTable (2, 8, 0)
	self.eventtable = eventtable
	selsoundtext = GtkEntry ()
	selsoundtext.set_text ( '[NONE]')
	selsoundtext.set_editable ( 0)
	tips.set_tip (selsoundtext, "Click 'Change' to change this sound")
	self.selsoundtext = selsoundtext
	eventtable.attach( selsoundtext, 1, 2, 1, 2, (GTK.EXPAND+GTK.FILL), (0), 0, 0)
	frame4 = GtkLabel(" ")
	self.frame4 = frame4
	eventtable.attach( frame4, 2, 3, 0, 1, (GTK.FILL), (GTK.FILL), 0, 0)
	frame5 = GtkLabel(" ")
	self.frame5 = frame5
	eventtable.attach( frame5, 4, 5, 0, 1, (GTK.FILL), (GTK.FILL), 0, 0)
	frame6 = GtkLabel(" ")
	self.frame6 = frame6
	eventtable.attach( frame6, 5, 6, 0, 1, (GTK.FILL), (GTK.FILL), 0, 0)
	eventlab = GtkLabel ("Sound Event")
	eventlab.set_padding ( 9, 0)
	eventlab.set_alignment ( 0, 0.5)
	self.eventlab = eventlab
	eventtable.attach( eventlab, 0, 1, 0, 1, (GTK.FILL), (0), 2, 0)
	eventcombo = GtkCombo ()
        global event_desc
        el=event_desc.keys()
        el.sort()
        eventcombo.set_popdown_strings(el)
	eventcombo.set_border_width ( 3)
	self.eventcombo = eventcombo
	event_entry =eventcombo.entry
	event_entry.set_editable ( 1)
	self.event_entry  = event_entry 
        global event_sig
        event_sig=event_entry.connect("changed",showSoundEvent,"clicked")
	eventtable.attach( eventcombo, 0, 1, 1, 2, (GTK.EXPAND+GTK.FILL), (0), 9, 0)
	selsoundbutt = getPixmapButton(icewindow,"./pixmaps/ism_choosesnd.xpm","Change...")
        selsoundbutt.connect("clicked",changeSound)
	tips.set_tip (selsoundbutt, "Select a sound for this event")
	self.selsoundbutt = selsoundbutt
	eventtable.attach( selsoundbutt, 2, 3, 1, 2, (GTK.FILL), (0), 13, 0)
	selsoundlab = GtkLabel ("Selected Sound")
	selsoundlab.set_padding ( 2, 0)
	selsoundlab.set_alignment ( 0, 0.5)
	self.selsoundlab = selsoundlab
	eventtable.attach( selsoundlab, 1, 2, 0, 1, (GTK.FILL), (0), 0, 0)
	frame7 = GtkLabel(" ")
	self.frame7 = frame7
	eventtable.attach( frame7, 6, 7, 0, 1, (GTK.FILL), (GTK.FILL), 0, 0)
	stopbutt = getIconButton(icewindow,"./pixmaps/ism_stop2.xpm","Stop")
        stopbutt.connect("clicked",stopSound)
	tips.set_tip (stopbutt, "Stop playing this sound")
	self.stopbutt = stopbutt
	eventtable.attach( stopbutt, 6, 7, 1, 2, (GTK.FILL), (0), 0, 0)
	playbutt = getIconButton(icewindow,"./pixmaps/ism_play2.xpm", "Play")
        playbutt.connect("clicked",playSound)
	tips.set_tip (playbutt, "Play this sound with your chosen wav player")
	self.playbutt = playbutt
	eventtable.attach( playbutt, 5, 6, 1, 2, (GTK.FILL), (0), 0, 0)
	nosoundbutt = getPixmapButton(icewindow,"./pixmaps/ism_nosound.xpm","No Sound")
        nosoundbutt.connect("clicked",doNoSound)
	tips.set_tip (nosoundbutt, "Set the sound for this event to NO sound")
	self.nosoundbutt = nosoundbutt
	eventtable.attach( nosoundbutt, 3, 4, 1, 2, (GTK.FILL), (0), 0, 0)
	enablebutt = GtkCheckButton ("Enabled")
        enablebutt.connect("toggled",checkEnable)
	enablebutt.set_border_width ( 2)
	tips.set_tip (enablebutt, "Enable/Disable the sound for the selected event")
	self.enablebutt = enablebutt
	eventtable.attach( enablebutt, 4, 5, 1, 2, (GTK.FILL), (0), 6, 0)
	editbutt = getIconButton(icewindow,"./pixmaps/ism_edit2.xpm","Edit")
        editbutt.connect("clicked",editSound)
	tips.set_tip (editbutt, "Edit this sound with your chosen sound editor")
	self.editbutt = editbutt
	eventtable.attach( editbutt, 7, 8, 1, 2, (GTK.FILL), (0), 10, 0)
	frame8 = GtkLabel(" ")
	self.frame8 = frame8
	eventtable.attach( frame8, 7, 8, 0, 1, (GTK.EXPAND+GTK.FILL), (GTK.FILL), 9, 0)
	eventvbox.pack_start(eventtable, 1, 1, 0)
	soundeventstat = GtkLabel ("  ")
	self.soundeventstat = soundeventstat
	eventvbox.pack_start(soundeventstat, 0, 0, 0)
	hseparator1 =GtkHSeparator ()
	hseparator1.set_usize ( -2, 4)
	self.hseparator1 = hseparator1
	eventvbox.pack_start(hseparator1, 1, 1, 3)
	mainvbox.pack_start( eventvbox, 1, 1, 10)
	servervbox = GtkVBox (0, 0)
	self.servervbox = servervbox
	cmdlinelab = GtkLabel ("Suggested Sound Server Command Line:")
	cmdlinelab.set_justify ( GTK.JUSTIFY_LEFT)
	cmdlinelab.set_alignment ( 0.0200001, 0.5)
	self.cmdlinelab = cmdlinelab
	servervbox.pack_start ( cmdlinelab, 0, 0, 0)
	cmdlinehbox = GtkHBox (0, 0)
	self.cmdlinehbox = cmdlinehbox
	entry2 = GtkEntry ()
	self.entry2 = entry2
	cmdlinehbox.pack_start( entry2, 1, 1, 11)
	frame2 = GtkLabel(" ")
	self.frame2 = frame2
	cmdlinehbox.pack_start( frame2, 1, 1, 2)
        serverwhat=getPixmapButton(icewindow,"./pixmaps/ism_help.xpm","What's this?")
        serverwhat.connect("clicked",showCmdHelp)
	tips.set_tip (serverwhat, "Help on the Sound Server Command Line")
	self.serverwhat = serverwhat
	cmdlinehbox.pack_start( serverwhat, 0, 0, 5)
	servervbox.pack_start( cmdlinehbox, 1, 1, 0)
	servbuttonhbox = GtkHBox (0, 0)
	servbuttonhbox.set_border_width(4)
	serverstartbutt = getPixmapButton(icewindow,"./pixmaps/ism_serveron.xpm","(Re)start IceSound Server")
        serverstartbutt.connect("clicked",startServer)
	tips.set_tip (serverstartbutt, "(Re)start  the IceSound Server to enable IceWM sound events")
	self.serverstartbutt = serverstartbutt
	servbuttonhbox.pack_start( serverstartbutt, 0, 0, 5)
	stopserverbutt = getPixmapButton(icewindow,"./pixmaps/ism_serveroff.xpm","Stop IceSound Server")
        stopserverbutt.connect("clicked",stopServer)
	tips.set_tip (stopserverbutt, "Stopping the IceSound server will disable IceWM sound events")
	self.stopserverbutt = stopserverbutt
	servbuttonhbox.pack_start(stopserverbutt, 0, 0, 0)
	servbuttonhbox.pack_start(  GtkLabel("     "), 1, 1, 1)
	atbutt = getPixmapButton(icewindow,"./pixmaps/ism_choosesnd.xpm","Apply Sound Theme")
        atbutt.connect("clicked",applySoundTheme)
	tips.set_tip (atbutt, "Apply this sound theme and start using this set of sounds now.")
	servbuttonhbox.pack_start(atbutt, 0, 0, 0)
	servervbox.pack_start( servbuttonhbox, 1, 1, 0)
	mainvbox.pack_start( servervbox, 1, 1, 0)
        global statusbar
	statusbar =GtkStatusbar ()
	self.statusbar = statusbar
        global statid
        global statmessid
        statid=statusbar.get_context_id("icesound context")
        #Startup stuff
        setupFiles()
        loadOptions()
        global current_theme_file
        searchForThemes()
        serrors=loadTheme(current_theme_file,1,0)
        try:
           icey.entry2.set_text(getSuggestedCommandLine())
        except:
          pass
        getServerVersion()
        global ism_version
        showStatus("Ice Sound Manager "+ism_version+".  Copyright 2003 Erica Andrews (PhrozenSmoke@yahoo.com).")
	mainvbox.pack_start( statusbar, 0, 0, 0)
	icewindow.add ( mainvbox)
	icewindow.show_all()
        global need_setup
        if need_setup: runSetup()
        else:
          if serrors: showSoundSetError(serrors)

def getPixmapButton (iwin,picon,btext) :  
	  b=GtkButton()
          b.add(getPixmapVBox(iwin,picon,btext))
          b.show_all()        
          return b

def getPixmapVBox(iwin,picon,btext):
        try:
	  p=GtkPixmap(iwin,str(picon),None)
	  v=GtkHBox(1,2)
          v.set_homogeneous(0)
          v.pack_start(p,0,0,1)
          v.pack_start(GtkLabel(str(btext)),0,0,1)
          v.show_all()
          return v
        except:
          v=GtkHBox(1,1)
          v.pack_start(GtkLabel(str(btext)),0,0,1)
          v.show_all()
          return v

def getIconButton (iwin,picon,lab="?") :  
        try:
	  b=GtkButton()
	  b.add(GtkPixmap(iwin,str(picon),None))
          b.show_all()        
          return b
        except:
          return GtkButton(str(lab))

def showStatus(statustext,*args):
    global statid
    global statmessid
    global statusbar
    statmessid=statusbar.push(statid," "+str(statustext))

def hideit(*args):
   global icemain
   icemain.hide()
   icemain.destroy()
   return TRUE

class iceoptions:
    def __init__ (self) :
	iceoptions = GtkWindow (GTK.WINDOW_TOPLEVEL)
	self._root = iceoptions
        tips = GtkTooltips()
        iceoptions.realize()
	iceoptions.set_title ("Ice Sound Manager Options")
	iceoptions.set_policy (1, 1, 0)
        iceoptions.connect("destroy", hideo)
	iceoptions.set_position ( GTK.WIN_POS_CENTER)
	tips.set_tip (iceoptions,"Ice Sound Manager Options")
	self.iceoptions = iceoptions
	vbox1 = GtkVBox(0, 0)
	self.vbox1 = vbox1
	titlelab = GtkLabel ("Ice Sound Manager Options")
	self.titlelab = titlelab
	vbox1.pack_start( titlelab, 0, 0, 11)
	table1 = GtkTable (2, 3, 0)
	self.table1 = table1
	playtext = GtkEntry ()
	self.playtext = playtext
	table1.attach(playtext, 1, 2, 0, 1, (GTK.EXPAND+GTK.FILL), (0), 0, 0)
	edittext = GtkEntry ()
	self.edittext = edittext
	table1.attach(edittext, 1, 2, 1, 2, (GTK.EXPAND+GTK.FILL), (0), 0, 0)
	playerlab = GtkLabel ("Wav Player Command Line")
	playerlab.set_alignment ( 0, 0.5)
	self.playerlab = playerlab
	table1.attach(playerlab, 0, 1, 0, 1, (GTK.FILL), (0), 4, 0)
	editorlab = GtkLabel ("Wav Editor Command Line")
	editorlab.set_alignment ( 0, 0.5)
	self.editorlab = editorlab
	table1.attach(editorlab, 0, 1, 1, 2, (GTK.FILL), (0), 4, 0)
	what1 = getPixmapButton(iceoptions,"./pixmaps/ism_help.xpm","What's this?")
        what1.connect("clicked",showPlayHelp)
	tips.set_tip (what1,"Click for help")
	self.what1 = what1
	table1.attach(what1, 2, 3, 0, 1, (GTK.FILL), (0), 4, 0)
	what2 = getPixmapButton(iceoptions,"./pixmaps/ism_help.xpm","What's this?")
        what2.connect("clicked",showEditHelp)
	tips.set_tip (what2,"Click for help")
	self.what2 = what2
	table1.attach(what2, 2, 3, 1, 2, (GTK.FILL), (0), 4, 0)
	vbox1.pack_start( table1, 1, 1, 0)
	hseparator1 = GtkHSeparator ()
	self.hseparator1 = hseparator1
	vbox1.pack_start( hseparator1, 1, 1, 8)
	table2 = GtkTable (3, 2, 0)
	self.table2 = table2
	autocheck = GtkCheckButton("Automatically start IceSound server when Ice Sound Manager starts")
	autocheck.set_active (1)
	tips.set_tip (autocheck,"Automatically start IceSound server when Ice Sound Manager starts")
	self.autocheck = autocheck
	table2.attach ( autocheck, 0, 1, 0, 1, (GTK.EXPAND+GTK.FILL), (0), 4, 0)
	copycheck = GtkCheckButton("Copy files instead of linking them")
	tips.set_tip (copycheck,"Copy files instead of linking them")
	self.copycheck = copycheck
	table2.attach ( copycheck, 0, 1, 1, 2, (GTK.EXPAND+GTK.FILL), (0), 4, 0)
	stdouttext = GtkCheckButton("Launch the IceSound Server in a terminal window")
	stdouttext.set_active ( 1)
	tips.set_tip (stdouttext,"Launch the IceSound Server in a terminal window")
	self.stdouttext = stdouttext
	table2.attach ( stdouttext, 0, 1, 2, 3, (GTK.EXPAND+GTK.FILL), (0), 4, 0)
	what4 = getPixmapButton(iceoptions,"./pixmaps/ism_help.xpm","What's this?")
	tips.set_tip (what4,"Click for help")
	self.what4 = what4
        what4.connect("clicked",showCopyHelp)
	table2.attach ( what4, 1, 2, 1, 2, (GTK.FILL), (0), 4, 0)
	what5 = getPixmapButton(iceoptions,"./pixmaps/ism_help.xpm","What's this?")
	tips.set_tip (what5,"Click for help")
	self.what5 = what5
        what5.connect("clicked",showTerminalHelp)
	table2.attach ( what5, 1, 2, 2, 3, (GTK.FILL), (0), 4, 0)
	what3 = getPixmapButton(iceoptions,"./pixmaps/ism_help.xpm","What's this?")
        what3.connect("clicked",showAutoHelp)
	tips.set_tip (what3,"Click for help")
	self.what3 = what3
	table2.attach ( what3, 1, 2, 0, 1, (GTK.FILL), (0), 4, 0)
	vbox1.pack_start( table2, 1, 1, 0)
	hseparator2 = GtkHSeparator ()
	self.hseparator2 = hseparator2
	vbox1.pack_start( hseparator2, 1, 1, 12)
	hbox1 = GtkHBox (0, 0)
	self.hbox1 = hbox1
	cancelbutt = getPixmapButton(iceoptions,"./pixmaps/ism_cancel.xpm","Cancel")
        cancelbutt.connect("clicked",hideo)
	tips.set_tip (cancelbutt,"Cancel without saving changes")
	self.cancelbutt = cancelbutt
	hbox1.pack_start( cancelbutt, 1, 1, 6)
	frame1 =GtkLabel("            ")
	self.frame1 = frame1
   	hbox1.pack_start( frame1, 1, 1, 0)
	savebutt = getPixmapButton(iceoptions,"./pixmaps/ism_save.xpm","Save")
        savebutt.connect("clicked",hideOptions)
   	tips.set_tip (savebutt,"Save changes and close this window")
	self.savebutt = savebutt
        iceoptions.set_modal(1)
	hbox1.pack_start( savebutt, 1, 1, 6)
	vbox1.pack_start( hbox1, 1, 1, 0)
	iceoptions.add( vbox1)

def hideo(*args):
   global iceo
   iceo.iceoptions.hide()
   iceo.iceoptions.destroy()
   return TRUE

def showOptions(*args):
   global iceo
   global wav_cmd
   global wav_edit
   global use_terminal
   global copy_wavs
   global start_server
   iceo=iceoptions()
   iceo.playtext.set_text(str(wav_cmd).strip())
   iceo.edittext.set_text(str(wav_edit).strip())
   iceo.copycheck.set_active(str(copy_wavs)=="1")
   iceo.autocheck.set_active(str(start_server)=="1")
   iceo.stdouttext.set_active(str(use_terminal)=="1")
   iceo.iceoptions.show_all()

def hideOptions(*args):
  global iceo
  global wav_cmd
  global wav_edit
  global use_terminal
  global copy_wavs
  global start_server
  global current_theme_file
  use_terminal=(iceo.stdouttext.get_active()==1)
  start_server=(iceo.autocheck.get_active()==1)
  copy_wavs=(iceo.copycheck.get_active()==1)  
  wav_cmd=iceo.playtext.get_text().strip()
  wav_edit=iceo.edittext.get_text().strip()
  iceo.iceoptions.hide()
  iceo.iceoptions.destroy()
  saveOptions()
  if iceo.playtext.get_text().find("%f") == -1 :
    showStatus("WARNING: '%f' needed in Wav Player Command Line for Ice Sound Manager to play them.")
  if iceo.edittext.get_text().find("%f") == -1 :
    showStatus("WARNING: '%f' needed in Wav Editor Command Line for Ice Sound Manager to edit them.")
  loadTheme(current_theme_file,0,1)


# HELP Section

class icehelp :
    def __init__ (self) :
	icehelp = GtkWindow (GTK.WINDOW_TOPLEVEL)
        icehelp.realize()
	icehelp.set_title ( "Ice Sound Manager Help")
	icehelp.set_policy ( 1, 1, 0)
	icehelp.set_position ( GTK.WIN_POS_CENTER)
        icehelp.set_usize(375,375)
        icehelp.connect("destroy",self.hideh)
        tips = GtkTooltips()
	tips.set_tip ( icehelp,"Ice Sound Manager Help")
	self.icehelp = icehelp
	vbox1 = GtkVBox (0, 0)
	vbox1.set_border_width ( 11)
	self.vbox1 = vbox1
	scrolledwindow1 = GtkScrolledWindow ()
	self.scrolledwindow1 = scrolledwindow1
	helptext =GtkText ()
	helptext.set_editable ( 0)
        helptext.set_word_wrap(1)
        helptext.set_line_wrap(1)
	self.helptext = helptext
	scrolledwindow1.add ( helptext)
	vbox1.pack_start ( scrolledwindow1, 1, 1, 0)
	closebutt = getPixmapButton(icehelp,"./pixmaps/ism_cancel.xpm","Close")
        closebutt.connect("clicked",self.hideh)
	tips.set_tip (closebutt, "Close this help window")
	self.closebutt = closebutt
	vbox1.pack_start ( closebutt, 0, 0, 13)
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
      return TRUE


def showModalHelp(helpText="Help text here.",*args):
   ih=icehelp()
   ih.icehelp.set_modal(1)
   ih.setHelpText(str(helpText))
   ih.icehelp.show_all()

def showGenericHelp(*args):
   global help_file
   ih=icehelp()
   ih.icehelp.set_modal(0)
   ih.setHelpText(help_file)
   ih.icehelp.show_all()

def showCmdHelp(*args):
  showModalHelp("This section of Ice Sound Manager deals with your IceSound Server itself.\n\nSUGGESTED SOUND SERVER COMMAND LINE:\nThis field shows you the auto-generated command line that is being used to launch your IceSound Server.  It is provided so YOU can see exactly what is being launched on your system and how, and may be useful for troubleshooting.  You can copy and paste the text in this field into a terminal window to aid in any troubleshooting you must do.  This field contains the EXACT same command that is executed when you click '(Re)Start IceSound Server' and is the command that will be executed if you have enabled automatic starting of your IceSound Server when Ice Sound Manager starts. NOTE: Editing the text in this field doeshas NOT effect command Ice Sound Manager launches to start your server. Anything you type here is ignored by Ice Sound Manager.  The 'Suggested Sound Server Command Line' is provided for informational purposes only.  You may also find this field useful for pasting into scripts, etc.\n\n(RE)START ICESOUND SERVER:\nThis will execute the UNEDITED version of the command line that appears in 'Suggested Sound Server Command Line'  to start/restart your IceSound Server.  Your IceWM sound events will NOT be enabled until the sound server has been started.  It is a good idea to restart the server whenever you have changed your sound theme, though Ice Sound Manager attempts to do this for you.\n\nSTOP ICESOUND SERVER:\nVery simply, this feature stops your IceSound Server, disabling all future sound events in IceWM.  NOTE:  Ice Sound Manager does not shutdown your sound server automatically when you exit Ice Sound Manager (it is assumed you want to keep your sound events enabled).  If you quit Ice Sound Manager, your IceSound Server will continue running, until you either click 'Stop IceSound Server' or issue a 'kill' command to your IceSound Server process. However, on most systems, the IceSound Server will shutdown automatically if you quit/restart your X Server.")

def showTerminalHelp(*args):
  showModalHelp("If CHECKED, your Ice Sound Server will be started in a terminal window.  The terminal application to be used is determined by the value of your $TERM environment variable (generally, xterm, gnome-terminal, kterm, or eterm).  You can determine the terminal application used to launch your Ice Sound Server, by setting the environment variable $TERM.  Type 'echo $TERM' to see what your default terminal application is.  NOTE: Your default terminal application in $TERM must accept the -e command line option and must understand it in a way similiar to xterm  for this option to work.  The only exception to this rule is gnome-terminal.")

def showAutoHelp(*args):
  showModalHelp("This option only applies to when ICE SOUND MANAGER is started, not when IceWM is started.  If CHECKED, Ice Sound Manager will start your Ice Sound Server (if it is not already running) whenever you start Ice Sound Manager.  Your Ice Sound Server will be started with the command shown in the field 'Suggested Sound Server Command Line'.  Leave this option unchecked if you do NOT want automatic starting of your Ice Sound Server when Ice Sound Manager is launched.")

def showCopyHelp(*args):
  showModalHelp("This option determines whether your .wav sounds will be LINKED or COPIED into your IceSound Server's Wav Storage Directory (as defined under the menu 'IceSound Server' -> 'Wav Directory'.  Leave this option UNCHECKED if you would rather your files be LINKED into your IceSound Server's Wav Storage Directory.  (This will save disk space, but your sound events will not play if you move/delete the original files.  Be sure to keep your sound themes current if you choose this method.)   CHECK this option if you would rather your .wav sounds be COPIED to your IceSound Server's Wav Storage Directory.  (This method uses more disk space, but ensures that your sound events will be unaffected should you move/delete the original .wav files.)  Which method you choose is up to you.")

def showPlayHelp(*args):
  showModalHelp("If you want to be able to PLAY your sounds in Ice Sound Manager, you must enter the command to launch a .wav sound player on your system.  Most people should be able to use the default command.  However, you may want or need to change this command.   NOTE:  Any command line you enter MUST contain the string '%f' somewhere in the command line  (this signifies where the .wav's filename should go in the command line.)  It is YOUR job to know or find out what command line to use to launch your .wav player.  If you fail to include '%f' in your command line, you will see an error message on the status bar when you attempt to play your sounds.  I personally suggest you use a command line that opens a fast, console-based .wav player (like wavplay) for the sake of speed. ")

def showEditHelp(*args):
  showModalHelp("If you want to be able to EDIT your sounds in Ice Sound Manager, you must enter the command to launch a .wav sound editor on your system.  Most people should be able to use the default command, IF they have the Sweep sound editor installed.  However, you may want or need to change this command.   NOTE:  Any command line you enter MUST contain the string '%f' somewhere in the command line  (this signifies where the .wav's filename should go in the command line.)  It is YOUR job to know or find out what command line to use to launch your .wav editor.  If you fail to include '%f' in your command line, you will see an error message on the status bar when you attempt to edit your sounds.")

def showBugHelp(*args):
  showModalHelp("Ice Sound Manager is in no way 'officially' affiliated with IceWM, IceSound Server, or its creators.  Ice Sound Manager is a FRONTEND to the IceSound Server.  The project is still in its formative stages.  Expect bugs. COURTEOUSLY report bugs in Ice Sound Manager to PhrozenSmoke@yahoo.com.  If you have any other comments or suggestions, those are welcomed as well.  Please do NOT report bugs in your IceSound Server, your IceWM environment, or ask me for help on getting your IceSound Server to run.  Also, please read 'Ice Sound Manager Help' under the 'Help' menu (or press F4) before reporting a 'bug'. You will find useful troubleshooting suggestions there. ")

def showServerHelp(*args):
   ih=icehelp()
   global server_version
   ih.setHelpText("ICESOUND SERVER HELP\nYou are running IceSound Server version: "+server_version+"\n\nThe following is the output returned by running your IceSound Server executable ("+str(getServerExec())+") with the --help switch.  You may find it useful for trouble shooting problems with your sound server.  If you see nothing below or see an error message, try modifying your IceSound Server command line, by clicking 'IceSound Server' on the menu. Also, make sure you have the permissions necessary to run the server executable:\n\n\n")
   try:
     f=os.popen(str(getServerExec())+" --help")
     x=f.readline()
     while x:
       if x=="":
         f.close() 
         break
       else:
         ih.appendHelpText(x)
         x=f.readline()
   except:
     pass
   ih.icehelp.show_all()

def showScriptHelp(*args):
  ih=icehelp()
  ih.helptext.set_editable ( 1)
  global server_version
  ih.setHelpText("SAMPLE SERVER SCRIPT\nHere is a simple bourne/bash shell script you can use to easily start and stop your IceSound server.  All you should have to do is copy and paste it to a file.  Save it as /usr/bin/icesoundsh.  Type 'chmod 755 /usr/bin/icesoundsh' at a terminal.  You can start the server by typing 'icesoundsh start', and shutdown the server by typing 'icesoundsh stop'.  Hopefully, this is useful.  Note: It's probably not a smart idea to save this script under /usr/bin/icesound, especially if the server executable itself is called 'icesound'.  Copy and paste the section below this line:\n\n---------------\n#! /bin/sh\n#\n# icesound          Start/Stop the icesound server.\n#\n# chkconfig: 2345 40 60\n# description: The IceWM sound events sound server\n# processname: icesound\n\n\n# Source function library.\n. /etc/rc.d/init.d/functions\n\nRETVAL=0\n\n# See how we were called.\ncase \"$1\" in\n  start)\n\techo -n \"Starting IceSound Server (version "+server_version+"): \"\n\texec "+str(getSuggestedCommandLine())+"  > /dev/null\n\tRETVAL=$?\n\techo\n\tRETVAL=$?\n\t;;\n  stop)\n\techo -n \"Stopping IceSound server: \"\n\tkillall -9 "+getServerExecShort()+"\n\tRETVAL=$?\n\techo\n\tRETVAL=$?\n\t;;\n  status)\n\tstatus icesound\n\tRETVAL=$?\n\t;;\n  restart)\n  \t$0 stop\n\t$0 start\n\tRETVAL=$?\n\t;;\n  reload)\n\tkillall -HUP "+getServerExecShort()+"\n\tRETVAL=$?\n\t;;\n  *)\n\techo \"Usage: icesound {start|stop|status|restart}\"\n\texit 1\nesac\n\nexit $RETVAL\n")
  ih.icehelp.show_all()

def showAbout(*args):
  global ism_version
  showModalHelp("ABOUT ICE SOUND MANAGER "+ism_version+"\n\nCopyright 2003 by Erica Andrews (PhrozenSmoke@yahoo.com).  All rights reserved.\n\nhttp://icesoundmanager.sourceforge.net\n\nIce Sound Manager is in no way 'officially' affiliated with IceWM, IceSound Server, or its creators.  Ice Sound Manager is a FRONTEND to the IceSound Server.  Ice Sound Manager is freeware and open source. This program is distributed under the W3C License. See the 'LICENSE' file for details.  All legal, non-profit use, modification, and distribution of this software is permitted provided all these credits are left in-tact and unmodified.  This software is distributed as-is, without any warranty or guarantee of any kind. Use at your own risk!\n\nIce Sound Manager was designed to ease the management of sound events, sound themes, and the sound server in the IceWM environment.  It is also intended to be an improvement upon the noble, but primitive icesndcfg. The project is still in its formative stages, so expect bugs.  (Report bugs to PhrozenSmoke@yahoo.com) \n\nVisit   http://members.aol.com/ButchWhipAppeal/  for my other software projects.")

# Directory Browsing

class dirdialog :
    def __init__ (self) :
	dirdialog = GtkWindow (GTK.WINDOW_TOPLEVEL)
	self._root = dirdialog
	dirdialog.set_title ( "Ice Sound Manager")
        tips =GtkTooltips()
	dirdialog.set_position (GTK.WIN_POS_CENTER)
	dirdialog.set_modal ( 1)
	tips.set_tip (dirdialog,"Ice Sound Manager")
	self.dirdialog = dirdialog
	vbox1 = GtkVBox (0, 0)
	vbox1.set_border_width ( 2)
	vbox1.set_usize (355, 158)
	self.vbox1 = vbox1
	scrolledwindow1 =GtkScrolledWindow ()
	self.scrolledwindow1 = scrolledwindow1
	htext = GtkText ()
	htext.set_editable (0)
        htext.set_word_wrap(1)
	self.htext = htext
	scrolledwindow1.add ( htext)
	vbox1.pack_start ( scrolledwindow1, 1, 1, 6)
	hbox1 =GtkHBox (0, 0)
	self.hbox1 = hbox1
	ltext = GtkEntry ()
        ltext.set_editable (0)
	ltext.set_usize (170, 23)
	self.ltext = ltext
	hbox1.pack_start ( ltext, 1, 1, 4)
	lbutton =getPixmapButton(dirdialog,"./pixmaps/ism_load.xpm","Browse...")
	self.lbutton = lbutton
	hbox1.pack_start ( lbutton, 0, 0, 5)
	vbox1.pack_start ( hbox1, 1, 1, 0)
	hbox2 = GtkHBox (0, 0)
	self.hbox2 = hbox2
	label1 = GtkLabel ("  ")
	self.label1 = label1
	hbox2.pack_start ( label1, 1, 1, 0)
	closebutton = getPixmapButton(dirdialog,"./pixmaps/ism_ok.xpm","OK")
        tips.set_tip(closebutton,"Close and save changes")
	self.closebutton = closebutton
	hbox2.pack_start ( closebutton, 1, 1, 0)
	label2 = GtkLabel ("  ")
	self.label2 = label2
	hbox2.pack_start ( label2, 1, 1, 0)
	vbox1.pack_start ( hbox2, 0, 0, 6)
	dirdialog.add ( vbox1)

    def setHelpText(self, helpstuff):
      try:
       self.htext.delete_text(0,self.htext.get_length())
       self.htext.delete_text(0,self.htext.get_length()+1)
      except:
       pass
      self.htext.insert_defaults(str(helpstuff))

def hideDir(*args):
   global fdir
   fdir.dirdialog.hide()
   fdir.dirdialog.destroy()
   return TRUE

def askExec(*args):
   global fdir
   global server_exec
   d=dirdialog()
   fdir=d
   d.closebutton.connect("clicked",updateExec) 
   d.lbutton.connect("clicked",doExecDia)
   d.ltext.set_text(str(server_exec))
   d.setHelpText("Select your IceSound Server executable.  This is generally either in the /usr/X11R6/bin/  or  /usr/bin/  directory and is generally named either 'icesound' or 'icesound-gnome'.  Be sure you have the correct permissions to execute the IceSound Server and that you have the permissions to run servers/daemons on your system.")
   d.dirdialog.show_all()

def updateExec(*args):
   global server_exec
   global fdir
   if checkExec(str(fdir.ltext.get_text())):
     server_exec=checkExec(str(fdir.ltext.get_text()))
     saveOptions()
     hideDir()  
   else:
     showStatus("ERROR!  Select your IceSound Server executable file first.")

def doExecDia(*args):
   global sel_file
   global server_exec
   sel_file=server_exec
   f=showFileSelection("Select Your IceSound Server Executable",execFile,execCancel)

def execFile(*args):
   global fdir
   fdir.ltext.set_text(checkExec(grabFile()))

def execCancel(*args):
  global fdir
  ffile=checkExec(str(fdir.ltext.get_text()))
  doFsHide()
  fdir.ltext.set_text(ffile)

def checkExec(fname,defname=""):
  if not fname:
    return defname
  if fname.strip().endswith("/"):
    return defname
  return fname

def askWavDir(*args):
   global fdir
   global wav_dir
   d=dirdialog()
   fdir=d
   d.closebutton.connect("clicked",updateWavDir) 
   d.ltext.set_text(str(wav_dir))
   d.lbutton.connect("clicked",doWavDia)
   d.setHelpText("Select the directory where your IceSound server looks  for its .wav files.  On most systems, this directory will be either    ~/.icesound/sounds/  or  /usr/X11R6/lib/X11/icewm/sounds/.  If you aren't sure, check the documentation for your IceSound server or do the trial-and-error method.   Generally, you should not go wrong with ~/.icesound/sounds/  under your home directory (example: /home/phrozen/.icewm/sounds/).  You may have to create this directory yourself if Ice Sound Manager can't. Be sure to select only directories where you have both 'read' and 'write' permissions. See the section entitled 'Wav Storage Directory' under 'Help' -> 'Ice Sound Manager Help' (or press F4).")
   d.dirdialog.show_all()

def updateWavDir(*args):
   global fdir
   f=extractDirectory(str(fdir.ltext.get_text()))
   if f:
     hideDir()  
     setWavDir(f)
   else:
     showStatus("ERROR!  Select the IceSound Server's Wav storage directory first.")

def setWavDir(wdir,*args):
  global wav_dir
  global sound_events
  if wdir:
    serr=[]
    showStatus("Updating Wav Storage Directory...")
    try:  # try to delete all the old sound event files
      s=sound_events.keys()
      for i in s:
        try:
          f=os.popen("rm -f "+wdir+s+".wav > /dev/null")
        except:
          pass
    except:
      pass
    try: # try to remove the old directory if its empty
      os.rmdir(wav_dir)
    except:
      pass
    wav_dir=wdir # set the new directory    
    try: # try to create the new directory
      os.mkdir(wav_dir)
    except:
      pass
    try:  # transfer the new sound events to new directory
      s=sound_events.keys()
      for i in s:
        try:
          stat=setSoundForEvent(i,sound_events[i],0,0,1)
          if not stat: serr.append("Link/Copy  "+sound_events[i]+" to "+wav_dir+" : TRANSFER TO NEW DIRECTORY FAILED")
        except:
          pass
    except:
      pass
    saveOptions()
    startServer()
    showStatus("Done.")
    if len(serr): showSoundSetError(serr)

def doWavDia(*args):
   global sel_file
   global wav_dir
   sel_file=wav_dir
   showFileSelection("Wav Storage Directory Selection",wavFile,wavCancel)

def wavFile(*args):
   global fdir
   fdir.ltext.set_text(extractDirectory(grabFile()))

def wavCancel(*args):
  global fdir
  ffile=extractDirectory(str(fdir.ltext.get_text()))
  doFsHide()
  fdir.ltext.set_text(ffile)

def extractDirectory(fname):
    if not fname:  
      return ""
    fname=fname.strip()
    try:
      fname=fname.replace("~",os.environ["HOME"])
    except:
      pass
    if fname.endswith("/"):
     return fname
    else: 
      if fname.find("/") > -1:
        return fname[0:fname.rfind("/")+1].strip()
      else:
        return "/"
    return fname
   
def createFileSel(winTitle,ok_method,cancel_method,*args):
        global fs
        global sel_file
        fs = GtkFileSelection()
        fs.set_title(str(winTitle))
        fs.set_position (GTK.WIN_POS_CENTER)
        fs.connect('destroy', cancel_method)
        if sel_file:
          fs.set_filename(sel_file)
        fs.ok_button.remove(fs.ok_button.children()[0])
        fs.ok_button.add(getPixmapVBox(fs,"./pixmaps/ism_ok.xpm","OK"))
        fs.ok_button.connect('clicked', ok_method)
        fs.set_modal(1)
        fs.cancel_button.remove(fs.cancel_button.children()[0])
        fs.cancel_button.add(getPixmapVBox(fs,"./pixmaps/ism_cancel.xpm","Cancel"))
        fs.cancel_button.connect('clicked', cancel_method)
        return fs

def grabFile(*args):   
        global fs
        global sel_file
        if not fs:
          return ""
 	sel_file=str(fs.get_filename()).strip()
	fs.hide()
        return sel_file

def doFsHide(*args):
     global fs
     fs.hide()
     fs.destroy()
     return TRUE

def showFileSelection(winTitle,ok_method,cancel_method,*args):
    f=createFileSel(winTitle,ok_method,cancel_method)
    f.show_all()
    return f


# Theme Management

class themewindow  :
    def __init__ (self) :
	themewindow = GtkWindow (GTK.WINDOW_TOPLEVEL)
	self._root = themewindow
        global themes
	themewindow.set_title ("Save Theme...")
	themewindow.set_position (GTK.WIN_POS_CENTER)
        themewindow.set_modal(1)
        tips=GtkTooltips()
	tips.set_tip (themewindow, "Ice Sound Manager: Save Theme...")
	self.themewindow = themewindow
	vbox1 = GtkVBox (0, 3)
	vbox1.set_border_width ( 10)
	self.vbox1 = vbox1
	label1 = GtkLabel ("Theme Name:")
	self.label1 = label1
	vbox1.pack_start( label1, 0, 0, 0)
	namecombo = GtkCombo ()
	self.namecombo = namecombo
	nameentry = namecombo.entry
	tips.set_tip (nameentry,  "Enter a descriptive name for your IceWM sound theme")
	self.nameentry = nameentry
	vbox1.pack_start( namecombo, 0, 0, 0)
	label2 = GtkLabel ("  ")
	self.label2 = label2
	vbox1.pack_start( label2, 0, 0, 0)
	label3 = GtkLabel ("Theme File:")
	self.label3 = label3
	vbox1.pack_start( label3, 0, 0, 0)
	hbox1 = GtkHBox (0, 12)
	self.hbox1 = hbox1
	filecombo = GtkCombo ()
        filecombo.set_usize(400,25)
	self.filecombo = filecombo
	file_entry = filecombo.entry
        if len(themes):
          tf=themes.values()
          tf.sort()
          filecombo.set_popdown_strings(copy.copy(tf))
          tf=themes.keys()
          tf.sort()
          namecombo.set_popdown_strings(copy.copy(tf))
	tips.set_tip (file_entry, "Theme files")
	self.file_entry = file_entry
        file_entry.set_editable(0)
	hbox1.pack_start( filecombo, 1, 1, 0)
	filebutton = getPixmapButton(themewindow,"./pixmaps/ism_load.xpm","Browse...")
        filebutton.connect("clicked",chooseThemeFile)
	tips.set_tip (filebutton, "Browse to select a file for saving")
	self.filebutton = filebutton
	hbox1.pack_start( filebutton, 0, 0, 0)
	vbox1.pack_start( hbox1, 1, 1, 0)
	label6 = GtkLabel ("  ")
	self.label6 = label6
	vbox1.pack_start( label6, 0, 0, 0)
	hbox2 = GtkHBox (1, 0)
	self.hbox2 = hbox2
	cancelbutt = getPixmapButton(themewindow,"./pixmaps/ism_cancel.xpm","Cancel")
        cancelbutt.connect("clicked",hideThemeWindow)
        themewindow.connect("destroy",hideThemeWindow)
	tips.set_tip (cancelbutt, "Close this window and discard all changes")
	self.cancelbutt = cancelbutt
	hbox2.pack_start( cancelbutt, 1, 1, 0)
	label7 = GtkLabel( "  ")
	self.label7 = label7
	hbox2.pack_start( label7, 1, 1, 0)
	savebutt = getPixmapButton(themewindow,"./pixmaps/ism_save.xpm","SAVE")
        savebutt.connect("clicked",saveTheme)
	tips.set_tip (savebutt, "Save theme and close this window")
	self.savebutt = savebutt
	hbox2.pack_start( savebutt, 1, 1, 0)
	vbox1.pack_start( hbox2, 1, 1, 0)
	themewindow.add ( vbox1)


def showThemeWindow(*args):
  global themew
  global current_theme_file
  global current_theme
  blockSignals()
  themew=themewindow()
  if current_theme_file:
     themew.file_entry.set_text(current_theme_file)
  if current_theme:
     themew.nameentry.set_text(current_theme)
  themew.themewindow.show_all()
  
def hideThemeWindow(*args):
   global themew
   themew.themewindow.hide()
   themew.themewindow.destroy()
   unblockSignals()
   return TRUE


def chooseThemeFile(*args):
   global sel_file
   global current_theme_file
   if current_theme_file:
     sel_file=current_theme_file
   showFileSelection("Save Sound Theme As...",themeFile,themeCancel)

def themeFile(*args):
   global themew
   c=checkExec(grabFile(),"")
   themew.file_entry.set_text(c)
   global current_theme_file
   current_theme_file=c

def themeCancel(*args):
  global themew
  ffile=checkExec(str(themew.file_entry.get_text()))
  doFsHide()
  themew.file_entry.set_text(ffile)
   
def saveTheme(*args):
   global themew
   global current_theme_file
   global current_theme
   tname=themew.nameentry.get_text().strip()
   current_theme_file=themew.file_entry.get_text()
   if current_theme_file:
     if tname:
       current_theme=tname
       associateTheme(current_theme,current_theme_file)
       saveThemeFile()
       hideThemeWindow()
   else:
     showStatus("ERROR!  Enter a theme NAME and theme FILE first.")

def saveThemeFile(*args):
  global current_theme_file
  global current_theme
  global icey
  global disabled_sounds
  global ism_version
  global filelock
  if current_theme_file:
    if current_theme:
      filelock.acquire(1)
      try:
        f=open(str(current_theme_file),"w")
        f.write("# Ice Sound Manager "+ism_version+" Sound Theme\n# The line below is necessary for Ice Sound Manager to identify this as a legitimate theme.\n# Include NO spaces at the end of the next 2 lines. Comments must be on their OWN line.\nTYPE=PhrozenSoundTheme\nThemeName="+current_theme+"\nstartup="+getSoundForEvent("startup")+"\nshutdown="+getSoundForEvent("shutdown")+"\nrestart="+getSoundForEvent("restart")+"\ncloseAll="+getSoundForEvent("closeAll")+"\nlaunchApp="+getSoundForEvent("launchApp")+"\nworkspaceChange="+getSoundForEvent("workspaceChange")+"\nwindowOpen="+getSoundForEvent("windowOpen")+"\nwindowClose="+getSoundForEvent("windowClose")+"\ndialogOpen="+getSoundForEvent("dialogOpen")+"\ndialogClose="+getSoundForEvent("dialogClose")+"\nwindowMin="+getSoundForEvent("windowMin")+"\nwindowMax="+getSoundForEvent("windowMax")+"\nwindowRestore="+getSoundForEvent("windowRestore")+"\nwindowHide="+getSoundForEvent("windowHide")+"\nwindowLower="+getSoundForEvent("windowLower")+"\nwindowRollup="+getSoundForEvent("windowRollup")+"\nwindowSized="+getSoundForEvent("windowSized")+"\nwindowMoved="+getSoundForEvent("windowMoved")+"\nstartMenu="+getSoundForEvent("startMenu")+"\n")
        for y in disabled_sounds.keys():
          f.write(str(y)+"=[DISABLED]\n")
        f.flush()
        f.close()
        icey.themetext.set_text(current_theme)
        showStatus("Theme '"+current_theme+"' successfully saved to: "+current_theme_file)
      except:
        showStatus("ERROR while saving theme '"+str(current_theme)+"' to '"+str(current_theme_file)+"'!")
      filelock.release()
     

def disableSoundEvent(sevt,refresh=1,doSave=1,doCheck=1,quiet=0,*args):
   global sound_events
   global disabled_sounds
   if sound_events.has_key(str(sevt)):
     disabled_sounds[str(sevt)]=str(sevt)
     blockSignals()
     try:
       i=setSoundForEvent(str(sevt),getSoundForEvent(str(sevt)),refresh,doSave,doCheck)
       if not quiet:
         if not i:   showSoundSetError(["Remove: "+str(evt)+"    "+getSoundForEvent(str(sevt)) ])
     except:
       pass
     unblockSignals()

def enableSoundEvent(sevt,*args):
   if disabled_sounds.has_key(str(sevt)):
     del disabled_sounds[str(sevt)]
     blockSignals()
     try:
       i=setSoundForEvent(str(sevt),getSoundForEvent(str(sevt)))
       if not i:   showSoundSetError(["Link/Copy: "+str(sevt)+"    "+getSoundForEvent(str(sevt))])
     except:
       pass
     unblockSignals()

def checkEnable(*args):
   global icey
   if icey.enablebutt.get_active():
     enableSoundEvent(getSoundEventForDesc(icey.event_entry.get_text()))
   else:
     disableSoundEvent(getSoundEventForDesc(icey.event_entry.get_text()))


def isDisabled(sevt,*args):
   global disabled_sounds
   return disabled_sounds.has_key(str(sevt))

def getSoundForEvent(sevt,*args):
   global sound_events
   if sound_events.has_key(str(sevt)):
     return str(sound_events[str(sevt)])

def setSoundForEvent(sevt,sfile,refresh=1,doSave=1,doCheck=1,*args):
   global sound_events
   global wav_dir
   global copy_wavs
   global disabled_sounds
   if sound_events.has_key(str(sevt)):
     if str(sfile).strip():
       sound_events[str(sevt)]=str(sfile).strip()
       if doSave: 
         try: 
           t=Thread(None,saveThemeFile)
           t.start()
         except: 
           showStatus("Could not save Theme File:  Unknown thread error.")
       if not doCheck: return 1
       wavd=str(wav_dir)
       if not wavd.endswith("/") : wavd=wavd+"/"
       try:
         deregisterWav(wavd+str(sevt).strip()+".wav") # erase old file
         if disabled_sounds.has_key(str(sevt)) or str(sfile)=="[NONE]": 
           if refresh:  refreshServer()
           if not doCheck: return 1
           return fileExists(wavd+str(sevt).strip()+".wav")==0  #done removing/disabling
         registerWav(str(sfile).strip(),wavd+str(sevt).strip()+".wav" , copy_wavs==0)
         if refresh:  refreshServer()
         if not doCheck: return 1
         return fileExists(wavd+str(sevt).strip()+".wav")        # check to see if it's there
       except:
         pass
   return 1

def registerWav(fromFile,toFile,link=1): # put the wav on the file system
  if not fromFile: return
  if not toFile: return
  global reg_lock
  reg_lock.acquire()
  try:
    if link:
      os.popen("ln -sf "+str(fromFile).strip()+" "+str(toFile).strip())
    else: #copy
      os.popen("cp -f "+str(fromFile).strip()+" "+str(toFile).strip())
  except:
    pass
  reg_lock.release()

def deregisterWav(wavFile):
  if not wavFile: return
  global reg_lock
  reg_lock.acquire()
  try:
    os.popen("rm -f "+str(wavFile).strip()+" > /dev/null")
  except:
    pass
  reg_lock.release()


def getSoundEventForDesc(desc,*args):
   global event_desc
   if event_desc.has_key(str(desc).strip()):
     return str(event_desc[str(desc).strip()])

def showSoundEvent(quiet=0,*args):
   global icey
   global wav_dir
   global sound_events
   blockSignals()
   try:
     wavd=str(wav_dir)
     if not str(wav_dir).endswith("/"): wavd=wav_dir+"/"
     icey.selsoundtext.set_text(showSoundShort( getSoundForEvent(getSoundEventForDesc(icey.event_entry.get_text())))  )
     e=isDisabled(getSoundEventForDesc(icey.event_entry.get_text()))
     if e:
       icey.enablebutt.set_active(FALSE)
     else:
       icey.enablebutt.set_active(TRUE)
     i=setSoundForEvent(getSoundEventForDesc(icey.event_entry.get_text()), getSoundForEvent(getSoundEventForDesc(icey.event_entry.get_text())),0,0,1)
     if not i : 
       if not quiet: 
         showSoundSetError(["Link/Copy:  "+getSoundEventForDesc(icey.event_entry.get_text())+"   "+ getSoundForEvent(getSoundEventForDesc(icey.event_entry.get_text()))])
   except:
     pass
   unblockSignals()


def showSoundShort(csound,*args):
   csound=str(csound)
   return csound[csound.rfind("/")+1:len(csound)].strip()


def doNoSound(*args):
  global icey
  global wav_dir
  blockSignals()
  icey.selsoundtext.set_text("[NONE]")
  sstat=setSoundForEvent(getSoundEventForDesc(icey.event_entry.get_text()),"[NONE]")
  if not sstat:
    showSoundSetError(["Remove  "+getSoundEventForDesc(icey.event_entry.get_text())+".wav  in  "+str(wav_dir) +"   FAILED"])
  unblockSignals()


def changeSound(*args):
   doSoundDia()

def doSoundDia(*args):
   global sel_file
   global icey
   csound=checkFile(getSoundForEvent(getSoundEventForDesc(icey.event_entry.get_text())))
   if not csound=="[NONE]":
     sel_file=csound
   f=showFileSelection("Select Wav File",soundFile,soundCancel)


def soundFile(*args):
   global icey
   global wav_dir
   blockSignals()
   ff=grabFile()
   sstat=setSoundForEvent(getSoundEventForDesc(icey.event_entry.get_text()),checkFile(ff))
   if not sstat:
    showSoundSetError(["Link/Copy   "+ff+"  to  "+getSoundEventForDesc(icey.event_entry.get_text())+".wav  in  "+str(wav_dir) +"   FAILED"])
   showSoundEvent(1)
   unblockSignals()

def soundCancel(*args):
  global fdir
  global icey
  blockSignals()
  csound=checkFile(getSoundForEvent(getSoundEventForDesc(icey.event_entry.get_text())))
  doFsHide()
  i=setSoundForEvent(getSoundEventForDesc(icey.event_entry.get_text()),csound)
  showSoundEvent()
  unblockSignals()


def checkFile(fname,*args):
  return checkExec(fname,"[NONE]")

def playSound(*args):
   global wav_cmd
   global wav_pid
   global icey
   if wav_cmd.strip():
     if getSoundForEvent(getSoundEventForDesc(icey.event_entry.get_text())) == "[NONE]":
       return
     stopSound()
     pl=wav_cmd.strip().replace("%f",getSoundForEvent(getSoundEventForDesc(icey.event_entry.get_text())) ).split(" ")
     if len(pl) > 1: # need at least 2 arguments
       if not wav_pid: # something already playing?
        wcmd=pl[0]
        pargs=pl[1:len(pl)+1]
        pargs.insert(0,"")
        wav_pid=os.spawnvpe(os.P_NOWAIT,wcmd,pargs,os.environ)
        showStatus("Playing sound '"+getSoundForEvent(getSoundEventForDesc(icey.event_entry.get_text()))+"' ...")
   else:
     showStatus("ERROR: No Wav player configured.  Click 'Options -> Preferences' on the menu.")

def stopSound(*args):
    global wav_pid
    try:
     if not wav_pid:
       return 
     os.popen("kill -9 "+str(wav_pid) +" > /dev/null" )
     os.popen("kill -9 "+str(wav_pid) +" > /dev/null" )
    except:
     pass
    wav_pid=0

def editSound(*args):
   global wav_edit
   try:
     if wav_edit.strip():
        if getSoundForEvent(getSoundEventForDesc(icey.event_entry.get_text())) == "[NONE]":
          return
        f=os.popen(wav_edit.strip().replace("%f",getSoundForEvent(getSoundEventForDesc(icey.event_entry.get_text())) ))
     else:
        showStatus("ERROR: No Wav editor configured.  Click 'Options -> Preferences' on the menu.")
   except:
     pass

def showSoundSetError(serrors,*args):
        global wav_dir
        serr="UH OH!:\n\n"+str(len(serrors))+" error(s) occurred setting your sound events for the IceSound server.  The errors were as follows:\n\n"
        for si in serrors:
           serr=serr+str(si)+"\n"
        serr=serr+"\n\nThese types of errors generally mean that Ice Sound Manager experienced trouble linking, copying, or removing Wav files from your selected Wav Storage Directory ("+str(wav_dir)+").  Please check that the files you are linking/copying exist, that you have the proper read/write permissions for this directory and these files, AND that you have correctly configured your Wav Storage Directory in the menu under 'IceSound Server'. Also, make sure you haven't run out of disk space."
        showModalHelp(serr)

def isTheme(ttext,*args):
  try:
     ttext.index("TYPE=PhrozenSoundTheme\n") 
     tt=ttext[ttext.index("ThemeName=")+10:len(ttext)]
     return tt[0:tt.index("\n")].strip() # return name of theme
  except: # this will throw exception of it's not a real theme
    return 0

def searchForThemes(*args):
  global current_theme_file
  try:
    tdir=extractDirectory(current_theme_file)
    l=os.listdir(tdir)
    for tfile in l:
      if tfile.lower().find("theme") > -1:
        if not os.path.isdir(tdir+tfile):
          f=open(tdir+tfile)
          ttext=f.read()
          f.close()
          tname=isTheme(ttext)
          if tname:  associateTheme(tname,tdir+tfile)
  except:
    pass

def loadTheme(fname,quiet=0,refresh=1,*args):
   global current_theme_file
   global current_theme
   global sound_events
   global icey
   blockSignals()
   serrors=[]
   if not fname: return
   try:
     f=open(fname,"r")
     th=f.read().strip()
     f.close()
     if not isTheme(th): raise TypeError
     thl=th.split("\n")
     current_theme_file=fname
     s=copy.copy(sound_events)
     for i in thl:
       tline=i.strip()
       if tline:
         if not tline.startswith("#"):
           if not tline=="TYPE=PhrozenSoundTheme":
             parts=tline.split("=")
             if parts > 1:
               propname=parts[0].strip()
               propval=parts[1].strip()
               if propname=="ThemeName":
                 current_theme=propval
                 icey.themetext.set_text(current_theme)
                 showStatus("Loading theme '"+current_theme+"'...")
               else: 
                 if propval.upper()=="[DISABLED]":
                   disableSoundEvent(propname,0,0,0,1)
                   if s.has_key(propname): del s[propname]
                 else: 
                   sstat=setSoundForEvent(propname,checkFile(propval),0,0,1) # dont refresh, save, or check while loading
                   if s.has_key(propname): del s[propname]
                   if not sstat:  serrors.append(propname+"  :    "+propval)
     if len(s): # any unaccounted for sounds get set to None
       for i in s.keys():
         sstat=setSoundForEvent(i,"[NONE]",0,0,0)
     associateTheme(current_theme,current_theme_file)
     refreshServer()
     showSoundEvent(1) 
     if not len(serrors):
       showStatus("Theme '"+current_theme+"' successfully loaded.")
     else:
       if not quiet:
         showSoundSetError(serrors)
   except:
     showStatus("ERROR loading theme from file '"+fname+"'! Unknown file type.")
   icey.themetext.set_text(str(current_theme))
   unblockSignals(refresh)
   return serrors

def updateThemes(*args):
   global themes
   global icey
   global current_theme
   if  len(themes):
     blockSignals()
     try: 
       l=themes.keys()
       l.sort()
       icey.themelist.set_popdown_strings(l)
       icey.themelist.show_all()
     except:
       pass
     unblockSignals()
     try:
       if current_theme: icey.themetext.set_text(current_theme)
     except:
       pass

def associateTheme(tname,tfile,*args):
    global themes
    themes[str(tname)]=str(tfile)
    updateThemes()

def showThemeLoad(*args):
   doThemeLoadDia()

def doThemeLoadDia(*args):
   global sel_file
   global current_theme_file
   sel_file=current_theme_file
   f=showFileSelection("Select Ice Sound Manager Theme File",lthemeFile,lthemeCancel)

def lthemeFile(*args):
   tfile=checkFile(grabFile())
   loadTheme(tfile)
   updateThemes()


def lthemeCancel(*args):
   doFsHide()

def selectTheme(*args):
    global icey
    global themes
    t=icey.themetext.get_text()
    if not t=="[NONE]":
      if themes.has_key(t):
        loadTheme(themes[t],0,1)
    return TRUE

def blockSignals(*args):
    global event_sig
    global icey
    icey.event_entry.signal_handler_block(event_sig)

def unblockSignals(refresh=1,*args):
    global event_sig
    global icey
    icey.event_entry.signal_handler_unblock(event_sig)
  

# Audio Interface

class iceaudio :
    def __init__ (self) :
	iceaudio = GtkWindow(GTK.WINDOW_TOPLEVEL)
	self._root = iceaudio
        iceaudio.realize()
	iceaudio.set_title ("Ice Sound Manager: Audio Interface")
	iceaudio.set_position ( GTK.WIN_POS_CENTER)
	iceaudio.set_modal ( 1)
        tips=GtkTooltips()
	tips.set_tip (iceaudio, "Ice Sound Manager: Audio Interface")
	self.iceaudio = iceaudio
	frame1 = GtkFrame(None)
	self.frame1 = frame1
	table1 = GtkTable (7, 3, 0)
	table1.set_row_spacings (3)
	table1.set_col_spacings ( 26)
	table1.set_border_width ( 3)
	self.table1 = table1
	oss = GtkRadioButton (None,"OSS")
        oss.connect("clicked",doAudioToggle,1)
	tips.set_tip (oss, "Select to use the OSS audio interface")
	self.oss = oss
	table1.attach(oss, 0, 1, 0, 1, (GTK.EXPAND+GTK.FILL), (0), 0, 0)
	esd = GtkRadioButton (self.oss, "ESD")
	tips.set_tip (esd, "Select to use the ESD audio interface")
        esd.connect("clicked",doAudioToggle,2)
	self.esd = esd
	table1.attach(esd, 1, 2, 0, 1, (GTK.EXPAND+GTK.FILL), (0), 0, 0)
	yiff =GtkRadioButton (self.oss, "YIFF")
	tips.set_tip (yiff, "Select to use the YIFF audio interface")
        yiff.connect("clicked",doAudioToggle,3)
	self.yiff = yiff
	table1.attach(yiff, 2, 3, 0, 1, (GTK.EXPAND+GTK.FILL), (0), 0, 0)
	label1 = GtkLabel ("Device")
	label1.set_alignment ( 0, 0.5)
	self.label1 = label1
	table1.attach(label1, 0, 1, 1, 2, (GTK.FILL), (0), 0, 0)
	label2 = GtkLabel ("Host")
	label2.set_alignment ( 0, 0.5)
	self.label2 = label2
	table1.attach(label2, 1, 2, 1, 2, (GTK.FILL), (0), 0, 0)
	label3 = GtkLabel ("Host")
	label3.set_alignment ( 0, 0.5)
	self.label3 = label3
	table1.attach(label3, 2, 3, 1, 2, (GTK.FILL), (0), 0, 0)
	devtext = GtkEntry ()
	tips.set_tip (devtext, "Enter the Digital Signal Processing  DSP  to use")
	self.devtext = devtext
	table1.attach(devtext, 0, 1, 2, 3, (GTK.EXPAND+GTK.FILL), (0), 0, 0)
	host1text = GtkEntry ()
	tips.set_tip (host1text, "Enter the host of the ESD server")
	self.host1text = host1text
	table1.attach(host1text, 1, 2, 2, 3, (GTK.EXPAND+GTK.FILL), (0), 0, 0)
	host2text = GtkEntry ()
	tips.set_tip (host2text, "Enter the host of the YIFF server")
	self.host2text = host2text
	table1.attach(host2text, 2, 3, 2, 3, (GTK.EXPAND+GTK.FILL), (0), 0, 0)
	label4 = GtkLabel ("Port")
	label4.set_alignment ( 0, 0.5)
	self.label4 = label4
	table1.attach(label4, 1, 2, 3, 4, (GTK.FILL), (0), 0, 0)
	label5 = GtkLabel ("Port")
	label5.set_alignment ( 0, 0.5)
	self.label5 = label5
	table1.attach(label5, 2, 3, 3, 4, (GTK.FILL), (0), 0, 0)
	port1text = GtkEntry ()
	tips.set_tip (port1text, "Enter the port number of the ESD server")
	self.port1text = port1text
	table1.attach(port1text, 1, 2, 4, 5, (GTK.EXPAND+GTK.FILL), (0), 0, 0)
	port2text = GtkEntry ()
	tips.set_tip (port2text, "Enter the port number of the YIFF server")
	self.port2text = port2text
	table1.attach(port2text, 2, 3, 4, 5, (GTK.EXPAND+GTK.FILL), (0), 0, 0)
	autocheck = GtkCheckButton("Auto Audio Mode")
	tips.set_tip (autocheck, "If checked  the YIFF server will attempt to automatically adjust to the correct audio mode")
	self.autocheck = autocheck
	table1.attach(autocheck, 2, 3, 5, 6, (GTK.FILL), (0), 0, 0)
	helpbutton = getPixmapButton(iceaudio,"./pixmaps/ism_help.xpm","Help")
        helpbutton.connect("clicked",showAudioHelp)
	tips.set_tip (helpbutton, "Click for help on the audio interface options")
	self.helpbutton = helpbutton
	table1.attach(helpbutton, 0, 1, 6, 7, (GTK.FILL), (0), 0, 0)
	label7 = GtkLabel ("  ")
	label7.set_alignment ( 0, 0.5)
	self.label7 = label7
	table1.attach(label7, 0, 1, 3, 4, (GTK.FILL), (0), 0, 0)
	label8 = GtkLabel ("  ")
	label8.set_alignment ( 0, 0.5)
	self.label8 = label8
	table1.attach(label8, 0, 1, 4, 5, (GTK.FILL), (0), 0, 0)
	label9 = GtkLabel ("  ")
	label9.set_alignment ( 0, 0.5)
	self.label9 = label9
	table1.attach(label9, 0, 1, 5, 6, (GTK.FILL), (0), 0, 0)
	label10 = GtkLabel ("  ")
	label10.set_alignment ( 0, 0.5)
	self.label10 = label10
	table1.attach(label10, 1, 2, 5, 6, (GTK.FILL), (0), 0, 0)
	okbutton = getPixmapButton(iceaudio,"./pixmaps/ism_ok.xpm","OK")
        okbutton.connect("clicked",closeAudio)
	tips.set_tip (okbutton, "Save changes and close this window")
	self.okbutton = okbutton
	table1.attach(okbutton, 1, 2, 6, 7, (GTK.FILL), (0), 0, 0)
	label11 = GtkLabel ("  ")
	label11.set_alignment ( 0, 0.5)
	self.label11 = label11
	table1.attach(label11, 2, 3, 6, 7, (GTK.FILL), (0), 0, 0)
	frame1.add(table1)
	iceaudio.add(frame1)

def showAudio(*args):
   global icea
   global audio_defaults
   icea=iceaudio()
   setAudioDefaults()
   doAudioToggle()
   icea.iceaudio.show_all()

def closeAudio(*args):
    global icea
    global audio_defaults
    try:
      string.atoi(icea.port2text.get_text().strip())
      string.atoi(icea.port1text.get_text().strip())
    except:
      showStatus("ERROR!   The port number must be an INTEGER!")
      return
    setAudioProp("yiff_port",icea.port2text.get_text())
    setAudioProp("yiff_host",icea.host2text.get_text())
    setAudioProp("esd_port",icea.port1text.get_text())
    setAudioProp("esd_host",icea.host1text.get_text())
    setAudioProp("oss_dev",icea.devtext.get_text())
    setAudioProp("yiff_auto",str(icea.autocheck.get_active()))
    if icea.esd.get_active():
      setAudioProp("interface","esd")
    if icea.oss.get_active():
      setAudioProp("interface","oss")
    if icea.yiff.get_active():
      setAudioProp("interface","yiff")
    icea.iceaudio.hide()
    icea.iceaudio.destroy()
    saveOptions()
    return TRUE

def saveAudioOptions(*args):
   global audio_defaults
   global audio_file
   global ism_version
   if audio_file:
     try:
       f=open(audio_file,"w")
       f.write("")
       f.write("# Ice Sound Manager "+ism_version+" audio interface preferences\n# DO NOT EDIT!\n")
       for i in audio_defaults.keys():
         f.write(str(i).strip().lower()+"="+str(audio_defaults[i]).strip().lower()+"\n")
       f.flush()
       f.close()
       showStatus("Audio interface options successfully saved.")
     except:
       showStatus("ERROR:  While saving audio interface preferences to '"+str(audio_file)+"'!")

def loadAudioOptions(*args):
   global audio_defaults
   global audio_file
   if audio_file:
     try:
       f=open(audio_file,"r")
       atext=f.read()
       f.close()
       alist=atext.split("\n")
       for i in alist:
         aline=i.strip()
         if not aline.startswith("#"):
            if aline.find("=") > -1:
              apair=aline.split("=")
              if len(apair) > 1:
                setAudioProp(apair[0],apair[1])
     except:
       pass

def showAudioHelp(*args):
    showModalHelp("Here is where you can configure the sound device you want your IceSound Server to use to play your sound events.  Most people will probably use either ESD or OSS.  It is YOUR job to know what kind of sound device your system uses. \n\nOSS:\nFor OSS, click 'OSS' and type in the 'dev' location of your digital signal processing device (i.e. /dev/dsp)\n\nESD:\nFor ESD, click 'ESD' and enter the host and port of the ESD sound server running on your system.  For most people, this will be host 'localhost' and port '16001'.  You can find out what port ESD is running on your system by typing 'netstat -lp' in a terminal window, then looking for any process named 'esound' or 'esd'.  If you dont see either of these, you probably don't have a currently running ESD sound server on your system and will not be able to use ESD as your interface until you get an ESD server running.\n\nYIFF:  If you know that you need to use YIFF, please make sure that your copy of the IceSound Server has been compiled with YIFF support (not all copies are YIFF-enabled.).  To use the YIFF interface, click 'YIFF', then enter the host and port of your YIFF sound server.  You may also check 'Auto Audio Mode' to enable your IceSound Server to automatically choose the best sampling rate and playback mode for your sounds. ")

def setAudioProp(propname,propval):
   global audio_defaults
   if audio_defaults.has_key(str(propname).strip().lower()):
     if str(propval).strip():
       audio_defaults[str(propname).strip().lower()]=str(propval).strip().lower()

def doAudioToggle(*args):
    global icea
    if icea.yiff.get_active():
      icea.autocheck.set_sensitive(1)
      icea.host2text.set_editable(1)
      icea.port2text.set_editable(1)
      icea.host1text.set_editable(0)
      icea.port1text.set_editable(0)
      icea.devtext.set_editable(0)
    if icea.oss.get_active():
      icea.devtext.set_editable(1)
      icea.host2text.set_editable(0)
      icea.port2text.set_editable(0)
      icea.autocheck.set_sensitive(0)
      icea.host1text.set_editable(0)
      icea.port1text.set_editable(0)
    if icea.esd.get_active():
      icea.host1text.set_editable(1)
      icea.port1text.set_editable(1)
      icea.devtext.set_editable(0)
      icea.host2text.set_editable(0)
      icea.port2text.set_editable(0)
      icea.autocheck.set_sensitive(0)

def setAudioDefaults(*args):
   global icea
   global audio_defaults
   icea.esd.set_active(1)
   icea.oss.set_active(str(audio_defaults["interface"])=="oss")
   icea.yiff.set_active(str(audio_defaults["interface"])=="yiff")
   icea.host2text.set_text(audio_defaults["yiff_host"])
   icea.port2text.set_text(audio_defaults["yiff_port"])
   icea.autocheck.set_active(str(audio_defaults["yiff_auto"])=="1")
   icea.devtext.set_text(audio_defaults["oss_dev"])
   icea.host1text.set_text(audio_defaults["esd_host"])
   icea.port1text.set_text(audio_defaults["esd_port"])
   

def fileExists(fname, *args):
  try:
     return os.path.exists(str(fname).strip())
  except:
     return 0

# SETUP 

class icesetup :
    def __init__ (self) :
	icesetup = GtkWindow (GTK.WINDOW_TOPLEVEL)
	self._root = icesetup
        icesetup.realize()
	icesetup.set_title ( "Ice Sound Manager Setup")
	icesetup.set_policy ( 1, 1, 0)
	icesetup.set_position ( GTK.WIN_POS_CENTER)
        icesetup.set_usize(450,400)
        tips = GtkTooltips()
	tips.set_tip ( icesetup,"Ice Sound Manager Setup")
	self.icesetup = icesetup
	vbox1 = GtkVBox (0, 0)
	vbox1.set_border_width ( 11)
	self.vbox1 = vbox1
        try:
	  headerpixmap = GtkPixmap(icesetup,"./pixmaps/ism_header.xpm",None)
        except:
          headerpixmap=GtkLabel("Ice Sound Manager")
	vbox1.pack_start ( headerpixmap, 1, 1, 2)
        try:
	  setuppixmap = GtkPixmap(icesetup,"./pixmaps/ism_setup.xpm",None)
        except:
          setuppixmap=GtkLabel("SETUP")
	vbox1.pack_start ( setuppixmap, 1, 1, 4)
        spacer=GtkLabel("  ")
	vbox1.pack_start ( spacer, 1, 1, 3)         
	scrolledwindow1 = GtkScrolledWindow ()
	self.scrolledwindow1 = scrolledwindow1
        scrolledwindow1.set_usize(430,250)
	helptext =GtkText ()
	helptext.set_editable ( 0)
        helptext.set_word_wrap(1)
	self.helptext = helptext
	scrolledwindow1.add ( helptext)
	vbox1.pack_start ( scrolledwindow1, 1, 1, 0)
	closebutt = getPixmapButton(icesetup,"./pixmaps/ism_next.xpm","Next")
        closebutt.connect("clicked",returnSetup)
	tips.set_tip (closebutt, "Proceed with Setup")
	self.closebutt = closebutt
	vbox1.pack_start ( closebutt, 0, 0, 13)
	icesetup.add (vbox1)
        self.setHelpText("WELCOME TO ICE SOUND MANAGER SETUP\n\nSetup will guide you through setting up your IceSound Server (the sound server for IceWM) as well as help you easily customize your settings in 4 easy steps. You may run this setup program at anytime by clicking 'Options', then 'Setup' on the menu.  Note: You will also see this setup window if Ice Sound Manager was unable to load your preferences files in your home directory. \n\nSTEP ONE:  Select the executable for your IceSound Server.   Click the 'Next' button to proceed.")

    def setHelpText(self, helpstuff):
      try:
       self.helptext.delete_text(0,self.helptext.get_length())
       self.helptext.delete_text(0,self.helptext.get_length()+1)
      except:
       pass
      self.helptext.insert_defaults(str(helpstuff))

def runSetup(*args):
  global isetup
  global exec_run
  global audio_run
  global wav_run
  global pref_run
  exec_run=0
  audio_run=0
  wav_run=0
  pref_run=0
  isetup=icesetup()
  isetup.icesetup.set_modal(1)
  isetup.icesetup.show_all()

def returnSetup(*args):
  global isetup
  global exec_run
  global audio_run
  global wav_run
  global pref_run
  global ism_version
  if not exec_run: 
    askExec()
    exec_run=1
    isetup.setHelpText("STEP TWO:  Configure the audio device your IceSound Server will use to play sound events.  Most people can probably use the ESD interface on localhost, port 16001.\n\nClick the 'Next' button to proceed.")
    return
  if not audio_run: 
    showAudio()
    audio_run=1
    isetup.setHelpText("STEP THREE:  Select the directory where your IceSound Server should look for the .wav files for sound events.  Normally, this directory is either ~/.icewm/sounds/ or /usr/X11R6/lib/X11/icewm/sounds/, but you should check the documentation for YOUR IceSound Server.   If you are unsure, your best bet is probably choosing the directory /home/yourUsername/.icewm/sounds/.  This may take a moment: So be patient.\n\nClick the 'Next' button to proceed.")
    return
  if not wav_run: 
    askWavDir()
    wav_run=1
    isetup.setHelpText("STEP FOUR:  This is the LAST step.  You can now configure your personal preferences for Ice Sound Manager "+ism_version+".\n\nClick the 'Next' button to proceed.")
    return
  if not pref_run: 
    showOptions()
    pref_run=1
    isetup.setHelpText("Setup for Ice Sound Manager is now COMPLETE.  Click the 'Next' button to exit setup.  Enjoy!")
    return
  isetup.icesetup.hide()
  isetup.icesetup.destroy()
  isetup=None
  return TRUE

#  MAIN methods

def setupFiles(*args):
  global audio_file
  global pref_file
  global xterm
  if os.environ.has_key("HOME"):
    audio_file=os.path.join(str(os.environ["HOME"]),".IceSoundManagerAudioRC")
    pref_file=os.path.join(str(os.environ["HOME"]),".IceSoundManagerRC")
  if os.environ.has_key("TERM"):
    if str(os.environ["TERM"]).strip():
      xterm=str(os.environ["TERM"])


def savePreferences(*args):
   global pref_file
   global ism_version
   global wav_cmd
   global wav_edit
   global use_terminal
   global copy_wavs
   global start_server
   global server_exec
   global wav_dir
   global current_theme_file
   global icey
   if pref_file:
      try:
        f=open(pref_file,"w")
        f.write("# Ice Sound Manager "+ism_version+" preferences file\n# DO NOT EDIT!\n")
        f.write("current_theme_file="+str(current_theme_file)+"\nserver_exec="+str(server_exec)+"\nstart_server="+str(start_server)+"\ncopy_wavs="+str(copy_wavs)+"\nuse_terminal="+str(use_terminal)+"\nwav_edit="+str(wav_edit)+"\nwav_cmd="+str(wav_cmd)+"\nwav_dir="+str(wav_dir))
        f.flush()
        f.close()
        showStatus("Preferences successfully saved.")
      except:
        showStatus("ERROR:  While saving preferences to '"+str(pref_file)+"'!")
   icey.entry2.set_text(getSuggestedCommandLine())

def loadPreferences(*args):
   global pref_file
   global wav_cmd
   global wav_edit
   global use_terminal
   global copy_wavs
   global start_server
   global server_exec
   global wav_dir
   global current_theme_file
   global need_setup
   global sel_file
   try:
     f=open(pref_file,"r")
     ptext=f.read()
     f.close()
     plist=ptext.split("\n")
     for i in plist:
        pline=i.strip()
        if not pline.startswith("#"):
          if pline.find("=") > -1:
            ppair=pline.split("=")
            if len(ppair) > 1:
              propname=ppair[0].strip().lower()
              propval=ppair[1].strip()
              if propval: #ignore blank values
                if propname=="server_exec": server_exec=propval
                if propname=="wav_cmd": wav_cmd=propval
                if propname=="wav_edit": wav_edit=propval
                if propname=="current_theme_file": current_theme_file=propval
                if propname=="wav_dir": wav_dir=propval
                if propname=="start_server": start_server=(propval=="1")
                if propname=="copy_wavs": copy_wavs=(propval=="1")
                if propname=="use_terminal": use_terminal=(propval=="1")
     if str(current_theme_file).find("/") > -1:  sel_file=extractDirectory(str(current_theme_file))
     if start_server:
       if not serverIsRunning():  startServer()
   except:
     need_setup=1 # Couldn't load preferences - run setup

def loadOptions(*args):
  loadAudioOptions()
  loadPreferences() 

def saveOptions(*args):
   global filelock2
   filelock2.acquire()
   try:
     saveAudioOptions()
   except:
     pass
   try:
     savePreferences()
   except:
     pass
   filelock2.release()

def startServer(*args):
  global xterm
  global server_exec
  global wav_dir
  global use_terminal
  global server_version

  if not xterm:  xterm="xterm"
  if xterm.lower().find("term") == -1: xterm="xterm" # the $TERM envir. variable was showing up as 'linux' when ISM not launched from a console

  #xterm="gnome-terminal"     # Uncomment and EDIT this line if you want to force the use of something other than your DEFAULT terminal in $TERM

  if not server_exec:       
    showStatus("ERROR: No IceSound SERVER EXECUTABLE selected!  Click 'IceSound Server' on the menu.")
    return 
  if not wav_dir:       
    showStatus("ERROR: No IceSound server WAV DIRECTORY selected!  Click 'IceSound Server' on the menu.")
    return
  if not stopServer():
    showStatus("ERROR: Your IceSound server could not be (re)started because it could not be STOPPED!")
    return
  if use_terminal:
    if xterm.lower()=="gnome-terminal": 
      f=os.popen(xterm+" -x "+getSuggestedCommandLine())
    else:  # for any other terminals that accept the -e command, i.e. xterm, etc.
      f=os.popen(xterm+" -e "+getSuggestedCommandLine())
  else:
    f=os.popen(getSuggestedCommandLine()+" &> /dev/null") 
  try:
    time.sleep(3)
    if serverIsRunning():
      showStatus("IceSound Server (version "+server_version+") successfully (RE)STARTED.")
    else:
      showStatus("ERROR!  IceSound server could NOT be (re)started!  Check 'Options' and 'IceSound Server' on the menu.")
  except:
    pass

def clearESDCache(*args):
	samp_names=[]
	f=os.popen("esdctl allinfo")
	fl=f.readlines()
	f.close()
	for ii in fl:
		if ii.find("icesound")>-1:
			if ii.find("=")< ii.find("icesound"):
				samp_names.append(ii[ii.find("=")+1:len(ii)].strip())
	for ii in samp_names:
		f=os.popen("esdctl free "+ii)
		f.readlines()

def stopServer(*args):
   global server_version
   global audio_defaults
   try:
     f=os.popen("killall -q -9 "+getServerExecShort() +" &> /dev/null")
     f=os.popen("killall -q -9 "+getServerExecShort() +" &> /dev/null")
     f=os.popen("killall -q -9 "+getServerExecShort() +" &> /dev/null")
     #if we are using the ESD interface, we need to clear the 'icesound' samples from ESD's server cache
     iface=str(audio_defaults["interface"]).lower()
     if iface=="esd":
	clearESDCache()
     s=serverIsRunning()
     if not s: showStatus("IceSound Server (version "+server_version+") successfully STOPPED.  Future sound events disabled until server re-start.")
     return s==0
   except:
     return 1

def applySoundTheme(*args):
	selectTheme() 
	startServer()

def serverIsRunning(*args):
   try:
     f=os.popen("pidof "+getServerExecShort())
     i=f.readline()
     f.close()
     if not i: return 0
     if i[0].isdigit(): 
       return 1
     else:
       return 0
   except:
     return 0
   return 0

def getServerVersion(*args):
   global server_version
   server_version="unknown"
   try:
     f=os.popen(getServerExec() + " -V")
     i=f.readline()
     f.close()
     if not i: return
     if i[0].isdigit(): 
       server_version=str(i).strip()
   except:
     pass
  
def refreshServer(*args):
   try:
     f=os.popen("killall -HUP "+getServerExecShort())
   except:
     pass

def quit (*args) :
    saveOptions()
    gtk_main_quit ()

def run () :
    global icemain
    global icey
    gtk_init ()
    try:
      gtk_set_locale ()
    except:
      pass
    w=icewindow ()
    icey=w
    icemain=w.icewindow
    w.icewindow.connect("destroy", quit)
    icemain.show_all() 
    gtk_main ()

if __name__ == "__main__" :
    t=Thread(None,run)
    t.start()