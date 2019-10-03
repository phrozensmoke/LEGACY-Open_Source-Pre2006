######################################
# IceWM Control Panel  - IcePref2 Search Module
# 
# Copyright 2003 by Erica Andrews 
# (PhrozenSmoke@yahoo.com)
# http://icesoundmanager.sourceforge.net
# 
# A simple module for searching preferences in IcePref2
# and IcePref2 Theme Designer, non-executable script
#
# This program is distributed under the GNU General
# Public License (open source).
# 
# Ported to PyGTK-2 by: David Moore (djm6202@yahoo.co.nz) - March 2003
#
#######################################
import gtk

#set translation support
from icewmcp_common import *
_=translateP   # from icewmcp_common.py

global TABS
global DEFAULTS
TABS=None
DEFAULTS=None
        
def hideSearch(*args):
                try:
                        args[0].get_data("window").hide()
                        args[0].get_data("window").destroy()
                        args[0].get_data("window").unmap()
                except:
                        pass

def showSearchWindow(pair_list):
        swin=gtk.Window(gtk.gtk.WINDOW_TOPLEVEL)
        swin.set_wmclass("icepref","IcePref")
        swin.realize()
        swin.set_title("IcePref2:  "+_("Search"))
        swin.set_position(gtk.WIN_POS_CENTER)        
        mainvbox=gtk.VBox(0,0)
        mainvbox.set_border_width(4)
        mainvbox.set_spacing(3)
        mainvbox.pack_start(getImage(getBaseDir()+"icepref2.png","IcePref2"),0,0,0)
        mainvbox.pack_start(gtk.Label(_("Search Results")),0,0,2)

        sc=gtk.ScrolledWindow()
        clist=gtk.CList(2,[" "+_('Section')," "+_('Property')])
        TIPS.set_tip(clist,_("Search Results"))
        clist.set_usize(425,250)
        clist.set_column_width(0,150)
        clist.set_column_width(1,600)
        sc.add(clist)
        for ii in pair_list:
                clist.append([ii[0],ii[1]])

        mainvbox.pack_start(sc,1,1,2)        

        mainvbox.pack_start(gtk.HSeparator(),0,0,2)
        closebutt=gtk.Button("  "+DIALOG_CLOSE+"  ")
        TIPS.set_tip(closebutt,DIALOG_CLOSE)
        closebutt.set_data("window",swin)
        closebutt.connect("clicked",hideSearch)
        mainvbox.pack_start(closebutt,0,0,3)
        swin.add(mainvbox)
        swin.connect("key-press-event", keyPressClose)
        swin.set_data("window",swin)
        swin.connect("destroy",hideSearch)
        swin.show_all()

def search_tabs(keyword, tab_list, default_dict):
        pair_list=[]
        for ii in tab_list:
                try:
                        ilist=ii[1]
                        tabname=ii[0]
                        for mj in ilist:
                                if (mj.lower().find(keyword.lower())>-1) or (_(mj).lower().find(keyword.lower())>-1):
                                        if default_dict.has_key(mj): 
                                                prop=default_dict[mj][2]
                                                pair_list.append([_(tabname), _(prop)])

                                if (default_dict[mj][2].lower().find(keyword.lower())>-1) or (_(default_dict[mj][2]).lower().find(keyword.lower())>-1):
                                        if default_dict.has_key(mj): 
                                                prop=default_dict[mj][2]
                                                pair_list.append([_(tabname), _(prop)])
                except:
                        pass
        if len(pair_list)>0:
                showSearchWindow(pair_list)
        else:
                msg_info(DIALOG_TITLE,_("No matches found.")+"\n\n'"+str(keyword)+"'")        


# GUI dialogs shared by IcePref Theme Designer and IcePref2

def runSearch(*args):
                # added 5.10.2003 - basic search functionality
                if len(args)<1: return
                widget=args[0]
                try:
                        if widget.get_data("entry").get_text().strip()=='': return
                except:
                        pass
                try:
                        widget.get_data("window").hide()
                        widget.get_data("window").destroy()
                        widget.get_data("window").unmap()
                except:
                        pass
                try:
                        # if data item 'entry' is not set (just closing window) this method will 
                        # simply throw a CAUGHT exception and return quietly

                        keyword=widget.get_data("entry").get_text().strip()
                        search_tabs(keyword,TABS,DEFAULTS)
                except:
                        pass


def runSearchDialog(*args):
                # added 5.10.2003 - basic search functionality
                swin=gtk.Window(gtk.gtk.WINDOW_TOPLEVEL)
                swin.set_wmclass("icepref","IcePref")
                swin.realize()
                swin.set_title("IcePref2:  "+_("Search"))
                swin.set_position(gtk.WIN_POS_CENTER)        
                mainvbox=gtk.VBox(0,0)
                mainvbox.set_border_width(6)
                mainvbox.set_spacing(3)                
                mainvbox.pack_start(gtk.Label(_("Enter a search word")+":"),0,0,3)
                sentry=gtk.Entry()
                sentry.set_usize(270,-2)
                mainvbox.pack_start(sentry,0,0,2)
                TIPS.set_tip(sentry,_("Enter a search word"))
                mainvbox.pack_start(gtk.HSeparator(),0,0,1)
                closebutt=gtk.Button("  "+_("Search")+"  ")
                TIPS.set_tip(closebutt,_("Search"))
                closebutt.set_data("entry",sentry)
                closebutt.set_data("window",swin)
                mainvbox.pack_start(closebutt,0,0,6)
                swin.add(mainvbox)
                swin.set_data("ignore_return",1)
                swin.connect("key-press-event", keyPressClose)
                swin.set_data("window",swin)
                closebutt.connect("clicked",runSearch)
                swin.connect("destroy",runSearch)
                swin.set_modal(1)
                swin.show_all()






