#!/usr/bin/env python

###################################################
#                IcePref 2.9
#
# This is (or will be) the best IceWM configuration utility
# known to man.  It requires a recent version of python as well as gtk. ( >
# 1.2.0) and Pygtk..  It should work well with versions of icewm around
# 0.9.54. You may place it anywhere in your path. You are free to copy,
# change, and distribute this program under the terms of the GNU GPL.  You
# can obtain the latest version of this script from via http at
# http://members.xoom.com/SaintChoj/icepref.html.  Please report all bugs
# to Dave at SLQTN@cc.usu.edu.  Please include IcePref in the subject line.
#
# Copyright 1999, David Mortensen
#
# This work comes with no warranty regarding its suitability for any purpose.
# The author is not responsible for any damages caused by the use of this
# program.
#
####################################################
##########################################################
#     This is an experimental IceWM Theme designer based on IcePref2 version 2.9
#    IcePref2-TD version 2.1
#
#     Updates by: Erica Andrews (PhrozenSmoke@yahoo.com) - April 2003
#     Copyright (c) 2003  Erica Andrews
#     This work comes with no warranty regarding its suitability for any purpose.
#     The author is not responsible for any damages caused by the use of this
#     program.
#
# Ported to PyGTK-2 by: David Moore (djm6202@yahoo.co.nz) - June 2003
#
##########################################################

import gtk
import pango
from string import *

import glob
import re
import sys
import os
import commands
import math
import icepref_search

#set translation support
from icewmcp_common import *
_=translateP   # from icewmcp_common.py



## new in version 2.8 - 4/27/2003, IcePref2-TD can take a moment to
## start...show a nice splash screen while we load

def donada(*args):  # does nothing, just a 'dummy' method for the Splash screen, app=Application() does all the real work
    pass

# import drag-n-drop support, 5.16.2003
import icewmcp_dnd
from icewmcp_dnd import *
initColorDrag()    # enable dnd support for 'color buttons'
addDragSupportColor=icewmcp_dnd.addDragSupportColor


# added 4/27/2003 - Erica Andrews, new prettier start-up process, more informative with Splash screen
# code comes from 'icewmcp_common.py'

if NOSPLASH==0:
    setSplash(donada, "  IcePref2-TD:  "+_("Loading...please wait.")+"  ",getBaseDir()+"icepref2.png")
    showSplash(0)
    while gtk.events_pending():
        gtk.mainiteration()


#######################################
# Constants in a Changing World
#
#######################################

VERSION = this_software_version
ICE_VERSION = "1.2.8pre2"


# these define the types of configuration widgets

TOGGLE = 0    #done
RANGE = 1     #done
FILE = 2    #done
PATH = 3    #done
COLOR = 4     #done
FONT = 5    #done
ENTRY = 6     #done
KEYSTROKE = 7   #done
MULTI = 8     #done
THEME = 9     #done
BITMASK = 10  #done
MOUSEBUTTON = 11 #done
IMAGE_WIDGET=12
OPTIONMENU=13

# these define the indexes for the data in DEFAULTS and self.settings

TYPE = 0
VALUE = 1
TITLE = 2
MIN = 3
MAX = 4
NUM = 3

# the standard spacing and border width, respectively

SP = 5
BD = 5

# some environmental variables -- more than are needed, really.

if os.environ.has_key('OSTYPE'):
  OSTYPE   = os.environ['OSTYPE']
else:
  OSTYPE   = os.uname()[0]

if os.environ.has_key('MACHTYPE'):
  MACHTYPE = os.environ['MACHTYPE']
else:
  MACHTYPE = 'i386-pc-linux-gnu'

if os.environ.has_key('HOME'):
  HOME   = os.environ['HOME']
else:
  import user
  HOME   = user.home

if os.environ.has_key('USER'):
  USER   = os.environ['USER']
else:
  USER = os.uname()[1]
  
if os.environ.has_key('PATH'):
  PATH   = os.environ['PATH']
else:
  PATH = ''

# finally, the file we're editing
global CONFIG_FILE
global CONFIG_THEME_PATH
global THEME_PATH
CONFIG_FILE = ''
CONFIG_THEME_PATH= ''

# and the paths to all your themes
# (If the path to your themes is not here, simply add it.  Just follow the
# example of the other paths!)

THEME_PATH = [  CONFIG_THEME_PATH ]
            
SAMPLE_TEXT = 'IcePref2 Theme Designer (http://icesoundmanager.sourceforge.net): AaBbCcDdEeFfGgHhIiJjKkLlMmNnOoPpQqRrSsTtUuVvWwXxYyZz1234567890'
ERROR_TEXT = 'Invalid Font'

## added 1.27.2003 - variables that require image previewers after them
REQUIRE_PREVIEW=['Theme','DesktopBackgroundImage']



DEFAULTS = {  

'Look':[OPTIONMENU,'gtk','Look'],
        # Images, will be ignored when saving theme file
'titleIR.xpm':[IMAGE_WIDGET,'','titleIR.xpm'],
'titleIL.xpm':[IMAGE_WIDGET,'','titleIL.xpm'],
'closeI.xpm':[IMAGE_WIDGET,'','closeI.xpm'],
'titleAR.xpm':[IMAGE_WIDGET,'','titleAR.xpm'],
'maximizeI.xpm':[IMAGE_WIDGET,'','maximizeI.xpm'],
'menuButtonA.xpm':[IMAGE_WIDGET,'','menuButtonA.xpm'],
'menuButtonI.xpm':[IMAGE_WIDGET,'','menuButtonI.xpm'],
'restoreI.xpm':[IMAGE_WIDGET,'','restoreI.xpm'],
'minimizeI.xpm':[IMAGE_WIDGET,'','minimizeI.xpm'],
'minimizeA.xpm':[IMAGE_WIDGET,'','minimizeA.xpm'],
'switchbg.xpm':[IMAGE_WIDGET,'','switchbg.xpm'],
'titleAB.xpm':[IMAGE_WIDGET,'','titleAB.xpm'],
'restoreA.xpm':[IMAGE_WIDGET,'','restoreA.xpm'],
'titleAT.xpm':[IMAGE_WIDGET,'','titleAT.xpm'],
'titleAS.xpm':[IMAGE_WIDGET,'','titleAS.xpm'],
'titleIB.xpm':[IMAGE_WIDGET,'','titleIB.xpm'],
'closeA.xpm':[IMAGE_WIDGET,'','closeA.xpm'],
'maximizeAb.xpm':[IMAGE_WIDGET,'','maximizeAb.xpm'],
'maximizeIb.xpm':[IMAGE_WIDGET,'','maximizeIb.xpm'],
'titleIT.xpm':[IMAGE_WIDGET,'','titleIT.xpm'],
'titleIS.xpm':[IMAGE_WIDGET,'','titleIS.xpm'],
'menubg.xpm':[IMAGE_WIDGET,'','menubg.xpm'],
'logoutbg.xpm':[IMAGE_WIDGET,'','logoutbg.xpm'],
'dialogbg.xpm':[IMAGE_WIDGET,'','dialogbg.xpm'],
'maximizeA.xpm':[IMAGE_WIDGET,'','maximizeA.xpm'],
'titleAL.xpm':[IMAGE_WIDGET,'','titleAL.xpm'],
'ledclock/a.xpm':[IMAGE_WIDGET,'','a.xpm'],
'ledclock/colon.xpm':[IMAGE_WIDGET,'','colon.xpm'],
'ledclock/dot.xpm':[IMAGE_WIDGET,'','dot.xpm'],
'ledclock/m.xpm':[IMAGE_WIDGET,'','m.xpm'],
'ledclock/n0.xpm':[IMAGE_WIDGET,'','n0.xpm'],
'ledclock/n1.xpm':[IMAGE_WIDGET,'','n1.xpm'],
'ledclock/n2.xpm':[IMAGE_WIDGET,'','n2.xpm'],
'ledclock/n3.xpm':[IMAGE_WIDGET,'','n3.xpm'],
'ledclock/n4.xpm':[IMAGE_WIDGET,'','n4.xpm'],
'ledclock/n5.xpm':[IMAGE_WIDGET,'','n5.xpm'],
'ledclock/n6.xpm':[IMAGE_WIDGET,'','n6.xpm'],
'ledclock/n7.xpm':[IMAGE_WIDGET,'','n7.xpm'],
'ledclock/n8.xpm':[IMAGE_WIDGET,'','n8.xpm'],
'ledclock/n9.xpm':[IMAGE_WIDGET,'','n9.xpm'],
'ledclock/p.xpm':[IMAGE_WIDGET,'','p.xpm'],
'ledclock/slash.xpm':[IMAGE_WIDGET,'','slash.xpm'],
'ledclock/space.xpm':[IMAGE_WIDGET,'','space.xpm'],
'taskbar/windows.xpm':[IMAGE_WIDGET,'','windows.xpm'],
'taskbar/taskbuttonbg.xpm':[IMAGE_WIDGET,'','taskbuttonbg.xpm'],
'taskbar/taskbuttonminimized.xpm':[IMAGE_WIDGET,'','taskbuttonminimized.xpm'],
'taskbar/taskbuttonactive.xpm':[IMAGE_WIDGET,'','taskbuttonactive.xpm'],
'taskbar/taskbarbg.xpm':[IMAGE_WIDGET,'','taskbarbg.xpm'],
'taskbar/workspacebuttonactive.xpm':[IMAGE_WIDGET,'','workspacebuttonactive.xpm'],
'taskbar/workspacebuttonbg.xpm':[IMAGE_WIDGET,'','workspacebuttonbg.xpm'],
'taskbar/linux.xpm':[IMAGE_WIDGET,'','linux.xpm'],
'taskbar/toolbuttonbg.xpm':[IMAGE_WIDGET,'','toolbuttonbg.xpm'],
'taskbar/menubg.xpm':[IMAGE_WIDGET,'','menubg.xpm'],
'mailbox/errmail.xpm':[IMAGE_WIDGET,'','errmail.xpm'],
'mailbox/mail.xpm':[IMAGE_WIDGET,'','mail.xpm'],
'mailbox/newmail.xpm':[IMAGE_WIDGET,'','newmail.xpm'],
'mailbox/nomail.xpm':[IMAGE_WIDGET,'','nomail.xpm'],
'mailbox/unreadmail.xpm':[IMAGE_WIDGET,'','unreadmail.xpm'],
'cursors/left.xpm':[IMAGE_WIDGET,'','left.xpm'],
'cursors/move.xpm':[IMAGE_WIDGET,'','move.xpm'],
'cursors/right.xpm':[IMAGE_WIDGET,'','right.xpm'],
'cursors/sizeB.xpm':[IMAGE_WIDGET,'','sizeB.xpm'],
'cursors/sizeBL.xpm':[IMAGE_WIDGET,'','sizeBL.xpm'],
'cursors/sizeBR.xpm':[IMAGE_WIDGET,'','sizeBR.xpm'],
'cursors/sizeL.xpm':[IMAGE_WIDGET,'','sizeL.xpm'],
'cursors/sizeR.xpm':[IMAGE_WIDGET,'','sizeR.xpm'],
'cursors/sizeT.xpm':[IMAGE_WIDGET,'','sizeT.xpm'],
'cursors/sizeTL.xpm':[IMAGE_WIDGET,'','sizeTL.xpm'],
'cursors/sizeTR.xpm':[IMAGE_WIDGET,'','sizeTR.xpm'],
'icons/folder_32x32.xpm':[IMAGE_WIDGET,'','folder_32x32.xpm'],
'icons/folder_16x16.xpm':[IMAGE_WIDGET,'','folder_16x16.xpm'],
'icons/xterm_32x32.xpm':[IMAGE_WIDGET,'','xterm_32x32.xpm'],
'icons/xterm_16x16.xpm':[IMAGE_WIDGET,'','xterm_16x16.xpm'],
'icons/app_32x32.xpm':[IMAGE_WIDGET,'','app_32x32.xpm'],
'icons/app_16x16.xpm':[IMAGE_WIDGET,'','app_16x16.xpm'],

'icons/netscape_32x32.xpm':[IMAGE_WIDGET,'','netscape_32x32.xpm'],
'icons/netscape_16x16.xpm':[IMAGE_WIDGET,'','netscape_16x16.xpm'],
'icons/opera_32x32.xpm':[IMAGE_WIDGET,'','opera_32x32.xpm'],
'icons/opera_16x16.xpm':[IMAGE_WIDGET,'','opera_16x16.xpm'],
'icons/mozilla_32x32.xpm':[IMAGE_WIDGET,'','mozilla_32x32.xpm'],
'icons/mozilla_16x16.xpm':[IMAGE_WIDGET,'','mozilla_16x16.xpm'],
'icons/galeon_32x32.xpm':[IMAGE_WIDGET,'','galeon_32x32.xpm'],
'icons/galeon_16x16.xpm':[IMAGE_WIDGET,'','galeon_16x16.xpm'],
'icons/terminal_32x32.xpm':[IMAGE_WIDGET,'','terminal_32x32.xpm'],
'icons/terminal_16x16.xpm':[IMAGE_WIDGET,'','terminal_16x16.xpm'],
'icons/email_32x32.xpm':[IMAGE_WIDGET,'','email_32x32.xpm'],
'icons/email_16x16.xpm':[IMAGE_WIDGET,'','email_16x16.xpm'],
'icons/gimp_32x32.xpm':[IMAGE_WIDGET,'','gimp_32x32.xpm'],
'icons/gimp_16x16.xpm':[IMAGE_WIDGET,'','gimp_16x16.xpm'],
'icons/vim_32x32.xpm':[IMAGE_WIDGET,'','vim_32x32.xpm'],
'icons/vim_16x16.xpm':[IMAGE_WIDGET,'','vim_16x16.xpm'],
'icons/emacs_32x32.xpm':[IMAGE_WIDGET,'','emacs_32x32.xpm'],
'icons/emacs_16x16.xpm':[IMAGE_WIDGET,'','emacs_16x16.xpm'],



        # new WINDOW stuff - added 3.14.2003 - PhrozenSmoke
        'ShapesProtectClientWindow': [TOGGLE, '1', 'Shaped pixmaps protect client window'],
        # New TASKBAR STUFF - added 1/25/2003 by PhrozenSmoke
        'TaskBarShowTray': [TOGGLE, '1', 'Show task tray on task bar'],
        'TrayDrawBevel': [TOGGLE, '0', 'Task tray has a bevel around it (Like the Windows system tray)'],

        # New QUICKSWITCH STUFF - added 1/25/2003 by PhrozenSmoke
        'QuickSwitchFillSelection': [TOGGLE, '0', 'Fill highlight rectangle'],
        'QuickSwitchTextFirst': [TOGGLE, '0', 'Show text above icons'],
        'QuickSwitchIconBorder': [RANGE, '2', 'Border size of quick-switch icons', 0, 30],
        'QuickSwitchIconMargin': [RANGE, '4', 'Size of icon margins of quick-switch icons', 0, 30],
        'QuickSwitchHorzMargin': [RANGE, '3', 'Horizontal margin of quick-switch window', 0, 30],
        'QuickSwitchVertMargin': [RANGE, '3', 'Vertical margin of quick-switch window', 0, 30],
        'QuickSwitchSeparatorSize': [RANGE, '6', 'Size of the quick-switch window separator', 0, 30],
        'ColorQuickSwitchActive': [COLOR, '"rgb:00/EE/FF"', 'Color of quick-swith highlight rectangle'],

        'DesktopBackgroundCenter': [TOGGLE, '0', 'Display desktop wallpaper centered, instead of tiled.'],



        # NEW Stuff - added 1.26.2003 - scrollbars
        'ScrollBarX': [RANGE, '16', 'Horizontal scrollbar button size', 0, 100],
        'ScrollBarY': [RANGE, '16', 'Vertical scrollbar button size', 0, 100],
        'ColorScrollBarButton': [COLOR, '"rgb:C0/C0/C0"', 'Scrollbar button color'],
        'ColorScrollBarButtonArrow': [COLOR, '"rgb:00/00/00"', 'Scrollbar button arrow color'],
        'ColorScrollBarInactiveArrow': [COLOR, '"rgb:80/80/80"', 'Inactive scrollbar button arrow color'],


        # NEW Stuff - added 1.26.2003 - desktop
        'DesktopTransparencyColor':[COLOR, '"rgb:FF/FF/FF"', 'Color to use for transparent applications'],
        'DesktopTransparencyImage': [IMAGE_WIDGET, '""', 'Image to use for transparent applications'],


        # NEW Stuff - added 1.26.2003 - title bars
        'TitleBarJoinRight':[TOGGLE, '0', 'Title bar join right'],
        'TitleBarJoinLeft':[TOGGLE, '0', 'Title bar join left'],
        'TitleBarJustify': [RANGE, '0', 'Title bar text position', 0, 100],
        'TitleBarHorzOffset': [RANGE, '0', 'Title bar text horizontal offset', -128, 128],
        'TitleBarVertOffset': [RANGE, '0', 'Title bar text vertical offset', -128, 128],
        'ColorActiveTitleBarShadow': [COLOR, '"rgb:00/00/00"', 'Active title bar shadow'],
        'ColorNormalTitleBarShadow': [COLOR, '"rgb:00/00/00"', 'Inactive title bar shadow'],



        # NEW MENU Stuff - added 1.26.2003
        'ColorDisabledMenuItemShadow':[COLOR, '"rgb:00/EE/FF"', 'Color of disabled menu items'],
        'MenuMaximalWidth':[RANGE, '0', 'Maximum menu width (0 to let IceWM decide)', 0, 20000],

        'TaskBarClockLeds': [TOGGLE, '1', 'Task bar clock uses nice pixmapped LCD display'],
        'TaskBarDoubleHeight': [TOGGLE, '1', 'Use double height task bar (highly recommended)'],

        'BorderSizeX': [RANGE, '6', 'Horizontal window border', 0, 128],
        'BorderSizeY': [RANGE, '6', 'Veritical window border', 0, 128],
        'DlgBorderSizeX': [RANGE, '2', 'Horizontal dialog window border', 0, 128],
        'DlgBorderSizeY': [RANGE, '2', 'Vertical dialog window border', 0, 128],

        'TitleBarHeight': [RANGE, '20', 'Title bar height', 0, 128],
        'CornerSizeX': [RANGE, '24', 'Resize corner width', 0, 64],
        'CornerSizeY': [RANGE, '24', 'Resize corner height', 0, 64],

        'TaskBarCPUSamples': [RANGE, '20', 'Width of CPU Monitor', 2, 1000,],
        'TitleButtonsLeft': [ENTRY, '"s"', 'Titlebar buttons from left to right (x=close, m=max, i=min, h=hide, r=rollup, s=sysmenu)'],
        'TitleButtonsRight': [ENTRY, '"xmir"', 'Titlebar buttons from right to left (x=close, m=max, i=min, h=hide, r=rollup, s=sysmenu)'],
        'TitleButtonsSupported' : [ENTRY, '"xmis"', 'Titlebar buttons supported by theme (x, m, i, r, h, s, d)'],

        'Theme': [THEME, '"mytheme/default.theme"', 'IceWM Themes'],
        'ThemeAuthor': [ENTRY, '""', 'Theme author'],
        'ThemeDescription': [ENTRY, '""', 'Theme description'],

        # added 1.26.2003
        'Gradients' : [FILE, '""', 'List of gradient images in the current theme'],

        'TitleFontName': [FONT, '"-adobe-helvetica-bold-r-*-*-*-120-*-*-*-*-*-*"', 'Title bar font'],
        'MenuFontName': [FONT, '"-adobe-helvetica-bold-r-*-*-*-120-*-*-*-*-*-*"', 'Menu font'],
        'StatusFontName': [FONT, '"-adobe-helvetica-bold-r-*-*-*-120-*-*-*-*-*-*"', 'Status font'],
        'QuickSwitchFontName': [FONT, '"-adobe-helvetica-bold-r-*-*-*-120-*-*-*-*-*-*"', 'Quick switch window font'],
        'NormalButtonFontName': [FONT, '"-adobe-helvetica-medium-r-*-*-*-120-*-*-*-*-*-*"', 'Normal button font'],
        'ActiveButtonFontName': [FONT, '"-adobe-helvetica-bold-r-*-*-*-120-*-*-*-*-*-*"', 'Active button font'],
        'NormalTaskBarFontName': [FONT, '"-adobe-helvetica-medium-r-*-*-*-120-*-*-*-*-*-*"', 'Normal taskbar font'],
        'ActiveTaskBarFontName': [FONT, '"-adobe-helvetica-bold-r-*-*-*-120-*-*-*-*-*-*"', 'Active taskbar font'],
        'MinimizedWindowFontName': [FONT, '"-adobe-helvetica-medium-r-*-*-*-120-*-*-*-*-*-*"', 'Minimized window font'],
        'ListBoxFontName': [FONT, '"-adobe-helvetica-medium-r-*-*-*-120-*-*-*-*-*-*"', 'List box font (Window list box)'],
        'ToolTipFontName': [FONT, '"-adobe-helvetica-medium-r-*-*-*-120-*-*-*-*-*-*"', 'Tooltip font'],
        'ClockFontName': [FONT, '"-adobe-helvetica-medium-r-*-*-*-140-*-*-*-*-*-*"', 'Clock font'],
        'ApmFontName': [FONT, '"-adobe-helvetica-medium-r-*-*-*-140-*-*-*-*-*-*"', 'APM font'],
        'LabelFontName': [FONT, '"-adobe-helvetica-medium-r-*-*-*-140-*-*-*-*-*-*"', 'Dialog Label font'],

        # NEW FONT STUFF added 1.25.2003
        'FxFontName': [FONT, '"-adobe-courier-bold-r-*-*-*-120-*-*-*-*-*-*"', 'FX Font font'],
        'InputFontName': [FONT, '"-adobe-courier-bold-r-*-*-*-140-*-*-*-*-*-*"', 'Address bar font'],
        'ToolButtonFontName': [FONT,'"-adobe-courier-bold-r-*-*-*-120-*-*-*-*-*-*"', 'Tool Button font'],
        'ActiveWorkspaceFontName': [FONT,'"-adobe-courier-bold-r-*-*-*-120-*-*-*-*-*-*"', 'Active workspace  font'],
        'NormalWorkspaceFontName': [FONT,'"-adobe-courier-bold-r-*-*-*-120-*-*-*-*-*-*"', 'Normal workspace  font'],

        # added 1.26.2003 - toolbar button colors
        'ColorToolButton': [COLOR, '"rgb:C0/C0/C0"', 'Toolbar button color'],
        'ColorToolButtonText': [COLOR, '"rgb:00/00/00"', 'Toolbar button text color'],

        'ColorDialog': [COLOR, '"rgb:C0/C0/C0"', 'Dialog box color'],
        'ColorActiveBorder': [COLOR, '"rgb:C0/C0/C0"', 'Active border color'],
        'ColorNormalBorder': [COLOR,'"rgb:C0/C0/C0"', 'Normal border color'],
        'ColorNormalTitleButton': [COLOR, '"rgb:C0/C0/C0"', 'Normal title button color'],
        'ColorNormalTitleButtonText': [COLOR, '"rgb:00/00/00"', 'Normal title button text color'],
        'ColorNormalButton': [COLOR, '"rgb:C0/C0/C0"', 'Normal button color'],
        'ColorNormalButtonText': [COLOR, '"rgb:00/00/00"', 'Normal button text color'],
        'ColorActiveButton': [COLOR, '"rgb:E0/E0/E0"', 'Active button color'],
        'ColorActiveButtonText': [COLOR, '"rgb:00/00/00"', 'Active button text color'],
        'ColorActiveTitleBar': [COLOR, '"rgb:00/00/A0"', 'Active title bar color'],
        'ColorNormalTitleBar': [COLOR, '"rgb:80/80/80"', 'Normal title bar color'],
        'ColorActiveTitleBarText': [COLOR, '"rgb:FF/FF/FF"', 'Active title bar text color'],
        'ColorNormalTitleBarText': [COLOR, '"rgb:00/00/00"', 'Normal title bar text color'],
        'ColorNormalMinimizedWindow': [COLOR, '"rgb:C0/C0/C0"', 'Normal minimized window color'],
        'ColorNormalMinimizedWindowText': [COLOR, '"rgb:00/00/00"', 'Normal minimized window text color'],
        'ColorActiveMinimizedWindow': [COLOR, '"rgb:E0/E0/E0"', 'Active minimized window color'],
        'ColorActiveMinimizedWindowText': [COLOR, '"rgb:00/00/00"', 'Active minimized window text color'],
        'ColorNormalMenu': [COLOR, '"rgb:C0/C0/C0"', 'Normal menu color'],
        'ColorActiveMenuItem': [COLOR, '"rgb:A0/A0/A0"', 'Active menu item color'],
        'ColorActiveMenuItemText': [COLOR, '"rgb:00/00/00"', 'Active menu item text color'],
        'ColorNormalMenuItemText': [COLOR, '"rgb:00/00/00"', 'Normal menu item text color'],
        'ColorDisabledMenuItemText': [COLOR, '"rgb:80/80/80"', 'Disabled menu item text color'],
        'ColorMoveSizeStatus': [COLOR, '"rgb:C0/C0/C0"', 'Move/size status color'],
        'ColorMoveSizeStatusText': [COLOR, '"rgb:00/00/00"', 'Move/size status text color'],
        'ColorQuickSwitch': [COLOR, '"rgb:C0/C0/C0"', 'Quick switch window color'],
        'ColorQuickSwitchText': [COLOR, '"rgb:00/00/00"', 'Quick switch text color'],
        'ColorDefaultTaskBar': [COLOR, '"rgb:C0/C0/C0"', 'Default taskbar color'],
        'ColorNormalTaskBarApp': [COLOR, '"rgb:C0/C0/C0"', 'Normal taskbar color'],
        'ColorNormalTaskBarAppText': [COLOR, '"rgb:00/00/00"', 'Normal taskbar app text color'],
        'ColorActiveTaskBarApp': [COLOR, '"rgb:E0/E0/E0"', 'Active taskbar app color'],
        'ColorActiveTaskBarAppText': [COLOR, '"rgb:00/00/00"', 'Active taskbar app text color'],
        'ColorMinimizedTaskBarApp': [COLOR, '"rgb:A0/A0/A0"', 'Minimized taskbar app color'],
        'ColorMinimizedTaskBarAppText': [COLOR, '"rgb:00/00/00"', 'Minimized taskbar app text'],
        'ColorInvisibleTaskBarApp': [COLOR, '"rgb:80/80/80"', 'Invisible taskbar app color'],
        'ColorInvisibleTaskBarAppText': [COLOR, '"rgb:00/00/00"', 'Invisible taskbar app text'],
        'ColorScrollBar': [COLOR, '"rgb:A0/A0/A0"', 'Scroll bar color'],
        'ColorScrollBarArrow': [COLOR, '"rgb:C0/C0/C0"', 'Scroll bar arrow color'],
        'ColorScrollBarSlider': [COLOR, '"rgb:C0/C0/C0"', 'Scroll bar slider color'],
        'ColorListBox': [COLOR, '"rgb:C0/C0/C0"', 'List box color (Window list)'],
        'ColorListBoxText': [COLOR, '"rgb:00/00/00"', 'List box text color (Window list)'],
        'ColorListBoxSelection': [COLOR, '"rgb:80/80/80"', 'List box selection color (Window list)'],
        'ColorListBoxSelectionText': [COLOR, '"rgb:00/00/00"', 'List box selection text color (Window list)'],
        'ColorToolTip': [COLOR, '"rgb:E0/E0/00"', 'Tooltip color'],
        'ColorToolTipText': [COLOR, '"rgb:00/00/00"', 'Tooltip text color'],
        'ColorClock': [COLOR, '"rgb:00/00/00"', 'Clock color'],
        'ColorClockText': [COLOR, '"rgb:00/FF/00"', 'Clock text color'],
        'ColorApm': [COLOR, '"rgb:00/00/00"', 'APM color'],
        'ColorApmText': [COLOR, '"rgb:00/FF/00"', 'APM text color'],
        'ColorLabel': [COLOR, '"rgb:C0/C0/C0"', 'Dialog Label color'],
        'ColorLabelText': [COLOR, '"rgb:00/00/00"', 'Dialog Label text color'],
        'ColorInput': [COLOR, '"rgb:FF/FF/FF"', 'Input color (Address bar)'],
        'ColorInputText': [COLOR, '"rgb:00/00/00"', 'Input text color (Address bar)'],
        'ColorInputSelection': [COLOR, '"rgb:80/80/80"', 'Input selection color (Address bar)'],
        'ColorInputSelectionText': [COLOR, '"rgb:00/00/00"', 'Input selection text color (Address bar)'],
        'DesktopBackgroundColor': [COLOR, '"rgb:00/20/40"', 'Desktop background color'],
        'DesktopBackgroundImage': [FILE, '"background.png"', 'Desktop wallpaper image'],
        'ColorCPUStatusUser': [COLOR, '"rgb:00/FF/00"', 'User CPU usage color'],
        'ColorCPUStatusSystem': [COLOR, '"rgb:FF/00/00"', 'System CPU usage color'],
        'ColorCPUStatusNice': [COLOR, '"rgb:00/00/FF"', 'Nice CPU usage color'],
        'ColorCPUStatusIdle': [COLOR, '"rgb:00/00/00"', 'CPU Idle color'],
        'ColorNetSend': [COLOR, '"rgb:FF/FF/00"', 'Color of sent data on net monitor'],
        'ColorNetReceive': [COLOR, '"rgb:FF/00/FF"', 'Color of received data on net monitor'],
        'ColorNetIdle': [COLOR, '"rgb:00/00/00"', 'Color representing idle on net monitor'],


        # added 1.26.2003
        'ColorNormalWorkspaceButtonText': [COLOR, '"rgb:00/00/00"', 'Color of normal workspace button text'],
        'ColorActiveWorkspaceButtonText': [COLOR, '"rgb:00/00/EE"', 'Color of inactive workspace button text'],
        'ColorNormalWorkspaceButton': [COLOR, '"rgb:C0/C0/C0"', 'Color of normal workspace button'],
        'ColorActiveWorkspaceButton': [COLOR, '"rgb:A0/A0/A0"', 'Color of active workspace button'],
    }
        

########################## ORDER ########################

# This list defines the order in which options will be output to the
# '.icewm/preferences' file.

##  NEW ADDITION - 1.26.2003  - 'ORDER' is now based on the logic arrangement of the TABS....see below
## THIS will help the order of the options saved in the preferences file to directly map over to the order 
## of stuff on the tabs

ORDER=[]  # initialize an empty list instead



########################## TABS ########################


# This list controls how the various configuration options are distributed
# on the tabs of the note book.  It is a list of two item sublists.  Each of
# these sublists contains the label of a tab and a list of the configuration
# options associated with that tab.  
TABS = [

        [_('Windows'),
            [
            'BorderSizeX',
            'BorderSizeY',
            'CornerSizeX',
            'CornerSizeY',
            'ColorActiveBorder',
            'ColorNormalBorder',
            'MinimizedWindowFontName',
            'ColorNormalMinimizedWindow',
            'ColorNormalMinimizedWindowText',
            'ColorActiveMinimizedWindow',
            'ColorActiveMinimizedWindowText',
            'ColorMoveSizeStatus',
            'ColorMoveSizeStatusText',
            'ShapesProtectClientWindow',
'menuButtonA.xpm',
'menuButtonI.xpm',
'closeA.xpm',
'closeI.xpm',
'maximizeA.xpm',
'maximizeAb.xpm',
'maximizeI.xpm',
'maximizeIb.xpm',
'minimizeI.xpm',
'minimizeA.xpm',
'restoreA.xpm',
'restoreI.xpm',
'titleAB.xpm',
'titleAL.xpm',
'titleAR.xpm',
'titleAS.xpm',
'titleAT.xpm',
'titleIB.xpm',
'titleIL.xpm',
'titleIR.xpm',
'titleIS.xpm',
'titleIT.xpm',
            ]],


        [_('Quick Switch'),
            [
            # added 1.25.2003
            'QuickSwitchFillSelection',
            'QuickSwitchTextFirst',
            'QuickSwitchIconBorder',
            'QuickSwitchIconMargin',
            'QuickSwitchHorzMargin',
            'QuickSwitchVertMargin',
            'QuickSwitchSeparatorSize',
            'QuickSwitchFontName',
            'ColorQuickSwitch',
            'ColorQuickSwitchText',
            'ColorQuickSwitchActive',
'switchbg.xpm',
            ]],


        [_('Dialogs'),
            [
            'DlgBorderSizeX',
            'DlgBorderSizeY',
            'NormalButtonFontName',
            'ActiveButtonFontName',
            'ListBoxFontName',
            'LabelFontName',
            'ColorDialog',
            'ColorListBox',
            'ColorListBoxText',
            'ColorListBoxSelection',
            'ColorListBoxSelectionText',
            'ColorLabel',
            'ColorLabelText',
            'ColorNormalButton',
            'ColorNormalButtonText',
            'ColorActiveButton',
            'ColorActiveButtonText',
'dialogbg.xpm',
'logoutbg.xpm',
            ]],

        [_('Menus'),
            [
            'MenuFontName',
            'ColorNormalMenu',
            'ColorActiveMenuItem',
            'ColorActiveMenuItemText',
            'ColorNormalMenuItemText',
            'ColorDisabledMenuItemText',
            # NEW MENU Stuff - added 1.26.2003
            'ColorDisabledMenuItemShadow',
'menubg.xpm',
            ]],
        [_('Task Bar'),
            [
            # added 1.25.2003
            'TrayDrawBevel',
            'TaskBarDoubleHeight',
            'NormalTaskBarFontName',
            'ActiveTaskBarFontName',
            'ColorDefaultTaskBar',
            'ColorNormalTaskBarApp',
            'ColorNormalTaskBarAppText',
            'ColorActiveTaskBarApp',
            'ColorActiveTaskBarAppText',
            'ColorMinimizedTaskBarApp',
            'ColorMinimizedTaskBarAppText',
            'ColorInvisibleTaskBarApp',
            'ColorInvisibleTaskBarAppText',
            # added 1.26.2003
            'ToolButtonFontName',
            'ColorToolButton',
            'ColorToolButtonText',
'taskbar/linux.xpm',
'taskbar/taskbarbg.xpm',
'taskbar/menubg.xpm',
'taskbar/windows.xpm',
'taskbar/toolbuttonbg.xpm',
'taskbar/taskbuttonbg.xpm',
'taskbar/taskbuttonactive.xpm',
'taskbar/taskbuttonminimized.xpm',
            ]],

        [_('Task Bar Applets'),
            [
            'TaskBarClockLeds',
            'ColorClock',
            'ColorClockText',
            'ClockFontName',

'ledclock/n0.xpm',
'ledclock/n1.xpm',
'ledclock/n2.xpm',
'ledclock/n3.xpm',
'ledclock/n4.xpm',
'ledclock/n5.xpm',
'ledclock/n6.xpm',
'ledclock/n7.xpm',
'ledclock/n8.xpm',
'ledclock/n9.xpm',
'ledclock/a.xpm',
'ledclock/m.xpm',
'ledclock/p.xpm',
'ledclock/colon.xpm',
'ledclock/dot.xpm',
'ledclock/slash.xpm',
'ledclock/space.xpm',
            'TaskBarCPUSamples',
            'ColorCPUStatusUser',
            'ColorCPUStatusSystem',
            'ColorCPUStatusNice',
            'ColorCPUStatusIdle',
            'ColorNetSend',
            'ColorNetReceive',
            'ColorNetIdle',
            'ColorApm',
            'ApmFontName',
            'ColorApmText',
'mailbox/mail.xpm',
'mailbox/newmail.xpm',
'mailbox/nomail.xpm',
'mailbox/unreadmail.xpm',
'mailbox/errmail.xpm',
            ]],

        [_('Scroll Bars'),
            [
            # NEW MENU Stuff - added 1.26.2003 - scrollbars
            'ScrollBarX',
            'ScrollBarY',
            'ColorScrollBar',
            'ColorScrollBarArrow',
            'ColorScrollBarSlider',
            'ColorScrollBarButton',
            'ColorScrollBarButtonArrow',
            'ColorScrollBarInactiveArrow',
            ]],


        [_('Cursors'),
            [     
'cursors/move.xpm',
'cursors/left.xpm',
'cursors/right.xpm',
'cursors/sizeL.xpm',
'cursors/sizeR.xpm',
'cursors/sizeB.xpm',
'cursors/sizeBL.xpm',
'cursors/sizeBR.xpm',
'cursors/sizeT.xpm',
'cursors/sizeTL.xpm',
'cursors/sizeTR.xpm',
            ]],

        [_('Icons'),
            [     
'icons/folder_16x16.xpm',
'icons/folder_32x32.xpm',
'icons/xterm_16x16.xpm',
'icons/xterm_32x32.xpm',
'icons/app_16x16.xpm',
'icons/app_32x32.xpm',
'icons/netscape_16x16.xpm',
'icons/netscape_32x32.xpm',
'icons/mozilla_16x16.xpm',
'icons/mozilla_32x32.xpm',
'icons/galeon_16x16.xpm',
'icons/galeon_32x32.xpm',
'icons/opera_16x16.xpm',
'icons/opera_32x32.xpm',
'icons/email_16x16.xpm',
'icons/email_32x32.xpm',
'icons/terminal_16x16.xpm',
'icons/terminal_32x32.xpm',
'icons/gimp_16x16.xpm',
'icons/gimp_32x32.xpm',
'icons/emacs_16x16.xpm',
'icons/emacs_32x32.xpm',
'icons/vim_16x16.xpm',
'icons/vim_32x32.xpm',

            ]],

        [_('Tool Tips'),
            [     
            'ToolTipFontName',
            'ColorToolTip',
            'ColorToolTipText',
            ]],
        [_('Title Bars'),
            [
            'TitleButtonsSupported',
            'TitleButtonsLeft',
            'TitleButtonsRight',  
            'TitleBarHeight',
            'TitleFontName',
            'ColorActiveTitleBar',
            'ColorNormalTitleBar',
            'ColorActiveTitleBarText',
            'ColorNormalTitleBarText',

            # NEW Stuff - added 1.26.2003 - title bars
            'ColorActiveTitleBarShadow',
            'ColorNormalTitleBarShadow',
            'ColorNormalTitleButton',
            'ColorNormalTitleButtonText',
            'TitleBarJoinRight',
            'TitleBarJoinLeft',
            'TitleBarJustify',
            'TitleBarHorzOffset',
            'TitleBarVertOffset',
            ]],
        [_('Theme Info'),
            [
            'Theme',
            'ThemeAuthor',
            'ThemeDescription',
            'Look',
            'Gradients'
            ]],

        [_('Address Bar'),
            [
            # added 1.26.2003
            'InputFontName',
            'ColorInput',
            'ColorInputText',
            'ColorInputSelection',
            'ColorInputSelectionText',
            ]],

        [_('Desktop'),
            [
            'DesktopBackgroundCenter',
            'DesktopBackgroundColor',
            'DesktopBackgroundImage',
            # NEW Stuff - added 1.26.2003 - desktop
            'DesktopTransparencyColor',
            'DesktopTransparencyImage',
            ]],

        [_('Workspaces'),
            [
            'ColorNormalWorkspaceButtonText',
            'ColorActiveWorkspaceButtonText',
            'ColorNormalWorkspaceButton',
            'ColorActiveWorkspaceButton',
            'NormalWorkspaceFontName',
            'ActiveWorkspaceFontName',
            'StatusFontName',
'taskbar/workspacebuttonbg.xpm',
'taskbar/workspacebuttonactive.xpm',
            ]]
    ]

## added 1.27.2003 - a list of tab names
TABS_NAMES=[]
for ii in TABS:
    TABS_NAMES.append(ii[0])

# image ignore list
IMAGE_IGNORE=['titleIR.xpm','closeI.xpm','titleAR.xpm','titleIL.xpm','logoutbg.xpm','maximizeI.xpm','menuButtonA.xpm','menuButtonI.xpm','restoreI.xpm','minimizeI.xpm','minimizeA.xpm','switchbg.xpm','titleAB.xpm','restoreA.xpm','titleAT.xpm','titleAS.xpm','titleIB.xpm','closeA.xpm','maximizeAb.xpm','maximizeIb.xpm','titleIT.xpm','titleIS.xpm','menubg.xpm','dialogbg.xpm','maximizeA.xpm','titleAL.xpm','ledclock/a.xpm','ledclock/colon.xpm','ledclock/dot.xpm','ledclock/m.xpm','ledclock/n0.xpm','ledclock/n1.xpm','ledclock/n2.xpm','ledclock/n3.xpm','ledclock/n4.xpm','ledclock/n5.xpm','ledclock/n6.xpm','ledclock/n7.xpm','ledclock/n8.xpm','ledclock/n9.xpm','ledclock/p.xpm','ledclock/slash.xpm','ledclock/space.xpm','taskbar/windows.xpm','taskbar/taskbuttonbg.xpm','taskbar/taskbuttonminimized.xpm','taskbar/taskbuttonactive.xpm','taskbar/taskbarbg.xpm','taskbar/workspacebuttonactive.xpm','taskbar/workspacebuttonbg.xpm','taskbar/linux.xpm','taskbar/toolbuttonbg.xpm','taskbar/menubg.xpm','mailbox/errmail.xpm','mailbox/mail.xpm','mailbox/newmail.xpm','mailbox/nomail.xpm','mailbox/unreadmail.xpm','cursors/left.xpm','cursors/move.xpm','cursors/right.xpm','cursors/sizeB.xpm','cursors/sizeBL.xpm','cursors/sizeBR.xpm','cursors/sizeL.xpm','cursors/sizeR.xpm','cursors/sizeT.xpm','cursors/sizeTL.xpm','cursors/sizeTR.xpm','icons/folder_32x32.xpm','icons/folder_16x16.xpm','icons/xterm_32x32.xpm','icons/xterm_16x16.xpm','icons/app_32x32.xpm','icons/app_16x16.xpm','icons/netscape_16x16.xpm','icons/netscape_32x32.xpm','icons/mozilla_16x16.xpm','icons/mozilla_32x32.xpm','icons/galeon_16x16.xpm','icons/galeon_32x32.xpm','icons/opera_16x16.xpm','icons/opera_32x32.xpm','icons/email_16x16.xpm','icons/email_32x32.xpm','icons/terminal_16x16.xpm','icons/terminal_32x32.xpm','icons/gimp_16x16.xpm','icons/gimp_32x32.xpm','icons/emacs_16x16.xpm','icons/emacs_32x32.xpm','icons/vim_16x16.xpm','icons/vim_32x32.xpm','Theme']

## NEW MORE LOGICAL 'ORDER' mapping for preferences file - added 1.26.2003
for ii in TABS:
    ilist=ii[1]
    for mj in ilist:
        if not mj in IMAGE_IGNORE: ORDER.append(mj)

del ORDER[ORDER.index("ThemeAuthor")]
del ORDER[ORDER.index("ThemeDescription")]
del ORDER[ORDER.index("Look")]
# Put theme author and desription at beginning of order, so it gets written near top of theme file
ORDER=['ThemeAuthor','ThemeDescription','Look']+ORDER
#print str(ORDER[0:20])

# added 5.10.2003 - basic search functionality
icepref_search.TABS=TABS
icepref_search.DEFAULTS=DEFAULTS
    
############################################################################
# Classes used in the general IcePref user interface
#
# These are placed at the beginning so that they may be inherited from by
# all other classes
#
############################################################################

## added 2.23.2003 - break long 'title' strings
def breakTitle(somestr,sstopper=30):
    stopper=sstopper
    if len(somestr)<stopper+1: return somestr
    scut=""
    scutter=0
    while scutter<len(somestr):
        if somestr.find(" ",scutter+stopper)==-1: part=somestr[scutter:len(somestr)].strip()
        else: part=somestr[scutter:somestr.find(" ",scutter+stopper)].strip()
        if len(part)==0: break
        #print str(scutter)
        if len(scut)>0: scut=scut+"\n"+part
        else: scut=part
        if somestr.find(" ",scutter+stopper)==-1: break
        scutter=somestr.find(" ",scutter+stopper)+1
    return scut
        
# This class defines the standard style of labels used in many other classes
# of IcePref

class Label(gtk.Label):
    def __init__(self, title,sstopper=30):
        gtk.Label.__init__(self, breakTitle(title,sstopper))
        self.set_line_wrap(gtk.TRUE)
        self.set_alignment(gtk.JUSTIFY_LEFT, 0)
        self.set_padding(2,2)
        
# This is a somewhat generic class for dialogs
 
class Dialog(gtk.Window):
    def __init__(self, titlebar, title):
        gtk.Window.__init__(self, title=titlebar)
        self.set_modal(gtk.TRUE)
        self.init_widgets(title)
        self.init_buttons()
        self.show()
    
    def init_widgets(self, title):
        vbox = gtk.VBox(spacing = SP)
        vbox.set_border_width(BD)
        self.add(vbox)
        vbox.show()

        label = Label(title,180)
        vbox.pack_start(label)
        label.show()

        self.bbox = gtk.HButtonBox()
        vbox.pack_start(self.bbox)
        self.bbox.show()

    def init_buttons(self):
        self.ok_button = gtk.Button(_('Ok'))
        self.ok_button.grab_focus()
        self.ok_button.connect('clicked', self.button_cb, 1)
        self.bbox.pack_start(self.ok_button)
        self.ok_button.show()
        
    def button_cb(self, object=None, data=None):
        ret = data
        self.hide()
        self.destroy()

##############################################################################
# The configuration option classes
#
# These classes are designed to implement the configuration user interface in
# generic way.  All of them (at least all of them that are directly used
# by the Application class) have the methods set_value and get_value, the
# function of which should be self evident.  To facilitate interaction with
# the text configuration file, set_value always takes a string argument and
# get_value always returns a string value.
#
##############################################################################

# The Entry class describes the text entries.
# title = the text of the label
# value = the intial value
        
class Entry(gtk.VBox):
    def __init__(self, title='', value=''):
        gtk.VBox.__init__(self, spacing=SP)
        self.set_border_width(BD)
        self.init_widgets(title)
        self.set_value(value)
        self.show_all()
        
    def init_widgets(self, title):
        label=gtk.Label(title)
        label.show()
        self.pack_start(label, gtk.FALSE, gtk.FALSE)
        label.set_line_wrap(1)
        label.set_alignment(0.0,0.5)      
        self.entry = gtk.Entry()
        self.entry.show()
        self.pack_start(self.entry, 0, 0)
        self.pack_start(gtk.Label(""),1,1,0)
        
    def set_value(self, value):
        #value = value[1 : len(value) - 1]  # changed 1.27.2003 - was buggy and could cause bad parsing
        value=value.replace("\"","")
        self.entry.set_text(value)
        
    def get_value(self):
        value = self.entry.get_text()
        value = '"' + value + '"'
        return value
        
# The Toggled class describes the check buttons (for boolean options).
# title = text of the label
# value = initial value
        
class Toggled(gtk.VBox):
    def __init__(self, title='', value=''):
        gtk.VBox.__init__(self, spacing=SP)
        self.set_border_width(BD)
        self.init_widgets(title)
        self.set_value(value)
        self.show_all()
        
    def init_widgets(self, title):
        self.button = gtk.CheckButton()  # changes made here - 1.26.2003
        try: # added 2.23.2003 - cleaner toggle buttons
            pop=gtk.Label(title)
            pop.set_alignment(0.0,0.5)  # set label alignment to left
            pop.set_line_wrap(gtk.TRUE)
            hb=gtk.HBox(0,0)
            hb.set_border_width(6)
            hb.pack_start(pop,1,1,0)
            self.button.add(hb)
            hb.show_all()
        except:
            pass
        self.button.show()
        self.pack_start(self.button, gtk.FALSE, gtk.FALSE, 0)
        self.pack_start(gtk.Label(""),1,1,0)
        
    def set_value(self, value):
        self.button.set_active(eval(value))
        
    def get_value(self):
        value = self.button.get_active()
        return str(value)

# The Range class describes the range widget.
# Currently, it includes both a gtk.Adjustment
# widget and a spinbutton. This could, of
# course, be changed if it is found to be
# unsatisfactory.  title = text of the label
# min = bottom of the allowed range max = top
# of the allowed range value = initial value

class Range(gtk.VBox):
    def __init__(self, title='', min=0, max=100, value=0):
        gtk.VBox.__init__(self, spacing=SP)
        self.set_border_width(BD)
        self.init_widgets(title, min, max)
        self.set_value(value)
        self.show()
        
    def init_widgets(self, title, min, max):
        hbox = gtk.HBox(gtk.FALSE, SP)
        hbox.set_border_width(BD)
        hbox.show()
        self.pack_start(hbox, gtk.FALSE, gtk.FALSE, 0)
        label = Label(title)
        label.show()
        hbox.pack_start(label, gtk.FALSE, gtk.FALSE, 0)
        adj = gtk.Adjustment(lower=min, upper=max, step_incr=1)
        self.spin = gtk.SpinButton(adj)
        self.spin.set_digits(0)
        self.spin.show()
        hbox.pack_end(self.spin, gtk.FALSE, gtk.FALSE, 0)
        scale = gtk.HScale(adj)
        scale.set_draw_value(gtk.FALSE)
        scale.show()
        self.pack_start(scale, gtk.FALSE, gtk.FALSE, 0)
        
    def set_value(self, value):
        self.spin.set_value(eval(value))
        
    def get_value(self):
        ret = self.spin.get_value_as_int()
        return str(ret)


def setDrag(*args): # drag-n-drop support, added 2.23.2003
    global CONFIG_THEME_PATH
    drago=args
    if len(drago)<7: 
        args[0].drag_highlight()
        args[0].grab_focus()
    else: 
        try:
            imfile=""+str(drago[4].data).replace("\r\n","").replace("\x00","").replace("file:","")
            try:
                args[0].get_data("run-conv")(imfile)
            except:
                pass
            try:
                args[0].get_data("bg_entry").get_data("run-conv")(imfile,CONFIG_THEME_PATH+"background.png",args[0].get_data("bg_entry").entry)
            except:
                pass
        except: 
            pass
        args[0].drag_unhighlight()


## Added 1.27.2003 - a simple image preview for the wallpaper and theme, using GdkImlib

class ImagePreviewer:
    def __init__(self,warn_type=0,restart_cmd=None,save_cmd=None):
        self.savec=save_cmd
        self.restartc=restart_cmd
        self.scbox=gtk.HBox(0,0)
        self.sc=gtk.ScrolledWindow()
        self.no_scale=0
        if warn_type==0: self.no_scale=1
        self.gl=gtk.Layout()

        self.gl.set_size(400,300)
        self.sc.add(self.gl)
        self.current_image=None
        self.scbox.pack_start(self.sc,0,0,0)
        if warn_type==0:
            addDragSupport(self.sc,setDrag)
            self.sc.set_size_request(100,80)
        else:       self.sc.set_size_request(240,190)     
        if warn_type==1:  #Background Image
            self.scbox.set_spacing(4)
            self.scbox.pack_start(gtk.Label("   "),0,0,0)
            addDragSupport(self.sc,setDrag)
        if warn_type==2:  # Theme previewer
            self.scbox.set_spacing(7)
            svert=gtk.VBox(0,0)
            bgen=gtk.Button(_(" Create Theme Preview... "))
            svert.pack_start(bgen,0,0,0)
            svert.pack_start(gtk.Label("  "),1,1,0)
            bgen.connect("clicked",self.generatePreview)
            self.scbox.pack_start(svert,0,0,0)
        if not warn_type==0: self.scbox.pack_start(gtk.Label("     "),0,0,0)

        # added 2.12.2003 - moved theme author and description to the right of the preview box
        self.entry_box=gtk.VBox(0,0)
        self.entry_box.set_spacing(10)
        self.scbox.pack_start(self.entry_box,1,1,0)
        self.scbox.show_all()

    def generatePreview(self,*args):  # added 3.14.2003 - auto generation of theme preview.jpg files 
        try:
            if os.path.exists(self.current_image):
                if not msg_confirm("IcePref2",_("The image")+":\n\n"+self.current_image+"\n\n"+_("already exists.")+"\n"+_("Do you wish to replace it?"))==1: return
        except:
            pass
        msg_info("IcePref2",_("Apply your theme by selecting it\nfrom the IceWM 'Themes' menu.\nWhen you are ready, click 'OK'."))
        gtk.timeout_add(1500,self.generatePreviewStep2)

    def generatePreviewStep2(self,*args):
        try:
            global CONFIG_THEME_PATH
            self.current_image=CONFIG_THEME_PATH+"preview.jpg"
            f=os.popen("import -window root -resize 200x200 "+self.current_image.replace(" ","\\ "))
            f.readlines()
            if not os.path.exists(self.current_image): raise TypeError
            self.update_image(self.current_image)
        except:
            Dialog("IcePref2",_("A theme preview image could not be created. This features requires the 'import' binary from ImageMagick to be on your path."))   

            # NOTE: We must leave the above message as using 'Dialog' instead of one of the common 
            # dialogs, because we need a Dialog box that doesn't call 'gtk.mainloop' while running in a gtk 'timeout'.
            # Using one of the 'common' dialogs will cause a bug where IcePref2-TD hangs.
        return 0

    def update_image(self,image_file):
        for ii in self.gl.get_children():
            try:
                self.gl.remove(ii)
                ii.destroy()
                ii.unmap()
            except:
                pass
#DJM!!!        try:
        if 1:
            self.current_image=str(image_file).strip()
            if not os.path.exists(self.current_image): return
            if os.path.isdir(self.current_image): return

            myim = gtk.Image()
            myim.set_from_file(self.current_image)
#myim.changed_image() # disabled cached regurgitation
#            myim.render()
#            pixtemp=myim.make_pixmap()
#            dims=pixtemp.size_request()
            dims = myim.size_request()
#            print "DJM dims",dims
#            pixtemp.destroy()
#            pixtemp.unmap()
            aspect=float(dims[0])/float(dims[1])
            sug_x=220
            sug_y=int( math.floor(sug_x/aspect)  )
            if self.no_scale==0:
#                myim_real=myim.clone_scaled_image(sug_x,sug_y)              
                tmppix=myim.get_pixbuf()
#                print "scale to ", sug_x, sug_y
                tmppix2 = tmppix.scale_simple(sug_x, sug_y, gtk.gdk.INTERP_BILINEAR)
                pixreal = gtk.Image()
                pixreal.set_from_pixbuf(tmppix2)
#                myim_real=myim.clone_scaled_image(sug_x,sug_y)              
            else: 
                pixreal=myim
            pixreal.show()
            self.gl.put(pixreal,3,2)
            self.gl.set_size(pixreal.size_request()[0]+15,pixreal.size_request()[1]+15)
#DJM!!!        except:
        else:
            lab=gtk.Label(" "+_("None"))
            lab.show()
            self.gl.put(lab,0,0)
            self.gl.set_size(220,180)

    def im_convert(self,imfile,tofile,some_entry):
        try:
            f=os.popen("convert "+imfile.replace(" ","\\ ")+" "+tofile.replace(" ","\\ "))
            f.readlines()
            if not os.path.exists(tofile): raise TypeError
            self.update_image(tofile)
            some_entry.set_text(tofile[tofile.rfind(os.sep)+1:len(tofile)])
        except:
            self.update_image("non_existant_image_file")
            some_entry.set_text(_("NO IMAGE"))

# The ButtonEntry class is a relatively generic class for metawidgets that
# contain both a text entry and a button to call up a dialog box.  Color,
# Font, and File all inherit from this class.  Path inherits from File.
# title = label text
# value = initial value

class ButtonEntry(gtk.VBox):
    def __init__(self, title='', value='',image_previewer=None):
        gtk.VBox.__init__(self, gtk.FALSE, SP)
        self.set_border_width(BD)
        self.INITVALUE= value[1 : len(value) - 1]
        self.init_widgets(title)
        self.iprev=image_previewer
        self.set_data("previewer",self.iprev)
        self.set_value(value)
        self.show_all()
        
    def init_widgets(self, title):
        
        hbox = gtk.HBox(gtk.FALSE, SP)
        vbsmall=gtk.VBox(0,0)
        vbsmall.set_spacing(4)
        hbox.pack_start(vbsmall, gtk.TRUE, gtk.TRUE, 0)
        self.pack_start(hbox, gtk.FALSE, gtk.FALSE, 0)
        vbsmall.pack_start(gtk.Label(""), 1, 1, 0)

        label = Label(title)
        vbsmall.pack_start(label, gtk.FALSE, gtk.FALSE, 0)
        
        self.entry = gtk.Entry()
        TIPS.set_tip(self.entry,_(title))
        vbsmall.pack_start(self.entry, 0, 0, 0)

        ## added 2.23.2003 - try to load a prettier select button
        button=gtk.Button()
        try:
            button.add(  loadScaledImage(getPixDir()+"ism_load.png",24,24)  )
        except:
            button.add(gtk.Label("..."))
        TIPS.set_tip(button,_("Select"))
        self.button=button
        button.connect('clicked', self.select)
        hbhide=gtk.VBox(0,0)
        hbhide.pack_start(gtk.Label(""),1,1,0)
        hbhide.pack_start(button,0,0,0)
        hbox.pack_start(hbhide, gtk.FALSE, gtk.FALSE, 0)
        self.hbox=hbox
        
    def set_value(self, value):
        value = value[1 : len(value) - 1]
        self.entry.set_text(value)
        if not self.iprev==None: 
            self.iprev.update_image(value)
        
    def get_value(self):
        value = '"' + self.entry.get_text() + '"'
        return value
        
    def cancel(self, data=None):
        self.win.hide()
        self.win.destroy()
        
# The Color class is used for configuring (obviously) options which require
# the rgb value for a color.  It is a derivative of the ButtonEntry class and
# therefore accepts the same options.   
                
class Color(ButtonEntry):
    def select(self, data=None):
        value = self.entry.get_text()
        self.win = gtk.ColorSelectionDialog(name=_('Select Color'))
        self.win.colorsel.set_has_opacity_control(gtk.FALSE)
        self.win.colorsel.set_update_policy(gtk.UPDATE_CONTINUOUS)
        self.win.set_position(WIN_POS_MOUSE)
        self.win.ok_button.connect('clicked', self.ok)
        self.win.cancel_button.connect('clicked', self.cancel)
        self.win.help_button.destroy()
        if value != '':
            r = atoi(value[4:6], 16) / 255.0
            g = atoi(value[7:9], 16) / 255.0
            b = atoi(value[10:12], 16) / 255.0
            self.win.colorsel.set_color((r, g, b))
        self.win.set_modal(gtk.TRUE)
        self.win.show()

    def init_widgets(self, title):
        label = Label(title)
        label.show()
        self.pack_start(label, gtk.FALSE, gtk.FALSE, 0)     
        hbox = gtk.HBox(gtk.FALSE, SP)
        hbox.show()
        self.pack_start(hbox, gtk.FALSE, gtk.FALSE, 0)      
        self.entry = gtk.Entry()
        self.entry.show()
        hbox.pack_start(self.entry, gtk.TRUE, gtk.TRUE, 0)
        self.hbox=hbox

        button=gtk.Button()
        TIPS.set_tip(button,_("Select"))
        self.button=button
        button.set_relief(gtk.RELIEF_NONE)
        self.gc=None
        self.color_i16=[0,0,0]
        value = self.INITVALUE
        if value != '':
            r = atoi(value[4:6], 16) / 255.0
            g = atoi(value[7:9], 16) / 255.0
            b = atoi(value[10:12], 16) / 255.0
            self.color_i16=[r,g,b]
        f=gtk.Frame()
        f.set_shadow_type(gtk.SHADOW_ETCHED_OUT)
        drawing_area = gtk.DrawingArea()
        drawing_area.set_size_request(22, 18)
        drawing_area.show() 
        self.drawing_area=drawing_area
        f.add(drawing_area)
        self.drawwin=drawing_area.window
        drawing_area.connect("expose_event", self.setColor)
        drawing_area.set_events(gtk.gdk.EXPOSURE_MASK |
        gtk.gdk.LEAVE_NOTIFY_MASK |
        gtk.gdk.BUTTON_PRESS_MASK )
        button.add(f)
        button.show()
        hbox.pack_start(button, gtk.FALSE, gtk.FALSE, 1)
        self.hbox.show_all()
        button.set_data("colorbutton",self)  # added 5.16.2003
        addDragSupportColor(button)  # added 5.16.2003
        self.entry.set_data("colorbutton",self)  # added 5.16.2003
        addDragSupportColor(self.entry)  # added 5.16.2003
        button.connect('clicked', self.select)

    def setColor(self,*args):
        if not self.drawwin: self.drawwin=self.drawing_area.window
        if not self.gc: self.gc=self.drawwin.new_gc()
        self.gc.foreground=self.drawwin.get_colormap().alloc_color(self.color_i16[0]*65535,
                                                                   self.color_i16[1]*65535,
                                                                   self.color_i16[2]*65535)
        width, height = self.drawwin.get_size()
        self.drawwin.draw_rectangle(self.gc,gtk.TRUE,0,0,width,height)

    def set_value(self, value):
        # make allowances for HTML-style color specs
        value=value.lower().replace("\"","").   \
                            replace("'","").    \
                            replace("/","").    \
                            replace("rgb:",""). \
                            replace(" ","").    \
                            replace("#","").strip()
        if not len(value)==6: 
            return
        hvalue=""+value
        value="rgb:"+ hvalue[0:2]+"/"+hvalue[2:4]+ "/"  +hvalue[4:6]
        self.entry.set_text(value)
        r,g,b=getRGBForHex(hvalue)        
        self.color_i16=[r/255.0,g/255.0,b/255.0]
        try:
            self.setColor()
        except:
            pass

    def updateColorProperties(self, r, g, b): # new method,functionality separation, 5.16.2003
        self.color_i16=[r,g,b]
        color = [r, g, b]
        for i in range(0,3):
            color[i] = hex(int(color[i] * 255.0))[2:]
            if len(color[i]) == 1:
                color[i] = '0' + color[i]                 
        r,g,b = color[0], color[1], color[2]          
        value = 'rgb:' + r + '/' + g + '/' + b
        self.entry.set_text(value)
        self.setColor()
        
    def ok(self, data=None):
        raw_values = self.win.colorsel.get_color()
        r,g,b = raw_values
        self.win.hide()
        self.win.destroy()
        self.updateColorProperties(r,g,b)


# Font class is used (wonder of wonders) to select a font and returns an X
# font descriptor, which coincidentally is just what icewm demands.  It is
# a derivative of the ButtonEntry superclass and takes the same options.
        
class Font(ButtonEntry):

    def init_widgets(self, title):
        label = Label(title)
        label.show()
        self.pack_start(label, gtk.FALSE, gtk.FALSE, 0)
        
        self.sample = gtk.TextView()
        self.sample.set_editable(gtk.FALSE)
        self.sample.set_size_request(200,50)
        self.pack_start(self.sample)
        self.sample.show()
        
        hbox = gtk.HBox(gtk.FALSE, SP)
        hbox.show()
        self.pack_start(hbox, 0, 0, 0)
        self.pack_start(gtk.Label(""),1,1,0)
        
        self.entry = gtk.Entry()
        self.entry.connect('changed', self.update)
        self.entry.show()
        hbox.pack_start(self.entry, gtk.TRUE, gtk.TRUE, 0)
        
        ## added 2.23.2003 - try to load a prettier select button
        button=gtk.Button()
        try:
            button.add(  loadScaledImage(getPixDir()+"ism_load.png",24,24)  )
        except:
            button.add(gtk.Label("..."))
        TIPS.set_tip(button,_("Select"))
        self.button=button
        button.connect('clicked', self.select)
        button.show()
        hbox.pack_start(button, gtk.FALSE, gtk.FALSE, 0)
        
    def set_value(self, value):
        value = value[1 : len(value) - 1]
        self.entry.set_text(value)
        self.set_sample(value)
        
    def set_sample(self, value):
#DJM        length = self.sample.get_length()
#DJM        if length > 0:
#DJM            self.sample.delete_text(0, length)        
        # The structure catches errors which are caused by invalid
        # font specifications.
#DJM        try:
        if 1:
#DJM            print "Font: "+str(value)
            font = pango.FontDescription(value)
            self.sample.modify_font(font)
            self.sample.get_buffer().set_text(SAMPLE_TEXT)
#DJM            font = load_font(value)
#DJM            self.sample.insert(fg=None, bg=None, font=font, string=SAMPLE_TEXT)
        # If the font is invalid, it should trigger this little wonder
#DJM        except RuntimeError:
#DJM            self.sample.in

    def select(self, data=None):
        self.win = gtk.FontSelectionDialog(_('Select Font'))
        self.win.ok_button.connect('clicked', self.ok)
        self.win.cancel_button.connect('clicked', self.cancel)
        value = self.entry.get_text()
        if value !='':
            self.win.fontsel.set_font_name(value)
        self.win.set_modal(gtk.TRUE)
        self.win.show()
        
    def ok(self, data=None):
        value = self.win.fontsel.get_font_name()
        self.win.hide()
        self.win.destroy()
        self.entry.set_text(value)
        self.set_sample(value)
        
    def update(self, widget, data=None):
        value = self.entry.get_text()
        self.set_sample(value)
        
# The File class is used for options that require a file name and path.  It is
# a derivative of the Button Entry class and takes the same options.
        
class File(ButtonEntry):
    def __init__(self,title="",value="",im_previewer=None):
        ButtonEntry.__init__(self,title,value,im_previewer)
        global CONFIG_THEME_PATH
        self.last_file=CONFIG_THEME_PATH

    def select(self, data=None):
        self.win = gtk.FileSelection(_("Select a file..."))
        self.win.ok_button.connect('clicked', self.ok)
        self.win.cancel_button.connect('clicked', self.cancel)
        value = self.entry.get_text()
        if value.find(os.sep)==-1: value=self.last_file
        if value != '""':
            self.win.set_filename(value)
        self.win.set_modal(gtk.TRUE)
        self.win.show()
        
    def ok(self, data=None):
        value = self.win.get_filename()
        if not value==None:
            if not value=='':
                self.last_file=value
        self.win.hide()
        self.win.destroy()
        if not self.iprev==None: 
            global CONFIG_THEME_PATH
            self.iprev.im_convert(value,CONFIG_THEME_PATH+"background.png",self.entry)
        else:       self.entry.set_text(value)


# for the theme .xpms
class ImageWidget(File):
    def __init__(self, title='', value='',image_previewer=None):
        self.PREV=image_previewer
        global CONFIG_THEME_PATH
        self.last_file=CONFIG_THEME_PATH
        ButtonEntry.__init__(self,title,value,image_previewer)

    def init_widgets(self, title):      
        ButtonEntry.init_widgets(self,title)
        self.PREV.sc.set_data("run-conv",self.run_conv)
        self.entry.set_data("run-conv",self.run_conv)
        self.button.set_data("run-conv",self.run_conv)
        addDragSupport(self.entry,setDrag)
        addDragSupport(self.button,setDrag)
        delbut=gtk.Button()
        try:
            delbut.add(loadScaledImage(getPixDir()+"ism_nosound.png",24,24)  )
        except:
            delbut.add(gtk.Label(_("Delete")))
        delbut.set_data("widget",self)
        TIPS.set_tip(delbut,_("Delete"))
        delbut.connect("clicked",self.do_del)

        ebut=gtk.Button()
        try:
            ebut.add(loadScaledImage(getPixDir()+"ism_edit2.png",24,24)  )
        except:
            ebut.add(gtk.Label(_("Edit")))
        ebut.set_data("widget",self)
        TIPS.set_tip(ebut,_("Edit with Gimp"))
        ebut.connect("clicked",self.do_edit)

        vd=gtk.VBox(0,0)
        vd.pack_start(ebut,0,0,0)
        vd.pack_start(gtk.Label(" "),1,1,0)
        vd.pack_start(delbut,0,0,0)
        self.PREV.scbox.pack_start(vd,0,0,4)
        self.hbox.pack_start(self.PREV.scbox,0,0,2)
        self.hbox.show_all()

    def run_conv(self,value):
#DJM!!        try: # convert to xpm and correct filename
        if 1:
            global CONFIG_THEME_PATH
            f=os.popen("convert "+value.replace(" ","\\ ")+" "+CONFIG_THEME_PATH.replace(" ","\\ ")+self.get_data("my_image"))
            f.readlines()
            if not os.path.exists(CONFIG_THEME_PATH+self.get_data("my_image")): raise TypeError
            self.entry.set_text(self.get_data("my_image")[self.get_data("my_image").rfind(os.sep)+1:len(self.get_data("my_image"))])
            if not self.iprev==None: self.iprev.update_image(CONFIG_THEME_PATH+self.get_data("my_image"))
#DJM!!        except:
        else:
            self.entry.set_text(_("NO IMAGE"))
            if not self.iprev==None: 
                self.iprev.update_image("NoN_exxxIstent_Image_38d_5")
            pass

    def ok(self, data=None):
        value = self.win.get_filename()
        if not value==None: self.last_file=value
        self.win.hide()
        self.win.destroy()
        self.run_conv(value)

    def do_del(self,*args):
        global CONFIG_THEME_PATH
        to_del=CONFIG_THEME_PATH+args[0].get_data("widget").get_data("my_image")
        if os.path.exists(to_del):
            if not msg_confirm(DIALOG_TITLE,_("Are you sure you want to delete")+":\n"+to_del)==1:  return
            try:
                os.remove(to_del)
            except:
                pass
            args[0].get_data("widget").PREV.update_image(to_del)

    def do_edit(self,*args):
        global CONFIG_THEME_PATH
        to_edit=CONFIG_THEME_PATH+args[0].get_data("widget").get_data("my_image")
#DJM!!!        os.popen("gimp-remote -n "+to_edit.replace(" ","\\ ")+" &")
        os.popen("gimp "+to_edit.replace(" ","\\ ")+" &")


# The ThemeData class, when passed the whole path of a theme, sets its members
# to the values required by the ThemeSel class.

class ThemeData:
    def __init__(self, full_path):
        self.full_path = full_path
        self.init_vars(full_path)

    def init_vars(self, full_path):
        slash_1 = rfind(full_path, '/')
        slash_2 = rfind(full_path[:slash_1], '/')
        self.name = full_path[slash_2 + 1 : slash_1]
        self.theme_file = full_path[slash_1 + 1:]
        self.full_name = self.name + ' (' + self.theme_file + ')'
        self.path = self.name + '/' + self.theme_file
        # added 1.27.2003 - try parsing for theme description and author info
        # default description:  "nice - default"  or "axxrom1.1 - spring", etc
        self.description=""+self.name+" - "+self.theme_file.replace(".theme","").strip()

        # added 2.12.2003 - more descriptive theme names in CLIST
        addon=self.theme_file.replace(".theme","").strip()
        if len(addon)>0:
            if not addon=="default":  self.name=self.name+"  ["+addon+"]"

        self.author="Author Unknown"
        try:
            f=open(full_path)
            flist=f.read().split("\n")[0:15]  # - check the first 15 lines of the theme file for info
            f.close()
            for ii in flist:
                ii=ii.replace("\t"," ")
                if (ii.strip().startswith("ThemeDescription")) and (ii.strip().find("=")>0): 
                    td=ii.replace("ThemeDescription","").replace("\"","").replace("\t"," ").replace("\r"," ").replace("=","").strip()
                    if len(td)>0: 
                        self.description=td
                if (ii.strip().startswith("ThemeAuthor")) and (ii.strip().find("=")>0): 
                    td=ii.replace("ThemeAuthor","").replace("\"","").replace("\t"," ").replace("\r"," ").replace("=","").strip()
                    if len(td)>0:
                        self.author=td
        except:
            pass

# This class creates a CList which can be used to select a theme.  It is quite
# specialized at this point.  Perhaps it could later be generalized to handle
# other options.
        
class ThemeSel(gtk.VBox):
    def __init__(self, title='', value='',Image_prev=None):
      gtk.VBox.__init__(self)
      self.set_border_width(BD)
      self.set_spacing(SP)
      self.init_theme_list()
      self.t_map={} # theme name to full path  # added 1.27.2003
      self.a_map={} # theme name to author # added 1.27.2003
      self.d_map={}  # theme name to theme description # added 1.27.2003
      self.author_entry=None # added 1.27.2003
      self.desc_entry=None # added 1.27.2003
      self.iprev=Image_prev  # added 1.27.2003
      self.set_value(value)
      self.init_widgets(title)
      self.show()
        
    def init_theme_list(self):
      self.theme_list = []
      for path in THEME_PATH:
        subdir_list = glob.glob(path)
        self.extract_theme_files(subdir_list)
            
    def extract_theme_files(self, subdir_list):
      for subdir in subdir_list:
            contents = glob.glob(subdir + '/*.theme')
            if contents != []:
                for file in contents:
                    theme = ThemeData(file)
                    self.append_theme(theme)
                    # self.theme_list.append(theme)
    
    def append_theme(self, theme):
      in_list = gtk.FALSE
      for entry in self.theme_list:
        if theme.full_name == entry.full_name:
          in_list = gtk.TRUE
      if not in_list:
        self.theme_list.append(theme)
                    
    def set_value(self, value):     
      self.value = value[1 : len(value) - 1]
      if self.value == '':
        self.value = self.theme_list[0].path
      if self.t_map.has_key(self.value):
        previm=self.t_map[self.value][0:self.t_map[self.value].rfind(os.sep)+1]+"preview.png"
        #print previm
        if not self.iprev==None: self.iprev.update_image(previm)
      if (self.a_map.has_key(self.value)) :     
        if not self.author_entry==None:
            self.author_entry.set_value(self.a_map[self.value])
      if (self.d_map.has_key(self.value)) :     
        if not self.desc_entry==None:
            self.desc_entry.set_value(self.d_map[self.value])
            
    def get_value(self):
      value = '"' + self.value + '"'
      return value
    
    def init_widgets(self, title):
        label = Label("IceWM Themes")
        self.pack_start(label)
        label.show()
        
        swin = gtk.ScrolledWindow()
        swin.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_AUTOMATIC)
        swin.set_size_request(150, 150)
        self.pack_start(swin)
        swin.show()
        
        clist = gtk.CList(2, [_('Theme Name')+'         ', _('File')+'   '])
        clist.set_column_width(0,290)
        clist.connect('select_row', self.clist_cb)
        swin.add(clist)
        clist.show()
        
        clist.freeze()
        row_count = 0
        for item in self.theme_list:
            row = [item.name, item.full_path]
            clist.append(row)
            clist.set_row_data(row_count, item.path)
            self.t_map[item.path]=item.full_path
            self.a_map[item.path]=item.author
            self.d_map[item.path]=item.description
            if item.path == self.value:
                clist.select_row(row_count,0)
            row_count = row_count + 1
        clist.sort()
        clist.columns_autosize()
        clist.thaw()
        
    def clist_cb(self, widget, row, col, event):
        self.value = widget.get_row_data(row)
        if self.t_map.has_key(self.value):
            previm=self.t_map[self.value][0:self.t_map[self.value].rfind(os.sep)+1]+"preview.jpg"
            #print previm
            if not self.iprev==None: 
                self.iprev.update_image(previm)
        if (self.a_map.has_key(self.value)) :   
            if not self.author_entry==None:
                self.author_entry.set_value(self.a_map[self.value])
        if (self.d_map.has_key(self.value)) :   
            if not self.desc_entry==None:
                self.desc_entry.set_value(self.d_map[self.value])

# The OptionMenu class is a generic class for numerically coded options with a small, fixed
# number of possible options.

class OptionMenu(gtk.VBox):
    def __init__(self, title='', value=''):
        gtk.VBox.__init__(self)
        self.set_border_width(BD)
        self.init_widgets(title)
        self.set_value(value)
        self.show()
        
    def init_widgets(self, title):
        label = Label(title)
        self.pack_start(label)
        label.show()
        
        self.option_menu =self.create_menu()
        self.pack_start(self.option_menu)
        self.option_menu.show()
        
    def init_options(self):
        self.options =['gtk','pixmap','metal','motif','nice']
        
    def create_menu(self):
        self.init_options()
        combo=gtk.Combo()
        self.combo=combo
        combo.set_popdown_strings(self.options)
        combo.entry.set_editable(0)                   
        return combo      
        
    def set_value(self, value):
        self.value=str(value).replace("\"","").replace("'","").lower()
        self.combo.entry.set_text(self.value)   

    def get_value(self):
        return self.combo.entry.get_text()

##############################################################################      
# The Application class -- mother of all IcePref2 classes
#
# The Application class is the main class of this little utility.  It is
# confusing and I don't feel like commenting it right now.
#
##############################################################################
    
class Application(gtk.Window):
    
    def __init__(self, argv):
        gtk.Window.__init__(self)
        self.set_title('IcePref2: '+_('Theme Designer'))
        global APP_WINDOW
        APP_WINDOW=self
        self.my_last_file=''
        self.set_position(gtk.WIN_POS_CENTER)
        self.set_wmclass("icepref-td","IcePref-TD")
        self.connect('destroy', gtk.mainquit)
        self.determine_os()
        self.find_global_preferences()
        self.init_settings()
        
        self.vbox1=gtk.VBox(0,0)
        self.vbox = gtk.VBox(0,4)
        self.vbox.set_border_width(6)
        self.add(self.vbox1)

        #added 1.26.2003
        self.my_tabs={} 

        TABS.sort()
        for tab in TABS:
            self.my_tabs[str(tab[0])]=TABS.index(tab)
        self.init_menu()
        self.widget_dict = {}
        # added 1.27.2003 - try to show a nice logo via GdkImlib
        try:
            logoim=GdkImlib.Image(getBaseDir()+"icepref2.png")
            logoim.render()
            logop=logoim.make_pixmap()
            self.vbox.pack_start(logop,0,0,1)
            logop.show_all()
        except:
            self.vbox.pack_start(gtk.Label("IcePref2"),0,0,1)
        self.vbox.pack_start(gtk.Label(_("Theme Designer")),0,0,1)
        self.init_notebook()
        self.init_buttons()
        self.set_default_size(gtk.gdk.screen_width()-100, gtk.gdk.screen_height()-150)
        self.stat_label=gtk.Label("IcePref2 "+_("Theme Designer")+" "+VERSION)
        self.vbox.pack_start(self.stat_label,0,0,2)
        self.set_default_size(gtk.gdk.screen_width()-150, gtk.gdk.screen_height()-280)
        self.vbox1.pack_start(self.vbox,1,1,0)
        self.show_all()
        if len(argv)>1:     
            if not argv[1].endswith(".py"): self.openThemeFile(argv[1])
            else:
                if len(argv)>2: self.openThemeFile(argv[2])
        
    def mainloop(self):
      gtk.mainloop()
    
    
    def determine_os(self):
      os_list = { 'freebsd'  : ['FreeBSD',  'BSD'],
            'pc-linux-gnu': ['Debian',   'Linux'],
            'mandrake' : ['Mandrake', 'Linux'],
            'redhat'   : ['RedHat',   'Linux'],
            'solaris'  : ['Solaris',  'Unix'] }
      self.os = 'Linux'
      self.distribution = 'Generic'
      for os_type in os_list.keys():
        if find( MACHTYPE, os_type ) != -1:
          self.distribution = os_list[os_type][0]
          self.os = os_list[os_type][1]

    
    def find_global_preferences(self):  
      self.global_preferences = ''  

    
    # Makes a copy of DEFAULTS in self.settings.
        
    def init_settings(self):
        self.settings = {}
        self.settings.update(DEFAULTS)

        
    def update_widgets(self, data=None):
        global CONFIG_THEME_PATH
        for widget in ORDER:
            try:
                if not widget in IMAGE_IGNORE: 
                    self.widget_dict[widget].set_value(self.settings[widget][VALUE])
                    if (widget=="DesktopBackgroundImage"):
                        self.widget_dict[widget].iprev.update_image(CONFIG_THEME_PATH+self.settings[widget][VALUE].replace("\"",""))
            except:
                pass
        for widget in self.widget_dict.keys():
            try:
                if not self.settings[widget][TYPE]==IMAGE_WIDGET: continue
                self.widget_dict[widget].set_value("\""+CONFIG_THEME_PATH+self.widget_dict[widget].get_data("my_image")+"\"")
            except:
                pass
    
    # Takes a line from the configuration file and returns the name of the
    # configuration option (name) and its value (value) as a tuple. This whole
    # thing could be dramatically simplified by the use of regex stuff (of which
    # I was ignorant at the time I originally wrote it.  Also, it is possible that
    # options which are commented out should still be read as the new default
    # preferences file for IceWM has _all_ options commented out.
            
    def analyze(self, string):
      
      whole_options = string.split("\n")
      
      all_options = {}

      for option in whole_options:
        if option.find("=")==-1: continue
        option = re.split('=', option)
        option_name = option[0].strip()
        option_value = option[1]
        
        # added 1.26.2003 - improved preferences file parsing
        if option_value.find("\n")>-1: 
            option_value=option_value[0:option_value.find("\n")]
        if option_value.find("\r")>-1: 
            option_value=option_value[0:option_value.find("\r")]
        if option_value.find("\t")>-1: 
            option_value=option_value[0:option_value.find("\t")]
        all_options[option_name] = option_value.strip()

      return all_options
        
    # This is a replacement for the old get current settings.  It is designed to use the self.analyze()
    # method rather than the primitive and clumsy self.trim() method.

    def get_current_settings(self, data=None):      
      # try to open preferences file.  Read whole file into variable and close.     
      try:
        f = open(CONFIG_FILE, 'r')
        contents = f.read()
        f.close()
      except:
        win = msg_warn(_('Warning'), _('Unable to read local preferences file!'))
        contents = ''
      # pass contents of preferences file to self.analyze() and store the result
      # in current.

      current = self.analyze( contents )

      # for each of the options in the dictionary current,
      # check for a matching option in self.settings.  If there
      # is no matching option, report it and record it.  If there is a matching option,
      # set that option in self.settings to the value from current.
      for option in current.keys():
        if option in ORDER:
            # added 1.26.2003 - dont erase default value if the current value is set to nothing (i.e. "")
            self.settings[option][VALUE] = current[option]


    # writes the contents of self.settings to the preferences file
            
    def save_current_settings(self, *data):
        # this stuff is to make a backup file
        global CONFIG_FILE
        global CONFIG_THEME_PATH
        if CONFIG_FILE=='': return 
        try: 
            f = open(CONFIG_FILE, 'r')
            old_pref = f.read()
            f.close()
            
            backup = CONFIG_FILE + '.backup-file'
            f = open(backup, 'w')
            f.write(old_pref)
            f.close()
        except:
            pass
        # this stuff saves the actual info
        try:          
            f = open(CONFIG_FILE, 'w')
            f.write('# This theme file automatically generated by IcePref2 Theme Designer %s (updated by PhrozenSmoke, March 2003) --your friendly pythonated config util (http://icesoundmanager.sourceforge.net).\n\n' % VERSION)
            for name in ORDER:
                string =''
                # this adds some descriptors depending upon the type of
                # option
                if self.settings[name][TYPE] == TOGGLE:
                    string =' # 0 / 1'
                elif self.settings[name][TYPE] == RANGE:
                    min = self.settings[name][MIN]
                    max = self.settings[name][MAX]
                    string = ' # ' + str(min) + '-' + str(max)
                # this sets up the comment descriptor, which is the same
                # as the label text.
                comment = '# ' + self.settings[name][TITLE] + '\n'
                if self.widget_dict.has_key(name):
                    line = name + '=' + self.widget_dict[name].get_value() + string + '\n'
                    # comment out desktop backgrounds that dont exist
                    # When a theme file designates a background image that doesnt exist, it won't load!
                    # Even worse, the person USING the theme cant load any other background either: 
                    # if the background doesn't exist, comment it out for now
                    if name=="DesktopBackgroundImage": 
                        if not os.path.exists(CONFIG_THEME_PATH+self.widget_dict[name].get_value()):
                            line="# "+line
                else:
                    if self.settings.has_key(name):  # added 5.5.2003, fall back to default
                        line = name + '=' + str(self.settings[name][VALUE]) + string + '\n'
                f.write(comment)
                f.write(line)
                f.write('\n')
            f.close()
            if str(data).find("nowarn")==-1: win = msg_info(_('Success'), _("IceWM Theme successfully saved to: ")+CONFIG_FILE)
        except:
            win = msg_err(_('Warning'), _("There was an ERROR saving your IceWM 'theme' file."))

    def restart(self, widget=None, data=None):
        if self.os=="BSD":
            win = msg_warn(_('Warning'), _("Restarting IceWM from IcePref is buggy on BSD systems.  You should restart IceWM yourself."))
            #return
        os.system('killall -HUP -q icewm &')
        os.system('killall -HUP -q icewm-gnome &')
    
    # this is the callback for the OK button
    
    def ok(self, data=None):
        self.save_current_settings()
        gtk.mainquit()
        
    # callback for the `about' menu option
        
    def about_cb(self, *data):
        commonAbout(_('About IcePref2 Theme Designer'), 'IcePref2 '+_('Theme Designer')+' '+VERSION+' \n\n'+_('An IceWM Theme Designer based on IcePref2')+'.\n\n'+_('This software is distributed under the GNU General Public License.'),1,"icepref2.png")
    
    # reloads the preferences file and sets all of the config widgets to
    # corresponding values.
            
    def set_file_settings(self, *data):
        self.init_settings()
        self.get_current_settings()
        self.update_widgets()
        
    # returns self.settings to the default values--doesn't work quite right
            
    def set_default_settings(self, *data):
        self.init_settings()
        self.update_widgets()

    # added 1.27.2003 - 2 methods to launch IceMe and IceSoundManager
    def run_iceme(self, *args):
        os.popen("iceme &> /dev/null &")
    def run_ism(self, *args):
        os.popen("IceSoundManager &> /dev/null &")
    def run_icecp(self, *args):
        os.popen("IceWMCP &> /dev/null &")
    def run_icepref(self, *args):
        os.popen("icepref &> /dev/null &")
    
    # creates the menubar
        
    def init_menu(self):
        menu_items = [  
        (_('/_File'), None, None, 0, '<Branch>'),
        (_('/File/tearoff1'), None, None, 0, '<Tearoff>'),
        (_('/File/_New Theme...'), '<control>N', self.newThemeAsk, 0, ''),
        (_('/File/_Open Theme...'), '<control>O', self.openTheme, 0, ''),

        # added 5.10.2003 - basic search functionality
        (_('/_File')+"/"+_('Search')+'...', '<control>B', icepref_search.runSearchDialog, 0, ''),

        (_('/File/sep1'), None, None, 0, '<Separator>'),
        (_('/File/_Save Theme'), '<control>S', self.save_current_settings, 0, ''),
        (_('/File/sep1'), None, None, 0, '<Separator>'),
        (_('/File/_Restart IceWM'), '<control>I', self.restart, 0, ''),
        (_('/File/sep2'), None, None, 0, '<Separator>'),
        (_("/_File")+"/_"+FILE_RUN,'<control>R', rundlg,421,""),
        (_('/_File')+"/_"+_("Check for newer versions of this program..."), '<control>U',checkSoftUpdate, 420, ''),
        (_('/File/sep2'), None, None, 0, '<Separator>'),
        (_('/File/_Exit IcePref2 Theme Designer'), '<control>Q', gtk.mainquit, 0, ''),
        (_('/_Category'), None, None, 400, '<Branch>'),
        (_('/_Category/tearoff2'), None, None, 401, '<Tearoff>'),
        (_('/_Tools'), None, None, 500, '<Branch>'),
        (_('/_Tools/Configure IceWM (requires IcePref2)...'), None, self.run_icepref, 509, ''),
        (_('/_Tools/Edit task bar menus (requires IceMe)...'), None, self.run_iceme, 501, ''),
        (_('/_Tools/Manage sound events (requires IceSoundManager)...'), None, self.run_ism, 503, ''),
        (_('/_Tools/Open Control Panel (requires IceControlPanel)...').replace("IceControlPanel","IceWMCP"), None, self.run_icecp, 502, ''),
        (_('/_Help'), None, None, 0, '<LastBranch>'),
        (_('/Help/_About IcePref2 Theme Designer...'), "F2", self.about_cb, 0, ''),
        (_("/_Help")+"/_"+APP_HELP_STR, "F4", displayHelp,5007, ""),
        (_("/_Help")+"/_"+CONTRIBUTORS+"...", "F3", show_credits,913, ""),
        (_("/_Help")+"/_"+BUG_REPORT_MENU+"...", "F5", file_bug_report,5007, ""),
        ]
                    
        ikeys=self.my_tabs.keys()
        ikeys.sort()
        for ii in ikeys:
            menu_items.append((_('/_Category/')+ii, None, self.switchTab, self.my_tabs[ii], ''))
        ag = gtk.AccelGroup()
        self.itemf = gtk.ItemFactory(gtk.MenuBar, '<main>', ag)
        self.add_accel_group(ag)
        self.itemf.create_items(menu_items)
        self.menubar = self.itemf.get_widget('<main>')
        self.vbox1.pack_start(self.menubar, expand=gtk.FALSE)
        self.menubar.show()

    # added 1.26.2003 - quick category tab switching - added by PhrozenSmoke
    def switchTab(self,*args):
        try:
            self.mynotebook.set_page(args[0])
        except:
            pass
        
        
    # creates the notebook and its friends.

    def init_notebook(self):
        notebook = gtk.Notebook()
        notebook.set_tab_pos(gtk.POS_TOP)
        notebook.set_scrollable(gtk.TRUE)
        notebook.show()

        self.vbox.pack_start(notebook,1,1,0)
        sep = gtk.HSeparator()
        sep.show()
        self.vbox.pack_start(sep, expand = gtk.FALSE)
        # sets up each of the tabs (one for each categorie of
        # configuration options

        #added 1.26.2003
        self.mynotebook=notebook
        notebook.set_sensitive(0) # disabled until a user either creates or opens a theme

        TABS.sort()
        for tab in TABS:
            label = gtk.Label(" "+tab[0]+" ")
            widgets = tab[1]
            # creates a scrolled window within the notepad page
            # for each tab.
            scroll = gtk.ScrolledWindow()
            #scroll.set_size_request(gtk.gdk.screen_width()-150, gtk.gdk.screen_height()-290)
            scroll.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_AUTOMATIC)
            scroll.show()
            notebook.append_page(scroll, label)
            
            vbox = gtk.VBox(spacing=SP)
            vbox.show()
            
            # Determines whether to use one column or two.
            # If there are more than 4 widgets, two columns
            # are used.
            
            num_widgets = len( widgets )
            
            if num_widgets > 4:
                rows = ( int( num_widgets / 2 ) + 1)
                cols = 2
            else:
                rows = num_widgets
                cols = 1
            if rows < 2: rows = 2
            
            # Creates a table for the widgets
            
            table = gtk.Table(rows, cols, gtk.FALSE)
            table.show()
            if cols==2: table.set_col_spacings(25)
            table.set_row_spacings(5)
            vbox.pack_start(table, gtk.FALSE, gtk.FALSE)
            scroll.add_with_viewport(vbox)
            
            # Add the widgets to the table.
            
            x, y = 0, 0 # x and y counters for the table
            ts=None
            for item in widgets:
                if not item in REQUIRE_PREVIEW: 
                    widget = self.widget_chooser(item)
                    # added 1.27.2003 - dynamic theme reading support
                    if (item=="ThemeAuthor"): 
                        if not ts==None: 
                            ts.author_entry=widget
                            ts.author_entry.entry.set_editable(1)
                            if not ts.iprev.entry_box==None:
                                ts.iprev.entry_box.pack_start(widget,0,0,0)
                                ts.iprev.entry_box.pack_start(gtk.Label("  "),1,1,7)
                    elif (item=="ThemeDescription"): 
                        if not ts==None: 
                            ts.desc_entry=widget
                            ts.desc_entry.entry.set_editable(1)
                            if not ts.iprev.entry_box==None:
                                ts.iprev.entry_box.pack_start(widget,0,0,0)
                                ts.iprev.entry_box.show_all()
                    elif (item=="Look"): 
                        if not ts==None: 
                            if not ts.iprev.entry_box==None:
                                ts.iprev.entry_box.pack_start(widget,0,0,3)
                                ts.iprev.entry_box.show_all()
                    elif (item=="Gradients"): 
                        if not ts==None: 
                            if not ts.iprev.entry_box==None:
                                ts.iprev.entry_box.pack_start(widget,0,0,3)
                                ts.iprev.entry_box.show_all()
                    else:
                        table.attach(widget, x,x+1, y,y+1)
                else: 
                    if item=="Theme": ipreview=ImagePreviewer(2,self.restart,self.save_current_settings)
                    elif item=="DesktopBackgroundImage": 
                        ipreview=ImagePreviewer(1, None,None)  #args[0].get_data("bg_entry").get_data("run-conv")(imfile)
                        ipreview.sc.set_data("bg_entry",widget)
                        widget.set_data("run-conv",ipreview.im_convert)
                        
                    else: ipreview=ImagePreviewer()
                    widget = self.widget_chooser(item,ipreview)
                    if item=="DesktopBackgroundImage": 
                        addDragSupport(widget.entry,setDrag)  # drag-n-drop support, 2.23.2003
                        addDragSupport(widget.button,setDrag)  # drag-n-drop support, 2.23.2003
                    if item=="Theme": ts=widget
                    else: table.attach(widget, x,x+1, y,y+1)
                    y = y + 1
                    table.attach(ipreview.scbox, x,x+1, y,y+1)
                self.widget_dict[item] = widget
                # if the y counter is greater than or equal to half
                # the number of widgets in a two column page,
                # move to the second column
                if y >= ((num_widgets / 2.0) - 1) and x == 0 and cols == 2:
                    y = 0
                    x = x + 1
                else:
                    y = y + 1
                    
    # accepts a configuration option as its argument and returns the
    # appropriate widget.  If a new type of widget is added, it will have
    # to be added here as well.
                    
    def widget_chooser(self, item,Image_prev=None):
        type = self.settings[item][TYPE]
        W_TITLE=_(self.settings[item][TITLE])
        W_VALUE=self.settings[item][VALUE]
        if type == TOGGLE:
            widget = Toggled(     W_TITLE,
                        W_VALUE)
        elif type == RANGE:
            widget = Range(     W_TITLE,
                        self.settings[item][MIN],
                        self.settings[item][MAX],
                        W_VALUE)
        elif type == FILE:
            widget = File(      W_TITLE,
                        W_VALUE,Image_prev)

        elif type == IMAGE_WIDGET:
            widget = ImageWidget(       W_TITLE,
                        W_VALUE,ImagePreviewer(0))
            widget.set_data("my_image",item)
        elif type == COLOR:
            widget = Color(     W_TITLE,
                        W_VALUE)
        elif type == FONT:
            widget = Font(      W_TITLE,
                        W_VALUE)
        elif type == ENTRY:
            widget = Entry(     W_TITLE,
                        W_VALUE)

        elif type == THEME:
            widget = ThemeSel(    W_TITLE,
                        W_VALUE,Image_prev)               

        elif type == OPTIONMENU:
            widget = OptionMenu(  W_TITLE,
                        W_VALUE)
        TIPS.set_tip(widget,W_TITLE+"\n["+item+"]")
        for ichild in widget.get_children():
            TIPS.set_tip(ichild,W_TITLE+"\n["+item+"]")
            try:
                for ichildd in ichild.get_children():
                    if not ichildd.__class__.__name__=="gtk.Button": TIPS.set_tip(ichildd,W_TITLE+"\n["+item+"]")
            except:
                pass
        return widget

    #   Theme opening stuff
    def makeNeededDirs(self,some_path): # make relevant theme directories if they dont exist
      try:
        os.makedirs(some_path)
      except:
        pass
      for ii in ['cursors','icons','taskbar','ledclock','mailbox']:
        try:
            os.makedirs(some_path+os.sep+ii+os.sep)
        except:
            pass

    def newThemeAsk(self,*args):
        w=gtk.Window()
        w.set_data("ignore_return",1)
        w.connect("key-press-event", keyPressClose)
        w.set_position(gtk.WIN_POS_CENTER)
        w.set_wmclass("icepref-td","IcePref-TD")
        w.connect('destroy', self.hideNewTheme)
        w.set_title(_("Create A New IceWM Theme")+"...")
        w.set_modal(0)
        v=gtk.VBox(0,0)
        v.set_border_width(5)
        v.pack_start(gtk.Label(_("Create A New IceWM Theme")),0,0,3)
        etable = gtk.Table (3, 4, 0)
        etable.attach( gtk.Label(_("Theme Name")+": "), 0, 1, 0, 1, (gtk.FILL), (0), 2, 0)
        etable.attach( gtk.Label(_("Theme Author")+": "), 0, 1, 1, 2, (gtk.FILL), (0), 2, 0)
        etable.attach( gtk.Label(_("Theme Description")+": "), 0, 1, 2, 3, (gtk.FILL), (0), 2, 0)
        etable.attach( gtk.Label(_("Directory")+": "), 0, 1, 3, 4, (gtk.FILL), (0), 2, 0)
        entries=[gtk.Entry(),gtk.Entry(),gtk.Entry(),gtk.Entry()]
        etable.attach(entries[0], 1, 3, 0, 1, (gtk.FILL), (0), 2, 0)
        etable.attach(entries[1], 1, 3, 1, 2, (gtk.FILL), (0), 2, 0)
        etable.attach(entries[2], 1, 3, 2, 3, (gtk.FILL), (0), 2, 0)
        etable.attach(entries[3], 1, 2, 3, 4, (gtk.FILL), (0), 2, 0)
        dirbutton=gtk.Button()
        try:
            dirbutton.add(  loadScaledImage(getPixDir()+"ism_load.png",24,24)  )
        except:
            dirbutton.add(gtk.Label(" "+_("Select")+"... "))
        TIPS.set_tip(dirbutton, _("Select"))
        dirbutton.set_data("cfg_path",entries[3])
        dirbutton.connect("clicked",self.newThemeFileSet)
        etable.attach(dirbutton, 2, 3, 3, 4, (gtk.FILL), (0), 2, 0)
        entries[3].set_text(getIceWMConfigPath()+"themes/")  # for my own personal easy use 
        v.pack_start(etable,1,1,3)
        hb=gtk.HBox(1,0)
        okbutt=gtk.Button(" "+_("OK")+" ")
        TIPS.set_tip(okbutt,_("OK"))
        cbutt=gtk.Button(" "+_("CANCEL")+" ")
        TIPS.set_tip(cbutt,_("CANCEL"))
        hb.pack_start(okbutt,0,0,0)
        hb.pack_start(gtk.Label(" "),1,1,0)
        hb.pack_start(cbutt,0,0,0)
        v.pack_start(hb,0,0,3)
        cbutt.connect("clicked",self.hideNewTheme)
        cbutt.set_data("window",w)
        okbutt.connect("clicked",self.newTheme)
        okbutt.set_data("window",w)
        okbutt.set_data("theme_name",entries[0])
        okbutt.set_data("theme_author",entries[1])
        okbutt.set_data("theme_desc",entries[2])
        okbutt.set_data("cfg_path",entries[3])
        w.set_default_size(375,-2)
        w.add(v)
        w.show_all()

    def file_cancel(self,*args):
        self.filewin.hide()
        self.filewin.destroy()
        self.filewin.unmap()

    def select_a_file(self, file_sel_cb,widget=None,title=_("Select a file...")):
        self.filewin = gtk.FileSelection(title)
        self.filewin.set_wmclass("icepref-td","IcePref-TD")
        self.filewin.ok_button.connect('clicked', file_sel_cb)
        value = self.my_last_file
        if not widget==None: 
            self.filewin.ok_button.set_data("cfg_path",widget.get_data("cfg_path"))
            if value=='': value=widget.get_data("cfg_path").get_text()
        self.filewin.cancel_button.connect('clicked', self.file_cancel)
        if value != '""':
            self.filewin.set_filename(value)
        self.filewin.set_modal(1)
        self.filewin.show()
        
    def newThemeFileSet(self,*args):
        self.select_a_file(self.newThemeFileSetCB,args[0],_("Select a directory"))

    def newThemeFileSetCB(self,*args):
        cfgp=self.filewin.get_filename()
        self.file_cancel()
        if not cfgp: return
        if cfgp=='': return
        self.my_last_file=cfgp
        try:
            args[0].get_data("cfg_path").set_text(cfgp[0:cfgp.rfind(os.sep)+1])
        except:
            pass

    def hideNewTheme(self,*args):
        try:
            args[0].get_data("window").hide()
            args[0].get_data("window").destroy()
            args[0].get_data("window").unmap()
        except:
            pass

    def newTheme(self,*args):
        tname=args[0].get_data("theme_name").get_text().strip().replace("\"","").replace("'","")
        tauth=args[0].get_data("theme_author").get_text().strip().replace("\"","").replace("'","")
        tdesc=args[0].get_data("theme_desc").get_text().strip().replace("\"","").replace("'","")
        cfgpath=args[0].get_data("cfg_path").get_text().strip().replace("\"","").replace("'","")
        if (tname=="") or (tauth=="") or (tdesc=="") or (cfgpath==""): 
            msg_warn("IcePref2",_("You must complete all the fields first."))
            return
        self.hideNewTheme(args[0])
        global CONFIG_FILE,CONFIG_THEME_PATH,THEME_PATH
        if not cfgpath.endswith(os.sep): cfgpath=cfgpath+os.sep
        CONFIG_THEME_PATH=cfgpath+tname+os.sep
        CONFIG_FILE=CONFIG_THEME_PATH+"default.theme"
        THEME_PATH=[CONFIG_THEME_PATH]
        self.makeNeededDirs(CONFIG_THEME_PATH)
        self.init_settings()
        self.widget_dict['ThemeAuthor'].set_value(tauth)
        self.widget_dict['ThemeDescription'].set_value(tdesc)
        # save a quick, basic theme
        self.save_current_settings("nowarn")
        self.get_current_settings()
        self.update_widgets()
        self.widget_dict['Theme'].iprev.update_image(CONFIG_THEME_PATH+"preview.jpg")
        self.set_status(CONFIG_FILE)
        self.mynotebook.set_sensitive(1)

    def openTheme(self,*args):
        self.select_a_file(self.openThemeCB)

    def openThemeCB(self,*args):
        cfgp=self.filewin.get_filename()
        self.file_cancel()
        if not cfgp: return
        if cfgp=='': return
        self.my_last_file=cfgp
        if not cfgp.endswith(os.sep): self.openThemeFile(cfgp)

    def openThemeFile(self,tfile):
        if len(tfile)==0: return # bug fix, 3.31.2003
        if not tfile.endswith(".theme"): 
            msg_err("IcePref2",_("The file does not appear to be an IceWM theme file."))
            return
        global CONFIG_FILE,CONFIG_THEME_PATH,THEME_PATH
        CONFIG_FILE=tfile
        CONFIG_THEME_PATH=tfile[0:tfile.rfind(os.sep)+1]
        THEME_PATH=[CONFIG_THEME_PATH]
        self.makeNeededDirs(CONFIG_THEME_PATH)
        self.init_settings()
        self.get_current_settings()
        self.update_widgets()
        self.widget_dict['Theme'].iprev.update_image(CONFIG_THEME_PATH+"preview.jpg")
        self.set_status(tfile)
        self.mynotebook.set_sensitive(1)

    def set_status(self,some_text):
        self.stat_label.set_text(str(some_text))


    # creates the buttons at the bottom of the window
    
    def init_buttons(self): 

        buttons = [
                    [_('Save'), self.save_current_settings,_('Save this theme')],
                    [_('Exit'), gtk.mainquit,_('Exit IcePref2 Theme Designer')]
                ]
                
        bbox = gtk.HButtonBox()
        bbox.set_layout(gtk.BUTTONBOX_SPREAD)
        bbox.show()
        self.vbox.pack_start(bbox, gtk.FALSE, gtk.FALSE, 0)
        for item in buttons:
            button = gtk.Button(item[0])
            button.connect('clicked', item[1])
            TIPS.set_tip(button,item[2])
            button.show()         
            bbox.pack_start(button, gtk.FALSE, gtk.FALSE, 0)

# makes the whole show run
def run():
    app = Application(sys.argv)
    hideSplash()
    app.mainloop()

if __name__ == '__main__':        
    run()
    hideSplash()





