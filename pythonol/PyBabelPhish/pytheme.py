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
import glob,sys

windows_for_update=[]

def getDefaultThemeFile():
        return sys.argv[0][0:sys.argv[0].rfind(os.sep)+1]+"theme"+os.sep+"gtk"+os.sep+"gtkrc-default"

def getPersonalThemeFile():
        return sys.argv[0][0:sys.argv[0].rfind(os.sep)+1]+"theme"+os.sep+"gtk"+os.sep+"gtkrc-personal"

def getAvailableThemeFile():
    try:
        if os.path.exists(getPersonalThemeFile()):
            return getPersonalThemeFile()
        return getDefaultThemeFile()
    except:
        return getDefaultThemeFile()

def installTheme(some_widget):	
	if wine_friendly==0:
                # as of verson 2.1, we skin everything with a Gtk theme
                # unless we are using Wine
		try:
                        if some_widget: some_widget.reset_rc_styles()
                        rc_reparse_all()
                        rc_parse(getAvailableThemeFile())
                        some_widget.ensure_style()
                        for ii in some_widget.get_children():
                            ii.reset_rc_styles()
                            ii.ensure_style()
		except:
			pass

def setWindowForUpdate(somewin):
    windows_for_update.append(somewin)

def applyThemeChange():
    for somewin in windows_for_update:
        installTheme(somewin)


def getThemeList():
    theme_path=sys.argv[0][0:sys.argv[0].rfind(os.sep)+1]+"theme"+os.sep+"gtk"+os.sep
    themes=[]
    try:
        tlist=glob.glob(theme_path+"*")
        for ii in tlist:
            if os.path.isdir(ii):
                if os.path.exists(ii+os.sep+"gtk-2.0"+os.sep+"gtkrc"):
                    themes.append(ii.replace(theme_path,""))
    except:
        pass
    themes.sort()
    themes=["[DEFAULT]"]+themes
    return themes

def getRCThemeData(theme,font):
    if theme=="[DEFAULT]":
        return "include \"basic-look-gtkrc\"\n\ninclude \"font-gtkrc\""
    else:
        osep=os.sep
        osep=osep.replace("\\","\\\\")
        gtkr= "include \"."+osep+theme+osep+"gtk-2.0"+osep+"gtkrc\"\n\n"
        gtkr=gtkr+"style \"personal_font\" {font_name=\""+font+"\" }\nclass \"GtkWidget\" style \"personal_font\"\nwidget_class \"*.GtkProgress*\" style \"personal_font\"\nwidget_class \"GtkWindow\" style \"personal_font\"\nwidget_class \"*\" style \"personal_font\"\nclass \"*\" style \"personal_font\"\n"
        return gtkr

def setGtkTheme(theme,font):
    try:
        gf=open(getPersonalThemeFile(),"w")
        gf.write(getRCThemeData(theme,font))
        gf.close()
    except:
        pass
    applyThemeChange()

def cancelFontSel(*args):
    try:
        args[0].get_data("window").destroy()
    except:
        pass

def acceptFontSel(*args):
    try:
        sf=str(args[0].get_data("window").get_font_name())
        if sf=="": sf="Arial 10"
        args[0].get_data("entry").set_text(sf)
    except:
        pass
    try:
        args[0].get_data("window").destroy()
    except:
        pass

def showFontSel(*args):
    fsel=FontSelectionDialog(to_utf8(filter("Select A Font...")))
    fsel.set_position(WIN_POS_CENTER)
    set_window_icon(fsel,pybabel_icon)
    fsel.set_preview_text(to_utf8(filter("Spanish Language Learning Software 123567890")))
    fsel.ok_button.set_data("window",fsel)
    fsel.set_font_name(args[0].get_data("entry").get_text())
    fsel.cancel_button.set_data("window",fsel)
    fsel.ok_button.set_data("entry",args[0].get_data("entry"))
    fsel.set_wmclass(DIALOG_WMC,DIALOG_WMN)
    fsel.set_modal(1)
    fsel.cancel_button.connect("clicked",cancelFontSel)
    fsel.ok_button.connect("clicked",acceptFontSel)
    fsel.set_data("window",fsel)
    fsel.connect("destroy",cancelFontSel)
    fsel.show_all()


def setGtkThemeCB(*args):
    try:
        theme=str(args[0].get_data("theme").get_text())
        font=str(args[0].get_data("font").get_text())
        setGtkTheme(theme,font)
    except:
        pass
    try:
        args[0].get_data("window").destroy()
    except:
        pass
    showThemeSelector()

def showThemeSelector(*args):
    twin=Window(WINDOW_TOPLEVEL)
    twin.set_position (WIN_POS_CENTER)
    twin.set_title("Theme Selector")
    set_window_icon(twin,pybabel_icon)
    twin.set_wmclass(DIALOG_WMC,DIALOG_WMN)
    twin.set_data("ignore_return",1)
    twin.connect("key-press-event", keyPressClose)
    twin.set_modal(1)
    tbox=VBox(0,0)
    tbox.set_border_width(4)
    tbox.set_spacing(4)
    explain=Label("Easily customize the look of this application.\n\nFont settings used here do not change the font used in the text boxes. You may need to restart this application afterwards to see all of the changes. If you have your own Gtk-2 themes, you may copy or link them into: '"+os.path.abspath(sys.argv[0][0:sys.argv[0].rfind(os.sep)+1]+"theme"+os.sep+"gtk"+os.sep)+"' to use them. Linux/Unix users: You may need to be Root to use this feature depending on where this application is installed.")
    explain.set_alignment(0.0,0.5)
    explain.set_line_wrap(WRAP_WORD)
    tbox.pack_start(explain,0,0,2)
    tbox.pack_start(HSeparator(),0,0,1)
    tbox.pack_start(Label("Theme:"),0,0,0)
    tcombo=Combo()
    tcombo.entry.set_editable(0)
    tcombo.set_popdown_strings(getThemeList())
    tcombo.entry.set_text("[DEFAULT]")
    tbox.pack_start(tcombo,0,0,0)
    tbox.pack_start(Label("  "),0,0,2)
    tbox.pack_start(Label("Font:"),0,0,0)

    fbox=HBox(0,0)
    fbox.set_spacing(2)
    tbox.pack_start(fbox,0,0,0)
    tentry=Entry()
    tentry.set_editable(0)
    tentry.set_text("Arial 10")
    fbox.pack_start(tentry,1,1,0)
    tbutton=Button()
    tbutton.add(getImage(STOCK_SELECT_FONT,"Select..."))
    fbox.pack_start(tbutton,0,0,0)
    TIPS.set_tip(tbutton,"Select A Font...")
    tbutton.connect("clicked",showFontSel)
    tbutton.set_data("entry",tentry)    
    tbox.pack_start(Label("  "),0,0,2)
    buttbox=HBox(1,0)
    buttbox.set_spacing(4)
    okbutton=Button("Apply Changes")
    TIPS.set_tip(okbutton,"Apply The Theme Changes")
    okbutton.set_data("window",twin)
    okbutton.set_data("theme",tcombo.entry)
    okbutton.set_data("font",tentry)
    okbutton.connect("clicked",setGtkThemeCB)
    cancelbutton=Button("Close")
    TIPS.set_tip(cancelbutton,"Close This Window")
    cancelbutton.set_data("window",twin)
    cancelbutton.connect("clicked",cancelFontSel)
    buttbox.pack_start(okbutton,1,1,0)
    buttbox.pack_start(cancelbutton,1,1,0)
    tbox.pack_start(buttbox,0,0,0)
    twin.add(tbox)
    twin.set_data("window",twin)
    twin.connect("destroy",cancelFontSel)
    twin.show_all()
    
