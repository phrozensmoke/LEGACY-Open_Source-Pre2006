#! /usr/bin/python

##############################
#  IceWMCP - Run Dialog
#  
#  Copyright 2003 by Erica Andrews
#  PhrozenSmoke@yahoo.com
#  http://icesoundmanager.sourceforge.net
#  
#  A simple gtk.-based cursor 'Run' dialog.
#  
#  This program is distributed free
#  of charge (open source) under the GNU
#  General Public License
#
# Ported to PyGTK-2 by: David Moore (djm6202@yahoo.co.nz) - July 2003
##############################

import os, copy,gtk
import gtk


#set translation support
from icewmcp_common import *
_=translateCP   # from icewmcp_common.py

class runwindow:
  def __init__ (self) :
    runwindow = gtk.Window (GTK.WINDOW_TOPLEVEL)
    runwindow.set_wmclass("icewmcontrolpanel","IceWMControlPanel")
    self._root = runwindow
    tips=gtk.Tooltips()
    self.rcommands=[]
    self.cmd_file=".icewmcp_gtkruncmd"
    self.last_file="/usr/X11R6/bin/gedit"
    runwindow.realize()
    runwindow.set_title (_("Run a program")+"...")
    runwindow.set_position (GTK.WIN_POS_NONE)
    runwindow.set_default_size(415,-2)
    self.runwindow = runwindow
    vbox1 = gtk.VBox (0, 0)
    vbox1.set_border_width ( 5)
    self.vbox1 = vbox1
    vbox1.pack_start(getImage(getBaseDir()+"icewmcp.png",DIALOG_TITLE),0,0,2)
    hbox1 = gtk.HBox (1, 0)
    self.hbox1 = hbox1
    cmdlab = gtk.Label (_("Command to run")+":")
    cmdlab.set_justify (GTK.JUSTIFY_LEFT)
    cmdlab.set_alignment ( 0.06, 0.5)
    self.cmdlab = cmdlab
    hbox1.pack_start ( cmdlab, 1, 1, 0)
    spacer1 = gtk.Label ("  ")
    self.spacer1 = spacer1
    hbox1.pack_start ( spacer1, 1, 1, 0)
    vbox1.pack_start ( hbox1, 1, 1, 0)
    hbox2 = gtk.HBox (0, 0)
    self.hbox2 = hbox2
    runcombo = gtk.Combo ()
    self.runcombo = runcombo
    runentry = runcombo.entry
    self.runentry = runentry
    hbox2.pack_start ( runcombo, 1, 1, 9)
    browsebutt = gtk.Button(" "+_("Browse...")+" ")
    tips.set_tip(browsebutt,_("Select A Program"))
    browsebutt.connect("clicked",self.showFileSel)
    self.browsebutt = browsebutt
    hbox2.pack_start ( browsebutt, 0, 0, 0)
    vbox1.pack_start (hbox2, 1, 1, 2)
    hbox3 = gtk.HBox (1, 0)
    hbox3.set_border_width ( 4)
    self.hbox3 = hbox3
    runbutt = gtk.Button(_("Run"))
    tips.set_tip(runbutt,_("Run the selected command"))
    self.runbutt = runbutt
    self.runbutt.connect("clicked",self.runCommand)
    hbox3.pack_start ( runbutt, 1, 1, 0)
    spacer2 = gtk.Label ("  ")
    self.spacer2 = spacer2
    hbox3.pack_start ( spacer2, 0, 0, 0)
    cancelbutt = gtk.Button(_("Close"))
    tips.set_tip(cancelbutt,_("Close and exit"))
    cancelbutt.connect("clicked",self.quitit)
    self.cancelbutt = cancelbutt
    hbox3.pack_start ( cancelbutt, 1, 1, 0)
    vbox1.pack_start ( hbox3, 1, 1, 5)
    runwindow.add(vbox1)
    runwindow.connect("destroy",self.quitit)
    self.loadCommands()
    runwindow.set_data("ignore_return",1)  # don't close the window on 'Return' key press, just 'Esc'
    runwindow.connect("key-press-event", keyPressClose)
    runwindow.show_all()

  def showFileSel(self,*args):
    self.fs = gtk.FileSelection()
    self.fs.set_title(_("Select A Program")+"...")
    self.fs.connect('destroy', self.doFsHide)
    self.fs.connect('delete_event', self.doFsHide)
    self.fslabel = self.fs.ok_button.children()[0]
    self.fslabel.set_text(_('Select'))
    self.fs.ok_button.connect('clicked', self.grabFile)
    self.fslabel = self.fs.cancel_button.children()[0]
    self.fslabel.set_text(_('Cancel'))
    self.fs.cancel_button.connect('clicked', self.doFsHide)
    self.fs.set_filename(self.last_file)
    self.fs.show_all()
    return gtk.TRUE

  def doFsHide(self,*args):
    self.fs.hide()
    self.fs.destroy()
    self.fs=None
    return gtk.TRUE

  def grabFile(self,*args):   
    rcmd=self.fs.get_filename()
    if rcmd:
      if not str(rcmd).endswith("/"):
        self.runentry.set_text(str(rcmd))
        self.last_file=str(rcmd)
        self.doFsHide()

  def runCommand(self,*args):
    r=self.runentry.get_text().strip()
    if r:
      self.addCommand(r)
      self.saveCommands()
      self.updateCombo()
      os.popen(r.replace(" ","\\ ")+" &")

  def addCommand(self,rcmd):
    if rcmd:
      try:
        self.rcommands.remove(rcmd)
      except:
        pass
      self.rcommands.insert(0,rcmd)

  def updateCombo(self):
    l=copy.copy(self.rcommands)
    self.runcombo.set_popdown_strings(l)
    self.runcombo.show_all()

  def saveCommands(self):
      if not self.cmd_file: return
      try:
        f=open(self.cmd_file,"w")
        f.write("# IceWMControlPanel gtk.Run file: DO NOT EDIT!\n")
        l=copy.copy(self.rcommands)
        if len(l) > 20:  l=l[0:20]
        l.reverse()
        for i in l:
          f.write(str(i)+"\n")
        f.flush()
        f.close()
      except:
        pass

  def loadCommands(self):
    try:
      hdir=str(os.environ["HOME"]).strip()
      if not hdir.endswith("/"): hdir=hdir+"/"
      self.cmd_file=hdir+".icewmcp_gtkruncmd"
    except:
      self.cmd_file=".icewmcp_gtkruncmd"
    try:
      f=open(self.cmd_file)
      scmd=f.read()
      f.close()
      clist=scmd.split("\n")
      for i in clist:
        cstr=i.strip()
        if cstr:
          if not cstr.startswith("#"): self.addCommand(cstr)
      self.updateCombo()
    except:
      pass

  def quitit(self,*args):
    self.saveCommands()
    self.runwindow.destroy()
    self.runwindow.unmap()




