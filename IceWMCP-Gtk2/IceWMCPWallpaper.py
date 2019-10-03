#! /usr/bin/env python

######################################
# IceWM Control Panel Wallpaper Settings
# 
# Copyright 2003 by Erica Andrews 
# (PhrozenSmoke@yahoo.com)
# http://icesoundmanager.sourceforge.net
# 
# A simple gtk.-based utility for setting the background 
# color and wallpaper settings in IceWM (written in 100% 
# Python).
# 
# This program is distributed under the GNU General
# Public License (open source).
# 
# WARNING: Some newer IceWM themes override the wallpaper 
# and background colors set here. If you see no changes, it 
# may be that your theme controls the background colors 
# and images.
#
# Ported to PyGTK-2 by: David Moore (djm6202@yahoo.co.nz) - June 2003
#
# DJM Note, addDragSupportColor all broken!!!
# Revert ColorButton colorsel etc!
#
#######################################

import os
import sys
import math
import gtk
from string import atoi

#set translation support
from icewmcp_common import *
_=translateCP   # from icewmcp_common.py


# import drag-n-drop support, 5.16.2003
import icewmcp_dnd
from icewmcp_dnd import *
initColorDrag()      # enable dnd support for 'color buttons'
addDragSupportColor=icewmcp_dnd.addDragSupportColor

from ColorButton import *

class wallwin:
    def __init__(self) :
        self.colvalue = "rgb:fb/f9/f5"
        self.dirvalue = ""
        self.preffile = ""
        self.imvalue  = ""
        self.version  = this_software_version
        self.imlist   = ( ".gif",
                          ".png",
                          ".jpeg",
                          ".jpe",
                          ".bmp",
                          ".xpm",
                          ".xbm",
                          ".ppm",
                          ".dib",
                          ".pnm",
                          ".tiff",
                          ".tif",
                          ".jpg",
                          ".ps")
        global WMCLASS
        WMCLASS="icewmcontrolpanelwallpaper"
        global WMNAME
        WMNAME="IceWMControlPanelWallpaper"
        wallwin=gtk.Window(gtk.WINDOW_TOPLEVEL)
        wallwin.set_wmclass(WMCLASS,WMNAME)
        wallwin.realize()
        wallwin.set_title("IceWM CP - "+_("Wallpaper Settings")+" "+self.version)
        wallwin.set_position(gtk.WIN_POS_CENTER)
        self.wallwin=wallwin

        menu_items = (
  (_('/_File'), None, None, 0, '<Branch>'),
  (_("/_File")+"/_"+FILE_RUN,'<control>R', rundlg,421,""),
  (_("/_File")+"/_"+_("Check for newer versions of this program..."), '<control>U', checkSoftUpdate,420,""),
  (_('/File/sep1'), None, None, 1, '<Separator>'),
  (_('/File/_Apply Changes Now...'), '<control>A', self.restart_ice, 0, ''),
  (_('/File/sep2'), None, None, 2, '<Separator>'),
  (_('/File/_Exit'), '<control>Q', self.doQuit, 0, ''),
  (_('/_Help'), None, None, 0, '<LastBranch>'),
  (_('/Help/_About...'), "F2", self.about_cb, 0, ''),
  (_("/_Help")+"/_"+APP_HELP_STR, "F4", displayHelp,5003, ""),
  (_("/_Help")+"/_"+CONTRIBUTORS+"...", "F3", show_credits,913, ""),
  (_("/_Help")+"/_"+BUG_REPORT_MENU+"...", "F5", file_bug_report,5003, ""),
                                )

        ag = gtk.AccelGroup()
        self.itemf = gtk.ItemFactory(gtk.MenuBar, '<main>', ag)
        wallwin.add_accel_group(ag)
        self.itemf.create_items(menu_items)
        self.menubar = self.itemf.get_widget('<main>')
        mainvbox1=gtk.VBox(0,1)
        mainvbox=gtk.VBox(0,1)
        mainvbox1.pack_start(self.menubar,0,0,0)
        mainvbox1.pack_start(mainvbox,1,1,0)
        self.menubar.show()
        mainvbox.set_border_width(4)
        mainvbox.set_spacing(2)
        mainvbox.pack_start(getImage(getBaseDir()+"icewmcp.png",DIALOG_TITLE),0,0,2)
        mainvbox.pack_start(gtk.Label(" "+_("Wallpaper Settings")+" "),0,0,3)   

        #color box
        colbox=gtk.HBox(0,0)
        colbox.set_spacing(10)
        colbox.set_border_width(3)
        colframe=gtk.Frame()
        colframe.set_label(_(" Desktop Color "))
        self.colentry=gtk.Entry()
        TIPS.set_tip(self.colentry,_(" Desktop Color ").strip())
        self.colbutton=  ColorButton()
        self.colbutton.connect("clicked", self.colok)
        #DJM!!!self.selectcol, self.colentry)
        TIPS.set_tip(self.colbutton,_(" Change... ").strip())
#        self.colchooser=gtk.Button()
#        addDragSupportColor(self.colchooser)
#        self.colchooser.set_data("colorbutton",self.colbutton)
        addDragSupportColor(self.colentry)
        self.colentry.set_data("colorbutton",self.colbutton)
#        try:
#            self.colchooser.add(  loadScaledImage(getPixDir()+"ism_load.png",24,24)  )
#        except:
#            self.colchooser.add(gtk.Label(_(" Change... ")))
#        self.colchooser.connect("clicked",self.colok)
#        TIPS.set_tip(self.colchooser,_(" Change... ").strip())
        colbox.pack_start(self.colentry,1,1,0)  
        colbox.pack_start(self.colbutton,0,0,0) 
#        colbox.pack_start(self.colchooser,0,0,0)        
        colbox.show_all()
        colframe.add(colbox)
        colframe.show_all()
        mainvbox.pack_start(colframe,0,0,0)

        #imabe box
        imbox=gtk.HBox(0,0)
        imbox.set_spacing(10)
        imbox.set_border_width(3)
        imframe=gtk.Frame()
        imframe.set_label(_(" Image "))

        leftbox=gtk.VBox(0,0)
        leftbox.set_spacing(3)
        self.sc=gtk.ScrolledWindow()
        self.clist=gtk.CList(1)
        self.clist.connect('select_row', self.clist_cb)
        self.clist.set_size_request(215,-1)
        self.sc.add(self.clist)
        leftbox.pack_start(self.sc,1,1,0)
        reloadbutt=gtk.Button(_(" Reload Images "))
        TIPS.set_tip(reloadbutt,_(" Reload Images ").strip())
        reloadbutt.connect("clicked",self.reloadImages)
        leftbox.pack_start(reloadbutt,0,0,0)
        imbox.pack_start(leftbox,0,0,0)
        leftbox.show_all()

        rightbox=gtk.VBox(0,0)
        rightbox.set_spacing(3)

        dirbox=gtk.HBox(0,0)
        dirbox.set_spacing(10)
        dirbox.set_border_width(3)
        dirframe=gtk.Frame()
        dirframe.set_label(_(" Directory "))
        self.direntry=gtk.Entry()
        TIPS.set_tip(self.direntry,_(" Directory ").strip())
        addDragSupport(self.direntry,self.setDrag) # drag-n-drop support, added 2.23.2003
        try:
            self.direntry.set_text(os.environ['HOME']+os.sep)
            self.preffile=getIceWMPrivConfigPath()+"preferences"
        except:
            pass
        self.dirchooser=gtk.Button()
        try:
            self.dirchooser.add(  loadScaledImage(getPixDir()+"ism_load.png",24,24)  )
        except:
            self.dirchooser.add(gtk.Label(_(" Select... ")))
        TIPS.set_tip(self.dirchooser,_(" Select... ").strip())
        self.dirchooser.connect("clicked",self.selectfile)
        addDragSupport(self.dirchooser,self.setDrag) # drag-n-drop support, added 2.23.2003
        dirbox.pack_start(self.direntry,1,1,0)  
        dirbox.pack_start(self.dirchooser,0,0,0)        
        dirbox.show_all()
        dirframe.add(dirbox)
        dirframe.show()
        rightbox.pack_start(dirframe,0,0,0)

        checkbox=gtk.HBox(0,0)
        checkbox.set_spacing(25)
        checkbox.set_border_width(2)
        self.checkbutton=gtk.RadioButton(None,_(" Centered "))  
        self.checkbutton2=gtk.RadioButton(self.checkbutton,_(" Tiled "))
        TIPS.set_tip(self.checkbutton,_(" Centered ").strip())
        TIPS.set_tip(self.checkbutton2,_(" Tiled ").strip())
        checkbox.pack_start(self.checkbutton,0,0,0)
        checkbox.pack_start(self.checkbutton2,0,0,0)
        checkbox.pack_start(gtk.Label("  "),1,1,0)
        edbutton=gtk.Button()
        edbox=gtk.HBox(0,0)
        edbox.set_border_width(2)
        edbox.set_spacing(3)
        edlab=gtk.Label(" "+_("Edit")+" ")
        try:
            edbox.pack_start(loadScaledImage(getPixDir()+"ism_edit2.png",21,21),0,0,0)
        except:
            pass
        edbox.pack_start(edlab,1,1,0)
        edbutton.add(edbox)
        TIPS.set_tip(edbutton,_("Edit with Gimp"))
        edbutton.connect("clicked",self.editWall)
        checkbox.pack_start(edbutton,0,0,0)
        rightbox.pack_start(checkbox,0,0,1)

        prevtable = gtk.Table (3, 3, 0)
        prevtabbox=gtk.HBox(0,0)
        prevtabbox.pack_start(gtk.Label(" "),1,1,0)
        sclay=gtk.Layout()
        self.gl=sclay
        self.cutoff1=356  #314
        self.cutoff2=267
        sclay.set_size_request( self.cutoff1,self.cutoff2)
        # top row
        prevtable.attach( gtk.Label(""), 0, 1, 0, 1, (gtk.EXPAND+gtk.FILL), (0), 0, 0)

        try:
            prevtable.attach( loadScaledImage(getPixDir()+"monitor_top1.png",411,22), 1, 2, 0, 1, (), (0), 0, 0)
        except:
            prevtable.attach( gtk.Label(""), 1, 2, 0, 1, (gtk.EXPAND+gtk.FILL), (0), 0, 0)

        prevtable.attach( gtk.Label(""), 2, 3, 0, 1, (gtk.EXPAND+gtk.FILL), (0), 0, 0)

        # middle row
        middlebox=gtk.HBox(0,0)
        prevtable.attach( gtk.Label(""), 0, 1, 1, 2, (gtk.EXPAND+gtk.FILL), (0), 0, 0)
        prevtable.attach( middlebox, 1, 2, 1, 2, (gtk.EXPAND+gtk.FILL), (0), 0, 0)
        prevtable.attach( gtk.Label(""), 2, 3, 1, 2, (gtk.EXPAND+gtk.FILL), (0), 0, 0)
        try:
            middlebox.pack_start( loadScaledImage(getPixDir()+"monitor_left1.png",27,268), 0, 0, 0)
        except:
            middlebox.pack_start( gtk.Label(""), 0, 0,0)

        evbox=gtk.EventBox()
        addDragSupport(sclay,self.setDrag)
        evbox.add(sclay)
        middlebox.pack_start( evbox, 1, 1, 0)

        try:
            middlebox.pack_start(loadScaledImage(getPixDir()+"monitor_right1.png",27,268), 0, 0, 0)
        except:
            middlebox.pack_start( gtk.Label(""), 0, 0, 0)


        # bottom row
        prevtable.attach( gtk.Label(""), 0, 1, 2, 3, (gtk.EXPAND+gtk.FILL), (0), 0, 0)
        try:
            prevtable.attach( loadScaledImage(getPixDir()+"monitor_bottom1.png",411,29), 1, 2, 2, 3, (), (0), 0, 0)
        except:
            prevtable.attach( gtk.Label(""), 1, 2, 2, 3, (gtk.EXPAND+gtk.FILL), (0), 0, 0)
        prevtable.attach( gtk.Label(""), 2, 3, 2, 3, (gtk.EXPAND+gtk.FILL), (0), 0, 0)


        prevtabbox.pack_start(prevtable,0,0,0)
        prevtabbox.pack_start(gtk.Label(" "),1,1,0)
        rightbox.pack_start(prevtabbox,1,1,0)

        imbox.pack_start(rightbox,1,1,0)
        rightbox.show_all()

        imbox.show_all()
        imframe.add(imbox)
        imframe.show_all()
        mainvbox.pack_start(imframe,1,1,0)
        self.status=gtk.Label("  ")
        mainvbox.pack_start(self.status,0,0,2)
        buttonbox=gtk.HBox(0,0)
        buttonbox.set_homogeneous(1)
        buttonbox.set_spacing(4)
        okbutt=gtk.Button(_("Ok"))
        applybutt=gtk.Button(_("Apply"))
        cancelbutt=gtk.Button(_("Close"))
        TIPS.set_tip(okbutt,_("Ok"))
        TIPS.set_tip(applybutt,_("Apply"))
        TIPS.set_tip(cancelbutt,_("Close"))
        buttonbox.pack_start(okbutt,1,1,0)
        buttonbox.pack_start(applybutt,1,1,0)
        buttonbox.pack_start(cancelbutt,1,1,0)

        okbutt.connect("clicked",self.ok_quit)
        applybutt.connect("clicked",self.restart_ice)
        cancelbutt.connect("clicked",self.doQuit)
        mainvbox.pack_start(buttonbox,0,0,0)

        wallwin.add(mainvbox1)
        wallwin.connect("destroy",self.doQuit)
        wallwin.set_default_size(690, -1)
        self.checkbutton.set_active(1)
        self.set_prefs()
        self.reloadImages()
        self.displayImage()
        self.checkbutton.connect("toggled",self.displayImage)
        wallwin.show_all()


    def setDrag(self,*args): # drag-n-drop support, added 2.23.2003
        drago=args
        if len(drago)<7: 
            args[0].drag_highlight()
            args[0].grab_focus()
        else: 
            #print str(drago[4].data)  # file:/root/something.txt
            try:
                imfile=""+str(drago[4].data).replace("\r\n","").replace("\x00","").replace("file:","").strip()
                imf=imfile[imfile.rfind(os.sep)+1:len(imfile)].strip()
                imdir=imfile[0:imfile.rfind(os.sep)+1].strip()
                self.direntry.set_text(imdir)
                self.imvalue=imfile
                self.displayImage()
                self.reloadImages()
            except: 
                pass
            args[0].drag_unhighlight()

    def about_cb(self,*args) :
                commonAbout(_("Wallpaper Settings")+" "+self.version,  DIALOG_TITLE+": "+_("Wallpaper Settings")+" "+self.version+"\n\n"+_("A simple gtk.-based utility for setting the background color\nand wallpaper settings in IceWM (written in 100% Python).\nThis program is distributed under the GNU General\nPublic License (open source).\n\nWARNING: Some newer IceWM themes override the wallpaper and\nbackground colors set here. If you see no changes, it may be that your\ntheme controls the background colors and images."))

    def ok_quit(self,*args) :
        self.restart_ice()
        self.doQuit()

    def restart_ice(self,*args) :
        i=self.save_prefs()
        if i==1:
            os.system('killall -HUP -q icewm &')
            os.system('killall -HUP -q icewm-gnome &')
        
    def save_prefs(self,*args) :
        g=self.get_prefs()
        try: # make a backup of the preferences file
            os.popen("cp -f "+self.preffile+" "+self.preffile+".backup-file")
        except:
            msg_err(DIALOG_TITLE,_("Error backing up IceWM preferences to:\n")+self.preffile+".backup-file")
            return 0

        try:
            b_write=0
            bc_write=0
            wc_write=0
            b="DesktopBackgroundImage=\""+self.imvalue+"\""
            if (len(self.imvalue)==0) or (self.imvalue=="[NONE]"):
                b="#"+b
            bc="DesktopBackgroundColor=\""+self.colentry.get_text().strip()+"\""
            if len(bc)==0: bc="DesktopBackgroundColor=\"rgb:FF/FF/FF\""
            wc="DesktopBackgroundCenter="+str(self.checkbutton.get_active())
            f=open(self.preffile,"w")
            for ii in g:
                if ii.find("DesktopBackgroundImage=")>-1:
                    f.write(b+"\n")
                    b_write=1
                elif ii.find("DesktopBackgroundColor=")>-1:
                    f.write(bc+"\n")
                    bc_write=1
                elif ii.find("DesktopBackgroundCenter=")>-1:
                    f.write(wc+"\n")
                    wc_write=1
                else:
                    f.write(ii+"\n")
            if b_write==0: f.write(b+"\n")
            if bc_write==0: f.write(bc+"\n")
            if wc_write==0: f.write(wc+"\n")
            f.flush()
            f.close()
        except:
            msg_err(DIALOG_TITLE,_("Error while saving IceWM preferences to:\n")+self.preffile)
            return 0
        return 1


    def set_prefs(self,*args) :
        g=self.get_prefs()
        cent="1"
        for ii in g:
            if ii.find("DesktopBackgroundImage=")>-1:
                while ii.strip().find("#")>5:
                    ii=ii[0:ii.rfind("#")].strip()
                self.imvalue=ii.replace("#","").replace("DesktopBackgroundImage=","").replace("\"","").strip()
            if ii.find("DesktopBackgroundColor=")>-1:
                while ii.strip().find("#")>5:
                    ii=ii[0:ii.rfind("#")].strip()
                self.colvalue=ii.replace("#","").replace("DesktopBackgroundColor=","").replace("\"","").strip()
            if ii.find("DesktopBackgroundCenter=")>-1:
                while ii.strip().find("#")>5:
                    ii=ii[0:ii.rfind("#")].strip()
                cent=ii.replace("#","").replace("DesktopBackgroundCenter=","").replace("\"","").strip()
        #self.checkbutton.set_active(str(cent)=="1")
        #print cent
        self.checkbutton2.set_active(str(cent)=="0")
        self.colentry.set_text(self.colvalue)
#DJM        self.colbutton.set_value(self.colvalue)
        self.colbutton.setValue(self.colvalue)
        if self.imvalue.find(os.sep)>-1:
            self.dirvalue=self.imvalue[0:self.imvalue.rfind(os.sep)+1]
            self.direntry.set_text(self.dirvalue)

    def get_prefs(self,*args) :
        try:
            f=open(self.preffile)
            ff=f.read().split("\n")
            f.close()
            return ff
        except:
            pass
        try: # this happens if we got an empty prefs file - try system prefs
            f=open(getIceWMConfigPath()+"preferences")
            ff=f.read().split("\n")
            f.close()
            return ff
        except:
            pass
        return []

    def doQuit(self,*args) :
        gtk.mainquit()


    def colcancel(self,*args):
        self.colwin.destroy()
        self.colwin.unmap()

    def colok(self, *args):
        color = self.colbutton.getRgb()

        r = "%02x" % color[0]  
        g = "%02x" % color[1]  
        b = "%02x" % color[2]  
        print r,g,b
            
        self.colvalue = 'rgb:' + r + '/' + g + '/' + b
        self.colentry.set_text(self.colvalue)
#DJM        self.colbutton.ok(raw_values)

    def selectfile(self, *args):
        self.filewin = gtk.FileSelection(_("Select a file..."))
        self.filewin.set_position(gtk.WIN_POS_CENTER)
        self.filewin.ok_button.connect('clicked', self.fileok)
        self.filewin.cancel_button.connect('clicked', self.filecancel)
        value = self.direntry.get_text().strip()
        if value != '""':
            self.filewin.set_filename(value)
        self.filewin.set_modal(gtk.TRUE)
        self.filewin.show()
        
    def fileok(self, *args):
        self.dirvalue = self.filewin.get_filename()
        if self.dirvalue.find(os.sep)>-1:
            self.dirvalue=self.dirvalue[0:self.dirvalue.rfind(os.sep)+1]
        self.filewin.hide()
        self.filewin.destroy()
        self.direntry.set_text(self.dirvalue)
        self.reloadImages()

    def filecancel(self,*args):
        self.filewin.destroy()
        self.filewin.unmap()

    def editWall(self, *args):  # added 5.10.2003, feature to launch Gimp for editing
        if self.imvalue==None: return
        if self.imvalue=='': return
        os.popen("gimp-remote -n "+str(self.imvalue).replace(" ","\\ ")+" &")

    def clist_cb(self, widget, row, col, event):
        self.imvalue = widget.get_row_data(row)
        self.status.set_text(self.imvalue[self.imvalue.rfind(os.sep)+1:len(self.imvalue)])
        self.displayImage()

    def displayImage(self,*args):
        if not len(self.imvalue)>0: return
        if self.checkbutton.get_active(): self.update_image(self.imvalue,0)
        else: self.update_image(self.imvalue,1)
        
    def reloadImages(self, *args):
        self.clist.freeze()
        self.clist.clear()
        self.clist.append(["[NONE]"])
        self.clist.set_row_data(0,"[NONE]")
        selrow=0
        try:
            d=self.direntry.get_text().strip()
            if not d.endswith(os.sep): d=d+os.sep
            plist=os.listdir(d)
            usable=[]
            for ii in plist:
                if not os.path.isdir(d+ii):
                    ifile=ii
                    for yy in self.imlist:
                            if ifile.lower().find(yy)>-1:
                                    usable.append(ifile)
                                    break
            usable.sort()
            inum=1
            for ii in usable:
                self.clist.append([ii])
                self.clist.set_row_data(inum,d+ii)
                if str(d+ii)==self.imvalue: selrow=inum
                inum=inum+1
        except:
            pass
        self.clist.thaw()
        self.clist.select_row(selrow,0)

    def update_image(self,image_file,tile=0):
        print "image_file", image_file
        for ii in self.gl.get_children():
            try:
                self.gl.remove(ii)
                ii.destroy()
                ii.unmap()
            except:
                pass
#DJM        try:                    
        if 1:
            stats=image_file[image_file.rfind(os.sep)+1:len(image_file)]
            myim=gtk.Image()
            myim.set_from_file(str(image_file).strip())
#DJM WTF            myim.changed_image() # disabled cached regurgitation
#            myim.render()
#            pixtemp=myim.make_pixmap()
#            dims=pixtemp.size_request()
            dims=myim.size_request()
#            print "dims",dims
#            pixtemp.destroy()
#            pixtemp.unmap()
            cutoff1=self.cutoff1
            cutoff2=self.cutoff2
            aspect=float(dims[0])/float(dims[1])
            sug_x=cutoff1

            # This feature tries to make the 'preview' monitor emulate the
            # typical resolution on the user's computer, using gtk's
            # gtk.gdk.screen_width() and gtk.gdk.screen_height()

            # float scale_aspect comes from  cutoff1/1024: reduce images and
            # pretend to be a  1024x768 screen if the image is smaller than the
            # screen width gtk. reports

            scale_aspect=float(cutoff1/1024.00000)
            if dims[0] < gtk.gdk.screen_height(): 
                sug_x = int( math.floor(float(dims[0]*scale_aspect ) ))
            else:
                if tile==1:
                        if dims[0]<gtk.gdk.screen_height(): 
                            sug_x=int( math.floor(sug_x*scale_aspect)  )
                        else:
                            stats=stats+"  ("+_("Too large to tile")+")" 
            
            print "sug_x:  "+str(sug_x)
            sug_y=int( math.floor( sug_x/aspect)   ) 
            scale_aspect_y=float(float(1024/768.000) / float(cutoff1/(cutoff2*1.0000))  )
            print str(scale_aspect_y) 
            sug_y= int( math.ceil(sug_y * scale_aspect_y)  )
            print str(sug_y)
            xput=0
            yput=0
            xlim=sug_x
            ylim=sug_y
            if tile==1:
                xlim=cutoff1
                ylim=cutoff2
            while yput<ylim:
                if xput>=xlim: 
                        xput=0
                        yput=yput+sug_y
#DJM             myim_real=myim.clone_scaled_image(sug_x,sug_y)

#DJM                myim_real.render()
#DJM                pixreal=myim_real.make_pixmap()
                tmppix=myim.get_pixbuf()
#DJM                print "scale to ", sug_x, sug_y
                tmppix2 = tmppix.scale_simple(sug_x, sug_y, gtk.gdk.INTERP_BILINEAR)
                pixreal = gtk.Image()
                pixreal.set_from_pixbuf(tmppix2)
                TIPS.set_tip(pixreal,str(image_file).strip())
                pixreal.show()
                self.gl.put(pixreal,xput,yput)
                if tile==0: break
                xput=xput+sug_x
            stats=stats+"   ["+str(dims[0])+" x "+str(dims[1])+"]"
            self.status.set_text(stats)
#        except:
#            lab=gtk.Label(" "+_("SORRY:\n Image could not\n be previewed."))
#            TIPS.set_tip(lab,str(image_file).strip())
#            lab.show()
#            self.gl.put(lab,2,2)
        TIPS.set_tip(self.gl,str(image_file).strip())



def run() :
    wallwin()
#    hideSplash()
    gtk.mainloop()

if __name__== "__main__" :
    run()





