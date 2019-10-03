#############################
#  IceWM Control Panel - Common
#  
#  Copyright 2003 by Erica Andrews
#  PhrozenSmoke@yahoo.com
#  http://icesoundmanager.sourceforge.net
#  
#  This program is distributed free
#  of charge (open source) under the GNU
#  General Public License
#
#  Ported to PyGTK-2 by: David Moore (djm6202@yahoo.co.nz) - March 2003
#
#############################



# added 4/28/2003, python-based vs. static binary, for bug reporter
global IS_STATIC_BINARY
IS_STATIC_BINARY="no"

import gtk
import gettext,sys,os,gtk.gdk

TIPS=gtk.Tooltips()  # added 4/27/2003

def getBaseDir() :
    if sys.argv[0][0:sys.argv[0].rfind(os.sep)+1]=='': return "."+os.sep
    return sys.argv[0][0:sys.argv[0].rfind(os.sep)+1]

locale_dirs = [ getBaseDir()+'/locale',
                '/usr/share/locale',
                '/usr/X11R6/share/locale',
                '/usr/locale',
                '/usr/local/share/locale' ]

def getPixDir() :
    return getBaseDir()+os.sep+"pixmaps"+os.sep

def getDocDir() :
    return getBaseDir()+os.sep+"doc"+os.sep

# added 4.25.2003 - centralized method for finding locale-based sub-directories in IceWMCP directory tree
def getLocaleDir():
    try:
        mylang=os.environ['LANG']  # try $LANG variable first
        if mylang.find("_")>-1: mylang=mylang[0:mylang.find("_")]  #  es_MX  -> 'es'
        mylang=mylang.strip().lower()+os.sep   # example:  "es/"
        return mylang
    except:
        try:
            mylang=os.environ['LANGUAGE']  # try $LANGUAGE variable next
            if mylang.find("_")>-1: mylang=mylang[0:mylang.find("_")]  #  es_MX  -> 'es'
            mylang=mylang.strip().lower()+os.sep   # example:  "es/"
            return mylang
        except:
            return ""  # default to no sub-directory at all

# added 4.25.2003 - centralized method for locating the 'Help' sub-directory
def getHelpDir():
    return getBaseDir()+os.sep+"help"+os.sep

# added 4.25.2003 - centralized method for locating locale-specific 'Help' file sub-directory
def getHelpDirLocale():
    return getHelpDir()+getLocaleDir()

def loadImage(picon,windowval):
    try:
        p=gtk.Image(windowval,str(picon),None)
        p.show_all()
        return p
    except:
        return None

def loadScaledImage(picon,newheight=40,newwidth=40):  # added in version 0.3, gdkpixbup support, load png, xpm, or gif, allow scaling of images
    try:
        img = gtk.gdk.pixbuf_new_from_file(picon)
        img2 = img.scale_simple(newheight,newwidth,gtk.gdk.INTERP_BILINEAR)
        pix,mask = img2.render_pixmap_and_mask()
        icon = gtk.Image()
        icon.set_from_pixmap(pix, mask)
        icon.show()
        return icon
    except:
        return None
    


global MY_ICEWM_PATH
MY_ICEWM_PATH=None

def getIceWMPrivConfigPath():  
    # implemented in 0.3 - check environ variable, though who really uses this
    # variable?
    if os.environ.has_key("ICEWM_PRVCFG"):  ppath=os.environ['ICEWM_PRVCFG']
    else:   ppath=os.environ['HOME']+os.sep+".icewm"+os.sep
    if not ppath.endswith(os.sep): ppath=ppath+os.sep
    return ppath

def getIceWMConfigPath():  
    # new in version 0.3, search for IceWM global config path in likely 
    # locations
    global MY_ICEWM_PATH
    if not MY_ICEWM_PATH==None: return MY_ICEWM_PATH
    for iipath in ( "/usr/share/icewm/",
                    "/usr/X11R6/lib/X11/icewm/",
                    "/usr/local/lib/X11/icewm/",
                    "/etc/X11/icewm/","/etc/icewm/",
                    "/usr/local/share/icewm/",
                    "/usr/local/lib/icewm/",
                    "/usr/X11R6/share/icewm/",
                    "/usr/lib/icewm/"):
#DJM! I want to cut out all this ignore exceptions code
#DJM! if we possibly can!
#DJM!        try:
            if os.path.exists(iipath) and os.path.isdir(iipath): 
                MY_ICEWM_PATH=iipath
#DJM                print "found", iipath
                return iipath
#DJM!        except:
#DJM!            pass
    MY_ICEWM_PATH="/usr/X11R6/lib/X11/icewm/"  # we didnt find the path, use a default
    return "/usr/X11R6/lib/X11/icewm/"


## added 2.18.2003 - some suggested consolidation, also  preliminary localization support

# gettext locale support - icewmcp
icewmcp_xtext=gettext.NullTranslations()

try:
    # open icewmcp.mo in /usr/share/locale or whatever
    f_gettext=0
    for LOCALE_DIR in locale_dirs:  # open the first catalog we find, try ./locale dir first
        try:
            icewmcp_xtext=gettext.translation("icewmcp",LOCALE_DIR)
            f_gettext=1
            break
        except:
            pass
    if not f_gettext==1: raise TypeError   # couldnt find a gettext catalog, raise exception, use empty catalog
except:
    icewmcp_xtext=gettext.NullTranslations()

translateCP=icewmcp_xtext.gettext


# gettext locale support - icepref2
icewmcp_icepref_xtext=gettext.NullTranslations()

try:
    # open icewmcp-icepref.mo in /usr/share/locale or whatever
    f_gettext=0
    for LOCALE_DIR in locale_dirs:  # open the first catalog we find, try ./locale dir first
        try:
            icewmcp_icepref_xtext=gettext.translation("icewmcp-icepref",LOCALE_DIR)
            f_gettext=1
            break
        except:
            pass
    if not f_gettext==1: raise TypeError   # couldnt find a gettext catalog, raise exception, use empty catalog
except:
    icewmcp_icepref_xtext=gettext.NullTranslations()

translateP=icewmcp_icepref_xtext.gettext


# gettext locale support - Ice Sound Manager
icewmcp_ism_xtext=gettext.NullTranslations()

try:
    # open icewmcp-ism.mo in /usr/share/locale or whatever
    f_gettext=0
    for LOCALE_DIR in locale_dirs:  # open the first catalog we find, try ./locale dir first
        try:
            icewmcp_ism_xtext=gettext.translation("icewmcp-ism",LOCALE_DIR)
            f_gettext=1
            break
        except:
            pass
    if not f_gettext==1: raise TypeError   # couldnt find a gettext catalog, raise exception, use empty catalog
except:
    icewmcp_ism_xtext=gettext.NullTranslations()

translateISM=icewmcp_ism_xtext.gettext


# gettext locale support - IceMe
icewmcp_iceme_xtext=gettext.NullTranslations()

try:
    # open icewmcp-iceme.mo in /usr/share/locale or whatever
    f_gettext=0
    for LOCALE_DIR in locale_dirs:  # open the first catalog we find, try ./locale dir first
        try:
            icewmcp_iceme_xtext=gettext.translation("icewmcp-iceme",LOCALE_DIR)
            f_gettext=1
            break
        except:
            pass
    if not f_gettext==1: raise TypeError   # couldnt find a gettext catalog, raise exception, use empty catalog
except:
    icewmcp_iceme_xtext=gettext.NullTranslations()

translateME=icewmcp_iceme_xtext.gettext


# gettext locale support - HW plugin
icewmcp_hw_xtext=gettext.NullTranslations()

try:
    # open icewmcp-hw.mo in /usr/share/locale or whatever
    f_gettext=0
    for LOCALE_DIR in locale_dirs:  # open the first catalog we find, try ./locale dir first
        try:
            icewmcp_hw_xtext=gettext.translation("icewmcp-hw",LOCALE_DIR)
            f_gettext=1
            break
        except:
            pass
    if not f_gettext==1: raise TypeError   # couldnt find a gettext catalog, raise exception
except:
    icewmcp_hw_xtext=gettext.NullTranslations()

translateHW=icewmcp_hw_xtext.gettext

_=translateCP
DIALOG_TITLE=_("IceWM Control Panel")
DIALOG_OK=_("OK")
DIALOG_CANCEL=_("CANCEL")
DIALOG_CLOSE=_("Close")
FILE_RUN=_("Run")+"..."
HELP_MENU=_("/_Help")
ABOUT_MENU=_('/Help/_About...')
FILE_MENU=_('/_File')
EXIT_MENU=_('/File/_Exit')
UPDATE_MENU=_("Check for newer versions of this program...")
CONTRIBUTORS=_("Credits")
APP_HELP_STRR=_("/_Help").replace("/_","")  # added 4/25/2003 - for Help files
APP_HELP_STR=_("/_Help").replace("/_","")+"..."  # added 4/25/2003 - for Help files
BUG_REPORT_MENU=_("Send A Bug Report")

def getImage(im_file,lab_err=DIALOG_TITLE) : # GdkImlib Image loading
    try:
        myim= gtk.Image()
        print im_file
        myim.set_from_file(str(im_file).strip())
#        myim.changed_image() # disabled cached regurgitation
#        myim.render()
#        pixtemp=myim.make_pixmap()
#        return pixtemp
        return myim
    except:
        return gtk.Label(str(lab_err))



# added 4.2.2003 - common message dialogs with attractive icons, version 1.2
import IceWMCP_Dialogs
def msg_info(wintitle,message):
    IceWMCP_Dialogs.message(wintitle,message.split("\n"),(DIALOG_OK,),2,1)

def msg_warn(wintitle,message):
    IceWMCP_Dialogs.message(wintitle,message.split("\n"),(DIALOG_OK,),1,1)

def msg_err(wintitle,message):
    IceWMCP_Dialogs.message(wintitle,message.split("\n"),(DIALOG_OK,),4,1)

def msg_confirm(wintitle,message,d_ok=DIALOG_OK,d_cancel=DIALOG_CANCEL):
    ret=IceWMCP_Dialogs.message(wintitle,message.split("\n"),(d_ok,d_cancel,),3,1)
    if str(ret)==d_ok: return 1
    else: return 0


# new in version 0.3 - let user quickly check for software updates
def closeUpdateWin(*args):
    try:
        args[0].get_data("window").hide()
        args[0].get_data("window").destroy()
        args[0].get_data("window").unmap()
    except:
        pass



this_software_version="2.3-experimental-PyGtk2-no-bug-reports-accepted"     # must match the same variable in IceWMCP_BugReport.py

SOFTWARE_UPDATE_URL= "http://icesoundmanager.sourceforge.net/ICEWMCP_WEB_VERSION"


def checkSoftUpdate(*args):
    try:
        import ICEWMCP_URLRead
        up_content=ICEWMCP_URLRead.openUrl(SOFTWARE_UPDATE_URL)
        if up_content.find(",")==-1: raise TypeError  # such as "404 Not Found"
        if len(up_content.split(","))<2: raise TypeError
        new_version=up_content.split(",")[0].replace("\n","").replace("\r","").replace("\t","").strip()
        dl_url=up_content.split(",")[1].replace("\n","").replace("\r","").replace("\t","").strip()

        w=gtk.Window(gtk.WINDOW_TOPLEVEL)
        w.set_wmclass("icewmcontrolpanel","IceWMControlPanel")
        w.realize()
        w.set_title(_("IceWM Control Panel"))
        w.set_position(gtk.WIN_POS_CENTER)
        w.set_default_size(430,200)
        v=gtk.VBox(0,0)
        v.set_spacing(4)
        v.set_border_width(7)
        v.pack_start(gtk.Label(_("You are currently using version")+":   "+this_software_version+"  (IceWMCP)"),1,1,0)
        v.pack_start(gtk.Label(_("The newest version available is")+":   "+new_version+"  (IceWMCP)"),1,1,0)
        v.pack_start(gtk.Label("  "),1,1,3)
        v.pack_start(gtk.Label(_("You can download the newest version from")+": "),1,1,0)
        e=gtk.Entry()
        e.set_text(dl_url)
        v.pack_start(e,1,1,2)
        b=gtk.Button(DIALOG_OK)
        b.set_data("window",w)
        w.set_data("window",w)
        w.connect("destroy",closeUpdateWin)
        b.connect("clicked",closeUpdateWin)
        v.pack_start(b,0,0,4)
        w.add(v)
        w.set_modal(1)
        w.show_all()
    except:
        msg_err(DIALOG_TITLE,_("Could not check for software update.")+"\n"+_("This feature requires an internet connection.")) 


#  moved from IceWMCP_BugReport, 4/25/2003
# maps file menu callback numbers to real application names, used by Bug-Reported and Help
app_map={
5000:["IceWMCP.py", "IceWMCP"],
# 5001:["IceWMCPCursors.py", "IceWMCP Cursors"], # disabled, 5.5.2003...tool merged
5002:["IceWMCPKeyEdit.py", "IceWMCP KeyEdit"],
5003:["IceWMCPWallpaper.py", "IceWMCP Wallpaper"],
5004:["IceWMCPWinOptions.py", "IceWMCP WinOptions"],
5005:["icesound.py", "Ice Sound Manager"],  # The help function for IceSoundManager not used, ISM has its own help
5006:["icepref.py", "IcePref2"],
5007:["icepref_td.py", "IcePref2 TD"],
5008:["pyspool.py", "PySpool"],
5009:["IceMe.py", "IceMe"],
5010:["phrozenclock.py", "PhrozenClock"],
5011:["IceWMCP_gtk.PCCard.py", "gtk.PCCard"],
5012:["IceWMCPMouse.py", "IceWMCP Mouse"],
5013:["IceWMCPKeyboard.py", "IceWMCP Keyboard"],
5014:["IceWMCPSystem.py", "IceWMCP System"],
7777:["template.py","Help Template Test"],  # for testing purposes only
}



# added 4/26/2003...allow some windows to be closed easily by pressing 'Esc' and/or 'Return' on the keyboard
# this is used for Help and About windows, and the Run Dialog, as well as the Bug-Reporter

def keyPressClose(widget, event,*args):
    if event.keyval == gtk.keysyms.Escape:
        widget.hide()
        widget.destroy()
        widget.unmap()
    elif event.keyval == gtk.keysyms.Return:
        if widget.get_data("ignore_return")==None: # some windows shouldn't close on 'Return'
            widget.hide()
            widget.destroy()
            widget.unmap()


# added 4.4.2003 - support for sending e-mail bug reports directly from within the program
# using new ICEWMCP_BugReport module, new in version 1.2

from ICEWMCP_BugReport import *



def calculateSegment(rgb_seg):
    if not rgb_seg: return 0
    if not len(rgb_seg)==2: return 0
    alpha_num_map={"0":0,"1":1,"2":2,"3":3,"4":4,"5":5,"6":6,"7":7,"8":8,"9":9,"a":10,"b":11,"c":12,"d":13,"e":14,"f":15}
    try:
        return (alpha_num_map[str(rgb_seg[0:1])]*16)+alpha_num_map[str(rgb_seg[1:2])]
    except:
        return 0

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


# Colors and fonts needed for nicely displaying help text files, 4.25.2003
SOME_BLANK_WIN=gtk.Window()
COL_BLUE=SOME_BLANK_WIN.get_colormap().alloc_color('DodgerBlue3')
COL_PURPLE=SOME_BLANK_WIN.get_colormap().alloc_color('DarkOrchid4')
COL_GRAY=SOME_BLANK_WIN.get_colormap().alloc_color('MidnightBlue')
COL_BLACK=SOME_BLANK_WIN.get_colormap().alloc_color('black')
COL_RED=SOME_BLANK_WIN.get_colormap().alloc_color('IndianRed3')
try:
    HELP_FONT1=load_font("-*-helvetica-bold-r-normal-*-14-*") 
except:
    HELP_FONT1=None
try:
    HELP_FONT3=load_font("-*-helvetica-bold-r-normal-*-150-*") 
except:
    HELP_FONT3=None
try:
    HELP_FONT2=load_font("-*-helvetica-medium-r-normal-*-120-*") 
except:
    HELP_FONT2=None

# Display help text from text file in a somewhat 'pretty' format, my own small markup language, 4.25.2003

def renderHelp(texta,mesg):
    for ii in mesg:
        mline=ii.strip()
        if mline.find("[PHTITLE]")>-1:
            texta.insert(HELP_FONT3,COL_BLUE,None,APP_HELP_STRR+":  "+mline.replace("[PHTITLE]","").strip()+"\n\n") 
            continue
        elif mline.find("[PHSECTION]")>-1:
            texta.insert(HELP_FONT1,COL_PURPLE,None,"\n"+mline.replace("[PHSECTION]","").strip()+":\n") 
            continue
        elif mline.find("[PHERROR]")>-1:
            texta.insert(HELP_FONT1,COL_RED,None,mline.replace("[PHERROR]","").strip()+"\n") 
            continue
        elif mline.find("[PH-HI]")>-1:
            mlist=mline.split("[PH-HI]")
            for yy in mlist:
                if yy.find("[/PH-HI]")>-1: 
                    texta.insert(HELP_FONT2,COL_GRAY,None,yy[0:yy.find("[/PH-HI]")]) 
                    texta.insert(HELP_FONT2,COL_BLACK,None,yy[yy.find("[/PH-HI]")+8:len(yy)])               
                else:  
                    texta.insert(HELP_FONT2,COL_BLACK,None,yy) 
            texta.insert(HELP_FONT2,COL_BLACK,None,"\n") 
        else:
            texta.insert(HELP_FONT2,COL_BLACK,None,mline.strip()+"\n") 



# added 4.1.2003 - common 'about' dialogs with uniform look, version 1.2
# 4/26/2003 - also used for displaying Help topics (displayHelp)
def commonAbout(wintitle, mesg, with_copy=1, logo="icewmcp_short.png",
                new_line=" ", editable=0, is_help=0) :
    if is_help==1:
        with_copy=0
        logo="icewmcp_short.png"
        editable=0
        mesg=mesg.replace("\r\n","\n").replace("\r","")
        if mesg.find("[PHERROR]")==-1: 
            mesg=mesg.replace("IceWM Control Panel","IceWMCP"). \
                      replace("IceWMCP","IceWM Control Panel (IceWMCP)")
        mesg=mesg.split("\n")

    if with_copy==1: 
        abouttext = "Copyright (c) 2003 by Erica Andrews\n"+\
                    "PhrozenSmoke@yahoo.com\n"+\
                    "http://icesoundmanager.sourceforge.net\n"+\
                    "License: GNU General Public License (GPL)\n\n"
    else: 
        abouttext=""

    if not is_help==1:
        abouttext=abouttext+mesg.replace("\n\n","[ICEWMCP-BREAK]").\
                                 replace("\n",new_line).\
                                 replace("[ICEWMCP-BREAK]","\n\n")+"\n"

    aboutwin=gtk.Window(gtk.WINDOW_TOPLEVEL)
    aboutwin.set_wmclass("icewmcontrolpanel","IceWMControlPanel")
    aboutwin.realize()
    aboutwin.set_title(wintitle)
    aboutwin.set_position(gtk.WIN_POS_CENTER)
    mainvbox=gtk.VBox(0,1)
    mainvbox.set_spacing(3)
    mainvbox.set_border_width(5)
    if str(logo).find(os.sep)==-1: 
        logo=getBaseDir()+logo
    p=getImage(logo,wintitle)
    if not p: 
        p=gtk.Label(wintitle)
    mainvbox.pack_start(p,0,0,0)
    if is_help==1: 
        helpbox=gtk.HBox(0,0)
        helpbox.set_spacing(4)
        helpbox.pack_start(gtk.Label(" "),1,1,0)
        helpbox.pack_start(getImage(getBaseDir()+"icewmcp_help.png",""),0,0,0)
        helpbox.pack_start(gtk.Label(APP_HELP_STRR.upper()),0,0,0)
        helpbox.pack_start(gtk.Label(" "),1,1,0)
        mainvbox.pack_start(helpbox,0,0,2)
    mainvbox.pack_start(gtk.HSeparator(),0,0,4)
    sc=gtk.ScrolledWindow()
    sctext=gtk.TextView()
    sctext.set_editable(editable)
    sctext.set_wrap_mode(gtk.WRAP_WORD)
    if is_help==1: 
        renderHelp(sctext,mesg)
    else: 
        sctext.get_buffer().set_text(abouttext)
    sc.add(sctext)
    mainvbox.pack_start(sc,1,1,0)
    if is_help==1: 
        b=gtk.Button(DIALOG_CLOSE)
        TIPS.set_tip(b,DIALOG_CLOSE)
    else: 
        b=gtk.Button(DIALOG_OK)
        TIPS.set_tip(b,DIALOG_OK)
    b.set_data("window",aboutwin)
    b.connect("clicked",closeUpdateWin)
    mainvbox.pack_start(b,0,0,4)    
    aboutwin.add(mainvbox)
    if not is_help==1: aboutwin.set_modal(1)
    aboutwin.connect("key-press-event",keyPressClose)
    if is_help==1: aboutwin.set_default_size(365,400)
    else: aboutwin.set_default_size(365,290)
    #aboutwin.set_default_size(375,375)
    aboutwin.show_all()

def displayHelp(appnum=7777,*args):
    try:
        htop=app_map[appnum][1]
        fname=app_map[appnum][0].replace(".py","")+".txt"
    except:
        htop="Not Found"
        fname=None
    wtitle=APP_HELP_STRR+":  "+htop
    mesg=""
    if fname==None:
        mesg="[PHERROR]"+_("No such help topic")+" :  "+str(appnum)
    else:
        try:
            mesg=open(getHelpDirLocale()+fname).read()  # try to read locale-specific help first
            if len(mesg)==0: raise TypeError
        except:
            try:  # locale-specific help failed, try standard English
                mesg=open(getHelpDir()+fname).read()
            except:
                mesg="[PHERROR]"+_("Could not open Help file")+" :  "+str(fname)
    if fname=="icesound.txt": # special case for IceSoundManager
        if mesg.find("[PHSECTION]")>-1:
            mesg=mesg[0:mesg.find("[PHSECTION]")]+translateISM("SPANISH HELP IS AVAILABLE IN THE 'icesound-es-help.html'  FILE.")+" ("+"./doc/"+")\n\n"+mesg[mesg.find("[PHSECTION]"):len(mesg)]
    commonAbout(wtitle,mesg,0,"icewmcp_short.png","",0,1)
    

# added 4.2.2003 - translators/contributors credits dialog
def show_credits(*args):
    commonAbout(CONTRIBUTORS,"PyGtk-2 Port:\n"+"David Moore (djm ['at'] e3.net.nz)"+"\n\n"+_("Translators").upper()+":\n\nRussian: Vasya Leushin <basileus@newmail.ru>\n\nSpanish: Erica Andrews <PhrozenSmoke@yahoo.com>\n\nSeveral Spanish Corrections: 'Martintxo' <fundamento@sindominio.net>"+"\n\n_____________\n"+_("If you wish to help us with the translations, please visit the following site on the web")+":\n\nhttp://icesoundmanager.sourceforge.net/translator.php\n\nA big THANKS to all who have contributed."    ,0,"icewmcp_short.png","\n")

import IceWMCPRun    # THis should always be the last function defined in this module
def rundlg(*args):  # new in versin 1.1, global access to a 'Run...' dialog 
    IceWMCPRun.runwindow()


# SPLASH SCREEN CODE
## added 4/27/2003 - common code for splash screens - used by IceMe, IcePref2, and IcePref2-TD

global SPLASH_WIN
global SPLASH_METHOD
global SPLASH_TEXT
global SPLASH_LOGO
SPLASH_WIN=None     # reference to the gtk.Window
SPLASH_METHOD=None      # method to run while Splash is showing
SPLASH_TEXT=None    # text to display in Splash win
SPLASH_LOGO=getBaseDir()+"icewmcp_short.png"    # a default Splash logo

def setSplash(splash_meth,splash_text=translateCP("Loading")+"..." ,splash_logo=getBaseDir()+"icewmcp_short.png"):
    global SPLASH_METHOD
    global SPLASH_TEXT
    global SPLASH_LOGO
    SPLASH_METHOD=splash_meth  # note: SPLASH_METHOD must make a call to 'hideSplash' when it is done
    SPLASH_TEXT=splash_text
    if not splash_logo==None: SPLASH_LOGO=splash_logo

def hideSplash(*args):
    # close the small Splash screen after loading is complete
    global SPLASH_WIN
    try:
        SPLASH_WIN.hide()
        SPLASH_WIN.destroy()
        SPLASH_WIN.unmap()
    except:
        pass
    return 0

#  This is a hidden/private method, and should not be called directly
def __initSplash(*args):
    global SPLASH_WIN
    global SPLASH_METHOD
    global SPLASH_TEXT
    global SPLASH_LOGO
    # added 4/25/2003 - Erica Andrews, new prettier start-up process, more informative - small Splash screen
    SPLASH_WIN=gtk.Window(gtk.WINDOW_POPUP)
    SPLASH_WIN.set_position(gtk.WIN_POS_CENTER)
    hb=gtk.VBox(0,0)
    hb.set_border_width(10)
    hb.set_spacing(6)
    hb.pack_start(gtk.HSeparator(),1,1,0)
    hb.pack_start(getImage(SPLASH_LOGO ,"IceWMCP") ,1,1,3)
    hb.pack_start(gtk.Label(SPLASH_TEXT) ,1,1,6)
    hb.pack_start(gtk.HSeparator(),1,1,0)
    SPLASH_WIN.add(hb)
    SPLASH_WIN.show_all()
    # added 4/25/2003 - Erica Andrews, call main initiation methods, SPLASH_METHOD
    gtk.timeout_add(50,SPLASH_METHOD)
    return 0

def showSplash(with_time=1,*args):
    if with_time==1: gtk.timeout_add(15,__initSplash)
    else: __initSplash()

#  variable to disable splash window in some apps (i.e. IceWMCPCursors), 4/29/2003
#  import icewmcp_common
#  icewmcp_common.NOSPLASH=1

global NOSPLASH
NOSPLASH=0





